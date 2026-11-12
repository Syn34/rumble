#include "test_framework.h"

#include "photon_deterministic/system_allocator.h"
#include "quantum_core/frame_heap.h"
#include "quantum_core/q_dictionary.h"
#include "quantum_core/q_list.h"
#include "quantum_core/q_string.h"
#include "quantum_core/unsafe_buffer.h"

#include <cstdint>
#include <vector>

using Photon::Deterministic::SystemAllocator;
using Quantum::Ptr;
using Quantum::Allocator::FrameHeap;
using Quantum::Allocator::Heap;
using Quantum::Allocator::HeapTrackingMode;
using namespace Quantum::Collections;

namespace {

template <class E, class F> bool Throws(F&& f) {
    try {
        f();
    } catch (const E&) {
        return true;
    }
    return false;
}

struct TestFrame {
    SystemAllocator allocator;
    Heap::Group* group;
    FrameHeap heap;

    TestFrame()
        : group(Heap::Group::Allocate(Heap::Config(12, 256, 1), allocator)),
          heap(group->GetHeap(0),
               FrameHeap::GetAllocationTrackerForMode(HeapTrackingMode::Disabled)) {}
    ~TestFrame() { Heap::Group::Free(group, allocator); }
    int BytesAllocated() { return Heap::GetStats(group->GetHeap(0)).BytesAllocated - 3 * 4096; }
};

struct Pair {
    int a = 0;
    int b = 0;
    int GetHashCode() const { return a * 31 + b; }
    friend bool operator==(const Pair&, const Pair&) = default;
};

enum class Color : std::uint8_t { Red = 1, Green = 2, Blue = 200 };

struct QStr32 {
    std::uint16_t byteCount = 0;
    std::uint8_t bytes[32] = {};
};

template <class K, class V> std::vector<K> Keys(const QDictionary<K, V>& dict) {
    std::vector<K> keys;
    auto it = dict.GetEnumerator();
    while (it.MoveNext()) {
        keys.push_back(it.get_Current().first);
    }
    return keys;
}

} // namespace

TEST_CASE("QCollectionsUtils prime table") {
    CHECK(QCollectionsUtils::GetNextPrime(0) == 3);
    CHECK(QCollectionsUtils::GetNextPrime(3) == 7);
    CHECK(QCollectionsUtils::GetNextPrimeGreaterThanOrEqual(3) == 3);
    CHECK(QCollectionsUtils::GetNextPrimeGreaterThanOrEqual(30) == 53);
    CHECK(QCollectionsUtils::GetNextPrimeGreaterThanOrEqual(std::uint32_t{98317}) == 98317u);
    CHECK(Throws<std::overflow_error>([] { QCollectionsUtils::GetNextPrime(1610612741); }));
}

TEST_CASE("QBuffer operations and hash") {
    TestFrame f;
    QBuffer buffer;
    QBuffer::Init(f.heap, &buffer, 8, 4);
    CHECK(buffer.Length == 8 && buffer.Stride == 4 && buffer.Ptr.Offset != 0);
    auto* values = Resolve<int>(f.heap, buffer.Ptr);
    for (int i = 0; i < 8; ++i) {
        values[i] = i;
    }
    QBuffer::Move(f.heap, buffer, 0, 1, 7); // overlapping shift right
    CHECK(values[0] == 0 && values[1] == 0 && values[7] == 6);
    QBuffer other;
    QBuffer::Init(f.heap, &other, 8, 4);
    QBuffer::Copy(f.heap, buffer, 5, other, 0, 3);
    CHECK(Resolve<int>(f.heap, other.Ptr)[2] == 6);
    QBuffer::Clear(f.heap, &buffer);
    CHECK(values[7] == 0);
    QBuffer::Free(f.heap, &buffer);
    CHECK(buffer.Ptr.Offset == 0 && buffer.Length == 0);

    QBuffer h{1, 2, Ptr{3}};
    CHECK(h.GetHashCode() == ((17 * 31 + 1) * 31 + 2) * 31 + 3);
    CHECK(QBuffer::Element(values, 3, 4) == values + 3);
}

TEST_CASE("QList adds, grows, removes and iterates") {
    TestFrame f;
    auto list = QList<int>::Allocate(&f.heap, 4);
    for (int i = 0; i < 10; ++i) {
        list.Add(i * 10);
    }
    CHECK(list.get_Count() == 10);
    CHECK(list._list->_items.Length == 16);
    CHECK(list.Get(9) == 90);

    list.RemoveAt(0);
    CHECK(list.Get(0) == 10 && list.get_Count() == 9);
    list.RemoveAtUnordered(0);
    CHECK(list.Get(0) == 90 && list.get_Count() == 8);
    CHECK(list.IndexOf(50) == 4);
    CHECK(list.Remove(50) && !list.Contains(50));
    *list.GetPointer(1) = 7;
    list.set_Item(2, 8);
    CHECK(list.get_Item(1) == 7 && list.get_Item(2) == 8);
    CHECK(Throws<std::out_of_range>([&] { list.Get(7); }));
    CHECK(Throws<std::out_of_range>([&] { list.Set(-1, 0); }));

    std::vector<int> seen;
    auto it = list.GetEnumerator();
    while (it.MoveNext()) {
        seen.push_back(it.get_Current());
    }
    CHECK((seen == std::vector<int>{90, 7, 8, 40, 60, 70, 80}));
    CHECK(!it.MoveNext());
    CHECK(Throws<std::logic_error>([&] { it.get_Current(); }));

    list.SetCapacity(3);
    CHECK(list.get_Count() == 3 && list._list->_items.Length == 3);

    // The stored reference resolves to the same list.
    QListPtr<int> ptr = list;
    QList<int> view(&f.heap, f.heap.Void<QListInternal>(ptr.Ptr));
    CHECK(view.Get(2) == 8);

    list.Free();
    CHECK(f.BytesAllocated() == 0);
    CHECK(Throws<std::logic_error>([&] { list.Add(1); }));
    CHECK(Throws<std::invalid_argument>([&] { QList<int>::Allocate(&f.heap, 0); }));
}

TEST_CASE("QListIterator wraps around the buffer") {
    TestFrame f;
    auto list = QList<int>::Allocate(&f.heap, 4);
    for (int i = 0; i < 4; ++i) {
        list.Add(i);
    }
    QListIterator<int> it(&f.heap, list._list->_items, 2, 4);
    std::vector<int> seen;
    while (it.MoveNext()) {
        seen.push_back(it.get_Current());
    }
    CHECK((seen == std::vector<int>{2, 3, 0, 1}));
    it.Reset();
    CHECK(it.MoveNext() && it.get_Current() == 2);
}

TEST_CASE("QDictionary stores, grows, removes and reuses slots in order") {
    TestFrame f;
    auto dict = QDictionary<int, int>::Allocate(&f.heap, 2); // capacity 3
    CHECK(dict._map->_collection.Entries.Length == 3);
    for (int i = 0; i < 100; ++i) {
        dict.Add(i, i * i);
    }
    CHECK(dict.get_Count() == 100);
    CHECK(dict._map->_collection.Entries.Length == 193);
    bool all = true;
    for (int i = 0; i < 100; ++i) {
        all = all && dict.get_Item(i) == i * i;
    }
    CHECK(all);
    CHECK(Throws<std::invalid_argument>([&] { dict.Add(5, 0); }));
    CHECK(Throws<std::out_of_range>([&] { dict.get_Item(1000); }));

    CHECK(dict.Remove(10) && !dict.Remove(10));
    CHECK(!dict.ContainsKey(10) && dict.get_Count() == 99);
    int value = -1;
    CHECK(!dict.TryGetValue(10, value) && value == 0);
    CHECK(dict.TryGetValue(11, value) && value == 121);
    int* pointer = nullptr;
    CHECK(dict.TryGetValuePointer(12, pointer) && *pointer == 144);
    *pointer = 1;
    CHECK(dict.get_Item(12) == 1);
    dict.set_Item(12, 2);
    dict.set_Item(500, 3);
    CHECK(dict.get_Item(12) == 2 && dict.get_Item(500) == 3);
    // Key 500 reused the slot freed by key 10.
    CHECK(Keys(dict)[10] == 500);
    CHECK(!dict.TryAdd(500, 0) && dict.TryAdd(501, 4));

    dict.Clear();
    CHECK(dict.get_Count() == 0 && Keys(dict).empty() && !dict.ContainsKey(1));

    QDictionaryInternal::Free(f.heap, dict._map);
    CHECK(f.BytesAllocated() == 0);
}

TEST_CASE("QDictionary iteration follows slot order after removals") {
    TestFrame f;
    auto dict = QDictionary<Pair, int>::Allocate(&f.heap, 8);
    dict.Add({1, 1}, 1);
    dict.Add({2, 2}, 2);
    dict.Add({3, 3}, 3);
    dict.Remove({2, 2});
    dict.Add({4, 4}, 4);
    auto it = dict.GetEnumerator();
    std::vector<int> values;
    while (it.MoveNext()) {
        values.push_back(it.get_Current().second);
    }
    CHECK((values == std::vector<int>{1, 4, 3}));

    QDictionaryPtr<Pair, int> ptr = dict;
    QDictionary<Pair, int> view(&f.heap, f.heap.Void<QDictionaryInternal>(ptr.Ptr));
    CHECK((view.get_Item({4, 4}) == 4));
    using PairPtr = QDictionaryPtr<Pair, int>;
    CHECK(ptr == PairPtr(ptr.Ptr));
    CHECK(static_cast<PairPtr>(QDictionary<Pair, int>()).Ptr.Offset == 0);
}

TEST_CASE("QEnumDictionary uses the enum value as hash") {
    TestFrame f;
    auto dict = QEnumDictionary<Color, std::uint16_t>::Allocate(&f.heap, 4);
    dict.Add(Color::Blue, 7);
    dict.set_Item(Color::Red, 9);
    CHECK(dict.get_Item(Color::Blue) == 7 && dict.get_Count() == 2);
    CHECK((QEnumDictionary<Color, std::uint16_t>::Hash(Color::Blue) == 200u));
    CHECK(dict.ContainsKey(Color::Red) && !dict.ContainsKey(Color::Green));
    // Byte keys and 2-byte values are padded to 4-byte strides after the 12-byte header.
    CHECK(dict._map->_collection.KeyOffset == 12 && dict._map->_valueOffset == 16);
    CHECK(dict._map->_collection.Entries.Stride == 20);
}

TEST_CASE("QHashCollection as a set") {
    TestFrame f;
    QHashCollection set;
    QHashCollection::Allocate<std::uint64_t>(f.heap, &set, 3);
    CHECK(set.KeyOffset == 16 && set.Entries.Stride == 24); // 8-byte keys align to 8
    for (std::uint64_t k = 0; k < 20; ++k) {
        CHECK(QHashCollection::Add(f.heap, &set, k << 40, GetKeyHash(k << 40)));
    }
    CHECK(!QHashCollection::Add(f.heap, &set, std::uint64_t{5} << 40,
                                GetKeyHash(std::uint64_t{5} << 40)));
    CHECK(set.UsedCount == 20 && set.Entries.Length == 29);
    auto* entry = QHashCollection::Find(f.heap, &set, std::uint64_t{7} << 40,
                                        GetKeyHash(std::uint64_t{7} << 40));
    CHECK(entry != nullptr &&
          QHashCollection::GetKey<std::uint64_t>(&set, entry) == std::uint64_t{7} << 40);
    CHECK(QHashCollection::Remove(f.heap, &set, std::uint64_t{7} << 40,
                                  GetKeyHash(std::uint64_t{7} << 40)));
    CHECK(set.FreeCount == 1 && QHashCollection::GetFreeHead(f.heap, &set) != nullptr);
    CHECK(QHashCollection::GetEntry(f.heap, &set, 7)->State == QHashCollection::EntryState::Free);
    QHashCollection::Free(f.heap, &set);
    CHECK(set.Buckets.Offset == 0 && f.BytesAllocated() == 0);
}

TEST_CASE("Collections::UnsafeBuffer") {
    SystemAllocator allocator;
    UnsafeBuffer a;
    UnsafeBuffer::InitDynamic(allocator, &a, 4, 8);
    CHECK(a.Dynamic == 1 && static_cast<std::uint64_t*>(a.Ptr)[3] == 0);
    static_cast<std::uint64_t*>(a.Ptr)[1] = 42;
    std::uint64_t storage[4] = {};
    UnsafeBuffer b;
    UnsafeBuffer::InitFixed(&b, storage, 4, 8);
    UnsafeBuffer::Copy(a, 1, b, 2, 1);
    CHECK(storage[2] == 42);
    UnsafeBuffer::Clear(&b);
    CHECK(storage[2] == 0);
    CHECK(Throws<std::logic_error>([&] { UnsafeBuffer::Free(allocator, &b); }));
    UnsafeBuffer::Free(allocator, &a);
    CHECK(a.Ptr == nullptr && a.Length == 0);
}

TEST_CASE("PhysicsCommon::UnsafeBuffer grows and iterates both ways") {
    using Quantum::PhysicsCommon::Utils;
    CHECK(Utils::GetNextPowerOfTwo(1) == 1);
    CHECK(Utils::GetNextPowerOfTwo(5) == 8);
    CHECK(Utils::GetNextPowerOfTwo(64) == 64);
    CHECK(Utils::GetNextPowerOfTwo(0) == 0);

    SystemAllocator allocator;
    auto buffer = Quantum::PhysicsCommon::UnsafeBuffer::InitBuffer<std::int32_t>(allocator, 3);
    CHECK(buffer._capacity == 4 && buffer.get_Count() == 0);
    for (int i = 0; i < 10; ++i) {
        int index = -1;
        *buffer.GetNextEntry<std::int32_t>(allocator, index) = i;
        CHECK(index == i);
    }
    CHECK(buffer._capacity == 16 && buffer.get_Count() == 10);

    std::vector<int> forward, backward;
    auto it = buffer.GetIterator<std::int32_t>(false);
    while (it.MoveNext()) {
        forward.push_back(it.get_Current());
    }
    auto rit = buffer.GetIterator<std::int32_t>(true);
    while (rit.MoveNext()) {
        backward.push_back(rit.get_Current());
    }
    CHECK(forward.size() == 10 && forward[9] == 9);
    CHECK(backward.size() == 10 && backward[0] == 9);

    buffer.ResetAndClear(true);
    CHECK(buffer.get_Count() == 0 && buffer.GetBuffer<std::int32_t>()[5] == 0);
    CHECK(buffer.ResetAndEnsureCapacity(allocator, 8) == buffer._buffer);
    buffer.ResetAndEnsureCapacity(allocator, 17);
    CHECK(buffer._capacity == 32);
    buffer.Dispose(allocator);
    CHECK(!buffer.IsInitialized<std::int32_t>() && buffer._stride == 0);
}

TEST_CASE("QString helpers") {
    using Quantum::QString;
    QStr32 s;
    QString::ConstructFrom(u"Ratchet", 32, s);
    CHECK(s.byteCount == 14);
    CHECK(QString::GetString(s) == u"Ratchet");
    CHECK(QString::AreEqual(s, u"Ratchet"));
    CHECK(QString::CompareOrdinal(s, u"Ratchez") == -1);
    CHECK(QString::CompareOrdinal(s, u"Ratch") == 1);
    CHECK(QString::CompareOrdinal(s, u"Ratchets") == -1);
    CHECK(QString::CompareOrdinal(s, u"") == 14);

    QStr32 t;
    QString::ConstructFrom(u"Ratchet", 32, t);
    CHECK(QString::AreEqual(s, t));
    QString::ConstructFrom(u"Clank", 32, t);
    CHECK(!QString::AreEqual(s, t));
    CHECK(QString::CompareOrdinal(t, s.bytes, s.byteCount) == -1);

    QStr32 empty;
    CHECK(QString::CompareOrdinal(empty, u"abc") == -3);
    QString::ConstructFrom(u"", 32, t);
    CHECK(t.byteCount == 0 && t.bytes[0] == 0);

    CHECK(!QString::CanHold(u"0123456789abcdefg", 32));
    CHECK(Throws<std::invalid_argument>(
        [&] { QString::ConstructFrom(u"0123456789abcdefg", 32, t); }));
    CHECK(QString::GetByteCount(u"abc") == 6);
    CHECK(QString::GetCharCount(7) == 3);
    CHECK(QString::CompareOridinalNot4ByteAligned(u"ab", 2, u"abc", 3) == -1);
    CHECK(QString::CompareOridinalNot4ByteAligned(u"abd", 3, u"abc", 3) == 1);
}
