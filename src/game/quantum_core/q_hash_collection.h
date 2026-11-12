// q_hash_collection.h - Quantum.Collections.QHashCollection: the chained hash table in
// frame memory behind QDictionary, QEnumDictionary and QHashSet.
//
// Layout (all in the frame heap):
//  * the collection header (32 bytes, embedded in its owner): bucket array Ptr, head of
//    the free-entry list, used/free counters, key offset, and the entry QBuffer;
//  * a bucket array of Entries.Length Ptrs (the capacity is always a table prime);
//  * the entry buffer: each entry is a 12-byte Entry header followed, at KeyOffset, by the
//    key and (for dictionaries) the value.
// UsedCount is a high-water mark of entry slots ever handed out; removed entries go onto
// the free list and are reused first, so Count = UsedCount - FreeCount. Iteration walks the
// slots 0..UsedCount-1 and skips those not in the Used state, so slot order (and with it
// the exact insert/remove behaviour below) is what makes iteration deterministic.
//
// The operations are templates over the key type and the heap type, matching the
// original's generic methods; each comment cites the instance for EntityRef keys (or the
// object-heap instance for key-independent methods).
#pragma once

#include "quantum_core/q_buffer.h"

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace Quantum::Collections {

// Hash of a key as the collections use it: the key's GetHashCode() viewed as unsigned.
// For built-in types this follows the .NET GetHashCode of the corresponding C# type.
template <class T> std::uint32_t GetKeyHash(const T& key) {
    if constexpr (requires { key.GetHashCode(); }) {
        return static_cast<std::uint32_t>(key.GetHashCode());
    } else if constexpr (std::is_enum_v<T>) {
        return GetKeyHash(static_cast<std::underlying_type_t<T>>(key));
    } else if constexpr (std::is_same_v<T, std::int64_t> || std::is_same_v<T, std::uint64_t>) {
        const auto bits = static_cast<std::uint64_t>(key);
        return static_cast<std::uint32_t>(bits) ^ static_cast<std::uint32_t>(bits >> 32);
    } else {
        // int, uint, ushort and byte hash to their value. The signed 8/16-bit types and
        // char have runtime-specific GetHashCode formulas; check them before allowing them.
        static_assert(std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t> ||
                          std::is_same_v<T, std::uint16_t> || std::is_same_v<T, std::uint8_t>,
                      "no verified C# hash for this key type");
        return static_cast<std::uint32_t>(key);
    }
}

struct QHashCollection {
    enum class EntryState : std::int32_t {
        None = 0, // never used
        Free = 1, // on the free list
        Used = 2,
    };

    struct Entry {
        Quantum::Ptr Next; // next entry in the bucket chain or the free list
        std::uint32_t Hash = 0;
        EntryState State = EntryState::None;
    };

    // Overload selector for the enum-key Find (the original compares enum keys by value
    // instead of through IEquatable; in C++ both are ==).
    struct Tag {};

    Quantum::Ptr Buckets;
    Quantum::Ptr FreeHead;
    int UsedCount = 0;
    int FreeCount = 0;
    int KeyOffset = 0;
    QBuffer Entries;

    // 0x7d20f84 - Pads the entry header and the key to a common alignment (4, or 8 for keys
    // whose size is a multiple of 8). Out: rounded keyStride, header size in entryStride.
    static void CalculateStrides(int& keyStride, int& entryStride);

    // 0x5112ff0
    template <Allocator::FrameHeapLike THeap>
    static Quantum::Ptr* GetBuckets(THeap& heap, QHashCollection* collection) {
        return Resolve<Quantum::Ptr>(heap, collection->Buckets);
    }

    // 0x51133c4
    template <Allocator::FrameHeapLike THeap>
    static Entry* GetFreeHead(THeap& heap, QHashCollection* collection) {
        return collection->FreeHead.Offset != 0 ? Resolve<Entry>(heap, collection->FreeHead)
                                                : nullptr;
    }

    // 0x51131c0 - Entry slot `index`.
    template <Allocator::FrameHeapLike THeap>
    static Entry* GetEntry(THeap& heap, QHashCollection* collection, int index) {
        return ElementAt<Entry>(heap.Void(collection->Entries.Ptr),
                                collection->Entries.Stride * index);
    }

    // 0x51135d0
    template <class TKey> static TKey GetKey(const QHashCollection* collection, Entry* entry) {
        return *ElementAt<TKey>(entry, collection->KeyOffset);
    }

    // 0x5110cfc - Sets up an empty table for at least `capacity` keys of type TKey (rounded
    // up to a table prime). The header must be zeroed beforehand.
    template <class TKey, Allocator::FrameHeapLike THeap>
    static void Allocate(THeap& heap, QHashCollection* qHash, int capacity) {
        capacity = QCollectionsUtils::GetNextPrimeGreaterThanOrEqual(capacity);
        int keyStride = static_cast<int>(sizeof(TKey));
        int entryStride = 0;
        CalculateStrides(keyStride, entryStride);
        QBuffer::Init(heap, &qHash->Entries, capacity, entryStride + keyStride);
        qHash->Buckets = heap.AllocateAndClearPtr(capacity * Quantum::Ptr::SIZE);
        qHash->KeyOffset = entryStride;
    }

    // 0x5112d7c - Releases buckets and entries and zeroes the header.
    template <Allocator::FrameHeapLike THeap>
    static void Free(THeap& heap, QHashCollection* qHash) {
        heap.Free(qHash->Buckets);
        QBuffer::Free(heap, &qHash->Entries);
        *qHash = QHashCollection{};
    }

    // 0x511104c - Removes everything, keeping the capacity.
    template <Allocator::FrameHeapLike THeap>
    static void Clear(THeap& heap, QHashCollection* collection) {
        collection->UsedCount = 0;
        collection->FreeCount = 0;
        collection->FreeHead = Quantum::Ptr::Null();
        std::memset(heap.Void(collection->Buckets), 0,
                    static_cast<std::size_t>(collection->Entries.Length) * Quantum::Ptr::SIZE);
        QBuffer::Clear(heap, &collection->Entries);
    }

    // 0x5112108 - The entry holding `value`, or null.
    template <class TKey, Allocator::FrameHeapLike THeap>
    static Entry* Find(THeap& heap, QHashCollection* collection, const TKey& value,
                       std::uint32_t valueHash) {
        const Quantum::Ptr* buckets = GetBuckets(heap, collection);
        Quantum::Ptr next = buckets[BucketIndex(collection, valueHash)];
        while (next.Offset != 0) {
            Entry* entry = Resolve<Entry>(heap, next);
            if (entry->Hash == valueHash &&
                *ElementAt<TKey>(entry, collection->KeyOffset) == value) {
                return entry;
            }
            next = entry->Next;
        }
        return nullptr;
    }

    // 0x51129e8 (instance for Int32Enum keys)
    template <class TKey, Allocator::FrameHeapLike THeap>
    static Entry* Find(THeap& heap, QHashCollection* collection, const TKey& value,
                       std::uint32_t valueHash, Tag) {
        return Find(heap, collection, value, valueHash);
    }

    // 0x5113cb4 - Stores a new entry for `key` (which must not be present yet) and returns
    // it. Reuses the most recently freed slot if any, otherwise the next unused slot,
    // growing the table when every slot has been used.
    template <class TKey, Allocator::FrameHeapLike THeap>
    static Entry* Insert(THeap& heap, QHashCollection* collection, const TKey& key,
                         std::uint32_t keyHash) {
        Entry* entry = GetFreeHead(heap, collection);
        if (entry != nullptr) {
            collection->FreeHead = entry->Next;
            --collection->FreeCount;
        } else {
            if (collection->UsedCount == collection->Entries.Length) {
                Expand(heap, collection);
            }
            entry = ElementAt<Entry>(heap.Void(collection->Entries.Ptr),
                                     collection->Entries.Stride * collection->UsedCount);
            ++collection->UsedCount;
        }

        const int bucket = BucketIndex(collection, keyHash);
        Quantum::Ptr* buckets = GetBuckets(heap, collection);
        entry->Hash = keyHash;
        entry->State = EntryState::Used;
        entry->Next = buckets[bucket];
        *ElementAt<TKey>(entry, collection->KeyOffset) = key;
        buckets[bucket] = heap.Void(static_cast<const void*>(entry));
        return entry;
    }

    // 0x511097c - Inserts `key` unless present; true if it was added.
    template <class TKey, Allocator::FrameHeapLike THeap>
    static bool Add(THeap& heap, QHashCollection* collection, const TKey& key,
                    std::uint32_t keyHash) {
        if (Find(heap, collection, key, keyHash) != nullptr) {
            return false;
        }
        Insert(heap, collection, key, keyHash);
        return true;
    }

    // 0x5114dd0 - Unlinks the entry for `value` and puts its slot on the free list (key and
    // value bytes are left as they were). True if the key was present.
    template <class TKey, Allocator::FrameHeapLike THeap>
    static bool Remove(THeap& heap, QHashCollection* collection, const TKey& value,
                       std::uint32_t valueHash) {
        const int bucket = BucketIndex(collection, valueHash);
        Quantum::Ptr current = GetBuckets(heap, collection)[bucket];
        Entry* previous = nullptr;
        while (current.Offset != 0) {
            Entry* entry = Resolve<Entry>(heap, current);
            if (entry->Hash == valueHash &&
                *ElementAt<TKey>(entry, collection->KeyOffset) == value) {
                if (previous != nullptr) {
                    previous->Next = entry->Next;
                } else {
                    GetBuckets(heap, collection)[bucket] = entry->Next;
                }
                entry->State = EntryState::Free;
                entry->Next = collection->FreeHead;
                collection->FreeHead = current;
                ++collection->FreeCount;
                return true;
            }
            previous = entry;
            current = entry->Next;
        }
        return false;
    }

    // 0x5111354 - Grows to the next table prime: copies the entry slots in order, then
    // rebuilds the bucket chains and the free list by walking the slots from last to first.
    template <Allocator::FrameHeapLike THeap>
    static void Expand(THeap& heap, QHashCollection* collection) {
        const int capacity = QCollectionsUtils::GetNextPrime(collection->Entries.Length);
        auto* buckets =
            static_cast<Quantum::Ptr*>(heap.AllocateAndClear(capacity * Quantum::Ptr::SIZE));
        QBuffer entries;
        QBuffer::Init(heap, &entries, capacity, collection->Entries.Stride);
        QBuffer::Copy(heap, collection->Entries, 0, entries, 0, collection->Entries.Length);

        collection->FreeHead = Quantum::Ptr::Null();
        collection->FreeCount = 0;
        void* base = heap.Void(entries.Ptr);
        for (int i = collection->Entries.Length - 1; i >= 0; --i) {
            Entry* entry = ElementAt<Entry>(base, entries.Stride * i);
            if (entry->State == EntryState::Free) {
                entry->Next = collection->FreeHead;
                collection->FreeHead = heap.Void(static_cast<const void*>(entry));
                ++collection->FreeCount;
            } else if (entry->State == EntryState::Used) {
                const std::uint32_t bucket = entry->Hash % static_cast<std::uint32_t>(capacity);
                entry->Next = buckets[bucket];
                buckets[bucket] = heap.Void(static_cast<const void*>(entry));
            }
        }

        heap.Free(collection->Buckets);
        QBuffer::Free(heap, &collection->Entries);
        collection->Buckets = heap.Void(static_cast<const void*>(buckets));
        collection->Entries = entries;
    }

    // TODO(0x5115bc4, Quantum.Collections.QHashCollection.Serialize<EntityRef>): writes the
    // used entries' keys through a key serializer; needs FrameSerializer.
    // TODO(0x7d2103c, Quantum.Collections.QHashCollection.Print): needs FramePrinter.

  private:
    static int BucketIndex(const QHashCollection* collection, std::uint32_t hash) {
        return static_cast<int>(hash % static_cast<std::uint32_t>(collection->Entries.Length));
    }
};

static_assert(sizeof(QHashCollection::Entry) == 12);
static_assert(sizeof(QHashCollection) == 32);

} // namespace Quantum::Collections
