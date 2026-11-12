#include "heap.h"

#include "photon_deterministic/native_allocator.h"

#include <bit>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace Quantum::Allocator {

namespace {

// Sizes of the original's bookkeeping records. They only matter for computing how many
// pages the bookkeeping occupies, which fixes where the first block can live.
constexpr int kDataRecordSize = 8;
constexpr int kBinRecordSize = 16;
constexpr int kPageRecordSize = 40;

std::uint32_t Mix397(std::uint32_t hash, int value) {
    return hash * 397u ^ static_cast<std::uint32_t>(value);
}

std::string Field(const char* name, int value) {
    return std::string(name) + ": " + std::to_string(value);
}

} // namespace

// ---------------------------------------------------------------------------------------
// Config and the record ToStrings

Heap::Config::Config(int pageShift, int pageCount, int heapCount)
    : PAGE_SHIFT(pageShift), PAGE_COUNT(pageCount),
      PAGE_SIZE(static_cast<int>(1u << (pageShift & 31))),
      HEAP_SIZE(static_cast<int>(static_cast<std::uint32_t>(pageCount) << (pageShift & 31))),
      HEAP_COUNT(heapCount) {
    // The small-size table covers 512 bytes for every page size of at least 512 bytes. The
    // original derives it as PAGE_SIZE / 2^(PAGE_SHIFT - 9), with C#'s shift-count masking.
    SMALL_BIN_SIZE = PAGE_SIZE / static_cast<int>(1u << ((pageShift - 9) & 31));
    SMALL_BIN_COUNT = SMALL_BIN_SIZE / WORD_SIZE;
    SMALL_BIN_ARRAY_SIZE = SMALL_BIN_COUNT + 1;
}

bool Heap::Config::Equals(const Config& other) const {
    return PAGE_SHIFT == other.PAGE_SHIFT && PAGE_COUNT == other.PAGE_COUNT &&
           PAGE_SIZE == other.PAGE_SIZE && HEAP_SIZE == other.HEAP_SIZE &&
           SMALL_BIN_SIZE == other.SMALL_BIN_SIZE && SMALL_BIN_COUNT == other.SMALL_BIN_COUNT &&
           SMALL_BIN_ARRAY_SIZE == other.SMALL_BIN_ARRAY_SIZE;
}

int Heap::Config::GetHashCode() const {
    std::uint32_t hash = static_cast<std::uint32_t>(PAGE_SHIFT);
    hash = Mix397(hash, PAGE_COUNT);
    hash = Mix397(hash, PAGE_SIZE);
    hash = Mix397(hash, HEAP_SIZE);
    hash = Mix397(hash, SMALL_BIN_SIZE);
    hash = Mix397(hash, SMALL_BIN_COUNT);
    hash = Mix397(hash, SMALL_BIN_ARRAY_SIZE);
    return static_cast<int>(hash);
}

std::string Heap::Config::ToString() const {
    return Field("PAGE_SHIFT", PAGE_SHIFT) + ", " + Field("PAGE_COUNT", PAGE_COUNT) + ", " +
           Field("PAGE_SIZE", PAGE_SIZE) + ", " + Field("HEAP_SIZE", HEAP_SIZE) + ", " +
           Field("HEAP_COUNT", HEAP_COUNT) + ", " + Field("SMALL_BIN_SIZE", SMALL_BIN_SIZE) + ", " +
           Field("SMALL_BIN_COUNT", SMALL_BIN_COUNT) + ", " +
           Field("SMALL_BIN_ARRAY_SIZE", SMALL_BIN_ARRAY_SIZE);
}

std::string Heap::Offsets::ToString() const {
    return Field("DataOffset", DataOffset) + ", " + Field("BinsOffset", BinsOffset) + ", " +
           Field("BinsFullOffset", BinsFullOffset) + ", " +
           Field("BinsSmallOffset", BinsSmallOffset) + ", " + Field("PagesOffset", PagesOffset) +
           ", " + Field("MetaDataPages", MetaDataPages);
}

std::string Heap::Data::ToString() const {
    return Field("PagesFree", PagesFree) + ", " + Field("PagesFreeCount", PagesFreeCount);
}

std::string Heap::Bin::ToString() const {
    return Field("Head", Head) + ", " + Field("Tail", Tail) + ", " + Field("BlockSize", BlockSize) +
           ", " + Field("PageCount", PageCount);
}

std::string Heap::Page::ToString() const {
    return Field("Bin", Bin) + ", " + Field("Index", Index) + ", " + Field("Flags", Flags) + ", " +
           Field("Reserved", Reserved) + ", " + Field("Committed", Committed) + ", " +
           Field("Allocated", Allocated) + ", " + Field("BlockSize", BlockSize) +
           ", BlockFree: " + BlockFree.ToString() + ", " + Field("Prev", Prev) + ", " +
           Field("Next", Next);
}

// ---------------------------------------------------------------------------------------
// Size classes

int Heap::CalculateWordSize(int size) {
    return (size + 7) / WORD_SIZE;
}

int Heap::BitScanReverse(std::uint32_t v) {
    return v == 0 ? 0 : 31 - std::countl_zero(v);
}

int Heap::CalculateBin(int size) const {
    const int words = CalculateWordSize(size);
    if (size + 7 < 9 * WORD_SIZE) {
        return words; // 0..8 words: one bin per word count
    }
    // Four bins per power of two: the position of the top bit selects the group, the two
    // bits below it the quarter.
    const std::uint32_t w = static_cast<std::uint32_t>(words - 1);
    const int msb = BitScanReverse(w);
    const int quarter = static_cast<int>((w >> (msb - 2)) & 3u);
    return ((msb << 2) | quarter) - 3;
}

// ---------------------------------------------------------------------------------------
// Construction

Heap Heap::Initialize(std::byte* memory, const Config& config, const Offsets& offsets,
                      std::span<const Bin> defaultBins) {
    if (defaultBins.size() < static_cast<std::size_t>(PAGE_BIN_COUNT)) {
        throw std::out_of_range("Heap::Initialize: not enough default bins");
    }

    Heap heap;
    std::memset(memory, 0, static_cast<std::size_t>(config.HEAP_SIZE));
    heap._config = config;
    heap._offsets = offsets;
    heap._reserved = false;
    heap._memory = memory;
    heap._pages.assign(static_cast<std::size_t>(config.PAGE_COUNT), Page{});
    heap._binsSmall.assign(static_cast<std::size_t>(config.SMALL_BIN_ARRAY_SIZE), 0);

    // Build the free list from the top down so the lowest free page ends up first.
    heap._data.PagesFreeCount = config.PAGE_COUNT - offsets.MetaDataPages;
    for (int i = config.PAGE_COUNT - 1; i >= 0; --i) {
        Page& page = heap._pages[static_cast<std::size_t>(i)];
        page.Index = i;
        page.Bin = PAGE_BIN_INVALID;
        page.Next = heap._data.PagesFree;
        if (i < offsets.MetaDataPages) {
            // Bookkeeping pages look like full pages with one page-sized block in use.
            page.Flags = Full | Meta;
            page.Reserved = 1;
            page.Committed = 1;
            page.Allocated = 1;
            page.BlockSize = config.PAGE_SIZE;
        } else {
            heap._data.PagesFree = i;
        }
    }

    for (int bin = 0; bin < PAGE_BIN_COUNT; ++bin) {
        heap._bins[static_cast<std::size_t>(bin)] = defaultBins[static_cast<std::size_t>(bin)];
        heap._binsFull[static_cast<std::size_t>(bin)] = defaultBins[static_cast<std::size_t>(bin)];
    }

    for (int words = 1; words <= config.SMALL_BIN_COUNT; ++words) {
        heap._binsSmall[static_cast<std::size_t>(words)] = heap.CalculateBin(words * WORD_SIZE);
    }
    return heap;
}

// ---------------------------------------------------------------------------------------
// Allocation

void* Heap::AllocateBlock(int size) {
    if (size < 1) {
        return nullptr;
    }
    if (size > PAGE_BIN_MAX_ALLOC || size > _config.PAGE_SIZE) {
        throw std::length_error("Heap::Allocate: " + std::to_string(size) +
                                " bytes exceeds the maximum allocation size");
    }

    const int bin = size <= _config.SMALL_BIN_SIZE
                        ? _binsSmall[static_cast<std::size_t>(CalculateWordSize(size))]
                        : CalculateBin(size);

    // Fast path: the bin's first page still has a free block.
    Page* head = PageAt(_bins[static_cast<std::size_t>(bin)].Head);
    if (head != nullptr && head->BlockFree.Offset != 0) {
        return PageNextBlock(*head, size, false);
    }
    return AllocateSlow(bin, head, size, 0);
}

void* Heap::AllocateAndClearBlock(int size) {
    void* block = AllocateBlock(size);
    if (block != nullptr) {
        std::memset(block, 0, static_cast<std::size_t>(size));
    }
    return block;
}

void* Heap::AllocateSlow(int bin, Page* p, int size, int tries) {
    if (tries == ALLOC_BIN_EXPANSION) {
        return nullptr;
    }

    for (;;) {
        // The page the fast path looked at: commit more of it, or retire it as full.
        if (p != nullptr) {
            if (p->Committed < p->Reserved) {
                PageExpand(*p);
                return PageNextBlock(*p, size, false);
            }
            PageMoveToFullBin(*p, bin);
        }

        // Walk the rest of the available pages, retiring exhausted ones as we go.
        for (int index = _bins[static_cast<std::size_t>(bin)].Head; index != 0;) {
            Page& page = _pages[static_cast<std::size_t>(index)];
            if (page.BlockFree.Offset != 0) {
                return PageNextBlock(page, size, false);
            }
            if (page.Committed < page.Reserved) {
                PageExpand(page);
                return PageNextBlock(page, size, false);
            }
            const int next = page.Next;
            PageMoveToFullBin(page, bin);
            index = next;
        }

        // A page on the full list may have had blocks freed since it was retired.
        for (int index = _binsFull[static_cast<std::size_t>(bin)].Head; index != 0;) {
            Page& page = _pages[static_cast<std::size_t>(index)];
            if (page.BlockFree.Offset != 0) {
                Block* block = PageNextBlock(page, size, false);
                if (page.BlockFree.Offset != 0) {
                    PageMoveToAvailableBin(page, bin);
                }
                return block;
            }
            index = page.Next;
        }

        if (Page* fresh = PageAllocate(bin)) {
            return PageNextBlock(*fresh, size, false);
        }

        // Out of pages: settle for a block from one of the next larger bins.
        ++bin;
        ++tries;
        if (bin >= PAGE_BIN_COUNT || tries == ALLOC_BIN_EXPANSION) {
            return nullptr;
        }
        p = nullptr;
    }
}

void Heap::FreeBlock(void* ptr) {
    if (ptr == nullptr) {
        throw std::invalid_argument("Heap::Free: null pointer");
    }
    const std::ptrdiff_t offset = static_cast<std::byte*>(ptr) - _memory;
    const std::ptrdiff_t pageIndex = offset >> _config.PAGE_SHIFT;
    if (offset < 0 || pageIndex >= _config.PAGE_COUNT) {
        throw std::out_of_range("Heap::Free: pointer outside the heap");
    }

    Page& page = _pages[static_cast<std::size_t>(pageIndex)];
    if (page.Bin == PAGE_BIN_INVALID) {
        // Bookkeeping or free page, e.g. Free(Ptr::Null()). The original would corrupt its
        // bookkeeping here; refuse instead.
        throw std::invalid_argument("Heap::Free: not an allocated block");
    }
    const Ptr block = Void(ptr);
    BlockNext(block) = page.BlockFree;
    page.BlockFree = block;
    if (--page.Allocated == 0) {
        PageFree(page);
    }
}

void Heap::CopyBytes(void* dest, const void* src, int count) {
    if (count > 0) {
        std::memcpy(dest, src, static_cast<std::size_t>(count));
    }
}

Ptr& Heap::BlockNext(Ptr block) {
    return Void<Block>(block)->Next;
}

// ---------------------------------------------------------------------------------------
// Page lists

void Heap::BinInsertFirst(Bin& b, Page& p) {
    p.Prev = 0;
    p.Next = b.Head;
    if (b.Head != 0) {
        _pages[static_cast<std::size_t>(b.Head)].Prev = p.Index;
    } else {
        b.Tail = p.Index;
    }
    b.Head = p.Index;
    ++b.PageCount;
}

void Heap::BinInsertLast(Bin& b, Page& p) {
    p.Next = 0;
    p.Prev = b.Tail;
    if (b.Tail != 0) {
        _pages[static_cast<std::size_t>(b.Tail)].Next = p.Index;
    } else {
        b.Head = p.Index;
    }
    b.Tail = p.Index;
    ++b.PageCount;
}

void Heap::BinRemove(Bin& b, Page& p) {
    if (p.Prev != 0) {
        _pages[static_cast<std::size_t>(p.Prev)].Next = p.Next;
    }
    if (p.Next != 0) {
        _pages[static_cast<std::size_t>(p.Next)].Prev = p.Prev;
    }
    if (b.Tail == p.Index) {
        b.Tail = p.Prev;
    }
    if (b.Head == p.Index) {
        b.Head = p.Next;
    }
    p.Prev = 0;
    p.Next = 0;
    --b.PageCount;
}

void Heap::PageMoveToFullBin(Page& p, int bin) {
    p.Flags |= Full;
    BinRemove(_bins[static_cast<std::size_t>(bin)], p);
    BinInsertLast(_binsFull[static_cast<std::size_t>(bin)], p);
}

void Heap::PageMoveToAvailableBin(Page& p, int bin) {
    p.Flags &= ~Full;
    BinRemove(_binsFull[static_cast<std::size_t>(bin)], p);
    BinInsertFirst(_bins[static_cast<std::size_t>(bin)], p);
}

// ---------------------------------------------------------------------------------------
// Pages

Heap::Block* Heap::PageNextBlock(Page& p, int /*size*/, bool /*allowFull*/) {
    if (p.BlockFree.Offset == 0) {
        return nullptr;
    }
    Block* block = Void<Block>(p.BlockFree);
    p.BlockFree = block->Next;
    ++p.Allocated;
    return block;
}

void Heap::PageExpand(Page& p) {
    int count = p.Reserved - p.Committed;
    if (count > PAGE_EXPAND_BYTES / p.BlockSize) {
        count = PAGE_EXPAND_BYTES / p.BlockSize;
    }
    if (count < 1) {
        count = 1;
    }

    std::byte* start = PageMemoryStart(p.Index);
    for (int i = 0; i < count; ++i) {
        const Ptr block =
            Void(start + static_cast<std::ptrdiff_t>(p.BlockSize) * (p.Committed + i));
        BlockNext(block) = p.BlockFree;
        p.BlockFree = block;
    }
    p.Committed += count;
}

Heap::Page* Heap::PageAllocate(int bin) {
    if (_data.PagesFree == 0) {
        PageForceFree();
        if (_data.PagesFree == 0) {
            return nullptr;
        }
    }

    Page& p = _pages[static_cast<std::size_t>(_data.PagesFree)];
    Bin& b = _bins[static_cast<std::size_t>(bin)];
    p.Bin = bin;
    p.BlockSize = b.BlockSize;
    p.Reserved = _config.PAGE_SIZE / b.BlockSize;
    _data.PagesFree = p.Next;
    --_data.PagesFreeCount;

    std::memset(PageMemoryStart(p.Index), 0, static_cast<std::size_t>(_config.PAGE_SIZE));
    BinInsertFirst(b, p);
    PageExpand(p);
    return &p;
}

void Heap::PageFree(Page& p) {
    Bin& b = (p.Flags & Full) != 0 ? _binsFull[static_cast<std::size_t>(p.Bin)]
                                   : _bins[static_cast<std::size_t>(p.Bin)];
    BinRemove(b, p);

    p.Bin = PAGE_BIN_INVALID;
    p.Reserved = 0;
    p.Committed = 0;
    p.Allocated = 0;
    p.BlockSize = 0;
    p.BlockFree = Ptr::Null();
    p.Prev = 0;
    p.Next = _data.PagesFree;
    p.Flags &= ~Full;

    _data.PagesFree = p.Index;
    ++_data.PagesFreeCount;
}

void Heap::PageForceFree() {
    for (int i = _offsets.MetaDataPages; i < _config.PAGE_COUNT; ++i) {
        Page& page = _pages[static_cast<std::size_t>(i)];
        // Only called with an empty free list, so every page here belongs to a bin; the
        // bin check just keeps an unexpected free page from being unlinked twice.
        if (page.Allocated == 0 && page.Bin != PAGE_BIN_INVALID) {
            PageFree(page);
        }
    }
}

// ---------------------------------------------------------------------------------------
// Whole-heap operations

Heap::Stats Heap::GetStats(const Heap* heap) {
    if (heap == nullptr) {
        throw std::invalid_argument("Heap::GetStats: null heap");
    }
    Stats stats;
    stats.TotalMemory = heap->_config.HEAP_SIZE;
    stats.TotalPages = heap->_config.PAGE_COUNT;
    stats.PagesFree = heap->_data.PagesFreeCount;
    stats.PagesUsed = heap->_config.PAGE_COUNT - heap->_data.PagesFreeCount;
    for (const Page& page : heap->_pages) {
        if (page.Bin != PAGE_BIN_INVALID || (page.Flags & Meta) != 0) {
            stats.BytesAllocated += page.Allocated * page.BlockSize;
            stats.BytesReserved += page.Reserved * page.BlockSize;
            stats.BytesCommited += page.Committed * page.BlockSize;
        }
        stats.PagesFull += page.Flags & Full;
    }
    return stats;
}

void Heap::Copy(Photon::Deterministic::Native::Allocator* /*allocator*/, Heap* to,
                const Heap* from) {
    if (to == nullptr || from == nullptr) {
        throw std::invalid_argument("Heap::Copy: null heap");
    }
    if (from->_config != to->_config) {
        throw std::invalid_argument("Heap::Copy: heaps have different configurations");
    }

    // The bookkeeping (in the original: the first pages of the block) ...
    to->_data = from->_data;
    to->_bins = from->_bins;
    to->_binsFull = from->_binsFull;
    to->_binsSmall = from->_binsSmall;
    to->_pages = from->_pages;

    // ... and every page that holds blocks.
    const auto pageSize = static_cast<std::size_t>(from->_config.PAGE_SIZE);
    for (const Page& page : from->_pages) {
        if (page.Reserved >= 1) {
            std::memcpy(to->PageMemoryStart(page.Index), from->PageMemoryStart(page.Index),
                        pageSize);
        }
    }
}

// ---------------------------------------------------------------------------------------
// Group

std::span<const Heap::Bin> Heap::Group::Bins::Default() {
    static const std::array<Bin, PAGE_BIN_COUNT> bins = [] {
        std::array<Bin, PAGE_BIN_COUNT> result{};
        std::size_t next = 0;
        for (int words = 0; words <= 8; ++words) {
            result[next++] = Bin(words);
        }
        for (int shift = 1; shift <= 10; ++shift) {
            for (int step = 5; step <= 8; ++step) {
                result[next++] = Bin(step << shift);
            }
        }
        return result;
    }();
    return bins;
}

Heap::Offsets Heap::Group::CalculateMetaOffsets(const Config& config) {
    using Photon::Deterministic::Native::RoundUpToAlignment;
    const int dataSize = RoundUpToAlignment(kDataRecordSize, 4);
    const int binsSize = RoundUpToAlignment(PAGE_BIN_COUNT * kBinRecordSize, 4);
    const int binsFullSize = RoundUpToAlignment(PAGE_BIN_COUNT * kBinRecordSize, 4);
    const int binsSmallSize = RoundUpToAlignment(config.SMALL_BIN_ARRAY_SIZE * 4, 4);
    const int pagesSize = RoundUpToAlignment(config.PAGE_COUNT * kPageRecordSize, 4);

    Offsets offsets;
    offsets.DataOffset = 0;
    offsets.BinsOffset = dataSize;
    offsets.BinsFullOffset = dataSize + binsSize;
    offsets.BinsSmallOffset = offsets.BinsFullOffset + binsFullSize;
    offsets.PagesOffset = offsets.BinsSmallOffset + binsSmallSize;
    offsets.MetaDataPages =
        (offsets.PagesOffset + config.PAGE_SIZE + pagesSize - 1) / config.PAGE_SIZE;
    return offsets;
}

void Heap::Group::InitDeBruijnTable(std::uint8_t* table) {
    // Entry k of the table answers "which bit is the highest set one" for the value whose
    // smeared form (all bits below the top one set) multiplied by the De Bruijn constant
    // has k in its top five bits.
    constexpr std::uint32_t kDeBruijn = 0x07C4ACDDu;
    for (int bit = 0; bit < DE_BRUIJN_TABLE_COUNT; ++bit) {
        const std::uint32_t smeared = bit == 31 ? 0xFFFFFFFFu : (2u << bit) - 1u;
        table[(smeared * kDeBruijn) >> 27] = static_cast<std::uint8_t>(bit);
    }
}

Heap::Group* Heap::Group::Allocate(const Config& config,
                                   Photon::Deterministic::Native::Allocator& allocator) {
    auto* group = new Group();
    group->_heapsCount = config.HEAP_COUNT;
    group->_config = config;
    group->_offsets = CalculateMetaOffsets(config);
    InitDeBruijnTable(group->_debrujinTable.data());
    try {
        InitHeaps(group, allocator);
    } catch (...) {
        delete group;
        throw;
    }
    return group;
}

void Heap::Group::InitHeaps(Group* group, Photon::Deterministic::Native::Allocator& allocator) {
    if (group == nullptr) {
        throw std::invalid_argument("Heap::Group::InitHeaps: null group");
    }
    const Config& config = group->_config;
    // One spare heap's worth of memory leaves room to align the start to a page boundary.
    const std::int64_t total =
        static_cast<std::int64_t>(config.HEAP_SIZE) * group->_heapsCount + config.HEAP_SIZE;
    if (total > std::numeric_limits<int>::max()) {
        throw std::length_error("Heap::Group: heaps need more than 2 GiB");
    }

    group->_heapsMemory = allocator.AllocAndClear(static_cast<int>(total), 8);
    const auto pageSize = static_cast<std::uintptr_t>(config.PAGE_SIZE);
    const auto address = reinterpret_cast<std::uintptr_t>(group->_heapsMemory);
    group->_heapsStart = static_cast<std::byte*>(group->_heapsMemory) +
                         ((address / pageSize) * pageSize + pageSize - address);

    const std::span<const Bin> bins = Bins::Default();
    group->_heaps.clear();
    group->_heaps.reserve(static_cast<std::size_t>(group->_heapsCount));
    for (int i = 0; i < group->_heapsCount; ++i) {
        std::byte* memory = group->_heapsStart + static_cast<std::ptrdiff_t>(config.HEAP_SIZE) * i;
        group->_heaps.push_back(Initialize(memory, config, group->_offsets, bins));
    }
}

void Heap::Group::Free(Group* group, Photon::Deterministic::Native::Allocator& allocator) {
    if (group == nullptr || group->_heapsMemory == nullptr) {
        throw std::logic_error("Heap::Group::Free: group is not allocated");
    }
    allocator.Free(group->_heapsMemory);
    group->_heapsMemory = nullptr;
    delete group;
}

Heap* Heap::Group::GetHeap(int index) {
    if (static_cast<unsigned>(index) >= static_cast<unsigned>(_heapsCount)) {
        throw std::out_of_range("Heap::Group::GetHeap: index out of range");
    }
    return &_heaps[static_cast<std::size_t>(index)];
}

void Heap::Group::Release(Heap* heap) {
    if (heap == nullptr) {
        throw std::invalid_argument("Heap::Group::Release: null heap");
    }
    *heap = Initialize(heap->_memory, _config, _offsets, Bins::Default());
}

Heap* Heap::Group::Reserve() {
    for (int i = 0; i < _heapsCount; ++i) {
        Heap* heap = GetHeap(i);
        if (!heap->_reserved) {
            heap->_reserved = true;
            return heap;
        }
    }
    throw std::logic_error("Heap::Group::Reserve: all " + std::to_string(_heapsCount) +
                           " heaps are in use");
}

} // namespace Quantum::Allocator
