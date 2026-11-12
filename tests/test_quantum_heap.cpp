#include "test_framework.h"

#include "photon_deterministic/native_allocator.h"
#include "quantum_core/frame_heap.h"
#include "quantum_core/heap.h"

#include <cstring>
#include <memory>
#include <vector>

using Photon::Deterministic::Native::SystemAllocator;
using Quantum::Ptr;
using Quantum::Allocator::FrameHeap;
using Quantum::Allocator::Heap;
using Quantum::Allocator::HeapTrackingMode;

namespace {

template <class E, class F> bool Throws(F&& f) {
    try {
        f();
    } catch (const E&) {
        return true;
    }
    return false;
}

// A group of heaps with 4 KiB pages, released when the test ends.
struct TestGroup {
    SystemAllocator allocator;
    Heap::Group* group;

    explicit TestGroup(int pageCount = 256, int heapCount = 2)
        : group(Heap::Group::Allocate(
              Heap::Config(Heap::Config::PAGE_SHIFT_SIZE_4KB, pageCount, heapCount), allocator)) {}
    ~TestGroup() { Heap::Group::Free(group, allocator); }
    Heap& heap(int i = 0) { return *group->GetHeap(i); }
};

} // namespace

TEST_CASE("Heap::Config derives sizes and ignores HEAP_COUNT when comparing") {
    const Heap::Config config(12, 256, 2);
    CHECK(config.PAGE_SIZE == 4096);
    CHECK(config.HEAP_SIZE == 1 << 20);
    CHECK(config.SMALL_BIN_SIZE == 512);
    CHECK(config.SMALL_BIN_COUNT == 64);
    CHECK(config.SMALL_BIN_ARRAY_SIZE == 65);
    CHECK(Heap::Config(13, 64, 1).SMALL_BIN_SIZE == 512);

    const Heap::Config other(12, 256, 7);
    CHECK(config == other);
    CHECK(config.GetHashCode() == other.GetHashCode());
    CHECK(config != Heap::Config(12, 128, 2));
    CHECK(config.GetHashCode() != Heap::Config(12, 128, 2).GetHashCode());
}

TEST_CASE("Heap::Group::CalculateMetaOffsets lays out the bookkeeping pages") {
    const Heap::Offsets offsets = Heap::Group::CalculateMetaOffsets(Heap::Config(12, 256, 1));
    CHECK(offsets.DataOffset == 0);
    CHECK(offsets.BinsOffset == 8);
    CHECK(offsets.BinsFullOffset == 8 + 784);
    CHECK(offsets.BinsSmallOffset == 8 + 784 * 2);
    CHECK(offsets.PagesOffset == 8 + 784 * 2 + 65 * 4);
    CHECK(offsets.MetaDataPages == 3);
    CHECK(Heap::Group::CalculateMetaOffsets(Heap::Config(12, 8, 1)).MetaDataPages == 1);
}

TEST_CASE("Heap size classes cover every request with the smallest fitting bin") {
    TestGroup g;
    const auto bins = Heap::Group::Bins::Default();
    CHECK(bins.size() == 49);
    CHECK(bins[8].BlockSize == 64);
    CHECK(bins[9].BlockSize == 80);
    CHECK(bins[48].BlockSize == 65536);
    bool ok = true;
    for (int size = 1; size <= 65536; ++size) {
        const int bin = g.heap().CalculateBin(size);
        ok = ok && bin >= 1 && bin < 49 && bins[static_cast<std::size_t>(bin)].BlockSize >= size &&
             bins[static_cast<std::size_t>(bin - 1)].BlockSize < size;
    }
    CHECK(ok);
    CHECK(Heap::CalculateWordSize(1) == 1);
    CHECK(Heap::CalculateWordSize(8) == 1);
    CHECK(Heap::CalculateWordSize(9) == 2);
    CHECK(Heap::CalculateWordSize(-7) == 0);
}

TEST_CASE("Heap::BitScanReverse agrees with the De Bruijn table lookup") {
    std::uint8_t table[32] = {};
    Heap::Group::InitDeBruijnTable(table);
    bool ok = true;
    for (std::uint32_t v = 0; v < 70000; v = v * 3 + 1) {
        std::uint32_t smeared = v;
        smeared |= smeared >> 1;
        smeared |= smeared >> 2;
        smeared |= smeared >> 4;
        smeared |= smeared >> 8;
        smeared |= smeared >> 16;
        ok = ok && table[(smeared * 0x07C4ACDDu) >> 27] == Heap::BitScanReverse(v);
    }
    CHECK(ok);
    CHECK(Heap::BitScanReverse(0) == 0);
    CHECK(Heap::BitScanReverse(0x80000000u) == 31);
}

TEST_CASE("Heap hands out blocks deterministically from the top of a page down") {
    TestGroup g;
    Heap& heap = g.heap();
    // Pages 0-2 hold bookkeeping; page 3 is the first data page. A 16-byte bin page
    // commits all 256 blocks at once and the last one carved is handed out first.
    const Ptr first = heap.AllocateAndClearPtr(16);
    const Ptr second = heap.AllocateAndClearPtr(16);
    CHECK(first.Offset == 3 * 4096 + 255 * 16);
    CHECK(second.Offset == 3 * 4096 + 254 * 16);
    // A different size class takes the next page.
    CHECK(heap.AllocateAndClearPtr(24).Offset == 4 * 4096 + (4096 / 24 - 1) * 24);

    heap.Free(second);
    CHECK(heap.AllocateAndClearPtr(16) == second);
    CHECK(heap.Void(heap.Void(first)) == first);
}

TEST_CASE("Heap zeroes blocks, rejects bad sizes and frees empty pages") {
    TestGroup g;
    Heap& heap = g.heap();
    const Heap::Stats fresh = Heap::GetStats(&heap);
    CHECK(fresh.TotalPages == 256);
    CHECK(fresh.PagesFree == 253);
    CHECK(fresh.PagesUsed == 3);
    CHECK(fresh.PagesFull == 3);
    CHECK(fresh.BytesAllocated == 3 * 4096);

    auto* block = static_cast<unsigned char*>(heap.AllocateAndClear(40));
    std::memset(block, 0xAB, 40);
    heap.Free(block);
    CHECK(Heap::GetStats(&heap).PagesFree == 253); // the page went back immediately
    auto* again = static_cast<unsigned char*>(heap.AllocateAndClear(40));
    CHECK(again[0] == 0 && again[39] == 0);

    CHECK(heap.AllocateAndClear(0) == nullptr);
    CHECK(Throws<std::length_error>([&] { heap.AllocateAndClear(4097); }));
    CHECK(Throws<std::invalid_argument>([&] { heap.Free(static_cast<void*>(nullptr)); }));
    CHECK(Throws<std::invalid_argument>([&] { heap.Free(Ptr::Null()); }));
}

TEST_CASE("Heap reuses blocks freed on full pages") {
    TestGroup g;
    Heap& heap = g.heap();
    std::vector<Ptr> blocks;
    for (int i = 0; i < 512; ++i) {
        blocks.push_back(heap.AllocateAndClearPtr(16)); // fills pages 3 and 4
    }
    CHECK(blocks[255].Offset / 4096 == 3);
    CHECK(blocks[256].Offset / 4096 == 4);
    CHECK(heap.GetPage(3).Flags == Heap::Full);

    heap.Free(blocks[10]);
    // Page 4 is found exhausted and retired; page 3 on the full list has a block again.
    CHECK(heap.AllocateAndClearPtr(16) == blocks[10]);
    CHECK(heap.AllocateAndClearPtr(16).Offset / 4096 == 5);
}

TEST_CASE("Heap returns null when out of pages") {
    TestGroup g(8, 1); // one bookkeeping page, seven data pages
    Heap& heap = g.heap();
    for (int i = 0; i < 7; ++i) {
        CHECK(heap.AllocateAndClear(4096) != nullptr);
    }
    CHECK(heap.AllocateAndClear(4096) == nullptr);
    CHECK(Heap::GetStats(&heap).PagesFree == 0);
}

TEST_CASE("Heap::Copy reproduces allocations and contents") {
    TestGroup g;
    Heap& a = g.heap(0);
    Heap& b = g.heap(1);
    const Ptr p = a.AllocateAndClearPtr(100);
    std::memcpy(a.Void(p), "frame data", 11);
    a.AllocateAndClearPtr(16);

    Heap::Copy(nullptr, &b, &a);
    CHECK(std::memcmp(b.Void(p), "frame data", 11) == 0);
    CHECK(a.AllocateAndClearPtr(16) == b.AllocateAndClearPtr(16));
    CHECK(a.AllocateAndClearPtr(300) == b.AllocateAndClearPtr(300));

    TestGroup other(128, 1);
    CHECK(Throws<std::invalid_argument>([&] { Heap::Copy(nullptr, &other.heap(), &a); }));
}

TEST_CASE("Heap::Group reserves and releases heaps") {
    TestGroup g;
    Heap* first = g.group->Reserve();
    Heap* second = g.group->Reserve();
    CHECK(first == g.group->GetHeap(0));
    CHECK(second == g.group->GetHeap(1));
    CHECK(Throws<std::logic_error>([&] { g.group->Reserve(); }));
    CHECK(Throws<std::out_of_range>([&] { g.group->GetHeap(2); }));

    const Ptr p = first->AllocateAndClearPtr(16);
    g.group->Release(first);
    CHECK(!first->IsReserved());
    CHECK(Heap::GetStats(first).PagesFree == 253);
    CHECK(g.group->Reserve() == first);
    CHECK(first->AllocateAndClearPtr(16) == p);
    // Heaps start on page boundaries.
    CHECK(reinterpret_cast<std::uintptr_t>(first->Memory()) % 4096 == 0);
}

TEST_CASE("FrameHeap trackers and the allocation tracker") {
    CHECK(dynamic_cast<Quantum::Allocator::DisabledHeapAllocTracker*>(
              FrameHeap::GetAllocationTrackerForMode(HeapTrackingMode::Disabled).get()) != nullptr);
    CHECK(dynamic_cast<Quantum::Allocator::SimpleHeapAllocTracker*>(
              FrameHeap::GetAllocationTrackerForMode(HeapTrackingMode::DetectLeaks).get()) !=
          nullptr);
    CHECK(dynamic_cast<Quantum::Allocator::TracedHeapAllocTracker*>(
              FrameHeap::GetAllocationTrackerForMode(HeapTrackingMode::TraceAllocations).get()) !=
          nullptr);
    CHECK(Throws<std::out_of_range>(
        [] { FrameHeap::GetAllocationTrackerForMode(static_cast<HeapTrackingMode>(3)); }));

    TestGroup g;
    FrameHeap frameHeap(&g.heap(),
                        FrameHeap::GetAllocationTrackerForMode(HeapTrackingMode::Disabled));
    Heap::AllocationTracker tracker;
    for (int i = 0; i < 10; ++i) {
        tracker.TrackAllocation(frameHeap, frameHeap.AllocateAndClearPtr(48));
    }
    CHECK(tracker._trackAllocCount == 10);
    CHECK(tracker._trackAllocCapacity == 16);
    tracker.FreeAllTracked(frameHeap);
    CHECK(tracker._trackAllocCount == 0);
    // Only the tracker's own array (64 bytes) remains.
    CHECK(Heap::GetStats(&g.heap()).BytesAllocated == 3 * 4096 + 64);
}

TEST_CASE("Native allocator helpers") {
    using namespace Photon::Deterministic::Native;
    CHECK(RoundUpToAlignment(13, 4) == 16);
    CHECK(RoundUpToAlignment(16, 16) == 16);
    CHECK(RoundUpToAlignment(1, 64) == 64);
    CHECK(Throws<std::invalid_argument>([] { RoundUpToAlignment(5, 3); }));
    CHECK(GetAlignmentForArrayElement(16) == 8);
    CHECK(GetAlignmentForArrayElement(12) == 4);
    CHECK(GetAlignmentForArrayElement(0) == 4);

    SystemAllocator allocator;
    auto* data = static_cast<int*>(allocator.AllocAndClear(4 * 4, 64));
    CHECK(reinterpret_cast<std::uintptr_t>(data) % 64 == 0);
    CHECK(data[3] == 0);
    data[0] = 7;
    data = allocator.ExpandArray(data, 4, 16);
    CHECK(data[0] == 7 && data[15] == 0);
    allocator.Free(data);
}
