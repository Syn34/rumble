// frame_meta_data.h - Quantum.FrameMetaData: framework bookkeeping stored inside the frame
// (in the game's global state, so it is copied, rolled back and checksummed with it).
//
// It holds the deferred-command sets - entities to destroy and components to remove at
// the next commit - and the per-frame allocation tracker (blocks freed at the start of
// the next simulated frame).
//
// A deferred set is a QHashCollectionList: a growable array of hash sets in frame memory.
// Entries are added to the last set in use; when it is full the next set is used (and
// allocated, with the same capacity, on first use) rather than growing the set, so adding
// never rehashes. After a commit only the first set stays in use; sets left unused for a
// while are released again, one per commit.
#pragma once

#include "quantum_core/frame_heap.h"
#include "quantum_core/heap.h"
#include "quantum_core/q_hash_collection.h"

#include <cstdint>
#include <cstring>

namespace Quantum {

class FrameSerializer;

struct FrameMetaData {
    static constexpr int SIZE = 40;

    struct QHashCollectionList {
        static constexpr int SIZE = 16;
        static constexpr int DEFAULT_LIST_CAPACITY = 4;
        static constexpr int DEFAULT_ENTRY_CAPACITY = 128;
        // Commits a spare set may stay unused before it is released.
        static constexpr int DISPOSE_BLOCK_AGE = 30;

        std::uint16_t _qHashListUsedCount = 0;
        std::uint16_t _qHashListCapacity = 0;
        int _qHashListAllocatedCount = 0;
        int _qHashListLastBlockAge = 0;
        Ptr _qHashListPtr;

        // 0x52e9ab8 (instance for EntityRef keys) - Releases any previous storage, then
        // allocates room for `qHashListCapacity` sets and sets up the first one with
        // `qHashEntryCapacity` entries.
        template <class TKey, Allocator::FrameHeapLike THeap>
        void Allocate(THeap& heap, int qHashEntryCapacity, int qHashListCapacity) {
            Dispose(heap);
            void* sets = heap.AllocateAndClear(qHashListCapacity *
                                               static_cast<int>(sizeof(Collections::QHashCollection)));
            if (sets == nullptr) {
                return;
            }
            _qHashListPtr = heap.Void(static_cast<const void*>(sets));
            _qHashListCapacity = static_cast<std::uint16_t>(qHashListCapacity);
            _qHashListUsedCount = 1;
            _qHashListAllocatedCount = 1;
            Collections::QHashCollection::Allocate<TKey>(
                heap, static_cast<Collections::QHashCollection*>(sets), qHashEntryCapacity);
        }

        // 0x52e9ee8 - Releases every set and the array. The age counter is kept.
        template <Allocator::FrameHeapLike THeap> void Dispose(THeap& heap) {
            if (_qHashListPtr.Offset != 0) {
                auto* sets = Sets(heap);
                for (int i = 0; i < _qHashListAllocatedCount; ++i) {
                    Collections::QHashCollection::Free(heap, &sets[i]);
                }
                heap.Free(_qHashListPtr);
            }
            _qHashListPtr = Ptr{};
            _qHashListUsedCount = 0;
            _qHashListCapacity = 0;
            _qHashListAllocatedCount = 0;
        }

        // 0x52eb248 - Back to one set in use. While spare sets exist, every call ages them;
        // past DISPOSE_BLOCK_AGE the last spare set is released and the age restarts.
        template <Allocator::FrameHeapLike THeap> void Reset(THeap& heap) {
            if (_qHashListAllocatedCount > _qHashListUsedCount) {
                if (++_qHashListLastBlockAge > DISPOSE_BLOCK_AGE) {
                    auto* sets = Sets(heap);
                    --_qHashListAllocatedCount;
                    Collections::QHashCollection::Free(heap, &sets[_qHashListAllocatedCount]);
                    _qHashListLastBlockAge = 0;
                }
            }
            _qHashListUsedCount = 1;
        }

        // 0x52eb040 - The sets in use.
        template <Allocator::FrameHeapLike THeap>
        void GetQHashCollections(THeap& heap, Collections::QHashCollection*& qHash,
                                 int& count) const {
            qHash = Sets(heap);
            count = _qHashListUsedCount;
        }

        // 0x52ea3d0 (instance for EntityRef keys) - The set to add to: the last one in use,
        // or the next one if that is full.
        template <class TKey, Allocator::FrameHeapLike THeap>
        Collections::QHashCollection* GetCurrentQHashCollection(THeap& heap) {
            Collections::QHashCollection* current = &Sets(heap)[_qHashListUsedCount - 1];
            if (current->UsedCount - current->FreeCount < current->Entries.Length) {
                return current;
            }
            return GetNextQHashCollection<TKey>(heap, current->Entries.Length);
        }

        // 0x52ea954 (instance for EntityRef keys) - Takes the next set into use, doubling
        // the array when it is full and allocating the set (with `entriesCapacity`
        // entries) if it never was.
        template <class TKey, Allocator::FrameHeapLike THeap>
        Collections::QHashCollection* GetNextQHashCollection(THeap& heap, int entriesCapacity) {
            auto* sets = Sets(heap);
            if (_qHashListUsedCount == _qHashListCapacity) {
                const int setSize = static_cast<int>(sizeof(Collections::QHashCollection));
                auto* grown = static_cast<Collections::QHashCollection*>(
                    heap.AllocateAndClear(_qHashListUsedCount * 2 * setSize));
                _qHashListPtr = heap.Void(static_cast<const void*>(grown));
                std::memcpy(grown, sets, static_cast<std::size_t>(_qHashListCapacity) * setSize);
                _qHashListCapacity = static_cast<std::uint16_t>(_qHashListUsedCount * 2);
                heap.Free(static_cast<void*>(sets));
                sets = grown;
            }
            Collections::QHashCollection* next = &sets[_qHashListUsedCount];
            ++_qHashListUsedCount;
            if (_qHashListAllocatedCount < _qHashListUsedCount) {
                ++_qHashListAllocatedCount;
                Collections::QHashCollection::Allocate<TKey>(heap, next, entriesCapacity);
            }
            return next;
        }

        // TODO(0x52eb7ac, Quantum.FrameMetaData.QHashCollectionList.Serialize<EntityRef>):
        // writes the counters and each set in use through QHashCollection.Serialize with a
        // key serializer; needs QHashCollection.Serialize.

      private:
        template <Allocator::FrameHeapLike THeap>
        Collections::QHashCollection* Sets(THeap& heap) const {
            return static_cast<Collections::QHashCollection*>(heap.Void(_qHashListPtr));
        }
    };

    QHashCollectionList _entityDestroyList;
    QHashCollectionList _componentRemovalList;
    Allocator::Heap::AllocationTracker _trackedInternal;

    // 0x7cef2bc - The original takes the frame and uses its tracked heap.
    void TrackAllocationInternal(Allocator::FrameHeap& heap, Ptr ptr) {
        _trackedInternal.TrackAllocation(heap, ptr);
    }
    // 0x7cef2cc
    void FreeAllTrackedInternal(Allocator::FrameHeap& heap) {
        _trackedInternal.FreeAllTracked(heap);
    }

    // TODO(0x7cef2d8, Quantum.FrameMetaData.Serialize): serialises both lists and the
    // allocation tracker; needs QHashCollectionList.Serialize and
    // AllocationTracker.Serialize.
};

static_assert(sizeof(FrameMetaData::QHashCollectionList) == FrameMetaData::QHashCollectionList::SIZE);
static_assert(sizeof(FrameMetaData) == FrameMetaData::SIZE);

} // namespace Quantum
