// flat_entity_prototype_container.h - entity prototypes stored as one list per component
// prototype type (Quantum.Prototypes.FlatEntityPrototypeContainer, quantum.code.dll).
//
// This is the serialised form of an entity prototype: maps store their baked entities this
// way. Collect gathers every prototype into one list (framework components first, then the
// game's, each type in its own list order); Store sorts prototypes into the lists.
#pragma once

#include "quantum_code/prototypes/component_prototype_visitor.h"
#include "quantum_code/prototypes/prototypes.h"

#include <memory>
#include <vector>

namespace Quantum::Prototypes {

class FlatEntityPrototypeContainer {
  public:
    template <class T> using List = std::vector<std::shared_ptr<T>>;

    List<AIBlackboardComponent_Prototype> AIBlackboardComponent;
    List<AIMoveForwardBehaviour_Prototype> AIMoveForwardBehaviour;
    List<AINavigationInitialiser_Prototype> AINavigationInitialiser;
    List<AISpawner_Prototype> AISpawner;
    List<ActorInputInitialiser_Prototype> ActorInputInitialiser;
    List<ActorManager_Prototype> ActorManager;
    List<ActorStatus_Prototype> ActorStatus;
    List<ActorStatusModifier_Prototype> ActorStatusModifier;
    List<ActorStunned_Prototype> ActorStunned;
    List<AddDamageIncreaseOnTargetedEffect_Prototype> AddDamageIncreaseOnTargetedEffect;
    List<AddEntitiesToSpawnOnProjectileDeath_Prototype> AddEntitiesToSpawnOnProjectileDeath;
    List<AgentOfDoom_Prototype> AgentOfDoom;
    List<AgentOfDoomGrouped_Prototype> AgentOfDoomGrouped;
    List<AmoeboidLauncherEnhancementPuddle_Prototype> AmoeboidLauncherEnhancementPuddle;
    List<AoeTrail_Prototype> AoeTrail;
    List<AreaOfEffect_Prototype> AreaOfEffect;
    List<AreaOfEffectLevelUp_Prototype> AreaOfEffectLevelUp;
    List<AreaOfEffectRaycastInfo_Prototype> AreaOfEffectRaycastInfo;
    List<ArenaCannon_Prototype> ArenaCannon;
    List<AttackInfo_Prototype> AttackInfo;
    List<BTAgent_Prototype> BTAgent;
    List<BattleStatsActor_Prototype> BattleStatsActor;
    List<BigBoomMine_Prototype> BigBoomMine;
    List<BigBoomWeapon_Prototype> BigBoomWeapon;
    List<BlackboardInitializer_Prototype> BlackboardInitializer;
    List<BlastBall_Prototype> BlastBall;
    List<BlastBallGame_Prototype> BlastBallGame;
    List<BlastBallGoal_Prototype> BlastBallGoal;
    List<BlastBallPlayer_Prototype> BlastBallPlayer;
    List<BlasterMoveSpeedEnhancement_Prototype> BlasterMoveSpeedEnhancement;
    List<BlasterSpecialShotEnhancement_Prototype> BlasterSpecialShotEnhancement;
    List<BoltCounterElement_Prototype> BoltCounterElement;
    List<BoltGathererActor_Prototype> BoltGathererActor;
    List<BoltPowerActor_Prototype> BoltPowerActor;
    List<BombardierDrone_Prototype> BombardierDrone;
    List<BombardierWeapon_Prototype> BombardierWeapon;
    List<Bot_Prototype> Bot;
    List<Bumper_Prototype> Bumper;
    List<BumperBar_Prototype> BumperBar;
    List<BuzzbladesRicochet_Prototype> BuzzbladesRicochet;
    List<CharacterColliderScaler_Prototype> CharacterColliderScaler;
    List<ChargeAttack_Prototype> ChargeAttack;
    List<Checkpoint_Prototype> Checkpoint;
    List<ColliderResize_Prototype> ColliderResize;
    List<CollisionForce_Prototype> CollisionForce;
    List<CombatParticipant_Prototype> CombatParticipant;
    List<CompoundHFSM_Prototype> CompoundHFSM;
    List<Cryoslider_Prototype> Cryoslider;
    List<CryosliderWeapon_Prototype> CryosliderWeapon;
    List<DamageChargeable_Prototype> DamageChargeable;
    List<DamageIncreasedOnTargetsEnhancement_Prototype> DamageIncreasedOnTargetsEnhancement;
    List<DamageMultiplier_Prototype> DamageMultiplier;
    List<DamageMultiplierHeroImprovement_Prototype> DamageMultiplierHeroImprovement;
    List<DamageSource_Prototype> DamageSource;
    List<DamageToImpulseConverter_Prototype> DamageToImpulseConverter;
    List<Damageable_Prototype> Damageable;
    List<DashActor_Prototype> DashActor;
    List<DrillDash_Prototype> DrillDash;
    List<DroppableCollector_Prototype> DroppableCollector;
    List<DroppableEntity_Prototype> DroppableEntity;
    List<DroppableManager_Prototype> DroppableManager;
    List<ElementalBuildUpMultiplier_Prototype> ElementalBuildUpMultiplier;
    List<EliminationGameMode_Prototype> EliminationGameMode;
    List<EnergyCellManager_Prototype> EnergyCellManager;
    List<EnforcerFirstShotBonusEnhancement_Prototype> EnforcerFirstShotBonusEnhancement;
    List<EntityDropper_Prototype> EntityDropper;
    List<EntityInfo_Prototype> EntityInfo;
    List<FireReactor_Prototype> FireReactor;
    List<GOAPAgent_Prototype> GOAPAgent;
    List<GOAPData_Prototype> GOAPData;
    List<GadgetUser_Prototype> GadgetUser;
    List<GameManager_Prototype> GameManager;
    List<GameModeActivator_Prototype> GameModeActivator;
    List<GameModeHandler_Prototype> GameModeHandler;
    List<GameModifiersManager_Prototype> GameModifiersManager;
    List<GatlingWeapon_Prototype> GatlingWeapon;
    List<GloveOfDoomWeapon_Prototype> GloveOfDoomWeapon;
    List<GoldenBolt_Prototype> GoldenBolt;
    List<GoldenBoltManager_Prototype> GoldenBoltManager;
    List<GrinderActor_Prototype> GrinderActor;
    List<GroundDashActor_Prototype> GroundDashActor;
    List<HFSMAgent_Prototype> HFSMAgent;
    List<HeadHunterWallHackEnhancement_Prototype> HeadHunterWallHackEnhancement;
    List<HealSource_Prototype> HealSource;
    List<Health_Prototype> Health;
    List<HelipackActor_Prototype> HelipackActor;
    List<Hierarchy_Prototype> Hierarchy;
    List<HoloshieldGloveHealZoneEnhancement_Prototype> HoloshieldGloveHealZoneEnhancement;
    List<HoloshieldGloveShield_Prototype> HoloshieldGloveShield;
    List<HoloshieldGloveShields_Prototype> HoloshieldGloveShields;
    List<HoloshieldGloveShieldsGrouped_Prototype> HoloshieldGloveShieldsGrouped;
    List<HoloshieldGloveWeapon_Prototype> HoloshieldGloveWeapon;
    List<HoverBootsWeapon_Prototype> HoverBootsWeapon;
    List<ItemSpawner_Prototype> ItemSpawner;
    List<JumpAttack_Prototype> JumpAttack;
    List<KccExtension_Prototype> KccExtension;
    List<KccModifier_Prototype> KccModifier;
    List<KillZone_Prototype> KillZone;
    List<Killable_Prototype> Killable;
    List<LedgeGrabbingActor_Prototype> LedgeGrabbingActor;
    List<LevelButton_Prototype> LevelButton;
    List<LevelButtonGameplayElement_Prototype> LevelButtonGameplayElement;
    List<LevelHolder_Prototype> LevelHolder;
    List<LevelUpActor_Prototype> LevelUpActor;
    List<Lifetime_Prototype> Lifetime;
    List<MeleeAttack_Prototype> MeleeAttack;
    List<MeleeJumpAttack_Prototype> MeleeJumpAttack;
    List<MineLauncherEnhancement_Prototype> MineLauncherEnhancement;
    List<MineMerger_Prototype> MineMerger;
    List<MobGroup_Prototype> MobGroup;
    List<MobGroupMember_Prototype> MobGroupMember;
    List<MobInfo_Prototype> MobInfo;
    List<MoveTo_Prototype> MoveTo;
    List<MovementPackCache_Prototype> MovementPackCache;
    List<MovingWall_Prototype> MovingWall;
    List<OnKillCallbacks_Prototype> OnKillCallbacks;
    List<OnTriggerEnter_Prototype> OnTriggerEnter;
    List<PlayerActor_Prototype> PlayerActor;
    List<PowerUp_Prototype> PowerUp;
    List<PowerUpUser_Prototype> PowerUpUser;
    List<PreciseHitReceiver_Prototype> PreciseHitReceiver;
    List<PrioritisedTargetHandler_Prototype> PrioritisedTargetHandler;
    List<Projectile_Prototype> Projectile;
    List<ProjectileSpawner_Prototype> ProjectileSpawner;
    List<Pushable_Prototype> Pushable;
    List<PushableTrigger_Prototype> PushableTrigger;
    List<Pusher_Prototype> Pusher;
    List<Rail_Prototype> Rail;
    List<RailCollider_Prototype> RailCollider;
    List<RankingManager_Prototype> RankingManager;
    List<RecoverAmmoOnDashEnhancement_Prototype> RecoverAmmoOnDashEnhancement;
    List<ReplaceMultiShotConfigsEnhancement_Prototype> ReplaceMultiShotConfigsEnhancement;
    List<RewardReceiver_Prototype> RewardReceiver;
    List<Roll_Prototype> Roll;
    List<RunnableWall_Prototype> RunnableWall;
    List<RunnableWallTriggerZone_Prototype> RunnableWallTriggerZone;
    List<RynoWeapon_Prototype> RynoWeapon;
    List<ShapeOverlapper_Prototype> ShapeOverlapper;
    List<ShieldedActor_Prototype> ShieldedActor;
    List<ShotgunBerserkerEnhancement_Prototype> ShotgunBerserkerEnhancement;
    List<ShotgunEnhancement_Prototype> ShotgunEnhancement;
    List<ShrinkingZone_Prototype> ShrinkingZone;
    List<ShrinkingZoneActor_Prototype> ShrinkingZoneActor;
    List<SpawnProjectilesOnDestroy_Prototype> SpawnProjectilesOnDestroy;
    List<Speeder_Prototype> Speeder;
    List<StatsManager_Prototype> StatsManager;
    List<StreamEffect_Prototype> StreamEffect;
    List<StreamEffectSpawner_Prototype> StreamEffectSpawner;
    List<SweepAttack_Prototype> SweepAttack;
    List<TargetFinder_Prototype> TargetFinder;
    List<TargetHolder_Prototype> TargetHolder;
    List<Targetable_Prototype> Targetable;
    List<TargetedEffect_Prototype> TargetedEffect;
    List<Team_Prototype> Team;
    List<TeslaClawMoreBeam_Prototype> TeslaClawMoreBeam;
    List<TimeScale_Prototype> TimeScale;
    List<TrailSpawner_Prototype> TrailSpawner;
    List<TravolatorActor_Prototype> TravolatorActor;
    List<TurnTo_Prototype> TurnTo;
    List<TutorialGameMode_Prototype> TutorialGameMode;
    List<UTAgent_Prototype> UTAgent;
    List<UltimateAbilityUser_Prototype> UltimateAbilityUser;
    List<UltimateEvolution_Prototype> UltimateEvolution;
    List<UltimateTank_Prototype> UltimateTank;
    List<UnlockedFeaturesManager_Prototype> UnlockedFeaturesManager;
    List<VoidRepulserShield_Prototype> VoidRepulserShield;
    List<VoidRepulserSpeedBoostEnhancement_Prototype> VoidRepulserSpeedBoostEnhancement;
    List<VoidRepulserVoidWaveEnhancement_Prototype> VoidRepulserVoidWaveEnhancement;
    List<VoidRepulserWeapon_Prototype> VoidRepulserWeapon;
    List<WakeUpSurroundingsOnPushOrDestroy_Prototype> WakeUpSurroundingsOnPushOrDestroy;
    List<WallRunningActor_Prototype> WallRunningActor;
    List<WarmongerMiniRocketsEnhancement_Prototype> WarmongerMiniRocketsEnhancement;
    List<WaveElement_Prototype> WaveElement;
    List<WaveManager_Prototype> WaveManager;
    List<WaveSpawner_Prototype> WaveSpawner;
    List<Weapon_Prototype> Weapon;
    List<WeaponBehaviour_Charging_Prototype> WeaponBehaviour_Charging;
    List<WeaponLoadoutActor_Prototype> WeaponLoadoutActor;
    List<WeaponProjectileSwitcherEnhancement_Prototype> WeaponProjectileSwitcherEnhancement;
    List<WeaponShowcaseGameMode_Prototype> WeaponShowcaseGameMode;
    List<WeaponUser_Prototype> WeaponUser;
    List<WrenchThrowProjectile_Prototype> WrenchThrowProjectile;
    List<WrenchThrowWeapon_Prototype> WrenchThrowWeapon;
    List<WrenchWeapon_Prototype> WrenchWeapon;
    List<ZoneControlGameMode_Prototype> ZoneControlGameMode;
    List<CharacterController2D_Prototype> CharacterController2D;
    List<CharacterController3D_Prototype> CharacterController3D;
    List<NavMeshAvoidanceAgent_Prototype> NavMeshAvoidanceAgent;
    List<NavMeshAvoidanceObstacle_Prototype> NavMeshAvoidanceObstacle;
    List<NavMeshPathfinder_Prototype> NavMeshPathfinder;
    List<NavMeshSteeringAgent_Prototype> NavMeshSteeringAgent;
    List<PhysicsBody2D_Prototype> PhysicsBody2D;
    List<PhysicsBody3D_Prototype> PhysicsBody3D;
    List<PhysicsCollider2D_Prototype> PhysicsCollider2D;
    List<PhysicsCollider3D_Prototype> PhysicsCollider3D;
    List<PhysicsCallbacks2D_Prototype> PhysicsCallbacks2D;
    List<PhysicsCallbacks3D_Prototype> PhysicsCallbacks3D;
    List<Transform2D_Prototype> Transform2D;
    List<Transform2DVertical_Prototype> Transform2DVertical;
    List<Transform3D_Prototype> Transform3D;
    List<View_Prototype> View;
    List<PhysicsJoints2D_Prototype> PhysicsJoints2D;
    List<PhysicsJoints3D_Prototype> PhysicsJoints3D;

    // 0x7c30e60 - every list empty.
    FlatEntityPrototypeContainer() = default;

    // 0x7c2eb10 - Appends the framework components' prototypes, then the game's
    // (CollectGen), to `target`.
    void Collect(std::vector<std::shared_ptr<ComponentPrototype>>& target) const;
    // 0x7c2ee38 - Appends the game components' prototypes to `target`.
    void CollectGen(std::vector<std::shared_ptr<ComponentPrototype>>& target) const;
    // 0x7c30b6c - Adds each prototype to the list of its type.
    void Store(const std::vector<std::shared_ptr<ComponentPrototype>>& prototypes);
};

// Sorts dispatched prototypes into a container's lists (StoreVisitor).
class StoreVisitor final : public ComponentPrototypeVisitor {
  public:
    using ComponentPrototypeVisitor::Visit;

    FlatEntityPrototypeContainer* Storage = nullptr;
    // The prototype being stored (the visitor receives it by reference; the lists share it).
    std::shared_ptr<ComponentPrototype> Current;

    // 0x7c30e58
    StoreVisitor() = default;

    // 0x7c30e68
    void Visit(AIBlackboardComponent_Prototype& prototype) override;
    // 0x7c30ec4
    void Visit(AIMoveForwardBehaviour_Prototype& prototype) override;
    // 0x7c30f20
    void Visit(AINavigationInitialiser_Prototype& prototype) override;
    // 0x7c30f7c
    void Visit(AISpawner_Prototype& prototype) override;
    // 0x7c30fd8
    void Visit(ActorInputInitialiser_Prototype& prototype) override;
    // 0x7c31034
    void Visit(ActorManager_Prototype& prototype) override;
    // 0x7c31090
    void Visit(ActorStatus_Prototype& prototype) override;
    // 0x7c310ec
    void Visit(ActorStatusModifier_Prototype& prototype) override;
    // 0x7c31148
    void Visit(ActorStunned_Prototype& prototype) override;
    // 0x7c311a4
    void Visit(AddDamageIncreaseOnTargetedEffect_Prototype& prototype) override;
    // 0x7c31200
    void Visit(AddEntitiesToSpawnOnProjectileDeath_Prototype& prototype) override;
    // 0x7c3125c
    void Visit(AgentOfDoom_Prototype& prototype) override;
    // 0x7c312b8
    void Visit(AgentOfDoomGrouped_Prototype& prototype) override;
    // 0x7c31314
    void Visit(AmoeboidLauncherEnhancementPuddle_Prototype& prototype) override;
    // 0x7c31370
    void Visit(AoeTrail_Prototype& prototype) override;
    // 0x7c313cc
    void Visit(AreaOfEffect_Prototype& prototype) override;
    // 0x7c31428
    void Visit(AreaOfEffectLevelUp_Prototype& prototype) override;
    // 0x7c31484
    void Visit(AreaOfEffectRaycastInfo_Prototype& prototype) override;
    // 0x7c314e0
    void Visit(ArenaCannon_Prototype& prototype) override;
    // 0x7c3153c
    void Visit(AttackInfo_Prototype& prototype) override;
    // 0x7c31598
    void Visit(BTAgent_Prototype& prototype) override;
    // 0x7c315f4
    void Visit(BattleStatsActor_Prototype& prototype) override;
    // 0x7c31650
    void Visit(BigBoomMine_Prototype& prototype) override;
    // 0x7c316ac
    void Visit(BigBoomWeapon_Prototype& prototype) override;
    // 0x7c31708
    void Visit(BlackboardInitializer_Prototype& prototype) override;
    // 0x7c31764
    void Visit(BlastBall_Prototype& prototype) override;
    // 0x7c317c0
    void Visit(BlastBallGame_Prototype& prototype) override;
    // 0x7c3181c
    void Visit(BlastBallGoal_Prototype& prototype) override;
    // 0x7c31878
    void Visit(BlastBallPlayer_Prototype& prototype) override;
    // 0x7c318d4
    void Visit(BlasterMoveSpeedEnhancement_Prototype& prototype) override;
    // 0x7c31930
    void Visit(BlasterSpecialShotEnhancement_Prototype& prototype) override;
    // 0x7c3198c
    void Visit(BoltCounterElement_Prototype& prototype) override;
    // 0x7c319e8
    void Visit(BoltGathererActor_Prototype& prototype) override;
    // 0x7c31a44
    void Visit(BoltPowerActor_Prototype& prototype) override;
    // 0x7c31aa0
    void Visit(BombardierDrone_Prototype& prototype) override;
    // 0x7c31afc
    void Visit(BombardierWeapon_Prototype& prototype) override;
    // 0x7c31b58
    void Visit(Bot_Prototype& prototype) override;
    // 0x7c31bb4
    void Visit(Bumper_Prototype& prototype) override;
    // 0x7c31c10
    void Visit(BumperBar_Prototype& prototype) override;
    // 0x7c31c6c
    void Visit(BuzzbladesRicochet_Prototype& prototype) override;
    // 0x7c31cc8
    void Visit(CharacterColliderScaler_Prototype& prototype) override;
    // 0x7c31d24
    void Visit(ChargeAttack_Prototype& prototype) override;
    // 0x7c31d80
    void Visit(Checkpoint_Prototype& prototype) override;
    // 0x7c31ddc
    void Visit(ColliderResize_Prototype& prototype) override;
    // 0x7c31e38
    void Visit(CollisionForce_Prototype& prototype) override;
    // 0x7c31e94
    void Visit(CombatParticipant_Prototype& prototype) override;
    // 0x7c31ef0
    void Visit(CompoundHFSM_Prototype& prototype) override;
    // 0x7c31f4c
    void Visit(Cryoslider_Prototype& prototype) override;
    // 0x7c31fa8
    void Visit(CryosliderWeapon_Prototype& prototype) override;
    // 0x7c32004
    void Visit(DamageChargeable_Prototype& prototype) override;
    // 0x7c32060
    void Visit(DamageIncreasedOnTargetsEnhancement_Prototype& prototype) override;
    // 0x7c320bc
    void Visit(DamageMultiplier_Prototype& prototype) override;
    // 0x7c32118
    void Visit(DamageMultiplierHeroImprovement_Prototype& prototype) override;
    // 0x7c32174
    void Visit(DamageSource_Prototype& prototype) override;
    // 0x7c321d0
    void Visit(DamageToImpulseConverter_Prototype& prototype) override;
    // 0x7c3222c
    void Visit(Damageable_Prototype& prototype) override;
    // 0x7c32288
    void Visit(DashActor_Prototype& prototype) override;
    // 0x7c322e4
    void Visit(DrillDash_Prototype& prototype) override;
    // 0x7c32340
    void Visit(DroppableCollector_Prototype& prototype) override;
    // 0x7c3239c
    void Visit(DroppableEntity_Prototype& prototype) override;
    // 0x7c323f8
    void Visit(DroppableManager_Prototype& prototype) override;
    // 0x7c32454
    void Visit(ElementalBuildUpMultiplier_Prototype& prototype) override;
    // 0x7c324b0
    void Visit(EliminationGameMode_Prototype& prototype) override;
    // 0x7c3250c
    void Visit(EnergyCellManager_Prototype& prototype) override;
    // 0x7c32568
    void Visit(EnforcerFirstShotBonusEnhancement_Prototype& prototype) override;
    // 0x7c325c4
    void Visit(EntityDropper_Prototype& prototype) override;
    // 0x7c32620
    void Visit(EntityInfo_Prototype& prototype) override;
    // 0x7c3267c
    void Visit(FireReactor_Prototype& prototype) override;
    // 0x7c326d8
    void Visit(GOAPAgent_Prototype& prototype) override;
    // 0x7c32734
    void Visit(GOAPData_Prototype& prototype) override;
    // 0x7c32790
    void Visit(GadgetUser_Prototype& prototype) override;
    // 0x7c327ec
    void Visit(GameManager_Prototype& prototype) override;
    // 0x7c32848
    void Visit(GameModeActivator_Prototype& prototype) override;
    // 0x7c328a4
    void Visit(GameModeHandler_Prototype& prototype) override;
    // 0x7c32900
    void Visit(GameModifiersManager_Prototype& prototype) override;
    // 0x7c3295c
    void Visit(GatlingWeapon_Prototype& prototype) override;
    // 0x7c329b8
    void Visit(GloveOfDoomWeapon_Prototype& prototype) override;
    // 0x7c32a14
    void Visit(GoldenBolt_Prototype& prototype) override;
    // 0x7c32a70
    void Visit(GoldenBoltManager_Prototype& prototype) override;
    // 0x7c32acc
    void Visit(GrinderActor_Prototype& prototype) override;
    // 0x7c32b28
    void Visit(GroundDashActor_Prototype& prototype) override;
    // 0x7c32b84
    void Visit(HFSMAgent_Prototype& prototype) override;
    // 0x7c32be0
    void Visit(HeadHunterWallHackEnhancement_Prototype& prototype) override;
    // 0x7c32c3c
    void Visit(HealSource_Prototype& prototype) override;
    // 0x7c32c98
    void Visit(Health_Prototype& prototype) override;
    // 0x7c32cf4
    void Visit(HelipackActor_Prototype& prototype) override;
    // 0x7c32d50
    void Visit(Hierarchy_Prototype& prototype) override;
    // 0x7c32dac
    void Visit(HoloshieldGloveHealZoneEnhancement_Prototype& prototype) override;
    // 0x7c32e08
    void Visit(HoloshieldGloveShield_Prototype& prototype) override;
    // 0x7c32e64
    void Visit(HoloshieldGloveShields_Prototype& prototype) override;
    // 0x7c32ec0
    void Visit(HoloshieldGloveShieldsGrouped_Prototype& prototype) override;
    // 0x7c32f1c
    void Visit(HoloshieldGloveWeapon_Prototype& prototype) override;
    // 0x7c32f78
    void Visit(HoverBootsWeapon_Prototype& prototype) override;
    // 0x7c32fd4
    void Visit(ItemSpawner_Prototype& prototype) override;
    // 0x7c33030
    void Visit(JumpAttack_Prototype& prototype) override;
    // 0x7c3308c
    void Visit(KccExtension_Prototype& prototype) override;
    // 0x7c330e8
    void Visit(KccModifier_Prototype& prototype) override;
    // 0x7c33144
    void Visit(KillZone_Prototype& prototype) override;
    // 0x7c331a0
    void Visit(Killable_Prototype& prototype) override;
    // 0x7c331fc
    void Visit(LedgeGrabbingActor_Prototype& prototype) override;
    // 0x7c33258
    void Visit(LevelButton_Prototype& prototype) override;
    // 0x7c332b4
    void Visit(LevelButtonGameplayElement_Prototype& prototype) override;
    // 0x7c33310
    void Visit(LevelHolder_Prototype& prototype) override;
    // 0x7c3336c
    void Visit(LevelUpActor_Prototype& prototype) override;
    // 0x7c333c8
    void Visit(Lifetime_Prototype& prototype) override;
    // 0x7c33424
    void Visit(MeleeAttack_Prototype& prototype) override;
    // 0x7c33480
    void Visit(MeleeJumpAttack_Prototype& prototype) override;
    // 0x7c334dc
    void Visit(MineLauncherEnhancement_Prototype& prototype) override;
    // 0x7c33538
    void Visit(MineMerger_Prototype& prototype) override;
    // 0x7c33594
    void Visit(MobGroup_Prototype& prototype) override;
    // 0x7c335f0
    void Visit(MobGroupMember_Prototype& prototype) override;
    // 0x7c3364c
    void Visit(MobInfo_Prototype& prototype) override;
    // 0x7c336a8
    void Visit(MoveTo_Prototype& prototype) override;
    // 0x7c33704
    void Visit(MovementPackCache_Prototype& prototype) override;
    // 0x7c33760
    void Visit(MovingWall_Prototype& prototype) override;
    // 0x7c337bc
    void Visit(OnKillCallbacks_Prototype& prototype) override;
    // 0x7c33818
    void Visit(OnTriggerEnter_Prototype& prototype) override;
    // 0x7c33874
    void Visit(PlayerActor_Prototype& prototype) override;
    // 0x7c338d0
    void Visit(PowerUp_Prototype& prototype) override;
    // 0x7c3392c
    void Visit(PowerUpUser_Prototype& prototype) override;
    // 0x7c33988
    void Visit(PreciseHitReceiver_Prototype& prototype) override;
    // 0x7c339e4
    void Visit(PrioritisedTargetHandler_Prototype& prototype) override;
    // 0x7c33a40
    void Visit(Projectile_Prototype& prototype) override;
    // 0x7c33a9c
    void Visit(ProjectileSpawner_Prototype& prototype) override;
    // 0x7c33af8
    void Visit(Pushable_Prototype& prototype) override;
    // 0x7c33b54
    void Visit(PushableTrigger_Prototype& prototype) override;
    // 0x7c33bb0
    void Visit(Pusher_Prototype& prototype) override;
    // 0x7c33c0c
    void Visit(Rail_Prototype& prototype) override;
    // 0x7c33c68
    void Visit(RailCollider_Prototype& prototype) override;
    // 0x7c33cc4
    void Visit(RankingManager_Prototype& prototype) override;
    // 0x7c33d20
    void Visit(RecoverAmmoOnDashEnhancement_Prototype& prototype) override;
    // 0x7c33d7c
    void Visit(ReplaceMultiShotConfigsEnhancement_Prototype& prototype) override;
    // 0x7c33dd8
    void Visit(RewardReceiver_Prototype& prototype) override;
    // 0x7c33e34
    void Visit(Roll_Prototype& prototype) override;
    // 0x7c33e90
    void Visit(RunnableWall_Prototype& prototype) override;
    // 0x7c33eec
    void Visit(RunnableWallTriggerZone_Prototype& prototype) override;
    // 0x7c33f48
    void Visit(RynoWeapon_Prototype& prototype) override;
    // 0x7c33fa4
    void Visit(ShapeOverlapper_Prototype& prototype) override;
    // 0x7c34000
    void Visit(ShieldedActor_Prototype& prototype) override;
    // 0x7c3405c
    void Visit(ShotgunBerserkerEnhancement_Prototype& prototype) override;
    // 0x7c340b8
    void Visit(ShotgunEnhancement_Prototype& prototype) override;
    // 0x7c34114
    void Visit(ShrinkingZone_Prototype& prototype) override;
    // 0x7c34170
    void Visit(ShrinkingZoneActor_Prototype& prototype) override;
    // 0x7c341cc
    void Visit(SpawnProjectilesOnDestroy_Prototype& prototype) override;
    // 0x7c34228
    void Visit(Speeder_Prototype& prototype) override;
    // 0x7c34284
    void Visit(StatsManager_Prototype& prototype) override;
    // 0x7c342e0
    void Visit(StreamEffect_Prototype& prototype) override;
    // 0x7c3433c
    void Visit(StreamEffectSpawner_Prototype& prototype) override;
    // 0x7c34398
    void Visit(SweepAttack_Prototype& prototype) override;
    // 0x7c343f4
    void Visit(TargetFinder_Prototype& prototype) override;
    // 0x7c34450
    void Visit(TargetHolder_Prototype& prototype) override;
    // 0x7c344ac
    void Visit(Targetable_Prototype& prototype) override;
    // 0x7c34508
    void Visit(TargetedEffect_Prototype& prototype) override;
    // 0x7c34564
    void Visit(Team_Prototype& prototype) override;
    // 0x7c345c0
    void Visit(TeslaClawMoreBeam_Prototype& prototype) override;
    // 0x7c3461c
    void Visit(TimeScale_Prototype& prototype) override;
    // 0x7c34678
    void Visit(TrailSpawner_Prototype& prototype) override;
    // 0x7c346d4
    void Visit(TravolatorActor_Prototype& prototype) override;
    // 0x7c34730
    void Visit(TurnTo_Prototype& prototype) override;
    // 0x7c3478c
    void Visit(TutorialGameMode_Prototype& prototype) override;
    // 0x7c347e8
    void Visit(UTAgent_Prototype& prototype) override;
    // 0x7c34844
    void Visit(UltimateAbilityUser_Prototype& prototype) override;
    // 0x7c348a0
    void Visit(UltimateEvolution_Prototype& prototype) override;
    // 0x7c348fc
    void Visit(UltimateTank_Prototype& prototype) override;
    // 0x7c34958
    void Visit(UnlockedFeaturesManager_Prototype& prototype) override;
    // 0x7c349b4
    void Visit(VoidRepulserShield_Prototype& prototype) override;
    // 0x7c34a10
    void Visit(VoidRepulserSpeedBoostEnhancement_Prototype& prototype) override;
    // 0x7c34a6c
    void Visit(VoidRepulserVoidWaveEnhancement_Prototype& prototype) override;
    // 0x7c34ac8
    void Visit(VoidRepulserWeapon_Prototype& prototype) override;
    // 0x7c34b24
    void Visit(WakeUpSurroundingsOnPushOrDestroy_Prototype& prototype) override;
    // 0x7c34b80
    void Visit(WallRunningActor_Prototype& prototype) override;
    // 0x7c34bdc
    void Visit(WarmongerMiniRocketsEnhancement_Prototype& prototype) override;
    // 0x7c34c38
    void Visit(WaveElement_Prototype& prototype) override;
    // 0x7c34c94
    void Visit(WaveManager_Prototype& prototype) override;
    // 0x7c34cf0
    void Visit(WaveSpawner_Prototype& prototype) override;
    // 0x7c34d4c
    void Visit(Weapon_Prototype& prototype) override;
    // 0x7c34da8
    void Visit(WeaponBehaviour_Charging_Prototype& prototype) override;
    // 0x7c34e04
    void Visit(WeaponLoadoutActor_Prototype& prototype) override;
    // 0x7c34e60
    void Visit(WeaponProjectileSwitcherEnhancement_Prototype& prototype) override;
    // 0x7c34ebc
    void Visit(WeaponShowcaseGameMode_Prototype& prototype) override;
    // 0x7c34f18
    void Visit(WeaponUser_Prototype& prototype) override;
    // 0x7c34f74
    void Visit(WrenchThrowProjectile_Prototype& prototype) override;
    // 0x7c34fd0
    void Visit(WrenchThrowWeapon_Prototype& prototype) override;
    // 0x7c3502c
    void Visit(WrenchWeapon_Prototype& prototype) override;
    // 0x7c35088
    void Visit(ZoneControlGameMode_Prototype& prototype) override;
    // 0x7c350e4
    void Visit(CharacterController2D_Prototype& prototype) override;
    // 0x7c35140
    void Visit(CharacterController3D_Prototype& prototype) override;
    // 0x7c3519c
    void Visit(NavMeshAvoidanceAgent_Prototype& prototype) override;
    // 0x7c351f8
    void Visit(NavMeshAvoidanceObstacle_Prototype& prototype) override;
    // 0x7c35254
    void Visit(NavMeshPathfinder_Prototype& prototype) override;
    // 0x7c352b0
    void Visit(NavMeshSteeringAgent_Prototype& prototype) override;
    // 0x7c3530c
    void Visit(PhysicsBody2D_Prototype& prototype) override;
    // 0x7c35368
    void Visit(PhysicsBody3D_Prototype& prototype) override;
    // 0x7c353c4
    void Visit(PhysicsCollider2D_Prototype& prototype) override;
    // 0x7c35420
    void Visit(PhysicsCollider3D_Prototype& prototype) override;
    // 0x7c3547c
    void Visit(PhysicsCallbacks2D_Prototype& prototype) override;
    // 0x7c354d8
    void Visit(PhysicsCallbacks3D_Prototype& prototype) override;
    // 0x7c35534
    void Visit(Transform2D_Prototype& prototype) override;
    // 0x7c35590
    void Visit(Transform2DVertical_Prototype& prototype) override;
    // 0x7c355ec
    void Visit(Transform3D_Prototype& prototype) override;
    // 0x7c35648
    void Visit(View_Prototype& prototype) override;
    // 0x7c356a4
    void Visit(PhysicsJoints2D_Prototype& prototype) override;
    // 0x7c35700
    void Visit(PhysicsJoints3D_Prototype& prototype) override;
};

} // namespace Quantum::Prototypes
