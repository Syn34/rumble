// system_groups.cpp - children of the system groups, in the original's order.
#include "quantum_code/systems/system_groups.h"

#include "quantum_code/systems/all_systems.h"

namespace Quantum {

namespace {
template <class... TSystems> std::vector<std::unique_ptr<SystemMainThread>> MakeChildren() {
    std::vector<std::unique_ptr<SystemMainThread>> children;
    children.reserve(sizeof...(TSystems));
    (children.push_back(std::make_unique<TSystems>()), ...);
    return children;
}
} // namespace

AreaOfEffectSystemGroup::AreaOfEffectSystemGroup()
    : SystemMainThreadGroup("AreaOfEffectSystemGroup",
                            MakeChildren<AoeTrailSystem, AreaOfEffectStartSystem, AreaOfEffectOverlapSystem,
                                         AreaOfEffectIgnitionSystem, AreaOfEffectHitsFilteringSystem,
                                         AreaOfEffectHealingSystem, AreaOfEffectPusherSystem,
                                         AreaOfEffectKccModifierSystem, AreaOfEffectActorStatusSystem,
                                         AreaOfEffectLevelUpSystem, AreaOfEffectLifetimeSystem>()) {}

AISystemsGroup::AISystemsGroup()
    : SystemMainThreadGroup("AISystemsGroup",
                            MakeChildren<AISpawnerSystem, AIRespawnDeadSystem, PrioritisedTargetSystem, HFSMSystem,
                                         CompoundHFSMSystem, AIBotSDKSystem, AINavigationSystem,
                                         AINavBlacklistSystem, AimBotSystem, AIMoveForwardSystem, MobGroupSystem,
                                         AgentOfDoomGroupSystem>()) {}

GameModeSystemsGroup::GameModeSystemsGroup()
    : SystemMainThreadGroup("GameModeSystemsGroup",
                            MakeChildren<GameModeHandlerSystem, TutorialGameModeSystem, TimeScaleSystem,
                                         GameTimerSystem, BlastBallGameSystem, ZoneControlGameModeSystem,
                                         EliminationGameModeSystem, TrainingGameModeSystem,
                                         WeaponShowcaseGameModeSystem, BoltRankingSystem, CoopBoltRankingSystem,
                                         FreeForAllRankingSystem, TeamTotalRumbleRankingSystem>()) {}

UltimateSystemGroup::UltimateSystemGroup()
    : SystemMainThreadGroup("UltimateSystemGroup",
                            MakeChildren<UltimateAbilitySystem, UltimateWeaponSystem, UltimateEvolutionSystem,
                                         UltimateTankSystem, AddForceOnUltimateSystem>()) {}

GadgetSystemGroup::GadgetSystemGroup()
    : SystemMainThreadGroup("GadgetSystemGroup", MakeChildren<GadgetSystem, DrillDashSystem>()) {}

WeaponsSystemsGroup::WeaponsSystemsGroup()
    : SystemMainThreadGroup(
          "WeaponsSystemsGroup",
          MakeChildren<WeaponLoadoutSystem, WeaponInputsSystem, WeaponTriggerSystem, WeaponMovementModifierSystem,
                       WeaponGatlingSystem, WeaponBehaviour_ChargingSystem, MeleeJumpAttackSystem,
                       WeaponFiringSystem, WeaponRynoSystem, VoidRepulserShieldSystem,
                       VoidRepulserBulldozerEnhancementSystem, WrenchEnhancementSystem, WeaponWrenchSystem,
                       GloveOfDoomWeaponSystem, BlasterMoveSpeedEnhancementSystem, ShotgunEnhancementSystem,
                       WeaponHoverBootsSystem, HoloshieldGloveWeaponSystem, HoloshieldGloveShieldSystem,
                       WeaponBigBoomSystem, BigBoomMineSystem, WeaponTeslaClawSystem>()) {}

} // namespace Quantum
