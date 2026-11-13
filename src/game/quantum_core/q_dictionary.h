// q_dictionary.h - Quantum.Collections.QDictionary<K,V>, QEnumDictionary<K,V> and their
// helpers: hash maps whose storage lives in a frame heap.
//
// QDictionaryInternal (36 bytes, allocated in the heap) is a QHashCollection whose entries
// carry the value right after the key, at _valueOffset. Components store a
// QDictionaryPtr (just the Ptr of the internal record); QDictionary is the transient view
// built from that Ptr plus the frame heap to operate on it.
//
// QEnumDictionary is the same structure for enum keys; the original hashes the key's
// underlying integer instead of calling GetHashCode on a boxed enum.
#pragma once

#include "quantum_core/q_hash_collection.h"

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Quantum::Collections {

struct QDictionaryInternal {
    int _valueOffset = 0;
    QHashCollection _collection;

    // 0x7d205a0 - Strides for entry header, key and value: all padded to the largest of 4
    // and the key/value element alignments.
    static void CalculateStrides(int& entryStride, int& keyStride, int& valStride);

    // 0x51100dc (instance for object) - Allocates the record and an empty table for at
    // least `capacity` keys.
    template <Allocator::FrameHeapLike THeap>
    static QDictionaryInternal* Allocate(THeap& heap, int capacity, int keyStride, int valStride) {
        capacity = QCollectionsUtils::GetNextPrimeGreaterThanOrEqual(capacity);
        int entryStride = 0;
        CalculateStrides(entryStride, keyStride, valStride);
        auto* map = static_cast<QDictionaryInternal*>(
            heap.AllocateAndClear(static_cast<int>(sizeof(QDictionaryInternal))));
        map->_collection.Buckets = heap.AllocateAndClearPtr(capacity * Quantum::Ptr::SIZE);
        QBuffer::Init(heap, &map->_collection.Entries, capacity,
                      keyStride + entryStride + valStride);
        map->_collection.UsedCount = 0;
        map->_collection.FreeCount = 0;
        map->_collection.KeyOffset = entryStride;
        map->_valueOffset = keyStride + entryStride;
        return map;
    }

    // 0x5110580 (object) - Releases entries, buckets and the record.
    template <Allocator::FrameHeapLike THeap>
    static void Free(THeap& heap, QDictionaryInternal* map) {
        QBuffer::Free(heap, &map->_collection.Entries);
        heap.Free(map->_collection.Buckets);
        heap.Free(static_cast<void*>(map));
    }

    // TODO(0x7d20694, Quantum.Collections.QDictionaryInternal.Print): needs FramePrinter.
    // 0x7d20b00 - The dictionary record behind `ptr`: counts, free list head, then every
    // entry slot (its address is keyed too, entries link to each other) with chain link,
    // hash and state, and key/value of the used ones; finally the bucket array.
    static void Serialize(Quantum::Ptr* ptr, FrameSerializer* serializer, int keyStride,
                          int valStride, QBuffer::ItemSerializer keySerializer,
                          QBuffer::ItemSerializer valueSerializer);
};

static_assert(sizeof(QDictionaryInternal) == 36);

// What components store for a dictionary field.
template <class K, class V> struct QDictionaryPtr {
    Quantum::Ptr Ptr;

    QDictionaryPtr() = default;
    // 0x6586850 (instance for <AssetRefConsideration, FP>)
    explicit QDictionaryPtr(Quantum::Ptr ptr) : Ptr(ptr) {}

    // 0x6586858
    int GetHashCode() const { return Ptr.GetHashCode(); }
    // 0x6586950 (Equals(QDictionaryPtr)); 0x6586860 is the boxed Equals(object).
    bool Equals(const QDictionaryPtr& other) const { return Ptr == other.Ptr; }
    friend bool operator==(const QDictionaryPtr& a, const QDictionaryPtr& b) { return a.Equals(b); }
};

// Walks the used entry slots in slot order.
template <class K, class V> struct QDictionaryIterator {
    int _index = -1;
    Allocator::IFrameHeap* _heap = nullptr;
    QDictionaryInternal* _map = nullptr;
    std::byte* _current = nullptr;

    QDictionaryIterator() = default;
    // 0x65847ec (instance for <AssetRefConsideration, FP>)
    QDictionaryIterator(Allocator::IFrameHeap* heap, QDictionaryInternal* map)
        : _index(-1), _heap(heap), _map(map), _current(nullptr) {}

    // 0x658481c - Throws std::logic_error before the first MoveNext / after the end.
    std::pair<K, V> get_Current() const {
        if (_current == nullptr) {
            throw std::logic_error("QDictionaryIterator: no current element");
        }
        return {*ElementAt<K>(_current, _map->_collection.KeyOffset),
                *ElementAt<V>(_current, _map->_valueOffset)};
    }

    // 0x65848b8
    bool MoveNext() {
        const QHashCollection& collection = _map->_collection;
        while (++_index < collection.UsedCount) {
            auto* entry = ElementAt<QHashCollection::Entry>(_heap->Void(collection.Entries.Ptr),
                                                            collection.Entries.Stride * _index);
            if (entry->State == QHashCollection::EntryState::Used) {
                _current = reinterpret_cast<std::byte*>(entry);
                return true;
            }
        }
        _current = nullptr;
        return false;
    }
};

namespace detail {

// Shared body of QDictionary and QEnumDictionary; they differ only in how a key is hashed
// and which Find overload is used. `Derived` supplies Hash(key) and FindEntry(key, hash).
template <class Derived, class K, class V> class QDictionaryBase {
  public:
    using Entry = QHashCollection::Entry;

    QDictionaryBase() = default;
    QDictionaryBase(Allocator::IFrameHeap* heap, QDictionaryInternal* map)
        : _heap(heap), _map(map) {}

    // 0x658777c - Number of keys.
    int get_Count() const { return Map()->_collection.UsedCount - Map()->_collection.FreeCount; }

    // 0x6587798 - Throws std::out_of_range (KeyNotFoundException) for a missing key.
    V get_Item(const K& key) const {
        Entry* entry = Self().FindEntry(key, Derived::Hash(key));
        if (entry == nullptr) {
            throw std::out_of_range("QDictionary: key not found");
        }
        return *ElementAt<V>(entry, Map()->_valueOffset);
    }

    // 0x6587890 - Adds or overwrites.
    void set_Item(const K& key, const V& value) {
        const std::uint32_t hash = Derived::Hash(key);
        Entry* entry = Self().FindEntry(key, hash);
        if (entry == nullptr) {
            entry = QHashCollection::Insert(*_heap, &Map()->_collection, key, hash);
        }
        *ElementAt<V>(entry, Map()->_valueOffset) = value;
    }

    // 0x65879a8
    bool ContainsKey(const K& key) const {
        return Self().FindEntry(key, Derived::Hash(key)) != nullptr;
    }

    // 0x6587aa4 - Default value when absent.
    bool TryGetValue(const K& key, V& value) const {
        Entry* entry = Self().FindEntry(key, Derived::Hash(key));
        value = entry != nullptr ? *ElementAt<V>(entry, Map()->_valueOffset) : V{};
        return entry != nullptr;
    }

    // 0x6587b94 - Pointer to the stored value (valid until the table grows), null when
    // absent.
    bool TryGetValuePointer(const K& key, V*& value) const {
        Entry* entry = Self().FindEntry(key, Derived::Hash(key));
        value = entry != nullptr ? ElementAt<V>(entry, Map()->_valueOffset) : nullptr;
        return entry != nullptr;
    }

    // 0x6587c84 - Throws std::invalid_argument if the key is already present.
    void Add(const K& key, const V& value) {
        if (!TryAdd(key, value)) {
            throw std::invalid_argument("QDictionary: an element with the same key already exists");
        }
    }

    // 0x6587d24
    bool TryAdd(const K& key, const V& value) {
        const std::uint32_t hash = Derived::Hash(key);
        if (Self().FindEntry(key, hash) != nullptr) {
            return false;
        }
        Entry* entry = QHashCollection::Insert(*_heap, &Map()->_collection, key, hash);
        *ElementAt<V>(entry, Map()->_valueOffset) = value;
        return true;
    }

    // 0x6587ee8
    QDictionaryIterator<K, V> GetEnumerator() const { return {_heap, _map}; }

    // 0x6587f3c - Value part of an entry.
    void* GetValue(Entry* entry) const {
        return reinterpret_cast<std::byte*>(entry) + Map()->_valueOffset;
    }

    // 0x6587f58 - The storable reference (null Ptr for an unallocated view).
    operator QDictionaryPtr<K, V>() const {
        if (_heap == nullptr || _map == nullptr) {
            return QDictionaryPtr<K, V>{};
        }
        return QDictionaryPtr<K, V>{_heap->Void(static_cast<const void*>(_map))};
    }

    Allocator::IFrameHeap* _heap = nullptr;
    QDictionaryInternal* _map = nullptr;

  protected:
    QDictionaryInternal* Map() const {
        if (_map == nullptr) {
            throw std::logic_error("QDictionary: not allocated");
        }
        return _map;
    }
    const Derived& Self() const { return static_cast<const Derived&>(*this); }
};

} // namespace detail

template <class K, class V>
class QDictionary : public detail::QDictionaryBase<QDictionary<K, V>, K, V> {
    using Base = detail::QDictionaryBase<QDictionary<K, V>, K, V>;

  public:
    QDictionary() = default;
    // 0x6588034 (instance for <AssetRefConsideration, FP>)
    QDictionary(Allocator::IFrameHeap* heap, QDictionaryInternal* map) : Base(heap, map) {}

    // 0x4aa53d8 - A new, empty dictionary for at least `capacity` keys.
    static QDictionary Allocate(Allocator::IFrameHeap* heap, int capacity) {
        QDictionaryInternal* map = QDictionaryInternal::Allocate(
            *heap, capacity, static_cast<int>(sizeof(K)), static_cast<int>(sizeof(V)));
        return QDictionary(heap, map);
    }

    // 0x6587a4c
    void Clear() { QHashCollection::Clear(*this->_heap, &this->Map()->_collection); }

    // 0x6587e48 - True if the key was present.
    bool Remove(const K& key) {
        return QHashCollection::Remove(*this->_heap, &this->Map()->_collection, key, Hash(key));
    }

    static std::uint32_t Hash(const K& key) { return GetKeyHash(key); }
    QHashCollection::Entry* FindEntry(const K& key, std::uint32_t hash) const {
        return QHashCollection::Find(*this->_heap, &this->Map()->_collection, key, hash);
    }
};

template <class K, class V>
class QEnumDictionary : public detail::QDictionaryBase<QEnumDictionary<K, V>, K, V> {
    static_assert(std::is_enum_v<K>, "QEnumDictionary keys are enums");
    using Base = detail::QDictionaryBase<QEnumDictionary<K, V>, K, V>;

  public:
    QEnumDictionary() = default;
    // 0x658ef68 (instance for <ByteEnum, CollectData>)
    QEnumDictionary(Allocator::IFrameHeap* heap, QDictionaryInternal* map) : Base(heap, map) {}

    // 0x4aa59ec
    static QEnumDictionary Allocate(Allocator::IFrameHeap* heap, int capacity) {
        QDictionaryInternal* map = QDictionaryInternal::Allocate(
            *heap, capacity, static_cast<int>(sizeof(K)), static_cast<int>(sizeof(V)));
        return QEnumDictionary(heap, map);
    }

    // The key's underlying value, zero-extended for byte enums, as-is for int enums (the
    // only two kinds the game uses; signed 8/16-bit enums would need checking).
    static std::uint32_t Hash(const K& key) {
        return static_cast<std::uint32_t>(static_cast<std::underlying_type_t<K>>(key));
    }
    QHashCollection::Entry* FindEntry(const K& key, std::uint32_t hash) const {
        return QHashCollection::Find(*this->_heap, &this->Map()->_collection, key, hash,
                                     QHashCollection::Tag{});
    }
};

// Method address map for QEnumDictionary (instance <ByteEnum, CollectData>): get_Item
// 0x658e88c, set_Item 0x658e960, ContainsKey 0x658ea5c, TryGetValue 0x658eae4,
// TryGetValuePointer 0x658ebb0, Add 0x658ec7c, TryAdd 0x658ed1c, GetEnumerator 0x658ee24,
// GetValue 0x658ee70, op_Implicit 0x658ee8c. The shared bodies are in QDictionaryBase.

// TODO(0x510fed8, Quantum.Collections.QDictionary.Serialize<K,V>): forwards to
// QDictionaryInternal.Serialize with the key/value sizes; needs FrameSerializer.

} // namespace Quantum::Collections
