// system_setup.cpp - Quantum.SystemSetup and the PC driver helpers.
#include "quantum_code/systems/system_setup.h"

#include "quantum_code/systems/all_systems.h"
#include "quantum_code/systems/enablers.h"

#include <iterator>

namespace Quantum {

namespace {
template <class... TSystems> void Append(SystemList& list) {
    (list.push_back(std::make_unique<TSystems>()), ...);
}
} // namespace

SystemList SystemSetup::CreateSystems(RuntimeConfig* gameConfig, SimulationConfig* simulationConfig) {
    (void)gameConfig;
    (void)simulationConfig;
    Frame::InitStatic();
    SystemList systems = GetEnablerSystems();
    SystemList main = GetMainSystems();
    SystemList disablers = GetDisablerSystems();
    systems.insert(systems.end(), std::make_move_iterator(main.begin()), std::make_move_iterator(main.end()));
    systems.insert(systems.end(), std::make_move_iterator(disablers.begin()), std::make_move_iterator(disablers.end()));
    return systems;
}

SystemList SystemSetup::GetEnablerSystems() {
    SystemList list;
    Append<KillZoneSystemEnabler, ShapeOverlappingEnabler, GloveOfDoomTargetEnabler, GloveOfDoomWeaponEnabler,
           MoveToSystemEnabler, TurnToSystemEnabler, AutoJumperActorEnablerSystem, RollBouncingEnablerSystem,
           MeleeAttackEnablerSystem, DamageToImpulseConverterSystemEnabler, ParabolaFollowerEnablerSystem,
           BlasterSpecialShotEnhancementEnablerSystem, ArenaCannonSystemEnabler, ShieldedActorSystemEnabler,
           AmoeboidLauncherEnhancementPuddleSystemEnabler, WeaponProjectileSwitcherEnhancementSystemEnabler>(list);
    return list;
}

SystemList SystemSetup::GetDisablerSystems() {
    SystemList list;
    Append<KillZoneSystemDisabler, ShapeOverlappingDisabler, GloveOfDoomTargetDisabler, GloveOfDoomWeaponDisabler,
           MoveToSystemDisabler, TurnToSystemDisabler, AutoJumperActorDisablerSystem, RollBouncingDisablerSystem,
           MeleeAttackDisablerSystem, DamageToImpulseConverterSystemDisabler, ParabolaFollowerDisablerSystem,
           BlasterSpecialShotEnhancementDisablerSystem, ArenaCannonSystemDisabler, ShieldedActorSystemDisabler,
           AmoeboidLauncherEnhancementPuddleSystemDisabler, WeaponProjectileSwitcherEnhancementSystemDisabler>(
        list);
    return list;
}

SystemList SystemSetup::GetMainSystems() {
    SystemList list;
    list.reserve(154);
    Append<Core::CullingSystem3D, KillableSystem, OnKillCallbacksSystem, KccSurfaceResetSystem, HealthSystem,
           LifetimeSystem, ProjectileSpawnerSystem, GameManagerSystem, TargetableSystem_FindTargetsInRange,
           TargetableSystem_UpdateTargetPositions, TargetableSystem_InjectRayCastToTargetsInRange,
           KccOverlapInjectionSystem, ProjectileCollisionInjectionSystem, Core::PhysicsSystem3D,
           ShapeOverlappingSystem, TargetableSystem_HandleRayCastToPotentialTargets, TargetableSystem_ScoreTargets,
           WeaponLobAutoTargetSystem, GloveOfDoomTargetSystem, BombardierWeaponAutoTargetSystem,
           CryosliderWeaponAutoTargetSystem, ProjectileCollisionHandlerSystem, ProjectileMovementSystem,
           HoloshieldGloveTargetSystem>(list);
    list.push_back(Core::DebugCommand::CreateSystem());
    Append<FireReactorSystem, StreamEffectSpawnerSystem, StreamEffectSystem, TrailSpawnerSystem,
           AreaOfEffectSystemGroup, TargetedEffectSystem, DamageAndPushOverTimeSystem, BuzzbladesRicochetSystem,
           BombardierDroneSystem, CryosliderSystem, MineMergingSystem, WeaponProjectileSwitcherEnhancementUpdateSystem,
           Core::NavigationSystem, Core::EntityPrototypeSystem, Core::PlayerConnectedSystem,
           AutoDestroyOnMapChangedSystem, PhysicsSleepSystem, ActorManagerSystem, ActorCombatActivitySystem,
           RewardSystem, ActorKillsHistorySystem, GameModeKillCountSystem, GameModeBoltCountSystem,
           GameModeSystemsGroup, SquadSiegeGameModeSystem, AISystemsGroup, UnlockedFeaturesManagerSystem,
           ActorInputConverterSystem, ActorInputUpdateSystem, EntityDropperSpawnSystem, DroppableManagerSystem,
           MoveToSystem, TurnToSystem, BoltSignalSystem, GoldenBoltSystem, BoltPowerSystem, ShieldedActorSystem,
           PushableSystem, PushStopSystem, MovingWallSystem, PendingPhysicsImpulseSystem, LevelButtonSystem,
           ArenaCannonSystem, ItemSpawnerSystem, BoltCounterElementSystem, EnergyCellSignalSystem,
           DamageChargeableSystem, UltimateSystemGroup, GadgetSystemGroup, WeaponsSystemsGroup>(list);
    Append<ColliderResizeSystem, GameModifiersSystem, DashSystem, GroundDashSystem, HelipackActorSystem,
           TravolatorActorSystem, MovementSystem, CollisionSystem, OnTriggerEnterSystem, PowerUpSystem, RailsSystem,
           WallRunningSystem, LedgeGrabbingActorSystem, AutoJumperActorSystem, JumpAttackSystem, SweepAttackSystem,
           MeleeAttackSystem, ChargeAttackSystem, KillZoneSystem, SpeederSystem, ParabolaFollowerSystem,
           HierarchySystem, CollisionForceSystem, BlastBallSystem, RollPhysicSystem, WaveManagerSystem,
           ShrinkingZoneSystem, ShrinkingZoneActorSystem, WakeUpSurroundingsOnPushOrDestroySystem, ActorStatusSystem,
           ActorStatusChangeSystem, ActorIcedSystem, ActorSheepedSystem, ActorStunnedSystem, ActorInitialisedSystem,
           DisableEnergyCollectionOnAbilityUseSystem, GameModeActivatorSystem, ActorInputInitialiserSystem,
           AINavigationInitialiserSystem, BlackboardInitializerSystem, ActorManagerRespawnTesterSystem,
           LifetimeInitSystem, WeaponUserInitSystem, WeaponInitSystem, ProjectileInitSystem, AreaOfEffectInitSystem,
           InitializeRollSystem, WeaponEquipSystem, WeaponLobEquipSystem, MobGroupMemberSystem>(list);
    Append<BlasterSpecialShotEnhancementSystem, MineLauncherEnhancementSystem, GrenadeLauncherEnhancementSystem,
           WarmongerMiniRocketsEnhancementSystem, WeaponProjectileSwitcherEnhancementSystem, EnforcerEnhancementSystem,
           AddEntitiesToSpawnOnProjectileDeathSystem, HoloshieldGloveHealZoneEnhancementSystem,
           AddDamageIncreaseOnTargetedEffectSystem, RecoverAmmoOnDashEnhancementSystem,
           DamageIncreasedOnTargetsEnhancementSystem, AmoeboidLauncherEnhancementPuddleSystem,
           ApplyingDamageMultiplierSystem, BattleStatsActorSystem, DamageableSystem, DamageToImpulseConverterSystem,
           ActorSlowDownOnHitSystem, RollSignalSystem, RollBouncingSystem, FireDamageSystem, ElectricDamageSystem,
           IceDamageSystem, ReviveTotemSystem, EntityInfoSystem, UltimateSlamSystem, EntityGroupSystem,
           VoidRepulserVoidWaveEnhancementSystem, VoidRepulserSpeedBoostEnhancementSystem,
           CharacterColliderScalerSystem>(list);
    return list;
}

// --- PC driver -------------------------------------------------------------------------

std::vector<SystemBase*> FlattenSystems(const SystemList& roots) {
    std::vector<SystemBase*> all;
    for (const auto& root : roots) {
        for (SystemBase* system : root->get_Hierarchy()) {
            all.push_back(system);
        }
    }
    for (std::size_t i = 0; i < all.size(); ++i) {
        if (!all[i]->HasRuntimeIndex()) {
            all[i]->set_RuntimeIndex(static_cast<int>(i));
        }
    }
    return all;
}

void StartSystems(Frame* f) {
    for (SystemBase* system : f->SystemsAll()) {
        if (system->get_StartEnabled()) {
            f->get_Global()->Systems.Set(system->get_RuntimeIndex());
        }
    }
    for (SystemBase* system : f->SystemsAll()) {
        system->OnInit(f);
    }
    for (SystemBase* system : f->SystemsAll()) {
        if (f->SystemIsEnabledInHierarchy(system)) {
            system->OnEnabled(f);
        }
    }
}

void UpdateSystems(Frame* f) {
    for (SystemBase* system : f->SystemsRoots()) {
        if (f->SystemIsEnabledSelf(system)) {
            system->OnSchedule(f);
        }
    }
}

} // namespace Quantum
