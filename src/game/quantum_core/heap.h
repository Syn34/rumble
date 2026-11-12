// heap.h - Quantum.Allocator.Heap, the per-frame memory allocator of the simulation.
//
// Every simulation frame owns one Heap: a fixed block of HEAP_SIZE bytes split into
// PAGE_COUNT pages of PAGE_SIZE bytes. Dynamic simulation data (lists, dictionaries,
// component buffers) is allocated from it and referenced by Ptr (byte offset into the
// block), which is what lets a whole frame be copied page by page for prediction and
// rollback.
//
// How allocation works (as in the original):
//  * Requests are rounded to 8-byte words and mapped to one of 49 size classes ("bins").
//    Bins 1..8 are exact word counts; above that each power of two is split into four
//    sub-classes (see CalculateBin and Group::Bins::Default).
//  * A page serves blocks of a single bin. Each bin keeps two doubly linked lists of
//    pages: pages with free blocks ("available") and exhausted pages ("full").
//  * A page's blocks are carved lazily ("committed") at most 4 KiB worth at a time and
//    pushed onto the page's intrusive free list; the link is stored in the first word of
//    each free block.
//  * Freeing a block pushes it back onto its page's free list; a page whose last block is
//    freed goes straight back to the heap's free-page list.
//  * The allocation order is fully deterministic, so the Ptr values handed out are the
//    same on every machine for the same sequence of calls.
//
// Memory model in this reimplementation: the original keeps the bookkeeping (free-page
// list head, bin tables, one record per page) inside the first pages of the heap block
// itself, linked by Ptrs. Here that bookkeeping lives in typed arrays owned by the Heap
// object, with page links stored as page indices (0 = none: page 0 is always a
// bookkeeping page, so it can never be linked). The first MetaDataPages pages of the
// block are still reserved and never handed out, so every block's Ptr is identical to
// the original's. Block contents and the free-block links live in the heap block.
#pragma once

#include "quantum_core/ptr.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Photon::Deterministic::Native {
class Allocator;
}

namespace Quantum::Allocator {

class FrameHeap;

// How a frame heap reports allocations (chosen in the simulation config).
enum class HeapTrackingMode : std::uint8_t {
    Disabled = 0,
    DetectLeaks = 1,
    TraceAllocations = 2,
};

// Allocation trackers. In this build of the game the Heap never calls into the tracker
// it is handed (the generic Allocate/Free bodies ignore it), so trackers only own a
// container that Reset() empties.
class IHeapAllocationTracker {
  public:
    virtual ~IHeapAllocationTracker() = default;
    virtual void Reset() = 0;
};

struct DisabledHeapAllocTrackerStruct {
    // 0x7d26040
    void Reset() {}
};

class DisabledHeapAllocTracker final : public IHeapAllocationTracker {
  public:
    // 0x7d26048 (.ctor), 0x7d26044 (Reset)
    void Reset() override {}
};

class SimpleHeapAllocTracker final : public IHeapAllocationTracker {
  public:
    // 0x7d26118 (.ctor), 0x7d26050 (Reset: creates the set on first use, then empties it)
    void Reset() override { _trackedAlloc.clear(); }

    std::unordered_set<Ptr> _trackedAlloc;
};

class TracedHeapAllocTracker final : public IHeapAllocationTracker {
  public:
    // 0x7d26200 (.ctor), 0x7d26120 (Reset: creates the map on first use, then empties it)
    void Reset() override { _trackedAlloc.clear(); }

    // Allocation -> where it was made (a stack trace in the original).
    std::unordered_map<Ptr, std::string> _trackedAlloc;
};

class Heap {
  public:
    static constexpr int WORD_SIZE = 8;
    static constexpr int PAGE_BIN_COUNT = 49;
    static constexpr int PAGE_BIN_INVALID = -1;
    // Largest single allocation, also limited to one page.
    static constexpr int PAGE_BIN_MAX_ALLOC = 0x10000;
    // How many bins (the requested one and the next larger ones) are tried before an
    // allocation gives up.
    static constexpr int ALLOC_BIN_EXPANSION = 3;
    static constexpr int DE_BRUIJN_TABLE_COUNT = 32;
    // Committing a page's blocks happens at most this many bytes at a time.
    static constexpr int PAGE_EXPAND_BYTES = 4096;

    struct Config {
        static constexpr int PAGE_SHIFT_SIZE_2KB = 11;
        static constexpr int PAGE_SHIFT_SIZE_4KB = 12;
        static constexpr int PAGE_SHIFT_SIZE_8KB = 13;
        static constexpr int PAGE_SHIFT_SIZE_16KB = 14;
        static constexpr int PAGE_SHIFT_SIZE_32KB = 15;
        static constexpr int PAGE_SHIFT_SIZE_64KB = 16;

        int PAGE_SHIFT = 0;
        int PAGE_COUNT = 0;
        int PAGE_SIZE = 0;
        int HEAP_SIZE = 0;
        int HEAP_COUNT = 0;
        // Requests up to this size use the precomputed small-bin table.
        int SMALL_BIN_SIZE = 0;
        int SMALL_BIN_COUNT = 0;
        int SMALL_BIN_ARRAY_SIZE = 0;

        Config() = default;
        // 0x7d24a4c
        Config(int pageShift, int pageCount, int heapCount);

        // 0x7d24a8c (Equals(Config)); 0x7d24b08 is the boxed Equals(object). HEAP_COUNT is
        // deliberately not compared: heaps of groups with different sizes are compatible.
        bool Equals(const Config& other) const;
        // 0x7d24b90 - Multiply-by-397 combination of every field except HEAP_COUNT.
        int GetHashCode() const;
        // 0x7d24bd8
        friend bool operator!=(const Config& a, const Config& b) { return !a.Equals(b); }
        friend bool operator==(const Config& a, const Config& b) { return a.Equals(b); }
        // 0x7d24c04 - Debug listing of the fields (wording is this project's own).
        std::string ToString() const;
    };

    // Byte offsets of the bookkeeping areas inside the heap block, and how many pages they
    // occupy (see Group::CalculateMetaOffsets).
    struct Offsets {
        int DataOffset = 0;
        int BinsOffset = 0;
        int BinsFullOffset = 0;
        int BinsSmallOffset = 0;
        int PagesOffset = 0;
        int MetaDataPages = 0;

        // 0x7d250b0
        std::string ToString() const;
    };

    struct Stats {
        int TotalMemory = 0;
        int TotalPages = 0;
        int PagesFree = 0;
        int PagesFull = 0;
        int PagesUsed = 0;
        int BytesAllocated = 0;
        int BytesReserved = 0;
        int BytesCommited = 0;
    };

    // Heap-wide state: the free-page list.
    struct Data {
        int PagesFree = 0; // first free page (index), 0 = none
        int PagesFreeCount = 0;

        // 0x7d254c8
        std::string ToString() const;
    };

    // One size class: its page list and block size.
    struct Bin {
        int Head = 0; // page index, 0 = none
        int Tail = 0;
        int BlockSize = 0;
        int PageCount = 0;

        Bin() = default;
        // 0x7d2569c
        explicit Bin(int words) : BlockSize(words * WORD_SIZE) {}

        // 0x7d256ac
        std::string ToString() const;
    };

    enum PageFlags : int {
        Full = 1, // all blocks handed out; the page sits on its bin's full list
        Meta = 2, // bookkeeping page, never used for blocks
    };

    struct Page {
        int Bin = PAGE_BIN_INVALID;
        int Index = 0;
        int Flags = 0;
        int Reserved = 0;  // blocks the page can hold
        int Committed = 0; // blocks carved so far
        int Allocated = 0; // blocks in use
        int BlockSize = 0;
        Ptr BlockFree; // first free block
        int Prev = 0;  // neighbouring pages in the bin list (indices, 0 = none)
        int Next = 0;

        // 0x7d259b0
        std::string ToString() const;
    };

    // Header of a free block: the next free block of the same page.
    struct Block {
        Ptr Next;
    };

    // A small list of Ptrs kept in frame memory; blocks recorded in it are all freed
    // together at the start of the next simulated frame (Quantum.Allocator.Heap
    // .AllocationTracker, embedded in the frame's metadata).
    struct AllocationTracker {
        static constexpr int SIZE = 8;
        static constexpr int DefaultCapacity = 8;

        std::uint16_t _trackAllocCount = 0;
        std::uint16_t _trackAllocCapacity = 0;
        Ptr _trackAllocPtr;

        // 0x7d231b4 - Appends `ptr` to the list, growing it as needed.
        // The original takes the FrameBase and uses its tracked frame heap; the frame heap
        // is passed directly here.
        void TrackAllocation(FrameHeap& heap, Ptr ptr);
        // 0x7d2333c - Frees every recorded block and empties the list.
        void FreeAllTracked(FrameHeap& heap);
        // 0x7d231e4 - Makes room for one more entry: the first call allocates room for 8,
        // later calls double the capacity. Returns the entry array.
        Ptr* EnsureBufferCapacity(FrameHeap& heap);

        // TODO(0x7d233f4, Quantum.Allocator.Heap.AllocationTracker.Serialize): writes count,
        // capacity and the Ptr array through the FrameSerializer; needs FrameSerializer.
    };

    // A set of equally configured heaps carved from one allocation: one per frame slot in
    // the frame pool (Quantum.Allocator.Heap.Group).
    class Group {
      public:
        struct Bins {
            // 0x7d23644 - The 49 size classes, in words: 0..8, then four steps per power of
            // two (10, 12, 14, 16, 20, 24, 28, 32, 40, ...) up to 8192 words (64 KiB). Built
            // once and cached.
            static std::span<const Bin> Default();
        };

        // 0x7d1ea6c - Creates the group: computes the bookkeeping layout, reserves one block
        // for all heaps and initialises every heap.
        static Group* Allocate(const Config& config,
                               Photon::Deterministic::Native::Allocator& allocator);
        // 0x7d1f28c - Releases the heaps' memory and the group. Throws if the group was
        // already freed.
        static void Free(Group* group, Photon::Deterministic::Native::Allocator& allocator);
        // 0x7d24098 - Allocates HEAP_SIZE * (HEAP_COUNT + 1) bytes, aligns the start to a page
        // boundary and initialises each heap in its slice.
        static void InitHeaps(Group* group, Photon::Deterministic::Native::Allocator& allocator);
        // 0x7d23f20 - Layout of the bookkeeping area at the start of each heap.
        static Offsets CalculateMetaOffsets(const Config& config);
        // 0x7d23fec - Fills the 32-entry De Bruijn table used for fast log2. Kept for
        // completeness; this implementation computes log2 directly (see BitScanReverse).
        static void InitDeBruijnTable(std::uint8_t* table);

        // 0x7d235f0 - Throws std::out_of_range for an invalid index.
        Heap* GetHeap(int index);
        // 0x7d1f320 - Resets the heap to its freshly initialised state and makes it
        // available again.
        void Release(Heap* heap);
        // 0x7d1f39c - Hands out the first heap not yet in use; throws if all are taken.
        Heap* Reserve();

        int HeapCount() const { return _heapsCount; }
        const Config& GetConfig() const { return _config; }

      private:
        std::vector<Heap> _heaps;
        int _heapsCount = 0;
        Config _config;
        Offsets _offsets;
        void* _heapsMemory = nullptr;
        std::byte* _heapsStart = nullptr;
        std::array<std::uint8_t, DE_BRUIJN_TABLE_COUNT> _debrujinTable{};
    };

    Heap() = default;

    // 0x7d22cb8 - Builds a heap over `memory` (HEAP_SIZE bytes, page aligned): clears it,
    // puts every page after the bookkeeping pages on the free list (lowest index first),
    // copies the default bins into both bin tables and precomputes the small-size table.
    static Heap Initialize(std::byte* memory, const Config& config, const Offsets& offsets,
                           std::span<const Bin> defaultBins);

    // 0x4f462b0 (instance for DisabledHeapAllocTrackerStruct) - A block of at least `size`
    // bytes, or null for size <= 0 or when the heap is exhausted. Sizes above 64 KiB or one
    // page throw std::length_error. The tracker is not used by this build of the game.
    template <class TTracker> void* Allocate(int size, TTracker& /*allocTracker*/) {
        return AllocateBlock(size);
    }
    // 0x4f467a8 (DisabledHeapAllocTrackerStruct) - Allocate, then zero `size` bytes.
    template <class TTracker> void* AllocateAndClear(int size, TTracker& /*allocTracker*/) {
        return AllocateAndClearBlock(size);
    }
    // 0x4f46a14 (DisabledHeapAllocTrackerStruct) - AllocateAndClear returning a Ptr
    // (Ptr::Null() when the allocation fails).
    template <class TTracker> Ptr AllocateAndClearPtr(int size, TTracker& allocTracker) {
        void* block = AllocateAndClear(size, allocTracker);
        return block != nullptr ? Void(block) : Ptr::Null();
    }
    // 0x4f46ef4 / 0x4f46f5c (DisabledHeapAllocTrackerStruct) - Returns a block to its page.
    template <class TTracker> void Free(void* ptr, TTracker& /*allocTracker*/) { FreeBlock(ptr); }
    template <class TTracker> void Free(Ptr ptr, TTracker& /*allocTracker*/) {
        FreeBlock(Void(ptr));
    }
    // 0x4f46bc0 (instance for Ptr) - Moves an array of `currentSize` elements into a new
    // zeroed array of `newSize` elements and frees the old one.
    template <class T, class TTracker>
    T* ExpandArray(T* buffer, int currentSize, int newSize, TTracker& allocTracker) {
        const int elementSize = static_cast<int>(sizeof(T));
        T* grown = static_cast<T*>(AllocateAndClear(newSize * elementSize, allocTracker));
        CopyBytes(grown, buffer, currentSize * elementSize);
        Free(static_cast<void*>(buffer), allocTracker);
        return grown;
    }

    // 0x7d2220c, 0x7d22268, 0x7d22f3c, 0x7d22f98 - The same operations with the heap's own
    // (disabled) tracker.
    void* AllocateAndClear(int size) { return AllocateAndClear(size, _disabledTracker); }
    Ptr AllocateAndClearPtr(int size) { return AllocateAndClearPtr(size, _disabledTracker); }
    void Free(void* ptr) { Free(ptr, _disabledTracker); }
    void Free(Ptr ptr) { Free(ptr, _disabledTracker); }

    // 0x7d23140 - Ptr of a block inside this heap.
    Ptr Void(const void* ptr) const {
        return Ptr{static_cast<int>(static_cast<const std::byte*>(ptr) - _memory)};
    }
    // 0x7d2314c - Address of a Ptr. Like the original, Ptr::Null() maps to the start of the
    // heap rather than to nullptr; callers check for null Ptrs themselves.
    void* Void(Ptr p) const { return _memory + p.Offset; }
    // 0x4f47180 (instance for QHashCollection)
    template <class T> T* Void(Ptr p) const { return static_cast<T*>(Void(p)); }
    // 0x7d23158, 0x7d23164 - Identical to Void (the checked forms only differ in debug
    // builds of the original).
    void* VoidUnchecked(Ptr p) const { return Void(p); }
    Ptr VoidUnchecked(const void* ptr) const { return Void(ptr); }

    // 0x7d1cffc - Page and byte counts. Bookkeeping pages count as used and as full.
    static Stats GetStats(const Heap* heap);
    // 0x7d22414 - Makes `to` an exact copy of `from` (same allocations at the same Ptrs).
    // Both heaps must have the same configuration (std::invalid_argument otherwise). Only
    // pages that are in use are copied. The allocator argument is unused.
    static void Copy(Photon::Deterministic::Native::Allocator* allocator, Heap* to,
                     const Heap* from);

    // TODO(0x7d2255c, Quantum.Allocator.Heap.Print): dumps config, bins and every page (with
    // its free list and a hex dump) to a FramePrinter; needs FramePrinter.
    // TODO(0x7d22ac0, Quantum.Allocator.Heap.RegisterInternalTypes): registers the heap's
    // record types with the serializer's type registry; needs the type registry.

    // 0x7d22ed4 - Size class of an allocation of `size` bytes.
    int CalculateBin(int size) const;
    // 0x7d23170 - Size in 8-byte words, rounded up (C# integer division semantics).
    static int CalculateWordSize(int size);
    // 0x7d23188 - Index of the highest set bit (0 for 0). The original looks the result up
    // in a De Bruijn table; computed directly here.
    static int BitScanReverse(std::uint32_t v);

    // 0x7d22548
    std::byte* PageMemoryStart(int index) const {
        return _memory + static_cast<std::ptrdiff_t>(_config.PAGE_SIZE) * index;
    }

    const Config& GetConfig() const { return _config; }
    const Offsets& GetOffsets() const { return _offsets; }
    const Data& GetData() const { return _data; }
    const Page& GetPage(int index) const { return _pages.at(static_cast<std::size_t>(index)); }
    const Bin& GetBin(int bin) const { return _bins.at(static_cast<std::size_t>(bin)); }
    const Bin& GetFullBin(int bin) const { return _binsFull.at(static_cast<std::size_t>(bin)); }
    bool IsReserved() const { return _reserved; }
    std::byte* Memory() const { return _memory; }

  private:
    friend class Group;

    void* AllocateBlock(int size);
    void* AllocateAndClearBlock(int size);
    void FreeBlock(void* ptr);
    static void CopyBytes(void* dest, const void* src, int count);

    // 0x7d21e0c - Slow path of Allocate: finds a page with room in bin `bin` (expanding,
    // reusing pages from the full list or taking a fresh page), falling back to the next
    // larger bins. `p` is the bin's head page when the fast path found it exhausted.
    void* AllocateSlow(int bin, Page* p, int size, int tries);
    // 0x7d222c4, 0x7d22324, 0x7d2237c - Doubly linked page lists.
    void BinInsertFirst(Bin& b, Page& p);
    void BinInsertLast(Bin& b, Page& p);
    void BinRemove(Bin& b, Page& p);
    // 0x7d22ff4 - Pops the page's first free block (null if none). 0x7d22038
    // (PageNextBlockMustSucceed) has the same body.
    Block* PageNextBlock(Page& p, int size, bool allowFull);
    Block* PageNextBlockMustSucceed(Page& p, int size, bool allowFull) {
        return PageNextBlock(p, size, allowFull);
    }
    // 0x7d23038 - Returns every empty page that is still attached to a bin to the free
    // list (used when the free list runs dry).
    void PageForceFree();
    // 0x7d220e8 - Takes a page from the free list for bin `bin`, zeroes it and commits its
    // first blocks. Null if no page is left.
    Page* PageAllocate(int bin);
    // 0x7d21fa4 - Carves up to 4 KiB of not yet committed blocks onto the free list.
    void PageExpand(Page& p);
    // 0x7d22040, 0x7d22094
    void PageMoveToFullBin(Page& p, int bin);
    void PageMoveToAvailableBin(Page& p, int bin);
    // 0x7d2309c - Detaches the page from its bin and puts it at the head of the free list.
    void PageFree(Page& p);

    Page* PageAt(int index) {
        return index != 0 ? &_pages[static_cast<std::size_t>(index)] : nullptr;
    }
    Ptr& BlockNext(Ptr block);

    Config _config;
    Offsets _offsets;
    bool _reserved = false;
    std::byte* _memory = nullptr; // HEAP_SIZE bytes, owned by the group
    Data _data;
    std::array<Bin, PAGE_BIN_COUNT> _bins{};
    std::array<Bin, PAGE_BIN_COUNT> _binsFull{};
    std::vector<int> _binsSmall; // bin for each word count up to SMALL_BIN_COUNT
    std::vector<Page> _pages;
    DisabledHeapAllocTrackerStruct _disabledTracker;
};

} // namespace Quantum::Allocator
