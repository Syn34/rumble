// q_list.h - Quantum.Collections.QList<T>: a growable array whose storage lives in a
// frame heap.
//
// QListInternal (16 bytes, in the heap) holds the element count and a QBuffer with the
// capacity. Components store a QListPtr<T> (the Ptr of the internal record); QList<T> is
// the transient view over it plus the frame heap. Using a view whose heap or list is
// missing throws std::logic_error (InvalidOperationException in the original).
#pragma once

#include "quantum_core/q_buffer.h"

#include <stdexcept>

namespace Quantum::Collections {

struct QListInternal {
    int _count = 0;
    QBuffer _items;

    // 0x51177ec (instance for object) - Allocates a list record with room for `capacity`
    // elements of `stride` bytes. Both must be positive (std::invalid_argument otherwise);
    // a failed allocation throws std::bad_alloc.
    template <Allocator::FrameHeapLike THeap>
    static QListInternal* Allocate(THeap& heap, int capacity, int stride) {
        if (capacity <= 0) {
            throw std::invalid_argument("QList: capacity must be positive");
        }
        if (stride <= 0) {
            throw std::invalid_argument("QList: element size must be positive");
        }
        auto* list = static_cast<QListInternal*>(
            heap.AllocateAndClear(static_cast<int>(sizeof(QListInternal))));
        if (list == nullptr) {
            throw std::bad_alloc();
        }
        return Init(list, heap, capacity, stride);
    }

    // 0x5116860 (instance for <AISpawned, object>)
    template <class T, Allocator::FrameHeapLike THeap>
    static QListInternal* Allocate(THeap& heap, int capacity) {
        return Allocate(heap, capacity, static_cast<int>(sizeof(T)));
    }

    // 0x5117c70 (object) - Empty list with fresh storage.
    template <Allocator::FrameHeapLike THeap>
    static QListInternal* Init(QListInternal* list, THeap& heap, int capacity, int stride) {
        QBuffer::Init(heap, &list->_items, capacity, stride);
        list->_count = 0;
        return list;
    }

    // TODO(0x7d2134c, Quantum.Collections.QListInternal.Print): needs FramePrinter.
    // TODO(0x7d215d8, Quantum.Collections.QListInternal.Serialize): serialises count,
    // buffer and items; needs FrameSerializer.
};

static_assert(sizeof(QListInternal) == 16);

// What components store for a list field.
template <class T> struct QListPtr {
    Quantum::Ptr Ptr;

    QListPtr() = default;
    // 0x65924c8 (instance for AISpawned)
    explicit QListPtr(Quantum::Ptr ptr) : Ptr(ptr) {}

    // 0x65924d0
    int GetHashCode() const { return Ptr.GetHashCode(); }
    // 0x65925c8 (Equals(QListPtr)); 0x65924d8 is the boxed Equals(object).
    bool Equals(const QListPtr& other) const { return Ptr == other.Ptr; }
    friend bool operator==(const QListPtr& a, const QListPtr& b) { return a.Equals(b); }
};

// Iterator over `count` elements of a buffer starting at `offset`, wrapping around the
// end of the buffer (so it also serves ring buffers). Original: QList`1.Iterator.
template <class T> struct QListIterator {
    T* _current = nullptr;
    int _index = -1;
    int _count = 0;
    int _offset = 0;
    QBuffer _buffer;
    Allocator::IFrameHeap* _heap = nullptr;

    QListIterator() = default;
    // 0x5bc57e0 (instance for AISpawned)
    QListIterator(Allocator::IFrameHeap* heap, QBuffer buffer, int offset, int count)
        : _current(nullptr), _index(-1), _count(count), _offset(offset), _buffer(buffer),
          _heap(heap) {}

    // 0x5bc5800
    bool MoveNext() {
        if (++_index >= _count) {
            _current = nullptr;
            return false;
        }
        const int slot = (_index + _offset) % _buffer.Length;
        _current = ElementAt<T>(_heap->Void(_buffer.Ptr), slot * _buffer.Stride);
        return true;
    }

    // 0x5bc5900
    void Reset() {
        _current = nullptr;
        _index = -1;
    }

    // 0x5bc5910 - Throws std::logic_error when not positioned on an element. 0x5bc5954 is
    // the boxed IEnumerator.Current.
    T get_Current() const {
        if (_current == nullptr) {
            throw std::logic_error("QList iterator: no current element");
        }
        return *_current;
    }

    // 0x5bc59ec
    void Dispose() {}
};

template <class T> class QList {
  public:
    using Iterator = QListIterator<T>;

    QList() = default;
    // 0x6596484 (instance for AISpawned)
    QList(Allocator::IFrameHeap* heap, QListInternal* list) : _list(list), _heap(heap) {}

    // 0x4aa5e68 - A new, empty list with room for `capacity` elements.
    static QList Allocate(Allocator::IFrameHeap* heap, int capacity) {
        return QList(heap, QListInternal::Allocate<T>(*heap, capacity));
    }

    // 0x65963c4
    int get_Count() const { return List()->_count; }
    // 0x65963dc / 0x6596424 - Same as Get / Set.
    T get_Item(int index) const { return Get(index); }
    void set_Item(int index, const T& value) { Set(index, value); }

    // 0x65964ac - Releases the storage and the record; the view becomes unallocated.
    void Free() {
        QListInternal* list = List();
        QBuffer::Free(*_heap, &list->_items);
        _heap->Free(static_cast<void*>(list));
        _heap = nullptr;
        _list = nullptr;
    }

    // 0x65965d8 - Forgets the elements, keeping the capacity.
    void Clear() { List()->_count = 0; }

    // 0x6596624 - Appends, doubling the capacity (at least 2) when full.
    void Add(const T& item) {
        QListInternal* list = List();
        if (list->_count >= list->_items.Length) {
            SetCapacity(list->_items.Length * 2 > 2 ? list->_items.Length * 2 : 2);
        }
        *Slot(list, list->_count) = item;
        ++list->_count;
    }

    // 0x659682c - Resizes the storage, truncating the list if it shrinks below the count;
    // capacity <= 0 releases the storage and empties the list.
    void SetCapacity(int capacity) {
        QListInternal* list = List();
        if (list->_items.Length == capacity) {
            return;
        }
        if (capacity > 0) {
            QBuffer items;
            QBuffer::Init(*_heap, &items, capacity, list->_items.Stride);
            if (list->_count > 0) {
                if (list->_count > capacity) {
                    list->_count = capacity;
                }
                QBuffer::Copy(*_heap, list->_items, 0, items, 0, list->_count);
            }
            if (list->_items.Ptr.Offset != 0) {
                QBuffer::Free(*_heap, &list->_items);
            }
            list->_items = items;
        } else {
            list->_count = 0;
            if (list->_items.Ptr.Offset != 0) {
                QBuffer::Free(*_heap, &list->_items);
            }
        }
    }

    // 0x6596a64 - Address of element `index` (std::out_of_range unless 0 <= index < count).
    T* GetPointer(int index) const {
        QListInternal* list = List();
        CheckIndex(list, index);
        return Slot(list, index);
    }

    // 0x6596b8c - Removes element `index`, shifting the rest down.
    void RemoveAt(int index) {
        QListInternal* list = List();
        CheckIndex(list, index);
        --list->_count;
        if (list->_count - index > 0) {
            QBuffer::Move(*_heap, list->_items, index + 1, index, list->_count - index);
        }
    }

    // 0x6596c84 - Removes element `index` by moving the last element into its place.
    void RemoveAtUnordered(int index) {
        QListInternal* list = List();
        CheckIndex(list, index);
        --list->_count;
        if (list->_count > index) {
            QBuffer::Move(*_heap, list->_items, list->_count, index, 1);
        }
    }

    // 0x6596d7c - First index holding a value equal to `item`, or -1.
    int IndexOf(const T& item) const {
        QListInternal* list = List();
        for (int i = 0; i < list->_count; ++i) {
            if (*Slot(list, i) == item) {
                return i;
            }
        }
        return -1;
    }

    // 0x6596fa8 - Removes the first equal element (order preserved). True if found.
    bool Remove(const T& item) {
        List();
        const int index = IndexOf(item);
        if (index >= 0) {
            RemoveAt(index);
        }
        return index >= 0;
    }

    // 0x6597070
    bool Contains(const T& item) const { return IndexOf(item) >= 0; }

    // 0x659727c
    Iterator GetIterator() const {
        QListInternal* list = List();
        return Iterator(_heap, list->_items, 0, list->_count);
    }
    // 0x659758c; 0x6597608 and 0x65976ac are the boxed IEnumerable forms.
    Iterator GetEnumerator() const { return GetIterator(); }

    // 0x6597324
    void Set(int index, const T& item) {
        QListInternal* list = List();
        CheckIndex(list, index);
        *Slot(list, index) = item;
    }

    // 0x6597460
    T Get(int index) const {
        QListInternal* list = List();
        CheckIndex(list, index);
        return *Slot(list, index);
    }

    // 0x6597750 - The storable reference (null Ptr for an unallocated view).
    operator QListPtr<T>() const {
        if (_heap == nullptr || _list == nullptr) {
            return QListPtr<T>{};
        }
        return QListPtr<T>{_heap->Void(static_cast<const void*>(_list))};
    }

    QListInternal* _list = nullptr;
    Allocator::IFrameHeap* _heap = nullptr;

  private:
    QListInternal* List() const {
        if (_heap == nullptr || _list == nullptr) {
            throw std::logic_error("QList: not allocated or already freed");
        }
        return _list;
    }
    T* Slot(QListInternal* list, int index) const {
        return ElementAt<T>(_heap->Void(list->_items.Ptr), list->_items.Stride * index);
    }
    static void CheckIndex(const QListInternal* list, int index) {
        if (static_cast<unsigned>(index) >= static_cast<unsigned>(list->_count)) {
            throw std::out_of_range("QList: index out of range");
        }
    }
};

// TODO(0x51163bc, Quantum.Collections.QList.Serialize<T>): forwards to
// QListInternal.Serialize with sizeof(T); needs FrameSerializer.

} // namespace Quantum::Collections
