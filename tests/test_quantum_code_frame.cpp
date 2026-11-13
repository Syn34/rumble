#include "test_framework.h"

#include "photon_deterministic/deterministic_platform_info.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_code/constants.h"
#include "quantum_code/frame.h"
#include "quantum_code/input_provider.h"
#include "quantum_core/frame_context.h"

#include <memory>
#include <vector>

using namespace Quantum;
using Photon::Deterministic::FP;
using Photon::Deterministic::SystemAllocator;
using Quantum::Allocator::Heap;
using Quantum::Core::FrameContext;

namespace {

// A system that listens to one signal and to Health being added.
class ListenerSystem final : public SystemBase,
                             public ISignalActorStatus_OnChanged,
                             public ISignalOnComponentAdded<Health> {
  public:
    void ActorStatus_OnChanged(Frame*, EntityRef actorEntity) override {
        ++statusChanges;
        lastEntity = actorEntity;
    }
    void OnAdded(Frame*, EntityRef, Health* component) override {
        ++healthAdded;
        lastHealth = component;
    }
    void Schedule(Frame*) override {}
    void OnEnabled(Frame*) override { ++enabled; }
    void OnDisabled(Frame*) override { ++disabled; }

    int statusChanges = 0;
    int healthAdded = 0;
    int enabled = 0;
    int disabled = 0;
    EntityRef lastEntity;
    Health* lastHealth = nullptr;
};

struct GameSimulation {
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<FrameContext> context;
    Photon::Deterministic::DeterministicSessionConfig session;
    RuntimeConfig runtime;
    SimulationConfig simulation;
    ListenerSystem listener;
    std::unique_ptr<Frame> frame;

    GameSimulation() {
        ComponentTypeId<>::ResetForTests();
        platform.Allocator = &allocator;
        FrameContext::Args args;
        args.PlatformInfo = &platform;
        args.HeapConfig = Heap::Config(12, 256, 4);
        context = std::make_unique<FrameContext>(args);
        session.PlayerCount = 4;
        simulation.Entities.DefaultEntityCapacity = 64;
        simulation.Entities.DefaultComponentBlockCapacity = 8;
        listener.set_RuntimeIndex(0);
        frame = std::make_unique<Frame>(context.get(), std::vector<SystemBase*>{&listener},
                                        std::vector<SystemBase*>{&listener}, &session, &runtime,
                                        &simulation, FP::FromRaw(1092));
        frame->get_Global()->Systems.Set(0);
    }

    ~GameSimulation() {
        frame->Free();
        frame.reset();
        context->Dispose();
        // Leave the registry clean for tests that register their own component types.
        ComponentTypeId<>::ResetForTests();
    }
};

} // namespace

TEST_CASE("quantum.code: component types register in the original order") {
    GameSimulation sim;
    // Index 0 is reserved and 19 framework components come first.
    CHECK(ComponentTypeId<AIBlackboardComponent>::Id == 20);
    CHECK(ComponentTypeId<ActorManager>::Id == 30);
    CHECK(ComponentTypeId<Health>::Id == 123);
    CHECK(ComponentTypeId<>::Flags[static_cast<std::size_t>(ComponentTypeId<ActorManager>::Id)] ==
          ComponentFlags::Singleton);
    CHECK(ComponentTypeId<>::GetComponentSize(ComponentTypeId<Health>::Id) == Health::SIZE);
    CHECK(ComponentTypeId<>::GetComponentName(ComponentTypeId<Health>::Id) == "Health");
    CHECK(ComponentTypeId<>::GetComponentIndex("EntityInfo") == ComponentTypeId<Code::EntityInfo>::Id);
}

TEST_CASE("quantum.code: frame globals, delta time and player input") {
    GameSimulation sim;
    Frame& f = *sim.frame;
    CHECK(f.get_DeltaTime().RawValue() == 1092);
    CHECK(f.get_PlayerCount() == 4);
    CHECK(f.get_CommitCommandsMode() == CommitCommandsModes::OnFrameSimulationBegin);
    CHECK(f.get_FrameMetaData() == &f.get_Global()->FrameMetaData);

    Input input{};
    input.Jump = Button::FromBool(true);
    input.EncodedDirection = 46;
    f.Number = 10;
    f.SetPlayerInput(2, input);
    Input* stored = f.GetPlayerInput(2);
    CHECK(stored->Jump.IsDown());
    CHECK(stored->Jump.WasPressed());
    CHECK(stored->EncodedDirection == 46);
    CHECK(!stored->Fire.IsDown());
    bool threw = false;
    try {
        f.GetPlayerInput(Input::MAX_COUNT);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("quantum.code: components allocate and free their collections") {
    GameSimulation sim;
    Frame& f = *sim.frame;
    const EntityRef entity = f.Create();
    Health* health = nullptr;
    CHECK(f.Add<Health>(entity, health) == AddResult::ComponentAdded);
    CHECK(health != nullptr);
    // OnAdded allocated the overhealth list; the Health signal reached the system.
    CHECK(health->ActiveOverhealthsPtr != Ptr::Null());
    CHECK(f.ResolveList(health->get_ActiveOverhealths()).get_Count() == 0);
    CHECK(sim.listener.healthAdded == 1);
    CHECK(sim.listener.lastHealth == health);

    ActorManager* manager = f.GetOrAddSingletonPointer<ActorManager>();
    CHECK(manager->respawnTimersPtr != Ptr::Null());

    CHECK(f.Remove<Health>(entity));
    f.CommitAllCommands();
    CHECK(!f.Has<Health>(entity));
}

TEST_CASE("quantum.code: events are pooled, stamped and listed") {
    GameSimulation sim;
    Frame& f = *sim.frame;
    f.Number = 7;
    const EntityRef entity = f.Create();
    EventActorIcedChange* ev = f.Events.ActorIcedChange(entity, QBoolean(true));
    CHECK(ev != nullptr);
    CHECK(ev->Id == EventActorIcedChange::ID);
    CHECK(ev->Tick == 7);
    CHECK(ev->entity == entity);
    CHECK(static_cast<bool>(ev->iced));
    CHECK(f.get_Context()->get_Events()->get_Count() == 1);

    // Synced events are only raised on verified frames.
    f.Verified = false;
    CHECK(f.Events.BlastBallGame_StateChanged(BlastBallGameState{}) == nullptr);
    f.Verified = true;
    CHECK(f.Events.BlastBallGame_StateChanged(BlastBallGameState{}) != nullptr);
    CHECK(f.get_Context()->get_Events()->get_Count() == 2);

    CHECK(FrameEvents::GetParentEventID(EventKillEvent::ID) == FrameEvents::EventBaseEntityEvent_ID);
    CHECK(FrameEvents::GetParentEventID(EventActorIcedChange::ID) == -1);
    while (f.get_Context()->get_Events()->get_Count() > 0) {
        f.get_Context()->ReleaseEvent(f.get_Context()->get_Events()->PopHead());
    }
}

TEST_CASE("quantum.code: signals reach enabled systems only") {
    GameSimulation sim;
    Frame& f = *sim.frame;
    const EntityRef entity = f.Create();
    f.Signals.ActorStatus_OnChanged(entity);
    CHECK(sim.listener.statusChanges == 1);
    CHECK(sim.listener.lastEntity == entity);

    f.SystemDisable<ListenerSystem>();
    CHECK(sim.listener.disabled == 1);
    CHECK(!f.SystemIsEnabledSelf(&sim.listener));
    f.Signals.ActorStatus_OnChanged(entity);
    CHECK(sim.listener.statusChanges == 1);

    f.SystemEnable<ListenerSystem>();
    CHECK(sim.listener.enabled == 1);
    CHECK(f.SystemIsEnabledInHierarchy(&sim.listener));
}

TEST_CASE("quantum.code: hashes match the original") {
    // Expected values were produced by the original code for the same bytes.
    Health health{};
    CHECK(static_cast<std::uint32_t>(health.GetHashCode()) == 2892692541u);
    health.regenFrameReference = 7;
    health.CurrentHP = FP::FromRaw(3 << 16);
    CHECK(static_cast<std::uint32_t>(health.GetHashCode()) == 856161348u);
    AssetRefHealthSpec spec{AssetGuid(5)};
    CHECK(spec.GetHashCode() == 20434);
    Input input{};
    CHECK(static_cast<std::uint32_t>(input.GetHashCode()) == 4277930415u);
    Button button{5, 7, 9};
    CHECK(button.GetHashCode() == 511478);
}

TEST_CASE("quantum.code: compressed input directions") {
    Input input{};
    input.set_CameraOrbitOffset(FP::FromRaw(8 << 16));
    CHECK(input.EncodedCameraOrbitOffset == 127);
    input.EncodedCameraOrbitOffset = 255;
    CHECK(input.get_CameraOrbitOffset().RawValue() == (16 << 16));
    EncodedDirection3 direction{};
    direction.set_Pitch(FP::FromRaw(120 << 16)); // clamped to 90
    CHECK(direction.encodedPitch == 8100);
    direction.set_Yaw(FP::FromRaw(190 << 16)); // wraps to 10 degrees
    CHECK(direction.encodedYaw == 450);
}

TEST_CASE("quantum.code: runtime config round trip and queries") {
    RuntimeConfig config;
    config.Seed = 1234;
    config.modificator = 3;
    config.FTUEStepId = 4;
    config.sequenceId = std::string("abc");
    config.BotsSettings.NumBotLoadoutPresets = 2;
    config.BotsSettings.BotLoadoutPresets[4] = 9;
    config.BotsSettings.BotLoadoutPresets[8] = 11;
    Photon::Deterministic::BitStream writer(2048);
    writer.SetWriting(true);
    config.Serialize(writer);
    std::vector<std::uint8_t> bytes = writer.ToArray();
    RuntimeConfig copy = RuntimeConfig::FromByteArray(bytes);
    CHECK(copy.Seed == 1234);
    CHECK(copy.get_IsTraining());
    CHECK(!copy.get_IsRealGame());
    CHECK(copy.get_IsInFTUE());
    CHECK(copy.sequenceId == std::optional<std::string>("abc"));
    BotLoadoutPreset preset;
    CHECK(copy.TryGetBotLoadoutPreset(1, preset));
    CHECK(preset.character == 9 && preset.weapon3 == 11);
    CHECK(!copy.TryGetBotLoadoutPreset(2, preset));
    CHECK(Constants::BUMPER_PUSH_FORCE_MULTIPLIER().RawValue() == 0x2999a);
}

TEST_CASE("quantum.code: input provider stores confirmed input") {
    InputProvider provider(2, 60, 4, 0);
    CHECK(provider.get_MaxFrame() == 64);
    CHECK(!provider.CanSimulate(60));
    auto data = std::make_shared<std::vector<std::uint8_t>>(std::vector<std::uint8_t>{1, 2, 3});
    for (int player = 0; player < 2; ++player) {
        provider.OnInputConfirmed(nullptr, Photon::Deterministic::DeterministicFrameInputTemp::Verified(
                                               70, player, nullptr, data, 3,
                                               Photon::Deterministic::DeterministicInputFlags::None));
    }
    CHECK(provider.get_MaxFrame() == 60 + 20); // grew to twice the needed index
    CHECK(provider.CanSimulate(70));
    CHECK(provider.GetInput(70, 1).DataLength == 3);
    CHECK(provider.GetInput(500, 0).DataLength == 0);
}
