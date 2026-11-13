#include "test_framework.h"

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/crc64.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_core/component_iterators.h"
#include "quantum_core/component_reference.h"
#include "quantum_core/dispatcher_base.h"
#include "quantum_core/events.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_checksumer_bit_stream.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/frame_thread_safe.h"
#include "quantum_core/transform.h"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Quantum;
using Photon::Deterministic::SystemAllocator;
using Quantum::Allocator::Heap;
using Quantum::Core::FrameBase;
using Quantum::Core::FrameContext;

namespace {

template <class E, class F> bool Throws(F&& f) {
    try {
        f();
    } catch (const E&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

// --- test components ---------------------------------------------------------------------

struct TestPosition {
    static constexpr const char* ComponentTypeName = "TestPosition";
    std::int32_t X = 0;
    std::int32_t Y = 0;
    static void Serialize(void* ptr, FrameSerializer* serializer) {
        auto* p = static_cast<TestPosition*>(ptr);
        serializer->Stream().Serialize(&p->X);
        serializer->Stream().Serialize(&p->Y);
    }
};

struct TestVelocity {
    static constexpr const char* ComponentTypeName = "TestVelocity";
    std::int64_t Speed = 0;
    static void Serialize(void* ptr, FrameSerializer* serializer) {
        serializer->Stream().Serialize(&static_cast<TestVelocity*>(ptr)->Speed);
    }
};

struct TestTag {
    static constexpr const char* ComponentTypeName = "TestTag";
    std::int32_t Value = 0;
    static void Serialize(void* ptr, FrameSerializer* serializer) {
        serializer->Stream().Serialize(&static_cast<TestTag*>(ptr)->Value);
    }
};

struct TestGlobal {
    static constexpr const char* ComponentTypeName = "TestGlobal";
    std::int32_t Counter = 0;
    static void Serialize(void* ptr, FrameSerializer* serializer) {
        serializer->Stream().Serialize(&static_cast<TestGlobal*>(ptr)->Counter);
    }
};

int g_positionAdded = 0;
int g_positionRemoved = 0;

void OnPositionAdded(Core::FrameBase*, EntityRef, void*) { ++g_positionAdded; }
void OnPositionRemoved(Core::FrameBase*, EntityRef, void*) { ++g_positionRemoved; }

void RegisterTestComponents() {
    ComponentTypeId<>::Setup([] {
        ComponentTypeId<>::Add<TestPosition>(&TestPosition::Serialize, &OnPositionAdded,
                                             &OnPositionRemoved, ComponentFlags::None);
        ComponentTypeId<>::Add<TestVelocity>(ComponentCallbacks(&TestVelocity::Serialize),
                                             ComponentFlags::None);
        ComponentTypeId<>::Add<TestTag>(ComponentCallbacks(&TestTag::Serialize),
                                        ComponentFlags::None);
        ComponentTypeId<>::Add<TestGlobal>(ComponentCallbacks(&TestGlobal::Serialize),
                                           ComponentFlags::Singleton);
    });
}

// --- a minimal game frame ------------------------------------------------------------------

class TestFrame final : public FrameBase {
  public:
    explicit TestFrame(FrameContext* context, CommitCommandsModes mode)
        : FrameBase(context), _mode(mode) {}

    FrameMetaData* get_FrameMetaData() override { return &_meta; }
    CommitCommandsModes get_CommitCommandsMode() const override { return _mode; }
    Photon::Deterministic::FP get_DeltaTime() const override { return _deltaTime; }
    void set_DeltaTime(Photon::Deterministic::FP value) override { _deltaTime = value; }

    std::string DumpFrame(int) override { return {}; }
    std::uint64_t CalculateChecksum() override {
        FrameChecksumerBitStream stream;
        stream.SetWriting(true);
        FrameSerializer serializer(Photon::Deterministic::DeterministicFrameSerializeMode::Serialize,
                                   this, &stream);
        serializer.Reset();
        stream.SetWriting(true);
        SerializeAll(&serializer);
        return stream.get_Checksum();
    }
    // What the game's frame does: the entity store, then its globals (here only the frame
    // metadata).
    void SerializeAll(FrameSerializer* serializer) {
        FrameBase::Serialize(this, serializer);
        FrameMetaData::Serialize(&_meta, serializer);
    }
    std::vector<std::uint8_t> Serialize(Photon::Deterministic::DeterministicFrameSerializeMode) override {
        return {};
    }
    void Deserialize(std::span<const std::uint8_t>) override {}
    void Copy(const Photon::Deterministic::DeterministicFrame& frame) override {
        FrameBase::Copy(this, static_cast<const FrameBase*>(&static_cast<const TestFrame&>(frame)));
    }

  private:
    FrameMetaData _meta{};
    CommitCommandsModes _mode;
    Photon::Deterministic::FP _deltaTime;
};

struct Simulation {
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<FrameContext> context;

    Simulation() {
        RegisterTestComponents();
        platform.Allocator = &allocator;
        FrameContext::Args args;
        args.PlatformInfo = &platform;
        args.HeapConfig = Heap::Config(12, 256, 4);
        context = std::make_unique<FrameContext>(args);
    }

    std::unique_ptr<TestFrame> MakeFrame(int entityCapacity = 32,
                                         CommitCommandsModes mode =
                                             CommitCommandsModes::OnFrameSimulationBegin) {
        auto frame = std::make_unique<TestFrame>(context.get(), mode);
        Core::EntitiesConfig config;
        config.DefaultEntityCapacity = entityCapacity;
        config.DefaultComponentBlockCapacity = 4;
        config.ComponentTypeConfiguration.push_back({"TestTag", 2});
        FrameBase::Initialize(frame.get(), config);
        return frame;
    }
};

} // namespace

TEST_CASE("CRC64 matches the CRC-64/Jones check value") {
    const char* text = "123456789";
    CHECK(Photon::Deterministic::CRC64::Calculate(~0ull, text, 9) == 0xCAA717168609F281ull);
    // Pieces chain.
    std::uint64_t crc = Photon::Deterministic::CRC64::Calculate(~0ull, text, 4);
    crc = Photon::Deterministic::CRC64::Calculate(crc, text + 4, 5);
    CHECK(crc == 0xCAA717168609F281ull);
    CHECK(Photon::Deterministic::CRC64::Calculate(123, text, 0) == 123);
}

TEST_CASE("EntityRef, ComponentSet and ComponentReference value semantics") {
    EntityRef a{5, static_cast<int>(0x80000003u)};
    CHECK(a.IsValid());
    CHECK(!EntityRef::None().IsValid());
    CHECK(a.Raw() == (0x80000003ull << 32 | 5u));
    CHECK(a.GetHashCode() == static_cast<int>(5u * 59209u + 0x80000003u));
    CHECK(a == (EntityRef{5, static_cast<int>(0x80000003u)}));
    CHECK(a != (EntityRef{5, 3}));

    ComponentSet set;
    CHECK(set.IsEmpty());
    set.Add(3);
    set.Add(200);
    CHECK(set.IsSet(3) && set.IsSet(200) && !set.IsSet(4));
    ComponentSet sub;
    sub.Add(200);
    CHECK(set.IsSupersetOf(sub));
    CHECK(!sub.IsSupersetOf(set));
    set.Remove(3);
    CHECK(!set.IsSet(3));
    // h = fold(b0); h = h*33 ^ fold(bi)
    ComponentSet h;
    h._set[0] = 0x0000000100000002ull;
    h._set[3] = 5;
    const std::uint32_t expected = ((((3u * 33u) ^ 0u) * 33u) ^ 0u) * 33u ^ 5u;
    CHECK(h.GetHashCode() == static_cast<int>(expected));

    ComponentReference r(a, 7);
    CHECK(r.GetHashCode() ==
          static_cast<int>(static_cast<std::uint32_t>(a.GetHashCode()) * 397u ^ 7u));
    CHECK(r == ComponentReference(a, 7));
}

TEST_CASE("ComponentTypeId registers framework components first") {
    RegisterTestComponents();
    CHECK(ComponentTypeId<>::GetComponentType(0) == nullptr);
    CHECK(ComponentTypeId<>::GetComponentSize(0) == -1);
    CHECK(ComponentTypeId<Transform2D>::Id == 1);
    CHECK(ComponentTypeId<Transform3D>::Id == 2);
    CHECK(ComponentTypeId<Transform2DVertical>::Id == 3);
    CHECK(ComponentTypeId<>::GetComponentIndex("MapEntityLink") == 19);
    CHECK(ComponentTypeId<>::GetComponentSize(ComponentTypeId<>::GetComponentIndex("PhysicsBody3D")) == 184);
    CHECK(ComponentTypeId<TestPosition>::Id == 20);
    CHECK(ComponentTypeId<TestGlobal>::Flags == ComponentFlags::Singleton);
    CHECK(ComponentTypeId<>::GetComponentIndex(typeid(TestVelocity)) == 21);
    CHECK(Throws<std::logic_error>([] { ComponentTypeId<>::GetComponentIndex(typeid(int)); }));
    CHECK(Throws<std::logic_error>(
        [] { ComponentTypeId<>::Add<int>(ComponentCallbacks(), ComponentFlags::None); }));
    ComponentSet set = ComponentSet::Create<TestPosition, TestTag>();
    CHECK(set.IsSet(20) && set.IsSet(22) && !set.IsSet(21));
    CHECK(set.ToString() == "{TestPosition, TestTag}");
}

TEST_CASE("Frame: create entities, add components, filter, remove and destroy") {
    Simulation sim;
    auto frame = sim.MakeFrame();
    g_positionAdded = 0;
    g_positionRemoved = 0;

    std::vector<EntityRef> entities;
    for (int i = 0; i < 6; ++i) {
        entities.push_back(frame->Create());
    }
    CHECK(entities[0].Index == 1);
    CHECK(entities[5].Index == 6);
    CHECK(entities[0].Version == static_cast<int>(0x80000001u));
    CHECK(frame->Exists(entities[3]));
    CHECK(!frame->Exists(EntityRef::None()));
    CHECK(frame->EntityCount() == 6);

    for (int i = 0; i < 6; ++i) {
        CHECK(frame->Add(entities[i], TestPosition{i, i * 10}) == AddResult::ComponentAdded);
        if (i % 2 == 0) {
            CHECK(frame->Add(entities[i], TestVelocity{100 + i}) == AddResult::ComponentAdded);
        }
    }
    CHECK(frame->Add<TestPosition>(entities[0]) == AddResult::ComponentAlreadyExists);
    CHECK(frame->Add<TestPosition>(EntityRef{40, 1}) == AddResult::EntityDoesNotExist);
    CHECK(g_positionAdded == 6);
    CHECK(frame->Set(entities[1], TestPosition{7, 70}) == SetResult::ComponentUpdated);
    CHECK(frame->Get<TestPosition>(entities[1]).X == 7);
    CHECK(frame->Has<TestVelocity>(entities[2]));
    CHECK(!frame->Has<TestVelocity>(entities[3]));
    CHECK(frame->ComponentCount<TestPosition>() == 6);

    TestVelocity* velocity = nullptr;
    CHECK(frame->TryGetPointer(entities[4], velocity) && velocity->Speed == 104);
    CHECK(!frame->TryGetPointer(entities[5], velocity) && velocity == nullptr);
    CHECK(Throws<std::logic_error>([&] { frame->GetPointer<TestVelocity>(entities[5]); }));

    // Filter: Position + Velocity, in buffer order.
    {
        auto filter = frame->Filter<TestPosition, TestVelocity>();
        EntityRef e;
        TestPosition* p = nullptr;
        TestVelocity* v = nullptr;
        std::vector<int> seen;
        while (filter.NextUnsafe(e, p, v)) {
            seen.push_back(e.Index);
            CHECK(p->X + 100 == v->Speed);
        }
        CHECK((seen == std::vector<int>{1, 3, 5}));
        CHECK(e == EntityRef::None() && p == nullptr && v == nullptr);
    }
    // "without" and "any" sets.
    {
        auto filter = frame->Filter<TestPosition>(ComponentSet::Create<TestVelocity>());
        EntityRef e;
        TestPosition p;
        std::vector<int> seen;
        while (filter.Next(e, p)) {
            seen.push_back(p.X);
        }
        CHECK((seen == std::vector<int>{7, 3, 5}));
    }

    // Remove a component: hidden at once, data dropped on commit.
    CHECK(frame->Remove<TestPosition>(entities[2]));
    CHECK(!frame->Remove<TestPosition>(entities[2]));
    CHECK(g_positionRemoved == 1);
    CHECK(!frame->Has<TestPosition>(entities[2]));
    CHECK(frame->ComponentCount<TestPosition>() == 5);
    CHECK(frame->ComponentCount<TestPosition>(true) == 6);
    {
        // The component iterator skips the pending removal.
        std::vector<int> seen;
        for (auto pair : frame->GetComponentIterator<TestPosition>().Range()) {
            seen.push_back(pair.Entity.Index);
        }
        CHECK((seen == std::vector<int>{1, 2, 4, 5, 6}));
    }
    frame->OnFrameSimulateBegin();
    CHECK(frame->ComponentCount<TestPosition>(true) == 5);
    {
        // The last instance (entity 6) moved into the freed slot.
        std::vector<int> seen;
        for (auto pair : frame->GetComponentBlockIterator<TestPosition>().Range()) {
            seen.push_back(pair.Entity.Index);
            CHECK(pair.Component->X == frame->Get<TestPosition>(pair.Entity).X);
        }
        CHECK((seen == std::vector<int>{1, 2, 6, 4, 5}));
    }

    // Destroy: the handle dies at once, the slot is reused after the commit.
    CHECK(frame->Destroy(entities[0]));
    CHECK(!frame->Destroy(entities[0]));
    CHECK(!frame->Exists(entities[0]));
    CHECK(g_positionRemoved == 2);
    CHECK(frame->IsCulled(entities[0]));
    frame->CommitAllCommands();
    CHECK(frame->ComponentCount<TestPosition>(true) == 4);
    CHECK(frame->ComponentCount<TestVelocity>(true) == 2);
    CHECK(frame->EntityCount() == 5);
    const EntityRef reused = frame->Create();
    CHECK(reused.Index == entities[0].Index);
    // Destroy bumped the version once (dropping the live bit), Create once more.
    CHECK(reused.Version == static_cast<int>(0x80000003u));
    CHECK(!frame->Has<TestPosition>(reused));

    // Iterating while removing throws once slots move.
    {
        auto it = frame->GetComponentIterator<TestPosition>();
        auto enumerator = it.GetEnumerator();
        CHECK(enumerator.MoveNext());
        frame->Remove<TestPosition>(entities[1]);
        frame->CommitRemoves();
        CHECK(Throws<std::logic_error>([&] { enumerator.MoveNext(); }));
    }

    frame->Free();
}

TEST_CASE("Frame: singletons, culling, capacity growth and per-type block size") {
    Simulation sim;
    auto frame = sim.MakeFrame();

    TestGlobal* global = nullptr;
    CHECK(!frame->TryGetPointerSingleton(global));
    CHECK(Throws<std::logic_error>([&] { frame->GetPointerSingleton<TestGlobal>(); }));
    TestGlobal* created = frame->GetOrAddSingletonPointer<TestGlobal>();
    created->Counter = 9;
    CHECK(frame->GetSingleton<TestGlobal>().Counter == 9);
    EntityRef holder;
    CHECK(frame->TryGetSingletonEntityRef<TestGlobal>(holder) && frame->Exists(holder));
    const EntityRef other = frame->Create();
    CHECK(Throws<std::logic_error>([&] { frame->Add<TestGlobal>(other); }));

    // Culling bits.
    frame->SetCulled(holder, true);
    CHECK(frame->IsCulled(holder));
    frame->SetCullable(holder, false);
    CHECK(!frame->IsCulled(holder));
    frame->ClearCulledState();

    // More entities than the initial capacity of 32.
    std::vector<EntityRef> many;
    for (int i = 0; i < 80; ++i) {
        many.push_back(frame->Create());
        frame->Add(many.back(), TestTag{i});
    }
    CHECK(frame->Capacity() == 128);
    for (int i = 0; i < 80; ++i) {
        CHECK(frame->Get<TestTag>(many[i]).Value == i);
    }
    // TestTag uses 2 instances per block (configuration by name): slots 1..80 (slot 0 is
    // reserved) need 41 blocks.
    CHECK(frame->GetDataBuffer<TestTag>()->_blockCapacity == 2);
    CHECK(frame->GetDataBuffer<TestTag>()->get_BlockListCount() == 41);
    CHECK(frame->GetDataBuffer<TestPosition>()->_blockCapacity == 4);
    frame->Free();
}

TEST_CASE("Frame: copy, serialize round trip and checksum") {
    Simulation sim;
    auto source = sim.MakeFrame();
    for (int i = 0; i < 10; ++i) {
        const EntityRef e = source->Create();
        source->Add(e, TestPosition{i, -i});
        if (i % 3 == 0) {
            source->Add(e, TestTag{i * 2});
        }
    }
    source->Destroy(EntityRef{4, static_cast<int>(0x80000001u)});
    source->CommitAllCommands();
    source->GetOrAddSingletonPointer<TestGlobal>()->Counter = 77;
    // Deferred commands still pending when the frame is serialised.
    source->Destroy(EntityRef{2, static_cast<int>(0x80000001u)});
    source->Remove<TestTag>(EntityRef{7, static_cast<int>(0x80000001u)});

    auto copy = sim.MakeFrame(64);
    FrameBase::Copy(copy.get(), source.get());
    CHECK(copy->Capacity() == 32);
    CHECK(copy->EntityCount() == source->EntityCount());
    CHECK(copy->Get<TestPosition>(EntityRef{7, static_cast<int>(0x80000001u)}).Y == -6);
    CHECK(copy->GetSingleton<TestGlobal>().Counter == 77);

    // Serialize into a bit stream and read it into a third frame.
    std::vector<std::uint8_t> bytes(64 * 1024);
    {
        FrameSerializer writer(Photon::Deterministic::DeterministicFrameSerializeMode::Serialize,
                               source.get(), std::span<std::uint8_t>(bytes));
        writer.Reset();
        writer.SetWriting(true);
        source->SerializeAll(&writer);
    }
    auto restored = sim.MakeFrame();
    {
        FrameSerializer reader(Photon::Deterministic::DeterministicFrameSerializeMode::Serialize,
                               restored.get(), std::span<std::uint8_t>(bytes));
        // (Reset would also clear the stream's bytes; only the key mapping is reset.)
        reader.Mapping().Reset();
        reader.SetReading(true);
        restored->SerializeAll(&reader);
        reader.VerifyNoUnresolvedPointers();
    }
    CHECK(restored->EntityCount() == source->EntityCount());
    CHECK(restored->Get<TestTag>(EntityRef{10, static_cast<int>(0x80000001u)}).Value == 18);
    CHECK(restored->GetSingleton<TestGlobal>().Counter == 77);
    CHECK(restored->CalculateChecksum() == source->CalculateChecksum());

    // The pending commands came along and commit the same way.
    CHECK(restored->ComponentCount<TestTag>(true) == source->ComponentCount<TestTag>(true));
    restored->CommitAllCommands();
    source->CommitAllCommands();
    CHECK(restored->ComponentCount<TestTag>(true) == 2);
    CHECK(restored->EntityCount() == source->EntityCount());
    CHECK(restored->CalculateChecksum() == source->CalculateChecksum());

    // A change shows in the checksum.
    restored->GetPointer<TestPosition>(EntityRef{1, static_cast<int>(0x80000001u)})->X = 1000;
    CHECK(restored->CalculateChecksum() != source->CalculateChecksum());

    // Thread-safe view.
    FrameThreadSafe view(source.get(), 1);
    CHECK(!view.get_IsMainThread());
    CHECK(FrameThreadSafe::FromFrame(source.get()).get_IsMainThread());
    CHECK(view.Has<TestPosition>(EntityRef{1, static_cast<int>(0x80000001u)}));

    source->Free();
    copy->Free();
    restored->Free();
}

TEST_CASE("FrameSerializer pointer keys") {
    FrameSerializer::PtrKeyMapping mapping;
    mapping.Reset();
    CHECK(mapping.IsKeyResolved(FrameSerializer::PtrKey(-1)));
    const FrameSerializer::PtrKey key = mapping.NextKey();
    CHECK(key.Key == 1);
    Ptr waiting{-5};
    mapping.AddUnresolved(key, &waiting);
    CHECK(Throws<std::runtime_error>([&] { mapping.VerifyAllKeysResolved(); }));
    mapping.AddMapping(key, Ptr{4096});
    CHECK(mapping.ResolveKey(key).Offset == 4096);
    CHECK(waiting.Offset == 4096);
    CHECK(Throws<std::logic_error>([&] { mapping.ResolveKey(key); }));
    mapping.VerifyAllKeysResolved();
    CHECK(Throws<std::invalid_argument>([&] { mapping.AddMapping(FrameSerializer::PtrKey(9), Ptr{4096}); }));
}

TEST_CASE("Checksum stream feeds raw bytes") {
    FrameChecksumerBitStream stream;
    std::int32_t value = 0x01020304;
    stream.Serialize(&value);
    const std::uint8_t raw[] = {4, 3, 2, 1};
    CHECK(stream.get_Checksum() == Photon::Deterministic::CRC64::Calculate(0, raw, 4));
    CHECK(stream.WriteBool(true));
    const std::uint8_t one = 1;
    CHECK(stream.get_Checksum() ==
          Photon::Deterministic::CRC64::Calculate(Photon::Deterministic::CRC64::Calculate(0, raw, 4), &one, 1));
    CHECK(Throws<std::logic_error>([&] { stream.ReadInt(); }));
    CHECK(stream.BytesRequired() == 8);
    stream.Reset();
    CHECK(stream.get_Checksum() == 0);
}

TEST_CASE("EventList keeps raising order; events are pooled per id") {
    struct TestEvent : EventBase {
        TestEvent() : EventBase(3, EventFlags::Synced) {}
        int Released = 0;
        void OnReleased() override { ++Released; }
    };
    Simulation sim;
    EventList list;
    TestEvent* a = sim.context->AcquireEvent<TestEvent>(3);
    TestEvent* b = sim.context->AcquireEvent<TestEvent>(3);
    TestEvent* c = sim.context->AcquireEvent<TestEvent>(3);
    list.AddLast(a);
    list.AddLast(b);
    list.AddLast(c);
    CHECK(list.get_Count() == 3);
    CHECK(list.Head() == a && a->Next == b && b->Next == c && c->Next == a && a->Prev == c);
    list.Remove(b);
    CHECK(a->Next == c && b->List == nullptr);
    CHECK(list.PopHead() == a);
    CHECK(list.PopHead() == c);
    CHECK(list.get_Count() == 0 && list.Head() == nullptr);
    CHECK(a->get_Synced());

    sim.context->ReleaseEvent(b);
    CHECK(b->Released == 1);
    CHECK(sim.context->AcquireEvent<TestEvent>(3) == b);
    sim.context->ReleaseEvent(a);
    sim.context->ReleaseEvent(b);
    sim.context->ReleaseEvent(c);

    EventKey k1(10, 3, 99);
    CHECK(k1 == EventKey(10, 3, 99));
    CHECK(k1.GetHashCode() == static_cast<int>((((10u * 33u) ^ 3u) * 33u) ^ 99u));
}

TEST_CASE("FrameContext prediction area") {
    Simulation sim;
    auto frame = sim.MakeFrame();
    using Photon::Deterministic::FP;
    using Photon::Deterministic::FPVector3;
    // Without culling everything is inside.
    CHECK(sim.context->InPredictionArea(frame.get(), FPVector3{FP::FromInt(1000), FP::FromInt(0), FP::FromInt(0)}));
    frame->Free();

    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    platform.Allocator = &allocator;
    FrameContext::Args args;
    args.PlatformInfo = &platform;
    args.HeapConfig = Heap::Config(12, 64, 1);
    args.UseCullingArea = true;
    FrameContext context(args);
    TestFrame predicted(&context, CommitCommandsModes::OnFrameSimulationBegin);
    context.SetPredictionArea(FPVector3{}, FP::FromInt(10));
    CHECK(context.InPredictionArea(&predicted, FPVector3{FP::FromInt(3), FP::FromInt(4), FP::FromInt(0)}));
    CHECK(!context.InPredictionArea(&predicted, FPVector3{FP::FromInt(6), FP::FromInt(8), FP::FromInt(0)}));
    predicted.Verified = true;
    CHECK(context.InPredictionArea(&predicted, FPVector3{FP::FromInt(60), FP::FromInt(8), FP::FromInt(0)}));
    context.ReleaseHeap(predicted.HeapPtr());

    CHECK(DispatcherBase::CheckFlag(0b1011, 0b0011));
    CHECK(!DispatcherBase::CheckFlag(0b1011, 0b0100));
}
