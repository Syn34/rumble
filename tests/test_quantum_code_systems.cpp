#include "test_framework.h"

#include "math_tables.h"
#include "photon_deterministic/deterministic_platform_info.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_code/frame.h"
#include "quantum_code/systems/all_systems.h"
#include "quantum_code/systems/enablers.h"
#include "quantum_code/systems/system_setup.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/damageable_systems.h"
#include "quantum_code/systems/health_systems.h"
#include "quantum_code/systems/game_mode_systems.h"
#include "quantum_code/systems/killable_systems.h"
#include "quantum_code/systems/reward_systems.h"
#include "quantum_code/systems/libs.h"
#include "quantum_code/systems/transform_systems.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/resource_manager.h"

#include <memory>
#include <vector>

using namespace Quantum;
using Photon::Deterministic::FP;
using Photon::Deterministic::SystemAllocator;
using Quantum::Allocator::Heap;
using Quantum::Core::FrameContext;

namespace {

// A frame running the given root systems.
struct SystemsSimulation {
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<FrameContext> context;
    Photon::Deterministic::DeterministicSessionConfig session;
    RuntimeConfig runtime;
    SimulationConfig simulation;
    SystemList roots;
    std::vector<SystemBase*> all;
    std::unique_ptr<ResourceManagerStatic> resources;
    std::unique_ptr<Frame> frame;

    explicit SystemsSimulation(SystemList systems, const std::vector<std::shared_ptr<AssetObject>>& assets = {})
        : roots(std::move(systems)) {
        ComponentTypeId<>::ResetForTests();
        platform.Allocator = &allocator;
        FrameContext::Args args;
        resources = std::make_unique<ResourceManagerStatic>(assets, nullptr);
        args.AssetDatabase = resources->CreateAssetDatabase();
        args.PlatformInfo = &platform;
        args.HeapConfig = Heap::Config(12, 256, 4);
        context = std::make_unique<FrameContext>(args);
        session.PlayerCount = 4;
        simulation.Entities.DefaultEntityCapacity = 64;
        simulation.Entities.DefaultComponentBlockCapacity = 8;
        all = FlattenSystems(roots);
        std::vector<SystemBase*> rootPointers;
        for (const auto& root : roots) {
            rootPointers.push_back(root.get());
        }
        // 1/60 s
        frame = std::make_unique<Frame>(context.get(), all, rootPointers, &session, &runtime, &simulation,
                                        FP::FromRaw(1092));
    }

    ~SystemsSimulation() {
        frame->Free();
        frame.reset();
        context->Dispose();
        ComponentTypeId<>::ResetForTests();
    }

    template <class T> T* Find() {
        for (SystemBase* system : all) {
            if (auto* typed = dynamic_cast<T*>(system)) {
                return typed;
            }
        }
        return nullptr;
    }
};

template <class... TSystems> SystemList MakeSystems() {
    SystemList list;
    (list.push_back(std::make_unique<TSystems>()), ...);
    return list;
}

} // namespace

TEST_CASE("quantum.code systems: the system list has the original's shape") {
    SystemList roots = SystemSetup::CreateSystems(nullptr, nullptr);
    CHECK(roots.size() == 16u + 154u + 16u);
    std::vector<SystemBase*> all = FlattenSystems(roots);
    // Groups contribute their children: 11 + 12 + 13 + 5 + 2 + 22.
    CHECK(all.size() == roots.size() + 65u);
    CHECK(dynamic_cast<KillZoneSystemEnabler*>(all[0]) != nullptr);
    CHECK(dynamic_cast<Core::CullingSystem3D*>(all[16]) != nullptr);
    CHECK(all[16]->get_RuntimeIndex() == 16);
    // Systems driven by an enabler start disabled.
    int startDisabled = 0;
    for (SystemBase* system : all) {
        startDisabled += system->get_StartEnabled() ? 0 : 1;
    }
    CHECK(startDisabled == 34);
    ComponentTypeId<>::ResetForTests();
}

TEST_CASE("quantum.code systems: start, enablers and group scheduling") {
    SystemsSimulation sim(SystemSetup::CreateSystems(nullptr, nullptr));
    Frame& f = *sim.frame;
    StartSystems(&f);
    auto* moveTo = sim.Find<MoveToSystem>();
    CHECK(moveTo != nullptr);
    CHECK(!f.SystemIsEnabledSelf(moveTo));
    const EntityRef entity = f.Create();
    f.Add<MoveTo>(entity);
    CHECK(f.SystemIsEnabledSelf(moveTo));
    f.Remove<MoveTo>(entity);
    CHECK(!f.SystemIsEnabledSelf(moveTo));
    // A whole frame of systems runs without touching anything unexpected.
    UpdateSystems(&f);
    CHECK(f.Exists(entity));
}

namespace {
template <class T> std::shared_ptr<T> MakeAsset(std::int64_t guid) {
    auto asset = std::make_shared<T>();
    asset->Guid = AssetGuid(guid);
    return asset;
}

// Runs the systems for `frames` frames, advancing the frame number like the session.
void RunFrames(SystemsSimulation& sim, int frames) {
    for (int i = 0; i < frames; ++i) {
        ++sim.frame->Number;
        UpdateSystems(sim.frame.get());
        sim.frame->CommitAllCommands();
    }
}
} // namespace

TEST_CASE("quantum.code systems: health initialises from its spec and regenerates") {
    auto spec = MakeAsset<HealthSpec>(100);
    spec->Mode = HealthSpecMode::FIXED;
    spec->FixedHP = FP::FromInt(100);
    spec->CanRegenerate = true;
    spec->TimeWithoutDamageBeforeStartRegen = FP::One();
    spec->TimeToRegenerate = FP::FromInt(2);
    SystemsSimulation sim(MakeSystems<HealthSystem>(), {spec});
    Frame& f = *sim.frame;
    StartSystems(&f);
    f.Number = 1;
    const EntityRef entity = f.Create();
    Health* health = nullptr;
    f.Add<Health>(entity, Health{}, health);
    health->Spec.Id = AssetGuid(100);
    // OnAdded set every multiplier to 1.
    CHECK(HealthLib::GetMaxHpMultiplier(&f, entity, HEALTH_MULTIPLIER_SOURCE::SPAWNER) == FP::One());

    RunFrames(sim, 1);
    health = f.GetPointer<Health>(entity);
    CHECK(health->Stats.Initialised);
    CHECK(health->CurrentHP == FP::FromInt(100));
    CHECK(health->Stats.MaxHP == FP::FromInt(100));

    // Halve the maximum through a multiplier: a full entity stays full.
    HealthLib::SetMaxHpMultiplier(&f, entity, HEALTH_MULTIPLIER_SOURCE::KING_CHASE, FP::FromRaw(32768), false);
    CHECK(health->Stats.MaxHP == FP::FromInt(50));
    CHECK(health->CurrentHP == FP::FromInt(50));
    HealthLib::SetMaxHpMultiplier(&f, entity, HEALTH_MULTIPLIER_SOURCE::KING_CHASE, FP::One(), true);
    CHECK(health->Stats.MaxHP == FP::FromInt(100));

    // Damage taken now: nothing happens for a second, then HP climbs back.
    health->CurrentHP = FP::FromInt(50);
    health->StartRegenHP = FP::FromInt(50);
    health->regenFrameReference = f.Number;
    RunFrames(sim, 30);
    CHECK(health->CurrentHP == FP::FromInt(50));
    RunFrames(sim, 60);
    CHECK(health->CurrentHP > FP::FromInt(60));
    CHECK(health->CurrentHP < FP::FromInt(100));
    RunFrames(sim, 60);
    CHECK(health->CurrentHP == FP::FromInt(100));
    CHECK(HealthLib::GetHPRatio(&f, entity) == FP::One());
}

TEST_CASE("quantum.code systems: kill marks, lifetimes and killables") {
    auto lifetimeSpec = MakeAsset<LifetimeSpec>(200);
    lifetimeSpec->StartLifetimeOnSpawn = true;
    lifetimeSpec->LifetimeDuration = FP::One();
    SystemsSimulation sim(MakeSystems<KillableSystem, LifetimeSystem, LifetimeInitSystem>(), {lifetimeSpec});
    Frame& f = *sim.frame;
    StartSystems(&f);
    f.Number = 1;

    // A delayed kill of a non-player destroys it once the delay has passed.
    const EntityRef marked = f.Create();
    KillableLib::KillNextFrame(&f, marked, FP::FromRaw(32768), KillType::GameRule, EntityRef{});
    // A second mark does not replace the first.
    KillableLib::KillNextFrame(&f, marked, FP::FromInt(10), KillType::Undefined, EntityRef{});
    CHECK(f.GetPointer<KillNextFrame>(marked)->killType == KillType::GameRule);
    RunFrames(sim, 20);
    CHECK(f.Exists(marked));
    RunFrames(sim, 20);
    CHECK(!f.Exists(marked));

    // A lifetime that starts on spawn kills its entity after its duration.
    const EntityRef temporary = f.Create();
    Lifetime lifetime{};
    lifetime.Spec.Id = AssetGuid(200);
    f.Add<Lifetime>(temporary, lifetime);
    CHECK(f.GetPointer<Lifetime>(temporary)->StartFrame == f.Number);
    RunFrames(sim, 30);
    CHECK(f.GetPointer<Lifetime>(temporary)->Stats.initialised);
    CHECK(f.Exists(temporary));
    RunFrames(sim, 40);
    CHECK(!f.Exists(temporary));

    // Killables: alive when added; extra lives only for limited lives; revive and kill
    // records.
    const EntityRef actor = f.Create();
    f.Add<Killable>(actor);
    CHECK(KillableLib::IsAlive(&f, actor));
    CHECK(KillableLib::GetLivesLeft(&f, actor) == 1);
    Killable* killable = f.GetPointer<Killable>(actor);
    killable->maxDeath = 3;
    KillableLib::IncrementDeathCount(&f, actor);
    KillableLib::AddExtraLives(&f, actor, 2);
    CHECK(KillableLib::GetLivesLeft(&f, actor) == 4);
    KilledByData killedBy{};
    CHECK(!KillableLib::TryGetKilledBy(&f, actor, killedBy));
}

TEST_CASE("quantum.code systems: game timer ends the game mode") {
    auto modeSpec = MakeAsset<GameModeSpec>(300);
    modeSpec->gameDuration = FP::One();
    SystemsSimulation sim(MakeSystems<GameTimerSystem>(), {modeSpec});
    Frame& f = *sim.frame;
    StartSystems(&f);
    f.Number = 10;
    f.set_UnscaledDeltaTime(f.get_DeltaTime());
    GameModeHandler* handler = f.GetOrAddSingletonPointer<GameModeHandler>();
    handler->state = GAME_MODE_HANDLER_STATE::MainLoop;
    handler->stateStartFrame = f.Number;
    handler->gameModeSpecRef.Id = AssetGuid(300);
    CHECK(GameTimerLib::GetGameDuration(&f) == FP::One());
    bool overtime = true;
    CHECK(GameTimerLib::GetTimeLeft(&f, overtime) == FP::One());
    CHECK(!overtime);
    RunFrames(sim, 30);
    CHECK(!GameModeHandlerLib::IsGameModeFinished(&f));
    RunFrames(sim, 31);
    CHECK(GameModeHandlerLib::IsGameModeFinished(&f));
    CHECK(f.GetPointerSingleton<GameModeHandler>()->finishFrame == f.Number);
    // A finished mode has no running timer.
    CHECK(GameTimerLib::GetTimeLeft(&f, overtime) == -FP::One());
}

TEST_CASE("quantum.code systems: teams, entity info and status events") {
    SystemsSimulation sim(MakeSystems<EntityInfoSystem, ActorIcedSystem, ActorInputInitialiserSystem>());
    Frame& f = *sim.frame;
    StartSystems(&f);
    const EntityRef pickup = f.Create();
    Code::EntityInfo info{};
    info.type = static_cast<EntityInfoType>(2);
    f.Add<Code::EntityInfo>(pickup, info);
    CHECK(Team::GetTeamID(&f, pickup) == Team::NeutralTeamID);
    CHECK(f.GetPointer<Team>(pickup)->isLeader);

    const EntityRef other = f.Create();
    info.type = static_cast<EntityInfoType>(1);
    f.Add<Code::EntityInfo>(other, info);
    CHECK(Team::GetTeamID(&f, other) == -1);

    const EntityRef actor = f.Create();
    f.Add<ActorInputInitialiser>(actor);
    f.CommitAllCommands();
    CHECK(f.Has<ActorInput>(actor));
    CHECK(!f.Has<ActorInputInitialiser>(actor));

    const int eventsBefore = f.get_Context()->get_Events()->get_Count();
    f.Add<ActorIced>(actor);
    f.Remove<ActorIced>(actor);
    CHECK(f.get_Context()->get_Events()->get_Count() == eventsBefore + 2);
}

TEST_CASE("quantum.code systems: kill history, combat activity and hierarchy") {
    SystemsSimulation sim(MakeSystems<ActorKillsHistorySystem, ActorCombatActivitySystem, HierarchySystem,
                                      ActorInitialisedSystem>());
    Frame& f = *sim.frame;
    StartSystems(&f);
    f.Number = 5;
    const EntityRef killer = f.Create();
    const EntityRef victim = f.Create();
    f.Add<PlayerActor>(killer);
    f.Add<PlayerActor>(victim);
    Team::JoinTeam(&f, killer, 1);
    Team::JoinTeam(&f, victim, 2);
    KilledByData killedBy{};
    killedBy.owner = killer;
    f.GetPointer<PlayerActor>(victim)->killStreak = 4;
    f.Signals.Killable_OnKill(victim, killedBy);
    CHECK(f.GetPointer<PlayerActor>(victim)->killStreak == 0);
    CHECK(f.GetPointer<PlayerActor>(killer)->killStreak == 1);
    CHECK(f.GetPointer<PlayerActor>(killer)->killCount == 1);
    CHECK(f.GetPointer<PlayerActor>(killer)->lastKillFrame == 5);
    // Team kills are not counted.
    Team::JoinTeam(&f, victim, 1);
    f.Signals.Killable_OnKill(victim, killedBy);
    CHECK(f.GetPointer<PlayerActor>(killer)->killCount == 1);

    // Hitting a player starts a combat; it ends after the reset duration.
    HitData hit{};
    hit.owner = killer;
    f.GetPointer<PlayerActor>(killer)->durationBeforeCombatReset = FP::One();
    f.Signals.Damageable_HitReceived(victim, hit, HitResult{});
    CHECK(f.GetPointer<PlayerActor>(killer)->inCombatWith == victim);
    RunFrames(sim, 30);
    CHECK(f.GetPointer<PlayerActor>(killer)->inCombatWith == victim);
    RunFrames(sim, 40);
    CHECK(f.GetPointer<PlayerActor>(killer)->inCombatWith == EntityRef{});
    CHECK(f.GetPointer<PlayerActor>(victim)->timeWithoutCombatActivity > FP::One());
    // The new players were initialised once.
    CHECK(f.Has<PlayerActorInitialized>(killer));

    // A child follows its parent at its offset.
    const EntityRef parent = f.Create();
    Transform3D parentTransform{};
    parentTransform.Position = FPVector3(FP::FromInt(10), FP{}, FP{});
    parentTransform.Rotation = FPQuaternion::Identity();
    f.Add<Transform3D>(parent, parentTransform);
    const EntityRef child = f.Create();
    f.Add<Transform3D>(child);
    Hierarchy hierarchy{};
    hierarchy.Parent = parent;
    hierarchy.Offset = FPVector3(FP{}, FP::FromInt(2), FP{});
    f.Add<Hierarchy>(child, hierarchy);
    CHECK(f.GetPointer<Hierarchy>(child)->LocalRotation == FPQuaternion::Identity());
    RunFrames(sim, 1);
    CHECK(f.GetPointer<Transform3D>(child)->Position == FPVector3(FP::FromInt(10), FP::FromInt(2), FP{}));
}

TEST_CASE("quantum.code systems: bolts, kill-count wins and slow motion") {
    auto gathererSpec = MakeAsset<BoltGathererActorSpec>(400);
    gathererSpec->MaxBoltsCollectable = 50;
    auto modeSpec = MakeAsset<FreeForAllGameModeSpec>(401);
    modeSpec->killsToWin = 2;
    modeSpec->boltLossMode = GameModeSpec::BoltsLossMode::Percentage;
    modeSpec->boltsLostOnDeathPercentage = 50;
    modeSpec->outroTimeScale = FP::FromRaw(32768);
    modeSpec->outroTimeScalingDuration = FP::One();
    SystemsSimulation sim(MakeSystems<BoltSignalSystem, GameModeKillCountSystem, TimeScaleSystem>(),
                          {gathererSpec, modeSpec});
    Frame& f = *sim.frame;
    f.get_Global()->Systems.Set(sim.Find<GameModeKillCountSystem>()->get_RuntimeIndex());
    StartSystems(&f);
    f.set_UnscaledDeltaTime(f.get_DeltaTime());
    f.Number = 1;
    GameModeHandler* handler = f.GetOrAddSingletonPointer<GameModeHandler>();
    handler->state = GAME_MODE_HANDLER_STATE::MainLoop;
    handler->gameModeSpecRef.Id = AssetGuid(401);

    const EntityRef player = f.Create();
    f.Add<PlayerActor>(player);
    BoltGathererActor gatherer{};
    gatherer.Spec.Id = AssetGuid(400);
    f.Add<BoltGathererActor>(player, gatherer);
    f.Signals.Reward_OnCollected(player, DroppableType::Bolt, 30);
    CHECK(BoltLib::GetCollectedBolts(&f, player) == 30);
    f.Signals.Reward_OnCollected(player, DroppableType::Bolt, 30);
    CHECK(BoltLib::GetCollectedBolts(&f, player) == 50); // capped by the spec
    // Dying costs half of the bolts carried.
    f.Signals.Killable_OnKill(player, KilledByData{});
    CHECK(BoltLib::GetCollectedBolts(&f, player) == 25);
    // A gatherer without a spec does not stay.
    const EntityRef stray = f.Create();
    f.Add<BoltGathererActor>(stray);
    f.CommitAllCommands();
    CHECK(!f.Has<BoltGathererActor>(stray));

    // Two player kills win the free-for-all.
    const EntityRef victim = f.Create();
    f.Add<PlayerActor>(victim);
    KilledByData killedBy{};
    killedBy.owner = player;
    f.GetPointer<PlayerActor>(player)->killCount = 1;
    f.Signals.Killable_OnKill(victim, killedBy);
    CHECK(!GameModeHandlerLib::IsGameModeFinished(&f));
    f.GetPointer<PlayerActor>(player)->killCount = 2;
    f.Signals.Killable_OnKill(victim, killedBy);
    CHECK(GameModeHandlerLib::IsGameModeFinished(&f));

    // The outro slows the simulation down to half speed over a second.
    f.Signals.GameMode_OnStateChanged(GAME_MODE_HANDLER_STATE::MainLoop, GAME_MODE_HANDLER_STATE::GameModeOutro);
    CHECK(TimeScaleLib::IsTimeScaleActive(&f));
    RunFrames(sim, 30);
    CHECK(f.get_DeltaTime() < f.get_UnscaledDeltaTime());
    CHECK(f.get_DeltaTime() > f.get_UnscaledDeltaTime() / 2);
    RunFrames(sim, 40);
    CHECK(!TimeScaleLib::IsTimeScaleActive(&f));
    f.Signals.GameMode_OnStateChanged(GAME_MODE_HANDLER_STATE::GameModeOutro, GAME_MODE_HANDLER_STATE::Ended);
    CHECK(f.get_DeltaTime() == f.get_UnscaledDeltaTime());
}

namespace {
HitData MakeHit(Frame& f, EntityRef owner, FP damages, DamageType type = DamageType::Normal) {
    HitData hit{};
    hit.type = type;
    hit.frame = f.Number;
    hit.owner = owner;
    hit.damages = damages;
    hit.baseDamage = damages;
    hit.health.damageMultiplier = FP::One();
    hit.shield.damageMultiplier = FP::One();
    return hit;
}
} // namespace

TEST_CASE("quantum.code systems: damage resolution through elements, shield and health") {
    auto damageableSpec = MakeAsset<DamageableSpec>(500);
    damageableSpec->ElementType = ElementType::Ice;
    damageableSpec->SpawnInvulnerabilityDuration = FP::FromRaw(32768); // 0.5 s
    auto healthSpec = MakeAsset<HealthSpec>(501);
    healthSpec->Mode = HealthSpecMode::FIXED;
    healthSpec->FixedHP = FP::FromInt(100);
    healthSpec->KillWhenNoMoreHP = true;
    auto table = MakeAsset<ElementalDamageTable>(502);
    DamageTypeToElementMapping fire;
    fire.damageType = DamageType::Fire;
    ElementMultiplier againstIce;
    againstIce.elementType = ElementType::Ice;
    againstIce.multiplier = FP::FromInt(2);
    fire.elementMultipliers.push_back(againstIce);
    table->mappings.push_back(fire);
    auto shieldSpec = MakeAsset<ShieldedActorSpec>(503);
    SystemsSimulation sim(MakeSystems<DamageableSystem, HealthSystem, KillableSystem>(),
                          {damageableSpec, healthSpec, table, shieldSpec});
    sim.runtime.elementalDamageTable.Id = AssetGuid(502);
    Frame& f = *sim.frame;
    StartSystems(&f);
    f.Number = 1;

    const EntityRef attacker = f.Create();
    // No Killable: losing all HP destroys the entity outright (a killable needs its entity
    // prototype).
    const EntityRef target = f.Create();
    Health health{};
    health.Spec.Id = AssetGuid(501);
    f.Add<Health>(target, health);
    Damageable damageable{};
    damageable.Spec.Id = AssetGuid(500);
    f.Add<Damageable>(target, damageable);
    RunFrames(sim, 1);
    CHECK(f.GetPointer<Health>(target)->CurrentHP == FP::FromInt(100));

    // Spawn invulnerability (30 frames): hits are not even queued.
    CHECK(DamageLib::IsInvulnerableToDamage(&f, target));
    CHECK(DamageLib::IsInvulnerable(&f, target));
    CHECK(DamageLib::GetInvulnerabilityRemainingDuration(&f, target) > FP{});
    f.GetPointer<Damageable>(target)->Hit(&f, target, MakeHit(f, attacker, FP::FromInt(30)));
    CHECK(f.ResolveList(f.GetPointer<Damageable>(target)->get_hits()).get_Count() == 0);
    RunFrames(sim, 30);
    CHECK(!DamageLib::IsInvulnerableToDamage(&f, target));

    // A plain hit reaches health and is remembered.
    f.GetPointer<Damageable>(target)->Hit(&f, target, MakeHit(f, attacker, FP::FromInt(30)));
    RunFrames(sim, 1);
    CHECK(f.GetPointer<Health>(target)->CurrentHP == FP::FromInt(70));
    CHECK(f.GetPointer<Damageable>(target)->hitBy.damages == FP::FromInt(30));
    CHECK(f.ResolveList(f.GetPointer<Damageable>(target)->get_hits()).get_Count() == 0);
    HitData lastHit{};
    CHECK(DamageLib::TryGetHitBy(&f, target, lastHit));
    CHECK(lastHit.owner == attacker);

    // Fire against ice doubles.
    f.GetPointer<Damageable>(target)->Hit(&f, target, MakeHit(f, attacker, FP::FromInt(20), DamageType::Fire));
    RunFrames(sim, 1);
    CHECK(f.GetPointer<Health>(target)->CurrentHP == FP::FromInt(30));

    // A shield takes hits instead of health until it breaks.
    ShieldedActor shield{};
    shield.spec.Id = AssetGuid(503);
    shield.shieldPoints = FP::FromInt(25);
    shield.stats.shieldMaxPoints = FP::FromInt(50);
    shield.stats.statsInitialised = true;
    shield.state = ShieldState::FULL;
    f.Add<ShieldedActor>(target, shield);
    f.GetPointer<Damageable>(target)->Hit(&f, target, MakeHit(f, attacker, FP::FromInt(10)));
    RunFrames(sim, 1);
    CHECK(f.GetPointer<ShieldedActor>(target)->shieldPoints == FP::FromInt(15));
    CHECK(f.GetPointer<ShieldedActor>(target)->state == ShieldState::WAIT_FOR_REGEN);
    CHECK(f.GetPointer<Health>(target)->CurrentHP == FP::FromInt(30));
    f.GetPointer<Damageable>(target)->Hit(&f, target, MakeHit(f, attacker, FP::FromInt(20)));
    RunFrames(sim, 1);
    CHECK(f.GetPointer<ShieldedActor>(target)->shieldIsBroken);
    CHECK(f.GetPointer<ShieldedActor>(target)->shieldPoints == FP{});
    CHECK(f.GetPointer<Health>(target)->CurrentHP == FP::FromInt(30));
    // Broken: health takes the hits again.
    f.GetPointer<Damageable>(target)->Hit(&f, target, MakeHit(f, attacker, FP::FromInt(10)));
    RunFrames(sim, 1);
    CHECK(f.GetPointer<Health>(target)->CurrentHP == FP::FromInt(20));

    // Zero HP kills.
    f.GetPointer<Damageable>(target)->Hit(&f, target, MakeHit(f, attacker, FP::FromInt(50)));
    RunFrames(sim, 1);
    CHECK(!f.Exists(target));
}

TEST_CASE("quantum.code systems: damage multipliers, entity info and shield regeneration") {
    auto shieldSpec = MakeAsset<ShieldedActorSpec>(510);
    shieldSpec->timeWithoutDamageBeforeStartRegen = FP::One();
    shieldSpec->timeToRegenerate = FP::FromInt(2);
    SystemsSimulation sim(MakeSystems<ShieldedActorSystem>(), {shieldSpec});
    Frame& f = *sim.frame;
    StartSystems(&f);
    f.Number = 1;

    // Damage source multipliers: timed and permanent, by origin.
    const EntityRef source = f.Create();
    f.Add<DamageSource>(source);
    DamageLib::SetDamageMultiplier(&f, source, FP::FromRaw(98304), DAMAGE_MULTIPLIER_ORIGIN::CriticalHit, FP::One());
    DamageLib::SetDamageMultiplier(&f, source, FP::FromInt(2), DAMAGE_MULTIPLIER_ORIGIN::Spawner);
    CHECK(DamageLib::GetTotalDamageMultiplier(&f, source) == FP::FromInt(3));
    FP critical;
    CHECK(DamageLib::TryGetDamageMultiplier(&f, source, DAMAGE_MULTIPLIER_ORIGIN::CriticalHit, critical));
    CHECK(critical == FP::FromRaw(98304));
    f.Number += 61;
    CHECK(DamageLib::GetTotalDamageMultiplier(&f, source) == FP::FromInt(2));
    CHECK(!DamageLib::TryGetDamageMultiplier(&f, source, DAMAGE_MULTIPLIER_ORIGIN::CriticalHit, critical));
    CHECK(critical == FP::One());
    DamageLib::ClearDamageMultiplier(&f, source, DAMAGE_MULTIPLIER_ORIGIN::Spawner);
    CHECK(DamageLib::GetTotalDamageMultiplier(&f, source) == FP::One());

    // Entity info lookups.
    const EntityRef boss = f.Create();
    Code::EntityInfo info{};
    info.type = EntityInfoType::Boss;
    info.id = 7;
    f.Add<Code::EntityInfo>(boss, info);
    CHECK(EntityInfoLib::EntityHasTypeMatching(&f, boss, ENTITY_INFO_FILTER::NPC));
    CHECK(!EntityInfoLib::EntityHasTypeMatching(&f, boss, ENTITY_INFO_FILTER::Player));
    CHECK(EntityInfoLib::EntityHasNoTypeOrMatches(&f, source, ENTITY_INFO_FILTER::Player));
    EntityRef found;
    CHECK(EntityInfoLib::TryFindEntity(&f, ENTITY_INFO_FILTER::BOSS, 7, found));
    CHECK(found == boss);
    CHECK(EntityInfoLib::HasId(&f, boss, 7));
    std::int32_t id = 0;
    CHECK(!EntityInfoLib::TryGetId(&f, source, id));
    CHECK(id == -1);

    // A broken shield waits, then regenerates to full (the system starts disabled).
    f.SystemEnable<ShieldedActorSystem>();
    const EntityRef actor = f.Create();
    ShieldedActor shield{};
    shield.spec.Id = AssetGuid(510);
    shield.stats.shieldMaxPoints = FP::FromInt(40);
    shield.stats.statsInitialised = true;
    shield.state = ShieldState::FULL;
    f.Add<ShieldedActor>(actor, shield);
    CHECK(f.GetPointer<ShieldedActor>(actor)->canRegen);
    ShieldedActorLib::SetState(&f, actor, ShieldState::WAIT_FOR_REGEN);
    RunFrames(sim, 59);
    CHECK(f.GetPointer<ShieldedActor>(actor)->state == ShieldState::WAIT_FOR_REGEN);
    RunFrames(sim, 2);
    CHECK(f.GetPointer<ShieldedActor>(actor)->state == ShieldState::REGENERATING);
    RunFrames(sim, 130);
    CHECK(f.GetPointer<ShieldedActor>(actor)->state == ShieldState::FULL);
    CHECK(f.GetPointer<ShieldedActor>(actor)->shieldPoints == FP::FromInt(40));
    f.SystemDisable<ShieldedActorSystem>();
    f.CommitAllCommands();
    CHECK(!f.Has<ShieldedActor>(actor));
}
