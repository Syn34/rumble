// flat_entity_prototype_container.cpp - FlatEntityPrototypeContainer and its StoreVisitor
// (quantum.code.dll).
#include "quantum_code/prototypes/flat_entity_prototype_container.h"

#include <stdexcept>

namespace Quantum::Prototypes {

namespace {

// Collect<T> (shared generic code): every prototype of one list, in order.
template <class T>
void CollectList(const std::vector<std::shared_ptr<T>>& source,
                 std::vector<std::shared_ptr<ComponentPrototype>>& target) {
    for (const auto& prototype : source)
        target.push_back(prototype);
}

// Store<T> (shared generic code): appends the prototype being visited to its list.
template <class T>
void StoreInList(std::vector<std::shared_ptr<T>>& list,
                 const std::shared_ptr<ComponentPrototype>& current, T& visited) {
    auto typed = std::dynamic_pointer_cast<T>(current);
    if (!typed || typed.get() != &visited) {
        throw std::logic_error("StoreVisitor: visited prototype is not the one being stored");
    }
    list.push_back(std::move(typed));
}

} // namespace

void FlatEntityPrototypeContainer::Collect(
    std::vector<std::shared_ptr<ComponentPrototype>>& target) const {
    CollectList(CharacterController2D, target);
    CollectList(CharacterController3D, target);
    CollectList(NavMeshAvoidanceAgent, target);
    CollectList(NavMeshAvoidanceObstacle, target);
    CollectList(NavMeshPathfinder, target);
    CollectList(NavMeshSteeringAgent, target);
    CollectList(PhysicsBody2D, target);
    CollectList(PhysicsBody3D, target);
    CollectList(PhysicsCollider2D, target);
    CollectList(PhysicsCollider3D, target);
    CollectList(PhysicsCallbacks2D, target);
    CollectList(PhysicsCallbacks3D, target);
    CollectList(Transform2D, target);
    CollectList(Transform2DVertical, target);
    CollectList(Transform3D, target);
    CollectList(View, target);
    CollectList(PhysicsJoints2D, target);
    CollectList(PhysicsJoints3D, target);
    CollectGen(target);
}

void FlatEntityPrototypeContainer::CollectGen(
    std::vector<std::shared_ptr<ComponentPrototype>>& target) const {
    CollectList(AIBlackboardComponent, target);
    CollectList(AIMoveForwardBehaviour, target);
    CollectList(AINavigationInitialiser, target);
    CollectList(AISpawner, target);
    CollectList(ActorInputInitialiser, target);
    CollectList(ActorManager, target);
    CollectList(ActorStatus, target);
    CollectList(ActorStatusModifier, target);
    CollectList(ActorStunned, target);
    CollectList(AddDamageIncreaseOnTargetedEffect, target);
    CollectList(AddEntitiesToSpawnOnProjectileDeath, target);
    CollectList(AgentOfDoom, target);
    CollectList(AgentOfDoomGrouped, target);
    CollectList(AmoeboidLauncherEnhancementPuddle, target);
    CollectList(AoeTrail, target);
    CollectList(AreaOfEffect, target);
    CollectList(AreaOfEffectLevelUp, target);
    CollectList(AreaOfEffectRaycastInfo, target);
    CollectList(ArenaCannon, target);
    CollectList(AttackInfo, target);
    CollectList(BTAgent, target);
    CollectList(BattleStatsActor, target);
    CollectList(BigBoomMine, target);
    CollectList(BigBoomWeapon, target);
    CollectList(BlackboardInitializer, target);
    CollectList(BlastBall, target);
    CollectList(BlastBallGame, target);
    CollectList(BlastBallGoal, target);
    CollectList(BlastBallPlayer, target);
    CollectList(BlasterMoveSpeedEnhancement, target);
    CollectList(BlasterSpecialShotEnhancement, target);
    CollectList(BoltCounterElement, target);
    CollectList(BoltGathererActor, target);
    CollectList(BoltPowerActor, target);
    CollectList(BombardierDrone, target);
    CollectList(BombardierWeapon, target);
    CollectList(Bot, target);
    CollectList(Bumper, target);
    CollectList(BumperBar, target);
    CollectList(BuzzbladesRicochet, target);
    CollectList(CharacterColliderScaler, target);
    CollectList(ChargeAttack, target);
    CollectList(Checkpoint, target);
    CollectList(ColliderResize, target);
    CollectList(CollisionForce, target);
    CollectList(CombatParticipant, target);
    CollectList(CompoundHFSM, target);
    CollectList(Cryoslider, target);
    CollectList(CryosliderWeapon, target);
    CollectList(DamageChargeable, target);
    CollectList(DamageIncreasedOnTargetsEnhancement, target);
    CollectList(DamageMultiplier, target);
    CollectList(DamageMultiplierHeroImprovement, target);
    CollectList(DamageSource, target);
    CollectList(DamageToImpulseConverter, target);
    CollectList(Damageable, target);
    CollectList(DashActor, target);
    CollectList(DrillDash, target);
    CollectList(DroppableCollector, target);
    CollectList(DroppableEntity, target);
    CollectList(DroppableManager, target);
    CollectList(ElementalBuildUpMultiplier, target);
    CollectList(EliminationGameMode, target);
    CollectList(EnergyCellManager, target);
    CollectList(EnforcerFirstShotBonusEnhancement, target);
    CollectList(EntityDropper, target);
    CollectList(EntityInfo, target);
    CollectList(FireReactor, target);
    CollectList(GOAPAgent, target);
    CollectList(GOAPData, target);
    CollectList(GadgetUser, target);
    CollectList(GameManager, target);
    CollectList(GameModeActivator, target);
    CollectList(GameModeHandler, target);
    CollectList(GameModifiersManager, target);
    CollectList(GatlingWeapon, target);
    CollectList(GloveOfDoomWeapon, target);
    CollectList(GoldenBolt, target);
    CollectList(GoldenBoltManager, target);
    CollectList(GrinderActor, target);
    CollectList(GroundDashActor, target);
    CollectList(HFSMAgent, target);
    CollectList(HeadHunterWallHackEnhancement, target);
    CollectList(HealSource, target);
    CollectList(Health, target);
    CollectList(HelipackActor, target);
    CollectList(Hierarchy, target);
    CollectList(HoloshieldGloveHealZoneEnhancement, target);
    CollectList(HoloshieldGloveShield, target);
    CollectList(HoloshieldGloveShields, target);
    CollectList(HoloshieldGloveShieldsGrouped, target);
    CollectList(HoloshieldGloveWeapon, target);
    CollectList(HoverBootsWeapon, target);
    CollectList(ItemSpawner, target);
    CollectList(JumpAttack, target);
    CollectList(KccExtension, target);
    CollectList(KccModifier, target);
    CollectList(KillZone, target);
    CollectList(Killable, target);
    CollectList(LedgeGrabbingActor, target);
    CollectList(LevelButton, target);
    CollectList(LevelButtonGameplayElement, target);
    CollectList(LevelHolder, target);
    CollectList(LevelUpActor, target);
    CollectList(Lifetime, target);
    CollectList(MeleeAttack, target);
    CollectList(MeleeJumpAttack, target);
    CollectList(MineLauncherEnhancement, target);
    CollectList(MineMerger, target);
    CollectList(MobGroup, target);
    CollectList(MobGroupMember, target);
    CollectList(MobInfo, target);
    CollectList(MoveTo, target);
    CollectList(MovementPackCache, target);
    CollectList(MovingWall, target);
    CollectList(OnKillCallbacks, target);
    CollectList(OnTriggerEnter, target);
    CollectList(PlayerActor, target);
    CollectList(PowerUp, target);
    CollectList(PowerUpUser, target);
    CollectList(PreciseHitReceiver, target);
    CollectList(PrioritisedTargetHandler, target);
    CollectList(Projectile, target);
    CollectList(ProjectileSpawner, target);
    CollectList(Pushable, target);
    CollectList(PushableTrigger, target);
    CollectList(Pusher, target);
    CollectList(Rail, target);
    CollectList(RailCollider, target);
    CollectList(RankingManager, target);
    CollectList(RecoverAmmoOnDashEnhancement, target);
    CollectList(ReplaceMultiShotConfigsEnhancement, target);
    CollectList(RewardReceiver, target);
    CollectList(Roll, target);
    CollectList(RunnableWall, target);
    CollectList(RunnableWallTriggerZone, target);
    CollectList(RynoWeapon, target);
    CollectList(ShapeOverlapper, target);
    CollectList(ShieldedActor, target);
    CollectList(ShotgunBerserkerEnhancement, target);
    CollectList(ShotgunEnhancement, target);
    CollectList(ShrinkingZone, target);
    CollectList(ShrinkingZoneActor, target);
    CollectList(SpawnProjectilesOnDestroy, target);
    CollectList(Speeder, target);
    CollectList(StatsManager, target);
    CollectList(StreamEffect, target);
    CollectList(StreamEffectSpawner, target);
    CollectList(SweepAttack, target);
    CollectList(TargetFinder, target);
    CollectList(TargetHolder, target);
    CollectList(Targetable, target);
    CollectList(TargetedEffect, target);
    CollectList(Team, target);
    CollectList(TeslaClawMoreBeam, target);
    CollectList(TimeScale, target);
    CollectList(TrailSpawner, target);
    CollectList(TravolatorActor, target);
    CollectList(TurnTo, target);
    CollectList(TutorialGameMode, target);
    CollectList(UTAgent, target);
    CollectList(UltimateAbilityUser, target);
    CollectList(UltimateEvolution, target);
    CollectList(UltimateTank, target);
    CollectList(UnlockedFeaturesManager, target);
    CollectList(VoidRepulserShield, target);
    CollectList(VoidRepulserSpeedBoostEnhancement, target);
    CollectList(VoidRepulserVoidWaveEnhancement, target);
    CollectList(VoidRepulserWeapon, target);
    CollectList(WakeUpSurroundingsOnPushOrDestroy, target);
    CollectList(WallRunningActor, target);
    CollectList(WarmongerMiniRocketsEnhancement, target);
    CollectList(WaveElement, target);
    CollectList(WaveManager, target);
    CollectList(WaveSpawner, target);
    CollectList(Weapon, target);
    CollectList(WeaponBehaviour_Charging, target);
    CollectList(WeaponLoadoutActor, target);
    CollectList(WeaponProjectileSwitcherEnhancement, target);
    CollectList(WeaponShowcaseGameMode, target);
    CollectList(WeaponUser, target);
    CollectList(WrenchThrowProjectile, target);
    CollectList(WrenchThrowWeapon, target);
    CollectList(WrenchWeapon, target);
    CollectList(ZoneControlGameMode, target);
}

void FlatEntityPrototypeContainer::Store(
    const std::vector<std::shared_ptr<ComponentPrototype>>& prototypes) {
    StoreVisitor visitor;
    visitor.Storage = this;
    for (const auto& prototype : prototypes) {
        if (!prototype)
            throw std::invalid_argument("FlatEntityPrototypeContainer::Store: null prototype");
        visitor.Current = prototype;
        prototype->Dispatch(visitor);
    }
}

void StoreVisitor::Visit(AIBlackboardComponent_Prototype& prototype) {
    StoreInList(Storage->AIBlackboardComponent, Current, prototype);
}
void StoreVisitor::Visit(AIMoveForwardBehaviour_Prototype& prototype) {
    StoreInList(Storage->AIMoveForwardBehaviour, Current, prototype);
}
void StoreVisitor::Visit(AINavigationInitialiser_Prototype& prototype) {
    StoreInList(Storage->AINavigationInitialiser, Current, prototype);
}
void StoreVisitor::Visit(AISpawner_Prototype& prototype) {
    StoreInList(Storage->AISpawner, Current, prototype);
}
void StoreVisitor::Visit(ActorInputInitialiser_Prototype& prototype) {
    StoreInList(Storage->ActorInputInitialiser, Current, prototype);
}
void StoreVisitor::Visit(ActorManager_Prototype& prototype) {
    StoreInList(Storage->ActorManager, Current, prototype);
}
void StoreVisitor::Visit(ActorStatus_Prototype& prototype) {
    StoreInList(Storage->ActorStatus, Current, prototype);
}
void StoreVisitor::Visit(ActorStatusModifier_Prototype& prototype) {
    StoreInList(Storage->ActorStatusModifier, Current, prototype);
}
void StoreVisitor::Visit(ActorStunned_Prototype& prototype) {
    StoreInList(Storage->ActorStunned, Current, prototype);
}
void StoreVisitor::Visit(AddDamageIncreaseOnTargetedEffect_Prototype& prototype) {
    StoreInList(Storage->AddDamageIncreaseOnTargetedEffect, Current, prototype);
}
void StoreVisitor::Visit(AddEntitiesToSpawnOnProjectileDeath_Prototype& prototype) {
    StoreInList(Storage->AddEntitiesToSpawnOnProjectileDeath, Current, prototype);
}
void StoreVisitor::Visit(AgentOfDoom_Prototype& prototype) {
    StoreInList(Storage->AgentOfDoom, Current, prototype);
}
void StoreVisitor::Visit(AgentOfDoomGrouped_Prototype& prototype) {
    StoreInList(Storage->AgentOfDoomGrouped, Current, prototype);
}
void StoreVisitor::Visit(AmoeboidLauncherEnhancementPuddle_Prototype& prototype) {
    StoreInList(Storage->AmoeboidLauncherEnhancementPuddle, Current, prototype);
}
void StoreVisitor::Visit(AoeTrail_Prototype& prototype) {
    StoreInList(Storage->AoeTrail, Current, prototype);
}
void StoreVisitor::Visit(AreaOfEffect_Prototype& prototype) {
    StoreInList(Storage->AreaOfEffect, Current, prototype);
}
void StoreVisitor::Visit(AreaOfEffectLevelUp_Prototype& prototype) {
    StoreInList(Storage->AreaOfEffectLevelUp, Current, prototype);
}
void StoreVisitor::Visit(AreaOfEffectRaycastInfo_Prototype& prototype) {
    StoreInList(Storage->AreaOfEffectRaycastInfo, Current, prototype);
}
void StoreVisitor::Visit(ArenaCannon_Prototype& prototype) {
    StoreInList(Storage->ArenaCannon, Current, prototype);
}
void StoreVisitor::Visit(AttackInfo_Prototype& prototype) {
    StoreInList(Storage->AttackInfo, Current, prototype);
}
void StoreVisitor::Visit(BTAgent_Prototype& prototype) {
    StoreInList(Storage->BTAgent, Current, prototype);
}
void StoreVisitor::Visit(BattleStatsActor_Prototype& prototype) {
    StoreInList(Storage->BattleStatsActor, Current, prototype);
}
void StoreVisitor::Visit(BigBoomMine_Prototype& prototype) {
    StoreInList(Storage->BigBoomMine, Current, prototype);
}
void StoreVisitor::Visit(BigBoomWeapon_Prototype& prototype) {
    StoreInList(Storage->BigBoomWeapon, Current, prototype);
}
void StoreVisitor::Visit(BlackboardInitializer_Prototype& prototype) {
    StoreInList(Storage->BlackboardInitializer, Current, prototype);
}
void StoreVisitor::Visit(BlastBall_Prototype& prototype) {
    StoreInList(Storage->BlastBall, Current, prototype);
}
void StoreVisitor::Visit(BlastBallGame_Prototype& prototype) {
    StoreInList(Storage->BlastBallGame, Current, prototype);
}
void StoreVisitor::Visit(BlastBallGoal_Prototype& prototype) {
    StoreInList(Storage->BlastBallGoal, Current, prototype);
}
void StoreVisitor::Visit(BlastBallPlayer_Prototype& prototype) {
    StoreInList(Storage->BlastBallPlayer, Current, prototype);
}
void StoreVisitor::Visit(BlasterMoveSpeedEnhancement_Prototype& prototype) {
    StoreInList(Storage->BlasterMoveSpeedEnhancement, Current, prototype);
}
void StoreVisitor::Visit(BlasterSpecialShotEnhancement_Prototype& prototype) {
    StoreInList(Storage->BlasterSpecialShotEnhancement, Current, prototype);
}
void StoreVisitor::Visit(BoltCounterElement_Prototype& prototype) {
    StoreInList(Storage->BoltCounterElement, Current, prototype);
}
void StoreVisitor::Visit(BoltGathererActor_Prototype& prototype) {
    StoreInList(Storage->BoltGathererActor, Current, prototype);
}
void StoreVisitor::Visit(BoltPowerActor_Prototype& prototype) {
    StoreInList(Storage->BoltPowerActor, Current, prototype);
}
void StoreVisitor::Visit(BombardierDrone_Prototype& prototype) {
    StoreInList(Storage->BombardierDrone, Current, prototype);
}
void StoreVisitor::Visit(BombardierWeapon_Prototype& prototype) {
    StoreInList(Storage->BombardierWeapon, Current, prototype);
}
void StoreVisitor::Visit(Bot_Prototype& prototype) {
    StoreInList(Storage->Bot, Current, prototype);
}
void StoreVisitor::Visit(Bumper_Prototype& prototype) {
    StoreInList(Storage->Bumper, Current, prototype);
}
void StoreVisitor::Visit(BumperBar_Prototype& prototype) {
    StoreInList(Storage->BumperBar, Current, prototype);
}
void StoreVisitor::Visit(BuzzbladesRicochet_Prototype& prototype) {
    StoreInList(Storage->BuzzbladesRicochet, Current, prototype);
}
void StoreVisitor::Visit(CharacterColliderScaler_Prototype& prototype) {
    StoreInList(Storage->CharacterColliderScaler, Current, prototype);
}
void StoreVisitor::Visit(ChargeAttack_Prototype& prototype) {
    StoreInList(Storage->ChargeAttack, Current, prototype);
}
void StoreVisitor::Visit(Checkpoint_Prototype& prototype) {
    StoreInList(Storage->Checkpoint, Current, prototype);
}
void StoreVisitor::Visit(ColliderResize_Prototype& prototype) {
    StoreInList(Storage->ColliderResize, Current, prototype);
}
void StoreVisitor::Visit(CollisionForce_Prototype& prototype) {
    StoreInList(Storage->CollisionForce, Current, prototype);
}
void StoreVisitor::Visit(CombatParticipant_Prototype& prototype) {
    StoreInList(Storage->CombatParticipant, Current, prototype);
}
void StoreVisitor::Visit(CompoundHFSM_Prototype& prototype) {
    StoreInList(Storage->CompoundHFSM, Current, prototype);
}
void StoreVisitor::Visit(Cryoslider_Prototype& prototype) {
    StoreInList(Storage->Cryoslider, Current, prototype);
}
void StoreVisitor::Visit(CryosliderWeapon_Prototype& prototype) {
    StoreInList(Storage->CryosliderWeapon, Current, prototype);
}
void StoreVisitor::Visit(DamageChargeable_Prototype& prototype) {
    StoreInList(Storage->DamageChargeable, Current, prototype);
}
void StoreVisitor::Visit(DamageIncreasedOnTargetsEnhancement_Prototype& prototype) {
    StoreInList(Storage->DamageIncreasedOnTargetsEnhancement, Current, prototype);
}
void StoreVisitor::Visit(DamageMultiplier_Prototype& prototype) {
    StoreInList(Storage->DamageMultiplier, Current, prototype);
}
void StoreVisitor::Visit(DamageMultiplierHeroImprovement_Prototype& prototype) {
    StoreInList(Storage->DamageMultiplierHeroImprovement, Current, prototype);
}
void StoreVisitor::Visit(DamageSource_Prototype& prototype) {
    StoreInList(Storage->DamageSource, Current, prototype);
}
void StoreVisitor::Visit(DamageToImpulseConverter_Prototype& prototype) {
    StoreInList(Storage->DamageToImpulseConverter, Current, prototype);
}
void StoreVisitor::Visit(Damageable_Prototype& prototype) {
    StoreInList(Storage->Damageable, Current, prototype);
}
void StoreVisitor::Visit(DashActor_Prototype& prototype) {
    StoreInList(Storage->DashActor, Current, prototype);
}
void StoreVisitor::Visit(DrillDash_Prototype& prototype) {
    StoreInList(Storage->DrillDash, Current, prototype);
}
void StoreVisitor::Visit(DroppableCollector_Prototype& prototype) {
    StoreInList(Storage->DroppableCollector, Current, prototype);
}
void StoreVisitor::Visit(DroppableEntity_Prototype& prototype) {
    StoreInList(Storage->DroppableEntity, Current, prototype);
}
void StoreVisitor::Visit(DroppableManager_Prototype& prototype) {
    StoreInList(Storage->DroppableManager, Current, prototype);
}
void StoreVisitor::Visit(ElementalBuildUpMultiplier_Prototype& prototype) {
    StoreInList(Storage->ElementalBuildUpMultiplier, Current, prototype);
}
void StoreVisitor::Visit(EliminationGameMode_Prototype& prototype) {
    StoreInList(Storage->EliminationGameMode, Current, prototype);
}
void StoreVisitor::Visit(EnergyCellManager_Prototype& prototype) {
    StoreInList(Storage->EnergyCellManager, Current, prototype);
}
void StoreVisitor::Visit(EnforcerFirstShotBonusEnhancement_Prototype& prototype) {
    StoreInList(Storage->EnforcerFirstShotBonusEnhancement, Current, prototype);
}
void StoreVisitor::Visit(EntityDropper_Prototype& prototype) {
    StoreInList(Storage->EntityDropper, Current, prototype);
}
void StoreVisitor::Visit(EntityInfo_Prototype& prototype) {
    StoreInList(Storage->EntityInfo, Current, prototype);
}
void StoreVisitor::Visit(FireReactor_Prototype& prototype) {
    StoreInList(Storage->FireReactor, Current, prototype);
}
void StoreVisitor::Visit(GOAPAgent_Prototype& prototype) {
    StoreInList(Storage->GOAPAgent, Current, prototype);
}
void StoreVisitor::Visit(GOAPData_Prototype& prototype) {
    StoreInList(Storage->GOAPData, Current, prototype);
}
void StoreVisitor::Visit(GadgetUser_Prototype& prototype) {
    StoreInList(Storage->GadgetUser, Current, prototype);
}
void StoreVisitor::Visit(GameManager_Prototype& prototype) {
    StoreInList(Storage->GameManager, Current, prototype);
}
void StoreVisitor::Visit(GameModeActivator_Prototype& prototype) {
    StoreInList(Storage->GameModeActivator, Current, prototype);
}
void StoreVisitor::Visit(GameModeHandler_Prototype& prototype) {
    StoreInList(Storage->GameModeHandler, Current, prototype);
}
void StoreVisitor::Visit(GameModifiersManager_Prototype& prototype) {
    StoreInList(Storage->GameModifiersManager, Current, prototype);
}
void StoreVisitor::Visit(GatlingWeapon_Prototype& prototype) {
    StoreInList(Storage->GatlingWeapon, Current, prototype);
}
void StoreVisitor::Visit(GloveOfDoomWeapon_Prototype& prototype) {
    StoreInList(Storage->GloveOfDoomWeapon, Current, prototype);
}
void StoreVisitor::Visit(GoldenBolt_Prototype& prototype) {
    StoreInList(Storage->GoldenBolt, Current, prototype);
}
void StoreVisitor::Visit(GoldenBoltManager_Prototype& prototype) {
    StoreInList(Storage->GoldenBoltManager, Current, prototype);
}
void StoreVisitor::Visit(GrinderActor_Prototype& prototype) {
    StoreInList(Storage->GrinderActor, Current, prototype);
}
void StoreVisitor::Visit(GroundDashActor_Prototype& prototype) {
    StoreInList(Storage->GroundDashActor, Current, prototype);
}
void StoreVisitor::Visit(HFSMAgent_Prototype& prototype) {
    StoreInList(Storage->HFSMAgent, Current, prototype);
}
void StoreVisitor::Visit(HeadHunterWallHackEnhancement_Prototype& prototype) {
    StoreInList(Storage->HeadHunterWallHackEnhancement, Current, prototype);
}
void StoreVisitor::Visit(HealSource_Prototype& prototype) {
    StoreInList(Storage->HealSource, Current, prototype);
}
void StoreVisitor::Visit(Health_Prototype& prototype) {
    StoreInList(Storage->Health, Current, prototype);
}
void StoreVisitor::Visit(HelipackActor_Prototype& prototype) {
    StoreInList(Storage->HelipackActor, Current, prototype);
}
void StoreVisitor::Visit(Hierarchy_Prototype& prototype) {
    StoreInList(Storage->Hierarchy, Current, prototype);
}
void StoreVisitor::Visit(HoloshieldGloveHealZoneEnhancement_Prototype& prototype) {
    StoreInList(Storage->HoloshieldGloveHealZoneEnhancement, Current, prototype);
}
void StoreVisitor::Visit(HoloshieldGloveShield_Prototype& prototype) {
    StoreInList(Storage->HoloshieldGloveShield, Current, prototype);
}
void StoreVisitor::Visit(HoloshieldGloveShields_Prototype& prototype) {
    StoreInList(Storage->HoloshieldGloveShields, Current, prototype);
}
void StoreVisitor::Visit(HoloshieldGloveShieldsGrouped_Prototype& prototype) {
    StoreInList(Storage->HoloshieldGloveShieldsGrouped, Current, prototype);
}
void StoreVisitor::Visit(HoloshieldGloveWeapon_Prototype& prototype) {
    StoreInList(Storage->HoloshieldGloveWeapon, Current, prototype);
}
void StoreVisitor::Visit(HoverBootsWeapon_Prototype& prototype) {
    StoreInList(Storage->HoverBootsWeapon, Current, prototype);
}
void StoreVisitor::Visit(ItemSpawner_Prototype& prototype) {
    StoreInList(Storage->ItemSpawner, Current, prototype);
}
void StoreVisitor::Visit(JumpAttack_Prototype& prototype) {
    StoreInList(Storage->JumpAttack, Current, prototype);
}
void StoreVisitor::Visit(KccExtension_Prototype& prototype) {
    StoreInList(Storage->KccExtension, Current, prototype);
}
void StoreVisitor::Visit(KccModifier_Prototype& prototype) {
    StoreInList(Storage->KccModifier, Current, prototype);
}
void StoreVisitor::Visit(KillZone_Prototype& prototype) {
    StoreInList(Storage->KillZone, Current, prototype);
}
void StoreVisitor::Visit(Killable_Prototype& prototype) {
    StoreInList(Storage->Killable, Current, prototype);
}
void StoreVisitor::Visit(LedgeGrabbingActor_Prototype& prototype) {
    StoreInList(Storage->LedgeGrabbingActor, Current, prototype);
}
void StoreVisitor::Visit(LevelButton_Prototype& prototype) {
    StoreInList(Storage->LevelButton, Current, prototype);
}
void StoreVisitor::Visit(LevelButtonGameplayElement_Prototype& prototype) {
    StoreInList(Storage->LevelButtonGameplayElement, Current, prototype);
}
void StoreVisitor::Visit(LevelHolder_Prototype& prototype) {
    StoreInList(Storage->LevelHolder, Current, prototype);
}
void StoreVisitor::Visit(LevelUpActor_Prototype& prototype) {
    StoreInList(Storage->LevelUpActor, Current, prototype);
}
void StoreVisitor::Visit(Lifetime_Prototype& prototype) {
    StoreInList(Storage->Lifetime, Current, prototype);
}
void StoreVisitor::Visit(MeleeAttack_Prototype& prototype) {
    StoreInList(Storage->MeleeAttack, Current, prototype);
}
void StoreVisitor::Visit(MeleeJumpAttack_Prototype& prototype) {
    StoreInList(Storage->MeleeJumpAttack, Current, prototype);
}
void StoreVisitor::Visit(MineLauncherEnhancement_Prototype& prototype) {
    StoreInList(Storage->MineLauncherEnhancement, Current, prototype);
}
void StoreVisitor::Visit(MineMerger_Prototype& prototype) {
    StoreInList(Storage->MineMerger, Current, prototype);
}
void StoreVisitor::Visit(MobGroup_Prototype& prototype) {
    StoreInList(Storage->MobGroup, Current, prototype);
}
void StoreVisitor::Visit(MobGroupMember_Prototype& prototype) {
    StoreInList(Storage->MobGroupMember, Current, prototype);
}
void StoreVisitor::Visit(MobInfo_Prototype& prototype) {
    StoreInList(Storage->MobInfo, Current, prototype);
}
void StoreVisitor::Visit(MoveTo_Prototype& prototype) {
    StoreInList(Storage->MoveTo, Current, prototype);
}
void StoreVisitor::Visit(MovementPackCache_Prototype& prototype) {
    StoreInList(Storage->MovementPackCache, Current, prototype);
}
void StoreVisitor::Visit(MovingWall_Prototype& prototype) {
    StoreInList(Storage->MovingWall, Current, prototype);
}
void StoreVisitor::Visit(OnKillCallbacks_Prototype& prototype) {
    StoreInList(Storage->OnKillCallbacks, Current, prototype);
}
void StoreVisitor::Visit(OnTriggerEnter_Prototype& prototype) {
    StoreInList(Storage->OnTriggerEnter, Current, prototype);
}
void StoreVisitor::Visit(PlayerActor_Prototype& prototype) {
    StoreInList(Storage->PlayerActor, Current, prototype);
}
void StoreVisitor::Visit(PowerUp_Prototype& prototype) {
    StoreInList(Storage->PowerUp, Current, prototype);
}
void StoreVisitor::Visit(PowerUpUser_Prototype& prototype) {
    StoreInList(Storage->PowerUpUser, Current, prototype);
}
void StoreVisitor::Visit(PreciseHitReceiver_Prototype& prototype) {
    StoreInList(Storage->PreciseHitReceiver, Current, prototype);
}
void StoreVisitor::Visit(PrioritisedTargetHandler_Prototype& prototype) {
    StoreInList(Storage->PrioritisedTargetHandler, Current, prototype);
}
void StoreVisitor::Visit(Projectile_Prototype& prototype) {
    StoreInList(Storage->Projectile, Current, prototype);
}
void StoreVisitor::Visit(ProjectileSpawner_Prototype& prototype) {
    StoreInList(Storage->ProjectileSpawner, Current, prototype);
}
void StoreVisitor::Visit(Pushable_Prototype& prototype) {
    StoreInList(Storage->Pushable, Current, prototype);
}
void StoreVisitor::Visit(PushableTrigger_Prototype& prototype) {
    StoreInList(Storage->PushableTrigger, Current, prototype);
}
void StoreVisitor::Visit(Pusher_Prototype& prototype) {
    StoreInList(Storage->Pusher, Current, prototype);
}
void StoreVisitor::Visit(Rail_Prototype& prototype) {
    StoreInList(Storage->Rail, Current, prototype);
}
void StoreVisitor::Visit(RailCollider_Prototype& prototype) {
    StoreInList(Storage->RailCollider, Current, prototype);
}
void StoreVisitor::Visit(RankingManager_Prototype& prototype) {
    StoreInList(Storage->RankingManager, Current, prototype);
}
void StoreVisitor::Visit(RecoverAmmoOnDashEnhancement_Prototype& prototype) {
    StoreInList(Storage->RecoverAmmoOnDashEnhancement, Current, prototype);
}
void StoreVisitor::Visit(ReplaceMultiShotConfigsEnhancement_Prototype& prototype) {
    StoreInList(Storage->ReplaceMultiShotConfigsEnhancement, Current, prototype);
}
void StoreVisitor::Visit(RewardReceiver_Prototype& prototype) {
    StoreInList(Storage->RewardReceiver, Current, prototype);
}
void StoreVisitor::Visit(Roll_Prototype& prototype) {
    StoreInList(Storage->Roll, Current, prototype);
}
void StoreVisitor::Visit(RunnableWall_Prototype& prototype) {
    StoreInList(Storage->RunnableWall, Current, prototype);
}
void StoreVisitor::Visit(RunnableWallTriggerZone_Prototype& prototype) {
    StoreInList(Storage->RunnableWallTriggerZone, Current, prototype);
}
void StoreVisitor::Visit(RynoWeapon_Prototype& prototype) {
    StoreInList(Storage->RynoWeapon, Current, prototype);
}
void StoreVisitor::Visit(ShapeOverlapper_Prototype& prototype) {
    StoreInList(Storage->ShapeOverlapper, Current, prototype);
}
void StoreVisitor::Visit(ShieldedActor_Prototype& prototype) {
    StoreInList(Storage->ShieldedActor, Current, prototype);
}
void StoreVisitor::Visit(ShotgunBerserkerEnhancement_Prototype& prototype) {
    StoreInList(Storage->ShotgunBerserkerEnhancement, Current, prototype);
}
void StoreVisitor::Visit(ShotgunEnhancement_Prototype& prototype) {
    StoreInList(Storage->ShotgunEnhancement, Current, prototype);
}
void StoreVisitor::Visit(ShrinkingZone_Prototype& prototype) {
    StoreInList(Storage->ShrinkingZone, Current, prototype);
}
void StoreVisitor::Visit(ShrinkingZoneActor_Prototype& prototype) {
    StoreInList(Storage->ShrinkingZoneActor, Current, prototype);
}
void StoreVisitor::Visit(SpawnProjectilesOnDestroy_Prototype& prototype) {
    StoreInList(Storage->SpawnProjectilesOnDestroy, Current, prototype);
}
void StoreVisitor::Visit(Speeder_Prototype& prototype) {
    StoreInList(Storage->Speeder, Current, prototype);
}
void StoreVisitor::Visit(StatsManager_Prototype& prototype) {
    StoreInList(Storage->StatsManager, Current, prototype);
}
void StoreVisitor::Visit(StreamEffect_Prototype& prototype) {
    StoreInList(Storage->StreamEffect, Current, prototype);
}
void StoreVisitor::Visit(StreamEffectSpawner_Prototype& prototype) {
    StoreInList(Storage->StreamEffectSpawner, Current, prototype);
}
void StoreVisitor::Visit(SweepAttack_Prototype& prototype) {
    StoreInList(Storage->SweepAttack, Current, prototype);
}
void StoreVisitor::Visit(TargetFinder_Prototype& prototype) {
    StoreInList(Storage->TargetFinder, Current, prototype);
}
void StoreVisitor::Visit(TargetHolder_Prototype& prototype) {
    StoreInList(Storage->TargetHolder, Current, prototype);
}
void StoreVisitor::Visit(Targetable_Prototype& prototype) {
    StoreInList(Storage->Targetable, Current, prototype);
}
void StoreVisitor::Visit(TargetedEffect_Prototype& prototype) {
    StoreInList(Storage->TargetedEffect, Current, prototype);
}
void StoreVisitor::Visit(Team_Prototype& prototype) {
    StoreInList(Storage->Team, Current, prototype);
}
void StoreVisitor::Visit(TeslaClawMoreBeam_Prototype& prototype) {
    StoreInList(Storage->TeslaClawMoreBeam, Current, prototype);
}
void StoreVisitor::Visit(TimeScale_Prototype& prototype) {
    StoreInList(Storage->TimeScale, Current, prototype);
}
void StoreVisitor::Visit(TrailSpawner_Prototype& prototype) {
    StoreInList(Storage->TrailSpawner, Current, prototype);
}
void StoreVisitor::Visit(TravolatorActor_Prototype& prototype) {
    StoreInList(Storage->TravolatorActor, Current, prototype);
}
void StoreVisitor::Visit(TurnTo_Prototype& prototype) {
    StoreInList(Storage->TurnTo, Current, prototype);
}
void StoreVisitor::Visit(TutorialGameMode_Prototype& prototype) {
    StoreInList(Storage->TutorialGameMode, Current, prototype);
}
void StoreVisitor::Visit(UTAgent_Prototype& prototype) {
    StoreInList(Storage->UTAgent, Current, prototype);
}
void StoreVisitor::Visit(UltimateAbilityUser_Prototype& prototype) {
    StoreInList(Storage->UltimateAbilityUser, Current, prototype);
}
void StoreVisitor::Visit(UltimateEvolution_Prototype& prototype) {
    StoreInList(Storage->UltimateEvolution, Current, prototype);
}
void StoreVisitor::Visit(UltimateTank_Prototype& prototype) {
    StoreInList(Storage->UltimateTank, Current, prototype);
}
void StoreVisitor::Visit(UnlockedFeaturesManager_Prototype& prototype) {
    StoreInList(Storage->UnlockedFeaturesManager, Current, prototype);
}
void StoreVisitor::Visit(VoidRepulserShield_Prototype& prototype) {
    StoreInList(Storage->VoidRepulserShield, Current, prototype);
}
void StoreVisitor::Visit(VoidRepulserSpeedBoostEnhancement_Prototype& prototype) {
    StoreInList(Storage->VoidRepulserSpeedBoostEnhancement, Current, prototype);
}
void StoreVisitor::Visit(VoidRepulserVoidWaveEnhancement_Prototype& prototype) {
    StoreInList(Storage->VoidRepulserVoidWaveEnhancement, Current, prototype);
}
void StoreVisitor::Visit(VoidRepulserWeapon_Prototype& prototype) {
    StoreInList(Storage->VoidRepulserWeapon, Current, prototype);
}
void StoreVisitor::Visit(WakeUpSurroundingsOnPushOrDestroy_Prototype& prototype) {
    StoreInList(Storage->WakeUpSurroundingsOnPushOrDestroy, Current, prototype);
}
void StoreVisitor::Visit(WallRunningActor_Prototype& prototype) {
    StoreInList(Storage->WallRunningActor, Current, prototype);
}
void StoreVisitor::Visit(WarmongerMiniRocketsEnhancement_Prototype& prototype) {
    StoreInList(Storage->WarmongerMiniRocketsEnhancement, Current, prototype);
}
void StoreVisitor::Visit(WaveElement_Prototype& prototype) {
    StoreInList(Storage->WaveElement, Current, prototype);
}
void StoreVisitor::Visit(WaveManager_Prototype& prototype) {
    StoreInList(Storage->WaveManager, Current, prototype);
}
void StoreVisitor::Visit(WaveSpawner_Prototype& prototype) {
    StoreInList(Storage->WaveSpawner, Current, prototype);
}
void StoreVisitor::Visit(Weapon_Prototype& prototype) {
    StoreInList(Storage->Weapon, Current, prototype);
}
void StoreVisitor::Visit(WeaponBehaviour_Charging_Prototype& prototype) {
    StoreInList(Storage->WeaponBehaviour_Charging, Current, prototype);
}
void StoreVisitor::Visit(WeaponLoadoutActor_Prototype& prototype) {
    StoreInList(Storage->WeaponLoadoutActor, Current, prototype);
}
void StoreVisitor::Visit(WeaponProjectileSwitcherEnhancement_Prototype& prototype) {
    StoreInList(Storage->WeaponProjectileSwitcherEnhancement, Current, prototype);
}
void StoreVisitor::Visit(WeaponShowcaseGameMode_Prototype& prototype) {
    StoreInList(Storage->WeaponShowcaseGameMode, Current, prototype);
}
void StoreVisitor::Visit(WeaponUser_Prototype& prototype) {
    StoreInList(Storage->WeaponUser, Current, prototype);
}
void StoreVisitor::Visit(WrenchThrowProjectile_Prototype& prototype) {
    StoreInList(Storage->WrenchThrowProjectile, Current, prototype);
}
void StoreVisitor::Visit(WrenchThrowWeapon_Prototype& prototype) {
    StoreInList(Storage->WrenchThrowWeapon, Current, prototype);
}
void StoreVisitor::Visit(WrenchWeapon_Prototype& prototype) {
    StoreInList(Storage->WrenchWeapon, Current, prototype);
}
void StoreVisitor::Visit(ZoneControlGameMode_Prototype& prototype) {
    StoreInList(Storage->ZoneControlGameMode, Current, prototype);
}
void StoreVisitor::Visit(CharacterController2D_Prototype& prototype) {
    StoreInList(Storage->CharacterController2D, Current, prototype);
}
void StoreVisitor::Visit(CharacterController3D_Prototype& prototype) {
    StoreInList(Storage->CharacterController3D, Current, prototype);
}
void StoreVisitor::Visit(NavMeshAvoidanceAgent_Prototype& prototype) {
    StoreInList(Storage->NavMeshAvoidanceAgent, Current, prototype);
}
void StoreVisitor::Visit(NavMeshAvoidanceObstacle_Prototype& prototype) {
    StoreInList(Storage->NavMeshAvoidanceObstacle, Current, prototype);
}
void StoreVisitor::Visit(NavMeshPathfinder_Prototype& prototype) {
    StoreInList(Storage->NavMeshPathfinder, Current, prototype);
}
void StoreVisitor::Visit(NavMeshSteeringAgent_Prototype& prototype) {
    StoreInList(Storage->NavMeshSteeringAgent, Current, prototype);
}
void StoreVisitor::Visit(PhysicsBody2D_Prototype& prototype) {
    StoreInList(Storage->PhysicsBody2D, Current, prototype);
}
void StoreVisitor::Visit(PhysicsBody3D_Prototype& prototype) {
    StoreInList(Storage->PhysicsBody3D, Current, prototype);
}
void StoreVisitor::Visit(PhysicsCollider2D_Prototype& prototype) {
    StoreInList(Storage->PhysicsCollider2D, Current, prototype);
}
void StoreVisitor::Visit(PhysicsCollider3D_Prototype& prototype) {
    StoreInList(Storage->PhysicsCollider3D, Current, prototype);
}
void StoreVisitor::Visit(PhysicsCallbacks2D_Prototype& prototype) {
    StoreInList(Storage->PhysicsCallbacks2D, Current, prototype);
}
void StoreVisitor::Visit(PhysicsCallbacks3D_Prototype& prototype) {
    StoreInList(Storage->PhysicsCallbacks3D, Current, prototype);
}
void StoreVisitor::Visit(Transform2D_Prototype& prototype) {
    StoreInList(Storage->Transform2D, Current, prototype);
}
void StoreVisitor::Visit(Transform2DVertical_Prototype& prototype) {
    StoreInList(Storage->Transform2DVertical, Current, prototype);
}
void StoreVisitor::Visit(Transform3D_Prototype& prototype) {
    StoreInList(Storage->Transform3D, Current, prototype);
}
void StoreVisitor::Visit(View_Prototype& prototype) {
    StoreInList(Storage->View, Current, prototype);
}
void StoreVisitor::Visit(PhysicsJoints2D_Prototype& prototype) {
    StoreInList(Storage->PhysicsJoints2D, Current, prototype);
}
void StoreVisitor::Visit(PhysicsJoints3D_Prototype& prototype) {
    StoreInList(Storage->PhysicsJoints3D, Current, prototype);
}

} // namespace Quantum::Prototypes
