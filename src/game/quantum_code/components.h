// components.h - the component types of the gameplay simulation and the value types
// stored in them (quantum.code.dll).
//
// Layouts are the originals' field for field (offsets and sizes are checked below each
// type): component data is copied, serialised and checksummed as raw state, so the
// layout is part of the deterministic simulation. Members are declared in memory order;
// GetHashCode combines them in the original declaration order (hash = hash * 31 + field,
// starting from a per-type seed chosen by the original's code generator), and
// Serialize writes them in memory order.
//
// Collections live in the frame heap; a component stores their Ptr and offers typed
// accessors. Components whose collections are allocated on add / freed on remove
// register OnAdded / OnRemoved callbacks for that (ComponentTypeId registration is in
// frame_components.cpp).
#pragma once

#include "quantum_code/asset_refs.h"
#include "quantum_code/enums.h"
#include "quantum_code/value_support.h"
#include "quantum_code/core_types.h"

#include <array>
#include <cstdint>
#include <span>

namespace Quantum {

class Frame;
// Asset classes named by component methods (see systems/asset_decls.h).
class EnergyCellManagerSpec;
namespace Prototypes {
class Killable_Prototype;
}

// Every type below, so that collection accessors can name element types declared later.
struct AIBlackboardComponent;
struct AIMoveForwardBehaviour;
struct AINavigation;
struct AINavigationBlacklist;
struct AINavigationInitialiser;
struct AISpawnDone;
struct AISpawner;
struct ActorIced;
struct ActorInput;
struct ActorInputInitialiser;
struct ActorManager;
struct ActorSheeped;
struct ActorStatus;
struct ActorStatusChangeRequested;
struct ActorStatusModifier;
struct ActorStunned;
struct AddDamageIncreaseOnTargetedEffect;
struct AddEntitiesToSpawnOnProjectileDeath;
struct AddForceOnUltimate;
struct AgentOfDoom;
struct DoomGroupStats;
struct AgentOfDoomGroup;
struct AgentOfDoomGrouped;
struct AmoeboidLauncherEnhancementPuddle;
struct AoeTrail;
struct AreaOfEffect;
struct AreaOfEffectIgnitionShapeHolder;
struct AreaOfEffectLevelUp;
struct AreaOfEffectRaycastInfo;
struct ArenaCannon;
struct AttackInfo;
struct AutoDestroyOnMapChanged;
struct AutoJumperStartParams;
struct AutoJumperActor;
struct BTAgent;
struct BattleStatsActor;
struct BigBoomMine;
struct BigBoomWeapon;
struct BlackboardInitializer;
struct LastBallHitData;
struct BlastBall;
struct BlastBallTeamData;
struct BlastBallGame;
struct BlastBallGoal;
struct BlastBallPlayer;
struct BlasterMoveSpeedEnhancement;
struct BlasterSpecialShotEnhancement;
struct BoltCounterElement;
struct BoltGathererActor;
struct BoltPowerActor;
struct BombardierDrone;
struct BombardierWeapon;
struct BotAirSaveParams;
struct BotBlastBallParams;
struct BotShootingParams;
struct FireRequest;
struct ZoneControlBotParams;
struct Bot;
struct Bumper;
struct BumperBar;
struct BuzzbladesRicochet;
struct CharacterColliderScaler;
struct ChargeAttack;
struct CheatManager;
struct Cheats;
struct Checkpoint;
struct ColliderResize;
struct CollisionForce;
struct CombatParticipant;
struct HFSMData;
struct HFSMAgent;
struct RuntimeHFSM;
struct CompoundHFSM;
struct Cryoslider;
struct CryosliderWeapon;
struct DamageChargeable;
struct DamageIncreasedOnTargetsEnhancement;
struct DamageMultiplier;
struct DamageMultiplierHeroImprovement;
struct DamageSource;
struct DamageToImpulseConverter;
struct HitHealthData;
struct HitShieldData;
struct HitData;
struct Damageable;
struct DashActor;
struct DrillDash;
struct DroppableCollector;
struct DroppableEntity;
struct DroppableManager;
struct ElementalBuildUpMultiplier;
struct EliminationGameMode;
struct EnergyCellManager;
struct EnforcerFirstShotBonusEnhancement;
struct EntityDropper;
struct EntityGroupStats;
struct EntityGroup;
struct EntityGroupMemberGrouped;
struct FireReactor;
struct GOAPState;
struct GOAPAgent;
struct GOAPData;
struct GadgetStats;
struct GadgetUser;
struct GameManager;
struct GameModeActivator;
struct GameModeHandlerStats;
struct GameModeHandler;
struct GameModifiersManager;
struct GatlingEnhancement;
struct GatlingWeapon;
struct GloveOfDoomWeapon;
struct GoldenBolt;
struct GoldenBoltManager;
struct GrenadeLauncherAftershockEnhancement;
struct GrenadeLauncherStunGrenadeEnhancement;
struct GrinderActor;
struct GroundDashActor;
struct HeadHunterWallHackEnhancement;
struct HealSource;
struct HealthStats;
struct Health;
struct HelipackActor;
struct Hierarchy;
struct HoloshieldGloveHealZoneEnhancement;
struct HoloshieldGloveShield;
struct HoloshieldGloveShields;
struct HoloshieldGloveShieldsGrouped;
struct HoloshieldGloveWeapon;
struct HoverBootsWeapon;
struct ItemSpawner;
struct JumpAttack;
struct KccExtension;
struct KccModifier;
struct KillNextFrame;
struct KillZone;
struct KilledByData;
struct Killable;
struct LedgeGrabbingActor;
struct LevelButton;
struct LevelButtonGameplayElement;
struct LevelHolder;
struct LevelUpActor;
struct LifetimeStats;
struct Lifetime;
struct MeleeAttack;
struct MeleeJumpAttack;
struct MineLauncherEnhancement;
struct MineMerger;
struct MobGroup;
struct MobGroupMember;
struct MobInfo;
struct MoveTo;
struct MovementPackCache;
struct MovingWall;
struct OnKillCallbacks;
struct OnTriggerEnter;
struct Ownership;
struct ParabolaFollowerParams;
struct ParabolaFollower;
struct PendingPhysicsImpulse;
struct PushData;
struct PendingPushStop;
struct PlayerActor;
struct PlayerActorInitialized;
struct PowerUp;
struct PowerUpUser;
struct PreciseHitReceiver;
struct PrioritisedTarget;
struct PrioritisedTargetHandler;
struct Projectile;
struct ProjectileDebugInfo;
struct ProjectileHitInfo;
struct ProjectileSpawner;
struct PushedByData;
struct Pushable;
struct PushableTrigger;
struct Pusher;
struct Rail;
struct RailCollider;
struct RandomWeaponsActor;
struct RankingManager;
struct RecoverAmmoOnDashEnhancement;
struct ReplaceMultiShotConfigsEnhancement;
struct RewardReceiver;
struct Roll;
struct RunnableWall;
struct RunnableWallTriggerZone;
struct RynoStats;
struct RynoWeapon;
struct ShapeOverlapper;
struct ShieldedStats;
struct ShieldedActor;
struct ShieldGroupStats;
struct ShieldsGroup;
struct ShotgunBerserkerEnhancement;
struct ShotgunBerserkerEnhancementEffect;
struct ShotgunEnhancement;
struct ShotgunEnhancementEffect;
struct ShrinkingZone;
struct ShrinkingZoneActor;
struct SpawnProjectilesOnDestroy;
struct Spawned;
struct SpawnedGoldenBoltData;
struct Speeder;
struct StatsManager;
struct StreamEffect;
struct StreamEffectSpawner;
struct SweepAttack;
struct PotentialTarget;
struct TargetFinder;
struct TargetHolder;
struct Targetable;
struct TargetedEffect;
struct Team;
struct TeslaClawMoreBeam;
struct TimeScale;
struct TrailSpawner;
struct TravolatorActor;
struct TurnTo;
struct TutorialGameMode;
struct UtilityReasoner;
struct UTAgent;
struct UltimateStats;
struct UltimateAbilityUser;
struct UltimateEvolution;
struct UltimateSlam;
struct UltimateTank;
struct UnlockedFeaturesManager;
struct VoidRepulserBulldozerEnhancement;
struct VoidRepulserShield;
struct VoidRepulserSpeedBoostEnhancement;
struct VoidRepulserVoidWaveEnhancement;
struct VoidRepulserData;
struct VoidRepulserWeapon;
struct WakeUpSurroundingsOnPushOrDestroy;
struct WallRunningActor;
struct WarmongerMiniRocketsEnhancement;
struct WaveElement;
struct WaveManager;
struct SpawnConfig;
struct WaveSpawner;
struct WeaponStats;
struct Weapon;
struct ChargingStats;
struct WeaponBehaviour_Charging;
struct WeaponLoadoutItemData;
struct WeaponLoadoutItem;
struct WeaponLoadout;
struct WeaponLoadoutActor;
struct WeaponProjectileSwitcherEnhancement;
struct WeaponShowcaseGameMode;
struct WeaponUser;
struct WrenchGroundSlamEnhancement;
struct WrenchThrowEnhancement;
struct WrenchThrowProjectile;
struct WrenchThrowWeapon;
struct DelayedImpulseInfo;
struct WrenchWeapon;
struct ZoneControlTeamScoring;
struct ZoneControlScoring;
struct ZoneControlGameMode;
struct HealthHitResult;
struct ShieldHitResult;
struct HitResult;
struct DamageToImpulseData;
struct OnSpawnEntityOnDeathParams;
struct OnPushedData;
struct GameModeSettings;
struct BotWeightedExtraPower;
struct CustomBotInfo;
struct BotsSettings;
struct UnlockedFeatures;
struct BlackboardEntry;
struct NavBlacklistEntry;
struct AISpawned;
struct ActiveCheckpoint;
struct RespawnTimers;
struct SpawnPointDynamicData;
struct DoomGroupMember;
struct TrailPoint;
struct EffectHitInfo;
struct EffectedEntity;
struct PowerUpStats;
struct WeaponUsageStats;
struct CollidingEntity;
struct ResetValues;
struct GoalScoreFrame;
struct BotUTData;
struct RicochetTargetInfo;
struct DamageIncreaseData;
struct DamageMultiplierData;
struct CollectData;
struct DropAnimData;
struct Droppable;
struct EntityGroupMember;
struct GatlingFireRateLevel;
struct PendingGoldBoltSpawn;
struct Overhealth;
struct ModInfo;
struct KillZoneArea;
struct ActivePowerUp;
struct RankingItem;
struct Reward;
struct LockedTarget;
struct ShieldGroupMember;
struct Stat;
struct StatSummary;
struct StreamPoint;
struct TargetingData;
struct ControlZoneStateHolder;
struct BotSDKData;
struct UTMomentumData;
struct UTMomentumPack;
struct QMultiShotConfig;
struct QMultiShotData;
namespace Code {
struct EntityInfo;
}

struct alignas(8) AIBlackboardComponent {
    static constexpr const char* ComponentTypeName = "AIBlackboardComponent";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    Ptr EntriesPtr;
    std::uint8_t _pad0[4];
    AssetRefAIBlackboard Board;

    // 0x7b8784c
    Collections::QDictionaryPtr<std::int32_t, BlackboardEntry> get_Entries() const { return Collections::QDictionaryPtr<std::int32_t, BlackboardEntry>(EntriesPtr); }
    // 0x7b87bf4
    void set_Entries(Collections::QDictionaryPtr<std::int32_t, BlackboardEntry> value) { EntriesPtr = value.Ptr; }

    // 0x7b88f44
    std::int32_t GetHashCode() const;
    // 0x7b89014
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b88f8c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7b88f94 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(AIBlackboardComponent) == AIBlackboardComponent::SIZE);

struct alignas(8) AIMoveForwardBehaviour {
    static constexpr const char* ComponentTypeName = "AIMoveForwardBehaviour";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::int32_t BouncedFrame;
    QBoolean ConstrainToSpawnerEnabled;
    QBoolean Enabled;
    QBoolean WasEnabledBeforeSystemDisabled;
    AssetRefAIMoveForwardBehaviourSpec Spec;
    FPVector3 WantedDirection;

    // 0x7c35a9c
    std::int32_t GetHashCode() const;
    // 0x7c35b20
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AIMoveForwardBehaviour) == AIMoveForwardBehaviour::SIZE);

struct alignas(8) AINavigation {
    static constexpr const char* ComponentTypeName = "AINavigation";
    static constexpr int SIZE = 160;
    static constexpr int ALIGNMENT = 8;

    PATH_FINDING_STATUS pathfindingStatus;
    std::uint8_t _pad0[3];
    std::int32_t nextRepathFrame;
    QBoolean hasBouncedOnBalloon;
    QBoolean isOnJumpLink;
    QBoolean isOnRailLink;
    QBoolean isTakingBumper;
    QBoolean maintainDirectionUntilPathFound;
    QBoolean needRepathOnLanding;
    AssetRefAINavigationSpec spec;
    EntityRef bouncingBalloonEntity;
    FP pathTTL;
    FPVector2 bumperInputDirection;
    FPVector2 jumpLinkDirection;
    FPVector3 lastPos;
    FPVector3 lookAtDir;
    FPVector3 navTarget;

    // 0x7c35c24
    std::int32_t GetHashCode() const;
    // 0x7c35da0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AINavigation) == AINavigation::SIZE);

struct alignas(4) AINavigationBlacklist {
    static constexpr const char* ComponentTypeName = "AINavigationBlacklist";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    Ptr entriesPtr;

    // 0x7c35fa4
    Collections::QListPtr<NavBlacklistEntry> get_entries() const { return Collections::QListPtr<NavBlacklistEntry>(entriesPtr); }
    // 0x7c35fe0
    void set_entries(Collections::QListPtr<NavBlacklistEntry> value) { entriesPtr = value.Ptr; }

    // 0x7c35fe8
    std::int32_t GetHashCode() const;
    // 0x7c36248
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c36110 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c35ff8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c361c4 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3608c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(AINavigationBlacklist) == AINavigationBlacklist::SIZE);

struct alignas(8) AINavigationInitialiser {
    static constexpr const char* ComponentTypeName = "AINavigationInitialiser";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefAINavigationSpec spec;

    // 0x7c365b0
    std::int32_t GetHashCode() const;
    // 0x7c365cc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AINavigationInitialiser) == AINavigationInitialiser::SIZE);

struct alignas(1) AISpawnDone {
    static constexpr const char* ComponentTypeName = "AISpawnDone";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c365e0
    std::int32_t GetHashCode() const;
    // 0x7c365e8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AISpawnDone) == AISpawnDone::SIZE);

struct alignas(8) AISpawner {
    static constexpr const char* ComponentTypeName = "AISpawner";
    static constexpr int SIZE = 56;
    static constexpr int ALIGNMENT = 8;

    std::int32_t initialSpawnFrame;
    QBoolean enabled;
    Ptr SpawnedEntitiesPtr;
    std::uint8_t _pad0[4];
    EntityRef groupEntity;
    FP damageMultiplier;
    FP hpMultiplier;
    ComponentPrototypeRef Prototype;

    // 0x7c365ec
    Collections::QListPtr<AISpawned> get_SpawnedEntities() const { return Collections::QListPtr<AISpawned>(SpawnedEntitiesPtr); }
    // 0x7c36628
    void set_SpawnedEntities(Collections::QListPtr<AISpawned> value) { SpawnedEntitiesPtr = value.Ptr; }

    // 0x7c36630
    std::int32_t GetHashCode() const;
    // 0x7c36934
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c367fc - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c366e4 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c368b0 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c36778 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(AISpawner) == AISpawner::SIZE);

struct alignas(1) ActorIced {
    static constexpr const char* ComponentTypeName = "ActorIced";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c36d18
    std::int32_t GetHashCode() const;
    // 0x7c36d20
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActorIced) == ActorIced::SIZE);

struct alignas(8) ActorInput {
    static constexpr const char* ComponentTypeName = "ActorInput";
    static constexpr int SIZE = 208;
    static constexpr int ALIGNMENT = 8;

    ActorInputLockFlags inputsLocked;
    QBoolean DashStateWanted;
    QBoolean FireStateWanted;
    QBoolean GadgetStateWanted;
    QBoolean JumpStateWanted;
    QBoolean MeleeStateWanted;
    QBoolean ReloadStateWanted;
    QBoolean UltimateStateWanted;
    QBoolean areHumanInputs;
    Button DashButton;
    Button FireButton;
    Button GadgetButton;
    Button JumpButton;
    Button MeleeButton;
    Button ReloadButton;
    Button UltimateButton;
    FPVector2 Direction;
    FPVector3 CameraPosition;
    FPVector3 CameraSight;
    FPVector3 LookInputVector;

    // 0x7c36d24
    std::int32_t GetHashCode() const;
    // 0x7c36eec
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActorInput) == ActorInput::SIZE);

struct alignas(1) ActorInputInitialiser {
    static constexpr const char* ComponentTypeName = "ActorInputInitialiser";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c370d0
    std::int32_t GetHashCode() const;
    // 0x7c370d8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActorInputInitialiser) == ActorInputInitialiser::SIZE);

struct alignas(8) ActorManager {
    static constexpr const char* ComponentTypeName = "ActorManager";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int16_t nbInitialSpawnPoints;
    std::uint8_t _pad0[2];
    Ptr activeCheckpointsPtr;
    Ptr availableBotNickNamesPtr;
    Ptr initialSpawnPointsPtr;
    Ptr podiumPointsPtr;
    Ptr respawnTimersPtr;
    Ptr respawnablePointsPtr;
    Ptr spawnPointDynamicDataPtr;
    AssetRefActorManagerConfig Config;

    // 0x7c370dc
    Collections::QListPtr<ActiveCheckpoint> get_activeCheckpoints() const { return Collections::QListPtr<ActiveCheckpoint>(activeCheckpointsPtr); }
    // 0x7c37118
    void set_activeCheckpoints(Collections::QListPtr<ActiveCheckpoint> value) { activeCheckpointsPtr = value.Ptr; }
    // 0x7c37120
    Collections::QListPtr<QString50> get_availableBotNickNames() const { return Collections::QListPtr<QString50>(availableBotNickNamesPtr); }
    // 0x7c3715c
    void set_availableBotNickNames(Collections::QListPtr<QString50> value) { availableBotNickNamesPtr = value.Ptr; }
    // 0x7c37164
    Collections::QListPtr<std::int16_t> get_initialSpawnPoints() const { return Collections::QListPtr<std::int16_t>(initialSpawnPointsPtr); }
    // 0x7c371a0
    void set_initialSpawnPoints(Collections::QListPtr<std::int16_t> value) { initialSpawnPointsPtr = value.Ptr; }
    // 0x7c371a8
    Collections::QListPtr<std::int16_t> get_podiumPoints() const { return Collections::QListPtr<std::int16_t>(podiumPointsPtr); }
    // 0x7c371e4
    void set_podiumPoints(Collections::QListPtr<std::int16_t> value) { podiumPointsPtr = value.Ptr; }
    // 0x7c371ec
    Collections::QListPtr<RespawnTimers> get_respawnTimers() const { return Collections::QListPtr<RespawnTimers>(respawnTimersPtr); }
    // 0x7c37228
    void set_respawnTimers(Collections::QListPtr<RespawnTimers> value) { respawnTimersPtr = value.Ptr; }
    // 0x7c37230
    Collections::QListPtr<std::int16_t> get_respawnablePoints() const { return Collections::QListPtr<std::int16_t>(respawnablePointsPtr); }
    // 0x7c3726c
    void set_respawnablePoints(Collections::QListPtr<std::int16_t> value) { respawnablePointsPtr = value.Ptr; }
    // 0x7c37274
    Collections::QListPtr<SpawnPointDynamicData> get_spawnPointDynamicData() const { return Collections::QListPtr<SpawnPointDynamicData>(spawnPointDynamicDataPtr); }
    // 0x7c372b0
    void set_spawnPointDynamicData(Collections::QListPtr<SpawnPointDynamicData> value) { spawnPointDynamicDataPtr = value.Ptr; }

    // 0x7c372b8
    std::int32_t GetHashCode() const;
    // 0x7c37a98
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c37690 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c37350 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c37a14 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3760c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(ActorManager) == ActorManager::SIZE);

struct alignas(1) ActorSheeped {
    static constexpr const char* ComponentTypeName = "ActorSheeped";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c3ac34
    std::int32_t GetHashCode() const;
    // 0x7c3ac3c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActorSheeped) == ActorSheeped::SIZE);

struct alignas(8) ActorStatus {
    static constexpr const char* ComponentTypeName = "ActorStatus";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    ActorStatusType currentType;
    ActorStatusType previousType;
    std::int32_t typeChangedFrame;
    KccSurfaceType currentSurfaceType;
    AssetRefActorStatusSpec spec;
    FP wantedDuration;

    // 0x7c3ac40
    std::int32_t GetHashCode() const;
    // 0x7c3accc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActorStatus) == ActorStatus::SIZE);

struct alignas(8) ActorStatusChangeRequested {
    static constexpr const char* ComponentTypeName = "ActorStatusChangeRequested";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    ActorStatusType wantedType;
    std::uint8_t _pad0[4];
    FP wantedDuration;

    // 0x7c3aee0
    std::int32_t GetHashCode() const;
    // 0x7c3af10
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActorStatusChangeRequested) == ActorStatusChangeRequested::SIZE);

struct alignas(8) ActorStatusModifier {
    static constexpr const char* ComponentTypeName = "ActorStatusModifier";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefActorStatusModifierSpec spec;

    // 0x7c3afd4
    std::int32_t GetHashCode() const;
    // 0x7c3aff0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActorStatusModifier) == ActorStatusModifier::SIZE);

struct alignas(1) ActorStunned {
    static constexpr const char* ComponentTypeName = "ActorStunned";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c3b004
    std::int32_t GetHashCode() const;
    // 0x7c3b00c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActorStunned) == ActorStunned::SIZE);

struct alignas(1) AddDamageIncreaseOnTargetedEffect {
    static constexpr const char* ComponentTypeName = "AddDamageIncreaseOnTargetedEffect";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c3b010
    std::int32_t GetHashCode() const;
    // 0x7c3b018
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AddDamageIncreaseOnTargetedEffect) == AddDamageIncreaseOnTargetedEffect::SIZE);

struct alignas(8) AddEntitiesToSpawnOnProjectileDeath {
    static constexpr const char* ComponentTypeName = "AddEntitiesToSpawnOnProjectileDeath";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefAddEntitiesToSpawnOnProjectileDeathSpec Spec;

    // 0x7c3b01c
    std::int32_t GetHashCode() const;
    // 0x7c3b038
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AddEntitiesToSpawnOnProjectileDeath) == AddEntitiesToSpawnOnProjectileDeath::SIZE);

struct alignas(4) AddForceOnUltimate {
    static constexpr const char* ComponentTypeName = "AddForceOnUltimate";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    QBoolean delayIsOver;

    // 0x7c3b04c
    std::int32_t GetHashCode() const;
    // 0x7c3b05c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AddForceOnUltimate) == AddForceOnUltimate::SIZE);

struct alignas(1) AgentOfDoom {
    static constexpr const char* ComponentTypeName = "AgentOfDoom";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c3b070
    std::int32_t GetHashCode() const;
    // 0x7c3b078
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AgentOfDoom) == AgentOfDoom::SIZE);

struct alignas(4) DoomGroupStats {
    static constexpr int SIZE = 12;
    static constexpr int ALIGNMENT = 4;

    std::int32_t DoomGroupMaxMembers;
    std::int32_t LastMaxedOutFrame;
    QBoolean Initialised;

    // 0x7b96d4c
    std::int32_t GetHashCode() const;
    // 0x7b96d9c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DoomGroupStats) == DoomGroupStats::SIZE);

struct alignas(4) AgentOfDoomGroup {
    static constexpr const char* ComponentTypeName = "AgentOfDoomGroup";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 4;

    Ptr membersPtr;
    DoomGroupStats Stats;

    // 0x7c3b07c
    Collections::QListPtr<DoomGroupMember> get_members() const { return Collections::QListPtr<DoomGroupMember>(membersPtr); }
    // 0x7c3b0b8
    void set_members(Collections::QListPtr<DoomGroupMember> value) { membersPtr = value.Ptr; }

    // 0x7c3b0c0
    std::int32_t GetHashCode() const;
    // 0x7c3b340
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c3b208 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c3b0f0 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c3b2bc - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3b184 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(AgentOfDoomGroup) == AgentOfDoomGroup::SIZE);

struct alignas(1) AgentOfDoomGrouped {
    static constexpr const char* ComponentTypeName = "AgentOfDoomGrouped";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c3b3e4
    std::int32_t GetHashCode() const;
    // 0x7c3b3ec
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AgentOfDoomGrouped) == AgentOfDoomGrouped::SIZE);

struct alignas(8) AmoeboidLauncherEnhancementPuddle {
    static constexpr const char* ComponentTypeName = "AmoeboidLauncherEnhancementPuddle";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefAmoeboidLauncherEnhancementPuddleSpec Spec;

    // 0x7c3b3f0
    std::int32_t GetHashCode() const;
    // 0x7c3b40c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AmoeboidLauncherEnhancementPuddle) == AmoeboidLauncherEnhancementPuddle::SIZE);

struct alignas(8) AoeTrail {
    static constexpr const char* ComponentTypeName = "AoeTrail";
    static constexpr int SIZE = 176;
    static constexpr int ALIGNMENT = 8;

    Ptr pointsPtr;
    std::uint8_t _pad0[4];
    AssetRefAoeTrailSpec Spec;
    EntityRef followedEntity;
    FP distanceTravelled;
    FP distanceWhenLastTrailSegmentWasDropped;
    FP trailSegmentLength;
    FP trailWidth;
    FPVector3 currentEndPosition;
    Shape3D compoundShape;

    // 0x7c3b420
    Collections::QListPtr<TrailPoint> get_points() const { return Collections::QListPtr<TrailPoint>(pointsPtr); }
    // 0x7c3b45c
    void set_points(Collections::QListPtr<TrailPoint> value) { pointsPtr = value.Ptr; }

    // 0x7c3b464
    std::int32_t GetHashCode() const;
    // TODO(0x7c3b7ac, Quantum.AoeTrail.Serialize): all fields in layout order; waits
    // on Shape3D serialisation in quantum_core (throws until then).
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c3b674 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c3b55c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c3b728 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3b5f0 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(AoeTrail) == AoeTrail::SIZE);

struct alignas(8) AreaOfEffect {
    static constexpr const char* ComponentTypeName = "AreaOfEffect";
    static constexpr int SIZE = 112;
    static constexpr int ALIGNMENT = 8;

    std::int32_t EnabledFrame;
    std::int32_t StartFrame;
    QBoolean HasMissed;
    QBoolean IsEnabled;
    QBoolean IsRunning;
    QBoolean KillInstantly;
    Ptr CachedHitInfoPtr;
    Ptr EffectedEntitiesPtr;
    AssetRefAreaOfEffectSpec Spec;
    FP Duration;
    FP Range;
    FP RemainingDuration;
    FPVector3 Scale;
    FPVector3 ScaleCoefficient;

    // 0x7c3b8c0
    Collections::QListPtr<EffectHitInfo> get_CachedHitInfo() const { return Collections::QListPtr<EffectHitInfo>(CachedHitInfoPtr); }
    // 0x7c3b8fc
    void set_CachedHitInfo(Collections::QListPtr<EffectHitInfo> value) { CachedHitInfoPtr = value.Ptr; }
    // 0x7c3b904
    Collections::QListPtr<EffectedEntity> get_EffectedEntities() const { return Collections::QListPtr<EffectedEntity>(EffectedEntitiesPtr); }
    // 0x7c3b940
    void set_EffectedEntities(Collections::QListPtr<EffectedEntity> value) { EffectedEntitiesPtr = value.Ptr; }

    // 0x7c3b948
    std::int32_t GetHashCode() const;
    // 0x7c3bdcc
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c3bc14 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c3ba9c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c3bd48 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3bb90 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(AreaOfEffect) == AreaOfEffect::SIZE);

struct alignas(8) AreaOfEffectIgnitionShapeHolder {
    static constexpr const char* ComponentTypeName = "AreaOfEffectIgnitionShapeHolder";
    static constexpr int SIZE = 120;
    static constexpr int ALIGNMENT = 8;

    FPVector3 Scale;
    Shape3D Shape;

    // 0x7c3c03c
    std::int32_t GetHashCode() const;
    // TODO(0x7c3c07c, Quantum.AreaOfEffectIgnitionShapeHolder.Serialize): all fields in layout order; waits
    // on Shape3D serialisation in quantum_core (throws until then).
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AreaOfEffectIgnitionShapeHolder) == AreaOfEffectIgnitionShapeHolder::SIZE);

struct alignas(8) AreaOfEffectLevelUp {
    static constexpr const char* ComponentTypeName = "AreaOfEffectLevelUp";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefAreaOfEffectLevelUpSpec Spec;

    // 0x7c3c0b4
    std::int32_t GetHashCode() const;
    // 0x7c3c0d0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AreaOfEffectLevelUp) == AreaOfEffectLevelUp::SIZE);

struct alignas(8) AreaOfEffectRaycastInfo {
    static constexpr const char* ComponentTypeName = "AreaOfEffectRaycastInfo";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    FPVector3 endPos;
    FPVector3 hitPoint;

    // 0x7c3c0e4
    std::int32_t GetHashCode() const;
    // 0x7c3c124
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AreaOfEffectRaycastInfo) == AreaOfEffectRaycastInfo::SIZE);

struct alignas(4) ArenaCannon {
    static constexpr const char* ComponentTypeName = "ArenaCannon";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::int32_t startedToFireFrameNumber;
    QBoolean firing;

    // 0x7c3c15c
    std::int32_t GetHashCode() const;
    // 0x7c3c184
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ArenaCannon) == ArenaCannon::SIZE);

struct alignas(8) AttackInfo {
    static constexpr const char* ComponentTypeName = "AttackInfo";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean isActive;
    QBoolean wasRunningBeforeAttack;
    EntityRef targetEntity;

    // 0x7c3c540
    std::int32_t GetHashCode() const;
    // 0x7c3c570
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AttackInfo) == AttackInfo::SIZE);

struct alignas(1) AutoDestroyOnMapChanged {
    static constexpr const char* ComponentTypeName = "AutoDestroyOnMapChanged";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c3c5b8
    std::int32_t GetHashCode() const;
    // 0x7c3c5c0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AutoDestroyOnMapChanged) == AutoDestroyOnMapChanged::SIZE);

struct alignas(8) AutoJumperStartParams {
    static constexpr int SIZE = 56;
    static constexpr int ALIGNMENT = 8;

    FP airDrag;
    FP gravity;
    FP travelTimeTotal;
    FP travelTimeUntilAirControlRestored;
    FPVector3 launchVelocity;

    // 0x7b94bdc
    std::int32_t GetHashCode() const;
    // 0x7b94c70
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AutoJumperStartParams) == AutoJumperStartParams::SIZE);

struct alignas(8) AutoJumperActor {
    static constexpr const char* ComponentTypeName = "AutoJumperActor";
    static constexpr int SIZE = 88;
    static constexpr int ALIGNMENT = 8;

    std::int32_t startFrame;
    std::uint8_t _pad0[4];
    FPVector3 velocity;
    AutoJumperStartParams startParams;

    // 0x7c3c5c4
    std::int32_t GetHashCode() const;
    // 0x7c3c620
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AutoJumperActor) == AutoJumperActor::SIZE);

struct alignas(8) BTAgent {
    static constexpr const char* ComponentTypeName = "BTAgent";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::int32_t AbortNodeId;
    Ptr ActiveServicesPtr;
    Ptr BTDataValuesPtr;
    Ptr DynamicCompositesPtr;
    Ptr NodesStatusPtr;
    Ptr ServicesEndTimesPtr;
    AssetRefAIConfig Config;
    AssetRefBTNode Current;
    AssetRefBTRoot Tree;

    // 0x7b7d454
    Collections::QListPtr<AssetRefBTService> get_ActiveServices() const { return Collections::QListPtr<AssetRefBTService>(ActiveServicesPtr); }
    // 0x7b7d490
    void set_ActiveServices(Collections::QListPtr<AssetRefBTService> value) { ActiveServicesPtr = value.Ptr; }
    // 0x7b7d498
    Collections::QListPtr<BTDataValue> get_BTDataValues() const { return Collections::QListPtr<BTDataValue>(BTDataValuesPtr); }
    // 0x7b7d4d4
    void set_BTDataValues(Collections::QListPtr<BTDataValue> value) { BTDataValuesPtr = value.Ptr; }
    // 0x7b7d4dc
    Collections::QListPtr<AssetRefBTComposite> get_DynamicComposites() const { return Collections::QListPtr<AssetRefBTComposite>(DynamicCompositesPtr); }
    // 0x7b7d518
    void set_DynamicComposites(Collections::QListPtr<AssetRefBTComposite> value) { DynamicCompositesPtr = value.Ptr; }
    // 0x7b7d520
    Collections::QListPtr<std::uint8_t> get_NodesStatus() const { return Collections::QListPtr<std::uint8_t>(NodesStatusPtr); }
    // 0x7b7d55c
    void set_NodesStatus(Collections::QListPtr<std::uint8_t> value) { NodesStatusPtr = value.Ptr; }
    // 0x7b7d564
    Collections::QListPtr<FP> get_ServicesEndTimes() const { return Collections::QListPtr<FP>(ServicesEndTimesPtr); }
    // 0x7b7d5a0
    void set_ServicesEndTimes(Collections::QListPtr<FP> value) { ServicesEndTimesPtr = value.Ptr; }

    // 0x7b7d5a8
    std::int32_t GetHashCode() const;
    // 0x7b7d784
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b7d6b8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7b7d6c8 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(BTAgent) == BTAgent::SIZE);

struct alignas(8) BattleStatsActor {
    static constexpr const char* ComponentTypeName = "BattleStatsActor";
    static constexpr int SIZE = 160;
    static constexpr int ALIGNMENT = 8;

    std::int32_t nbDeathEnviro;
    std::int32_t nbDeathFromBotPlayers;
    std::int32_t nbDeathFromHumanPlayers;
    std::int32_t nbDeathPve;
    std::int32_t numAmmoDepletedReloads;
    std::int32_t numDash;
    std::int32_t numDoubleJumps;
    std::int32_t numGadgetUse;
    std::int32_t numGlide;
    std::int32_t numGoalsScored;
    std::int32_t numJumpersUsed;
    std::int32_t numJumps;
    std::int32_t numManualReloads;
    std::int32_t numRailSlides;
    std::int32_t numUltimateUse;
    std::int32_t numWallRuns;
    std::int32_t numWeaponSwaps;
    std::int32_t numWeaponSwaps_0;
    std::int32_t totalBoltsCollected;
    std::int32_t totalGoldenBoltsCollected;
    Ptr mobTypeKilledCountsPtr;
    Ptr powerupStatsPtr;
    Ptr weaponUsageStatsPtr;
    std::uint8_t _pad0[4];
    FP damageTakenFromBots;
    FP damageTakenFromHumanPlayers;
    FP pveDamageDealt;
    FP pveDamageTaken;
    FP pveMeleeDamageDealt;
    FP pvpDamageDealt;
    FP pvpMeleeDamageDealt;
    FP totalGrindSeconds;

    // 0x7c3c6f4
    Collections::QDictionaryPtr<EntityInfoType, std::int32_t> get_mobTypeKilledCounts() const { return Collections::QDictionaryPtr<EntityInfoType, std::int32_t>(mobTypeKilledCountsPtr); }
    // 0x7c3c730
    void set_mobTypeKilledCounts(Collections::QDictionaryPtr<EntityInfoType, std::int32_t> value) { mobTypeKilledCountsPtr = value.Ptr; }
    // 0x7c3c738
    Collections::QListPtr<PowerUpStats> get_powerupStats() const { return Collections::QListPtr<PowerUpStats>(powerupStatsPtr); }
    // 0x7c3c774
    void set_powerupStats(Collections::QListPtr<PowerUpStats> value) { powerupStatsPtr = value.Ptr; }
    // 0x7c3c77c
    Collections::QDictionaryPtr<GENERATED_WEAPON_ID, WeaponUsageStats> get_weaponUsageStats() const { return Collections::QDictionaryPtr<GENERATED_WEAPON_ID, WeaponUsageStats>(weaponUsageStatsPtr); }
    // 0x7c3c7b8
    void set_weaponUsageStats(Collections::QDictionaryPtr<GENERATED_WEAPON_ID, WeaponUsageStats> value) { weaponUsageStatsPtr = value.Ptr; }

    // 0x7c3c7c0
    std::int32_t GetHashCode() const;
    // 0x7c3ce20
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c3cce4 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c3cb0c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c3cd9c - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3cc60 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(BattleStatsActor) == BattleStatsActor::SIZE);

struct alignas(8) BigBoomMine {
    static constexpr const char* ComponentTypeName = "BigBoomMine";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t TriggeredFrame;
    QBoolean IsStuck;
    AssetRefBigBoomMineSpec Spec;
    EntityRef StuckTo;
    FP FallingSpeed;

    // 0x7c3d960
    std::int32_t GetHashCode() const;
    // 0x7c3d9e8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BigBoomMine) == BigBoomMine::SIZE);

struct alignas(1) BigBoomWeapon {
    static constexpr const char* ComponentTypeName = "BigBoomWeapon";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c3dadc
    std::int32_t GetHashCode() const;
    // 0x7c3dae4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BigBoomWeapon) == BigBoomWeapon::SIZE);

struct alignas(8) BlackboardInitializer {
    static constexpr const char* ComponentTypeName = "BlackboardInitializer";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefAIBlackboardInitializer BBInitializer;

    // 0x7c3dae8
    std::int32_t GetHashCode() const;
    // 0x7c3db04
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BlackboardInitializer) == BlackboardInitializer::SIZE);

struct alignas(8) LastBallHitData {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t frame;
    std::uint8_t _pad0[4];
    EntityRef player;
    EntityRef weapon;

    // 0x7b99918
    std::int32_t GetHashCode() const;
    // 0x7b99964
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(LastBallHitData) == LastBallHitData::SIZE);

struct alignas(8) BlastBall {
    static constexpr const char* ComponentTypeName = "BlastBall";
    static constexpr int SIZE = 136;
    static constexpr int ALIGNMENT = 8;

    std::int32_t lockRotationFrame;
    std::int32_t shotFrame;
    Ptr dashingPlayersPtr;
    std::uint8_t _pad0[4];
    AssetRefBlastBallSpec Spec;
    FP initialDrag;
    FPVector3 surfaceNormal;
    LastBallHitData lastHitTeamA;
    LastBallHitData lastHitTeamB;
    FPQuaternion lockRotation;

    // 0x7c3db18
    Collections::QListPtr<CollidingEntity> get_dashingPlayers() const { return Collections::QListPtr<CollidingEntity>(dashingPlayersPtr); }
    // 0x7c3db54
    void set_dashingPlayers(Collections::QListPtr<CollidingEntity> value) { dashingPlayersPtr = value.Ptr; }

    // 0x7c3db5c
    std::int32_t GetHashCode() const;
    // 0x7c3dea8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c3dd70 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c3dc58 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c3de24 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3dcec - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(BlastBall) == BlastBall::SIZE);

struct alignas(8) BlastBallTeamData {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t score;
    std::uint8_t _pad0[2];
    BlastBallTeamState state;
    FPVector3 goalPosition;

    // 0x7b94cd8
    std::int32_t GetHashCode() const;
    // 0x7b94d2c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BlastBallTeamData) == BlastBallTeamData::SIZE);

struct alignas(8) BlastBallGame {
    static constexpr const char* ComponentTypeName = "BlastBallGame";
    static constexpr int SIZE = 136;
    static constexpr int ALIGNMENT = 8;

    BlastBallGameState previousState;
    BlastBallGameState state;
    std::int32_t stateStartFrame;
    QBoolean isOverTime;
    Ptr resetValuesPtr;
    Ptr scoringFramesPtr;
    TeamFlag lastScoringTeam;
    std::uint8_t _pad0[4];
    EntityRef ballEntity;
    EntityRef lastScoringPlayer;
    FPVector3 ballPosition;
    BlastBallTeamData teamA;
    BlastBallTeamData teamB;

    // 0x7c3e08c
    Collections::QDictionaryPtr<EntityRef, ResetValues> get_resetValues() const { return Collections::QDictionaryPtr<EntityRef, ResetValues>(resetValuesPtr); }
    // 0x7c3e0c8
    void set_resetValues(Collections::QDictionaryPtr<EntityRef, ResetValues> value) { resetValuesPtr = value.Ptr; }
    // 0x7c3e0d0
    Collections::QListPtr<GoalScoreFrame> get_scoringFrames() const { return Collections::QListPtr<GoalScoreFrame>(scoringFramesPtr); }
    // 0x7c3e10c
    void set_scoringFrames(Collections::QListPtr<GoalScoreFrame> value) { scoringFramesPtr = value.Ptr; }

    // 0x7c3e114
    std::int32_t GetHashCode() const;
    // 0x7c3e54c
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c3e394 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c3e21c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c3e4c8 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3e310 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(BlastBallGame) == BlastBallGame::SIZE);

struct alignas(8) BlastBallGoal {
    static constexpr const char* ComponentTypeName = "BlastBallGoal";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean ballInside;
    TeamFlag OwnerTeam;
    EntityRef ballEntity;

    // 0x7c3e850
    std::int32_t GetHashCode() const;
    // 0x7c3e884
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BlastBallGoal) == BlastBallGoal::SIZE);

struct alignas(2) BlastBallPlayer {
    static constexpr const char* ComponentTypeName = "BlastBallPlayer";
    static constexpr int SIZE = 6;
    static constexpr int ALIGNMENT = 2;

    std::uint16_t goals;
    std::uint16_t meleeGoals;
    std::uint16_t rangedGoals;

    // 0x7c3e958
    std::int32_t GetHashCode() const;
    // 0x7c3e9b4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BlastBallPlayer) == BlastBallPlayer::SIZE);

struct alignas(8) BlasterMoveSpeedEnhancement {
    static constexpr const char* ComponentTypeName = "BlasterMoveSpeedEnhancement";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean MoveSpeedModifierIsApplied;
    std::uint8_t _pad0[4];
    AssetRefBlasterMoveSpeedEnhancementSpec Spec;

    // 0x7c3eb40
    std::int32_t GetHashCode() const;
    // 0x7c3eb68
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BlasterMoveSpeedEnhancement) == BlasterMoveSpeedEnhancement::SIZE);

struct alignas(8) BlasterSpecialShotEnhancement {
    static constexpr const char* ComponentTypeName = "BlasterSpecialShotEnhancement";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t ShotCounter;
    std::uint8_t _pad0[4];
    AssetRefBlasterSpecialShotEnhancementSpec Spec;
    AssetRefEntityPrototype OriginalAmmoAsset;

    // 0x7c3eba0
    std::int32_t GetHashCode() const;
    // 0x7c3ec00
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BlasterSpecialShotEnhancement) == BlasterSpecialShotEnhancement::SIZE);

struct alignas(8) BoltCounterElement {
    static constexpr const char* ComponentTypeName = "BoltCounterElement";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t currentBolts;
    std::int32_t requiredBolts;
    QBoolean reachedRequired;
    std::uint8_t _pad0[4];
    AssetRefLevelButtonGameplayElementConfig gameplayElementConfig;

    // 0x7c3ecd4
    std::int32_t GetHashCode() const;
    // 0x7c3ed44
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BoltCounterElement) == BoltCounterElement::SIZE);

struct alignas(8) BoltGathererActor {
    static constexpr const char* ComponentTypeName = "BoltGathererActor";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t BoltsCollected;
    QBoolean hasTeamCounter;
    AssetRefBoltGathererActorSpec Spec;
    FP PreventBoltCollectionEndFrame;

    // 0x7c3ee88
    std::int32_t GetHashCode() const;
    // 0x7c3eef0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BoltGathererActor) == BoltGathererActor::SIZE);

struct alignas(8) BoltPowerActor {
    static constexpr const char* ComponentTypeName = "BoltPowerActor";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t highestLevelReached;
    std::uint16_t level;
    std::uint16_t maxLevel;
    std::uint8_t _pad0[2];
    AssetRefBoltPowerActorSpec Spec;

    // 0x7c3efd4
    std::int32_t GetHashCode() const;
    // 0x7c3f054
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BoltPowerActor) == BoltPowerActor::SIZE);

struct alignas(8) BombardierDrone {
    static constexpr const char* ComponentTypeName = "BombardierDrone";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    BombardierDroneState state;
    BombDropSide lastBombDropSide;
    std::int32_t bombDropCount;
    std::int32_t startFrame;
    AssetRefBombardierDroneSpec Spec;
    FP cachedHeight;
    FP fallingSpeed;

    // 0x7c3f1f0
    std::int32_t GetHashCode() const;
    // 0x7c3f2a8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BombardierDrone) == BombardierDrone::SIZE);

struct alignas(8) BombardierWeapon {
    static constexpr const char* ComponentTypeName = "BombardierWeapon";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefBombardierWeaponAutoTargetSpec Spec;

    // 0x7c3f4cc
    std::int32_t GetHashCode() const;
    // 0x7c3f4e8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BombardierWeapon) == BombardierWeapon::SIZE);

struct alignas(8) BotAirSaveParams {
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    FP affinity;
    FP cooldownTimer;
    FP dashTimer;
    FP maintainDirectionTimer;
    FPVector2 directionToMaintain;

    // 0x7b94e60
    std::int32_t GetHashCode() const;
    // 0x7b94ef4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BotAirSaveParams) == BotAirSaveParams::SIZE);

struct alignas(8) BotBlastBallParams {
    static constexpr int SIZE = 160;
    static constexpr int ALIGNMENT = 8;

    std::int32_t defensivePositionComputeFrame;
    std::int32_t inShootingZoneLastRepositionFrame;
    EntityRef passingTarget;
    FP blastBallDashCooldownTimer;
    FP inShootingZonePositionChangeCooldownTimer;
    FP shootDistance;
    FPVector3 passingTargetPos;
    FPVector3 targetBBPosition;
    FPVector3 targetDefensivePosition;
    FPVector3 targetEnemyGoal;
    FPVector3 targetStrikePos;

    // 0x7b94f5c
    std::int32_t GetHashCode() const;
    // 0x7b95098
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BotBlastBallParams) == BotBlastBallParams::SIZE);

struct alignas(8) BotShootingParams {
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::int32_t missChance_0_100;
    std::int32_t missChance_0_100_BehindHuman;
    FP successiveShotsCooldownMax;
    FP successiveShotsCooldownMin;
    FPVector3 targetOffset;

    // 0x7b95414
    std::int32_t GetHashCode() const;
    // 0x7b954a8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BotShootingParams) == BotShootingParams::SIZE);

struct alignas(1) FireRequest {
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    FIRE_REQUEST_TYPE type;
    std::array<std::uint8_t, 3> _alignment_padding_;

    // 0x7b97c94
    std::int32_t GetHashCode() const;
    // 0x7b97ca4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(FireRequest) == FireRequest::SIZE);

struct alignas(8) ZoneControlBotParams {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    FP repathTimer;
    FPVector3 targetZonePosition;

    // 0x7b9e688
    std::int32_t GetHashCode() const;
    // 0x7b9e6c8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ZoneControlBotParams) == ZoneControlBotParams::SIZE);

struct alignas(8) Bot {
    static constexpr const char* ComponentTypeName = "Bot";
    static constexpr int SIZE = 760;
    static constexpr int ALIGNMENT = 8;

    std::int16_t presetIndex;
    std::uint16_t targetDroppableId;
    std::uint16_t targetDroppableIndex;
    std::uint16_t targetWrenchCombo;
    BOT_AGGRO_BEHAVIOUR currentAggroBehaviour;
    FireRequest pendingFireRequest;
    std::int32_t disabledEndFrame;
    std::int32_t lastRepositionFrame;
    std::int32_t nextRepositionFrame;
    std::int32_t seekForbiddenEndFrame;
    std::int32_t useGadgetCooldownEndFrame;
    std::int32_t useWrenchRequestedFrame;
    std::int32_t weaponSwitchExcludedTags;
    QBoolean disabled;
    QBoolean forceAttackPosComputationSuccess;
    QBoolean hasReachedTransitionPoint;
    QBoolean useTransitionPoint;
    Ptr UTDebugActionPtr;
    Ptr UTDebugMovePtr;
    Ptr blankPushesListPtr;
    Ptr inRangeDroppablesPtr;
    std::uint8_t _pad0[4];
    EntityRef UTActionEntity;
    EntityRef UTMoveEntity;
    EntityRef attackingTarget;
    EntityRef targetCrate;
    EntityRef targetGoldenBolt;
    FP attackingTargetCoolDownTimer;
    FP difficulty_0_1;
    FP droppableAwarenessRangeSq;
    FP fireRequestCooldown;
    FP fireRequestCounter;
    FP randomJumpTimer;
    FP useWrenchCooldownTimer;
    FP weaponSwitchCooldownTimer;
    FP weaponSwitchNoCombatActivityDelay;
    FPVector3 attackingTargetPos;
    FPVector3 destination;
    FPVector3 destinationTransitionPoint;
    FPVector3 goldenBoltUnreachablePos;
    FPVector3 groupTeamNavPos;
    FPVector3 lastGroundedPosition;
    FPVector3 targetDroppableNavPos;
    FPVector3 targetGoldenBoltNavPos;
    FPVector3 targetKingPos;
    FPVector3 targetObjectivePos;
    ZoneControlBotParams zoneControl;
    QString34 nickname;
    std::uint8_t _pad1[4];
    BotAirSaveParams airSave;
    BotShootingParams shootingParams;
    BotBlastBallParams blastBall;

    // 0x7c3f4fc
    Collections::QDictionaryPtr<QString50, BotUTData> get_UTDebugAction() const { return Collections::QDictionaryPtr<QString50, BotUTData>(UTDebugActionPtr); }
    // 0x7c3f538
    void set_UTDebugAction(Collections::QDictionaryPtr<QString50, BotUTData> value) { UTDebugActionPtr = value.Ptr; }
    // 0x7c3f540
    Collections::QDictionaryPtr<QString50, BotUTData> get_UTDebugMove() const { return Collections::QDictionaryPtr<QString50, BotUTData>(UTDebugMovePtr); }
    // 0x7c3f57c
    void set_UTDebugMove(Collections::QDictionaryPtr<QString50, BotUTData> value) { UTDebugMovePtr = value.Ptr; }
    // 0x7c3f584
    Collections::QListPtr<EntityRef> get_blankPushesList() const { return Collections::QListPtr<EntityRef>(blankPushesListPtr); }
    // 0x7c3f5c0
    void set_blankPushesList(Collections::QListPtr<EntityRef> value) { blankPushesListPtr = value.Ptr; }
    // 0x7c3f5c8
    Collections::QDictionaryPtr<std::uint16_t, std::uint16_t> get_inRangeDroppables() const { return Collections::QDictionaryPtr<std::uint16_t, std::uint16_t>(inRangeDroppablesPtr); }
    // 0x7c3f604
    void set_inRangeDroppables(Collections::QDictionaryPtr<std::uint16_t, std::uint16_t> value) { inRangeDroppablesPtr = value.Ptr; }

    // 0x7c3f60c
    std::int32_t GetHashCode() const;
    // 0x7c3ffd8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c3fd38 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c3fb0c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c3ff54 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c3fcb4 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Bot) == Bot::SIZE);

struct alignas(8) Bumper {
    static constexpr const char* ComponentTypeName = "Bumper";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    AssetRefBumperConfig config;
    EntityRef entity;
    FP impulseStrength;
    FPVector3 impulseDirection;

    // 0x7c4087c
    std::int32_t GetHashCode() const;
    // 0x7c408fc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Bumper) == Bumper::SIZE);

struct alignas(8) BumperBar {
    static constexpr const char* ComponentTypeName = "BumperBar";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    AssetRefBumperBarConfig Config;
    EntityRef entity;

    // 0x7c40ccc
    std::int32_t GetHashCode() const;
    // 0x7c40d00
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BumperBar) == BumperBar::SIZE);

struct alignas(8) BuzzbladesRicochet {
    static constexpr const char* ComponentTypeName = "BuzzbladesRicochet";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t changeTargetFrame;
    Ptr targetsPtr;
    AssetRefBuzzbladesRicochetSpec Spec;

    // 0x7c41154
    Collections::QListPtr<RicochetTargetInfo> get_targets() const { return Collections::QListPtr<RicochetTargetInfo>(targetsPtr); }
    // 0x7c41190
    void set_targets(Collections::QListPtr<RicochetTargetInfo> value) { targetsPtr = value.Ptr; }

    // 0x7c41198
    std::int32_t GetHashCode() const;
    // 0x7c4143c
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c41304 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c411ec - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c413b8 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c41280 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(BuzzbladesRicochet) == BuzzbladesRicochet::SIZE);

struct alignas(8) CharacterColliderScaler {
    static constexpr const char* ComponentTypeName = "CharacterColliderScaler";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    FP CurrentScaleFactor;
    FP OriginRadius;
    FP OriginSphereHeight;
    FPVector3 OriginExtents;

    // 0x7c41568
    std::int32_t GetHashCode() const;
    // 0x7c415e4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(CharacterColliderScaler) == CharacterColliderScaler::SIZE);

struct alignas(8) ChargeAttack {
    static constexpr const char* ComponentTypeName = "ChargeAttack";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    ChargeAttackState state;
    std::int32_t changeStateFrame;
    QBoolean movementStarted;
    std::uint8_t _pad0[4];
    AssetRefChargeAttackSpec Spec;
    FPVector3 startPosition;

    // 0x7c4163c
    std::int32_t GetHashCode() const;
    // 0x7c416bc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ChargeAttack) == ChargeAttack::SIZE);

struct alignas(4) CheatManager {
    static constexpr const char* ComponentTypeName = "CheatManager";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    QBoolean finishRaceFast;

    // 0x7c41948
    std::int32_t GetHashCode() const;
    // 0x7c41958
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(CheatManager) == CheatManager::SIZE);

struct alignas(4) Cheats {
    static constexpr const char* ComponentTypeName = "Cheats";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 4;

    QBoolean godModeEnabled;
    QBoolean infiniteGadget;
    QBoolean infiniteUltimate;
    QBoolean noClipEnabled;

    // 0x7c4196c
    std::int32_t GetHashCode() const;
    // 0x7c4199c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Cheats) == Cheats::SIZE);

struct alignas(1) Checkpoint {
    static constexpr const char* ComponentTypeName = "Checkpoint";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c419f4
    std::int32_t GetHashCode() const;
    // 0x7c419fc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Checkpoint) == Checkpoint::SIZE);

struct alignas(8) ColliderResize {
    static constexpr const char* ComponentTypeName = "ColliderResize";
    static constexpr int SIZE = 112;
    static constexpr int ALIGNMENT = 8;

    ColliderResizeStatus Status;
    std::int32_t SpawnFrame;
    AssetRefColliderResizeSpec Spec;
    FPVector3 CurrentCentroid;
    FPVector3 CurrentExtents;
    FPVector3 OriginCentroid;
    FPVector3 OriginExtents;

    // 0x7c41a00
    std::int32_t GetHashCode() const;
    // 0x7c41ac4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ColliderResize) == ColliderResize::SIZE);

struct alignas(8) CollisionForce {
    static constexpr const char* ComponentTypeName = "CollisionForce";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t lastGroundedFrame;
    QBoolean pushSourceIsCollisionForceComponent;
    Ptr collidedDamageablesPtr;
    std::uint8_t _pad0[4];
    AssetRefCollisionForceSpec Spec;
    FP damageAmountToDealThisFrame;
    FP pushAmountToDealThisFrame;

    // 0x7c41c38
    Collections::QListPtr<CollidingEntity> get_collidedDamageables() const { return Collections::QListPtr<CollidingEntity>(collidedDamageablesPtr); }
    // 0x7c41c74
    void set_collidedDamageables(Collections::QListPtr<CollidingEntity> value) { collidedDamageablesPtr = value.Ptr; }

    // 0x7c41c7c
    std::int32_t GetHashCode() const;
    // 0x7c41f64
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c41e2c - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c41d14 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c41ee0 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c41da8 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(CollisionForce) == CollisionForce::SIZE);

struct alignas(1) CombatParticipant {
    static constexpr const char* ComponentTypeName = "CombatParticipant";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c420c0
    std::int32_t GetHashCode() const;
    // 0x7c420c8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(CombatParticipant) == CombatParticipant::SIZE);

struct alignas(8) HFSMData {
    static constexpr int SIZE = 96;
    static constexpr int ALIGNMENT = 8;

    std::uint8_t StatesHistory;
    std::uint8_t _pad0[3];
    QBoolean UsesStateHistory;
    AssetRefHFSMRoot Root;
    AssetRefHFSMState CurrentState;
    FP Time;
    std::array<FP, 8> _Times_;

    // 0x7b982ec
    std::span<FP> get_Times() { return _Times_; }
    std::span<const FP> get_Times() const { return _Times_; }

    // 0x7b98354
    std::int32_t GetHashCode() const;
    // 0x7b98494
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HFSMData) == HFSMData::SIZE);

struct alignas(8) HFSMAgent {
    static constexpr const char* ComponentTypeName = "HFSMAgent";
    static constexpr int SIZE = 104;
    static constexpr int ALIGNMENT = 8;

    AssetRefAIConfig Config;
    HFSMData Data;

    // 0x7b841e4
    std::int32_t GetHashCode() const;
    // 0x7b84224
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HFSMAgent) == HFSMAgent::SIZE);

struct alignas(8) RuntimeHFSM {
    static constexpr int SIZE = 120;
    static constexpr int ALIGNMENT = 8;

    AIBlackboardComponent blackboard;
    HFSMAgent agent;

    // 0x7b9bafc
    std::int32_t GetHashCode() const;
    // 0x7b9bb80
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b9bb78 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
};
static_assert(sizeof(RuntimeHFSM) == RuntimeHFSM::SIZE);

struct alignas(8) CompoundHFSM {
    static constexpr const char* ComponentTypeName = "CompoundHFSM";
    static constexpr int SIZE = 256;
    static constexpr int ALIGNMENT = 8;

    std::int32_t schedulePeriod;
    std::uint8_t _pad0[4];
    AssetRefCompoundHFSMSpec spec;
    RuntimeHFSM attackHFSM;
    RuntimeHFSM navHFSM;

    // 0x7b8440c
    std::int32_t GetHashCode() const;
    // 0x7b845bc
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b844a8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7b844e4 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(CompoundHFSM) == CompoundHFSM::SIZE);

struct alignas(8) Cryoslider {
    static constexpr const char* ComponentTypeName = "Cryoslider";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    AssetRefCryosliderSpec Spec;
    FP distanceTravelled;
    FP lastFloorHeightTouched;

    // 0x7c420cc
    std::int32_t GetHashCode() const;
    // 0x7c42128
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Cryoslider) == Cryoslider::SIZE);

struct alignas(8) CryosliderWeapon {
    static constexpr const char* ComponentTypeName = "CryosliderWeapon";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefCryosliderWeaponAutoTargetSpec Spec;

    // 0x7c42170
    std::int32_t GetHashCode() const;
    // 0x7c4218c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(CryosliderWeapon) == CryosliderWeapon::SIZE);

struct alignas(8) DamageChargeable {
    static constexpr const char* ComponentTypeName = "DamageChargeable";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t activatedFrame;
    std::int32_t lastHitFrame;
    AssetRefDamageChargeableSpec Spec;
    FP chargeAmount;

    // 0x7c421a0
    std::int32_t GetHashCode() const;
    // 0x7c42220
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DamageChargeable) == DamageChargeable::SIZE);

struct alignas(8) DamageIncreasedOnTargetsEnhancement {
    static constexpr const char* ComponentTypeName = "DamageIncreasedOnTargetsEnhancement";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    Ptr DamageIncreaseByTargetPtr;
    std::uint8_t _pad0[4];
    AssetRefDamageIncreasedOnTargetsEnhancementSpec Spec;

    // 0x7c42364
    Collections::QDictionaryPtr<EntityRef, DamageIncreaseData> get_DamageIncreaseByTarget() const { return Collections::QDictionaryPtr<EntityRef, DamageIncreaseData>(DamageIncreaseByTargetPtr); }
    // 0x7c423a0
    void set_DamageIncreaseByTarget(Collections::QDictionaryPtr<EntityRef, DamageIncreaseData> value) { DamageIncreaseByTargetPtr = value.Ptr; }

    // 0x7c423a8
    std::int32_t GetHashCode() const;
    // 0x7c42620
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c424e8 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c423d0 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c4259c - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c42464 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(DamageIncreasedOnTargetsEnhancement) == DamageIncreasedOnTargetsEnhancement::SIZE);

struct alignas(8) DamageMultiplier {
    static constexpr const char* ComponentTypeName = "DamageMultiplier";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefDamageMultiplierSpec Spec;

    // 0x7c426cc
    std::int32_t GetHashCode() const;
    // 0x7c426e8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DamageMultiplier) == DamageMultiplier::SIZE);

struct alignas(1) DamageMultiplierHeroImprovement {
    static constexpr const char* ComponentTypeName = "DamageMultiplierHeroImprovement";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c426fc
    std::int32_t GetHashCode() const;
    // 0x7c42704
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DamageMultiplierHeroImprovement) == DamageMultiplierHeroImprovement::SIZE);

struct alignas(8) DamageSource {
    static constexpr const char* ComponentTypeName = "DamageSource";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    QBoolean hitTrackingDone;
    Ptr multipliersPtr;
    AssetRefAmmunitionDamageSpec AmmunitionSpec;
    AssetRefDamageSpec Spec;

    // 0x7c42708
    Collections::QListPtr<DamageMultiplierData> get_multipliers() const { return Collections::QListPtr<DamageMultiplierData>(multipliersPtr); }
    // 0x7c42744
    void set_multipliers(Collections::QListPtr<DamageMultiplierData> value) { multipliersPtr = value.Ptr; }

    // 0x7c4274c
    std::int32_t GetHashCode() const;
    // 0x7c429f8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c428c0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c427a8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c42974 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c4283c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(DamageSource) == DamageSource::SIZE);

struct alignas(8) DamageToImpulseConverter {
    static constexpr const char* ComponentTypeName = "DamageToImpulseConverter";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::int32_t impulseFrame;
    std::int32_t priority;
    QBoolean resetVelocity;
    std::uint8_t _pad0[4];
    AssetRefDamageToImpulseConverterSpec Spec;
    FPVector3 impulse;

    // 0x7c42ac4
    std::int32_t GetHashCode() const;
    // 0x7c42b50
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DamageToImpulseConverter) == DamageToImpulseConverter::SIZE);

struct alignas(8) HitHealthData {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    FP damageMultiplier;

    // 0x7b98894
    std::int32_t GetHashCode() const;
    // 0x7b98aec
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HitHealthData) == HitHealthData::SIZE);

struct alignas(8) HitShieldData {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    QBoolean applyMoveModifier;
    QBoolean breakShield;
    QBoolean ignoreShield;
    std::uint8_t _pad0[4];
    FP damageMultiplier;

    // 0x7b988b0
    std::int32_t GetHashCode() const;
    // 0x7b98b00
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HitShieldData) == HitShieldData::SIZE);

struct alignas(8) HitData {
    static constexpr int SIZE = 144;
    static constexpr int ALIGNMENT = 8;

    DamageType type;
    std::int32_t frame;
    QBoolean isContinuous;
    QBoolean isCriticalHit;
    QBoolean slowDownOnHit;
    QBoolean useReticleFeedback;
    EntityRef owner;
    EntityRef source;
    EntityRef weapon;
    FP baseDamage;
    FP damages;
    HitHealthData health;
    FPVector3 direction;
    FPVector3 position;
    HitShieldData shield;

    // 0x7b9872c
    std::int32_t GetHashCode() const;
    // 0x7b988fc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HitData) == HitData::SIZE);

struct alignas(8) Damageable {
    static constexpr const char* ComponentTypeName = "Damageable";
    static constexpr int SIZE = 192;
    static constexpr int ALIGNMENT = 8;

    std::int32_t electricalDamageLastFrame;
    std::int32_t iceDamageLastFrame;
    std::int32_t invulnerabilityEndFrame;
    INVULNERABILITY_FILTER invulnerabilityFilter;
    Ptr activeDamageEffectsPtr;
    Ptr hitsPtr;
    AssetRefDamageableSpec Spec;
    FP electricalCurrentBuildUp;
    FP iceCurrentBuildUp;
    HitData hitBy;

    // 0x7c42ca4
    Collections::QListPtr<EntityRef> get_activeDamageEffects() const { return Collections::QListPtr<EntityRef>(activeDamageEffectsPtr); }
    // 0x7c42ce0
    void set_activeDamageEffects(Collections::QListPtr<EntityRef> value) { activeDamageEffectsPtr = value.Ptr; }
    // 0x7c42ce8
    Collections::QListPtr<HitData> get_hits() const { return Collections::QListPtr<HitData>(hitsPtr); }
    // 0x7c42d24
    void set_hits(Collections::QListPtr<HitData> value) { hitsPtr = value.Ptr; }

    // 0x7c42d2c
    std::int32_t GetHashCode() const;
    // 0x7c43158
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c42fa0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c42e28 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c430d4 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c42f1c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Damageable) == Damageable::SIZE);

struct alignas(8) DashActor {
    static constexpr const char* ComponentTypeName = "DashActor";
    static constexpr int SIZE = 136;
    static constexpr int ALIGNMENT = 8;

    DASH_DISABLED_REASON disabledReasons;
    std::int32_t lastHitFrame;
    QBoolean isDashing;
    QBoolean noCooldownCheat;
    Ptr hitEntitiesPtr;
    std::uint8_t _pad0[4];
    AssetRefDashActorConfig Spec;
    FP bonusCooldownReduction;
    FP bonusDashDamage;
    FP cooldownTimer;
    FP dashTimer;
    FP ignoreMoveSpeedCounter;
    FP speedBonusDurationOnDashEnded;
    FP speedBonusOnDashEnded;
    FPVector3 dashImpulse;
    FPVector3 lastValidMoveInput;

    // 0x7c43720
    Collections::QListPtr<EntityRef> get_hitEntities() const { return Collections::QListPtr<EntityRef>(hitEntitiesPtr); }
    // 0x7c4375c
    void set_hitEntities(Collections::QListPtr<EntityRef> value) { hitEntitiesPtr = value.Ptr; }

    // 0x7c43764
    std::int32_t GetHashCode() const;
    // 0x7c43b38
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c43a00 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c438e8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c43ab4 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c4397c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(DashActor) == DashActor::SIZE);

struct alignas(8) DrillDash {
    static constexpr const char* ComponentTypeName = "DrillDash";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    DrillDashState CurrentState;
    std::int32_t StateStartFrame;
    EntityRef AttackAoeEntity;
    FPVector3 DashDirection;

    // 0x7c43dd8
    std::int32_t GetHashCode() const;
    // 0x7c43e44
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DrillDash) == DrillDash::SIZE);

struct alignas(8) DroppableCollector {
    static constexpr const char* ComponentTypeName = "DroppableCollector";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    Ptr disabledDroppablesPtr;
    Ptr lastCollectDataByDropTypePtr;
    Ptr nbCollectedByTypePtr;
    std::uint8_t _pad0[4];
    AssetRefDroppableCollectorSpec Spec;

    // 0x7c440ac
    Collections::QListPtr<DroppableType> get_disabledDroppables() const { return Collections::QListPtr<DroppableType>(disabledDroppablesPtr); }
    // 0x7c440e8
    void set_disabledDroppables(Collections::QListPtr<DroppableType> value) { disabledDroppablesPtr = value.Ptr; }
    // 0x7c440f0
    Collections::QDictionaryPtr<DroppableType, CollectData> get_lastCollectDataByDropType() const { return Collections::QDictionaryPtr<DroppableType, CollectData>(lastCollectDataByDropTypePtr); }
    // 0x7c4412c
    void set_lastCollectDataByDropType(Collections::QDictionaryPtr<DroppableType, CollectData> value) { lastCollectDataByDropTypePtr = value.Ptr; }
    // 0x7c44134
    Collections::QDictionaryPtr<DroppableType, std::uint16_t> get_nbCollectedByType() const { return Collections::QDictionaryPtr<DroppableType, std::uint16_t>(nbCollectedByTypePtr); }
    // 0x7c44170
    void set_nbCollectedByType(Collections::QDictionaryPtr<DroppableType, std::uint16_t> value) { nbCollectedByTypePtr = value.Ptr; }

    // 0x7c44178
    std::int32_t GetHashCode() const;
    // 0x7c445d8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c44398 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c441c0 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c44554 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c44314 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(DroppableCollector) == DroppableCollector::SIZE);

struct alignas(8) DroppableEntity {
    static constexpr const char* ComponentTypeName = "DroppableEntity";
    static constexpr int SIZE = 72;
    static constexpr int ALIGNMENT = 8;

    std::int32_t destroyFrame;
    std::int32_t removeOwnershipFrame;
    std::int32_t spawnFrame;
    std::uint8_t _pad0[4];
    AssetRefEntityDropperSpec dropperSpec;
    FPVector3 spawnPos;
    FPVector3 targetPos;

    // 0x7c44720
    std::int32_t GetHashCode() const;
    // 0x7c447d4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DroppableEntity) == DroppableEntity::SIZE);

struct alignas(8) DroppableManager {
    static constexpr const char* ComponentTypeName = "DroppableManager";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t nextDroppableId;
    std::uint8_t _pad0[2];
    Ptr dropAnimsPtr;
    Ptr droppablesHiPtr;
    Ptr droppablesLoPtr;
    AssetRefDroppableManagerSpec Spec;

    // 0x7c44990
    Collections::QDictionaryPtr<std::uint16_t, DropAnimData> get_dropAnims() const { return Collections::QDictionaryPtr<std::uint16_t, DropAnimData>(dropAnimsPtr); }
    // 0x7c449cc
    void set_dropAnims(Collections::QDictionaryPtr<std::uint16_t, DropAnimData> value) { dropAnimsPtr = value.Ptr; }
    // 0x7c449d4
    Collections::QListPtr<Droppable> get_droppablesHi() const { return Collections::QListPtr<Droppable>(droppablesHiPtr); }
    // 0x7c44a10
    void set_droppablesHi(Collections::QListPtr<Droppable> value) { droppablesHiPtr = value.Ptr; }
    // 0x7c44a18
    Collections::QListPtr<Droppable> get_droppablesLo() const { return Collections::QListPtr<Droppable>(droppablesLoPtr); }
    // 0x7c44a54
    void set_droppablesLo(Collections::QListPtr<Droppable> value) { droppablesLoPtr = value.Ptr; }

    // 0x7c44a5c
    std::int32_t GetHashCode() const;
    // 0x7c44eb8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c44c98 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c44acc - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c44e34 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c44c14 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(DroppableManager) == DroppableManager::SIZE);

struct alignas(8) ElementalBuildUpMultiplier {
    static constexpr const char* ComponentTypeName = "ElementalBuildUpMultiplier";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    Ptr electricalBuildUpMultipliersPtr;
    Ptr iceBuildUpMultipliersPtr;
    FP electricalBuildUpMultiplier;
    FP iceBuildUpMultiplier;

    // 0x7c45078
    Collections::QListPtr<FP> get_electricalBuildUpMultipliers() const { return Collections::QListPtr<FP>(electricalBuildUpMultipliersPtr); }
    // 0x7c450b4
    void set_electricalBuildUpMultipliers(Collections::QListPtr<FP> value) { electricalBuildUpMultipliersPtr = value.Ptr; }
    // 0x7c450bc
    Collections::QListPtr<FP> get_iceBuildUpMultipliers() const { return Collections::QListPtr<FP>(iceBuildUpMultipliersPtr); }
    // 0x7c450f8
    void set_iceBuildUpMultipliers(Collections::QListPtr<FP> value) { iceBuildUpMultipliersPtr = value.Ptr; }

    // 0x7c45100
    std::int32_t GetHashCode() const;
    // 0x7c4546c
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c452cc - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c45160 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c453e8 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c45248 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(ElementalBuildUpMultiplier) == ElementalBuildUpMultiplier::SIZE);

struct alignas(8) EliminationGameMode {
    static constexpr const char* ComponentTypeName = "EliminationGameMode";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean CloseToWinSlowMotionDone;
    Ptr DeadPlayersPtr;
    AssetRefEliminationGameModeSpec Spec;

    // 0x7c4581c
    Collections::QListPtr<EntityRef> get_DeadPlayers() const { return Collections::QListPtr<EntityRef>(DeadPlayersPtr); }
    // 0x7c45858
    void set_DeadPlayers(Collections::QListPtr<EntityRef> value) { DeadPlayersPtr = value.Ptr; }

    // 0x7c45860
    std::int32_t GetHashCode() const;
    // 0x7c45aec
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c459b4 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c4589c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c45a68 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c45930 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(EliminationGameMode) == EliminationGameMode::SIZE);

struct alignas(8) EnergyCellManager {
    static constexpr const char* ComponentTypeName = "EnergyCellManager";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefEnergyCellManagerSpec Spec;

    // 0x7c45bd8 - Gives the charge of an energy cell of the given type (per the manager's
    // spec) to the collector (implemented in systems/game_mode_libs.cpp).
    static void TryGiveEnergyCell(Frame* frame, EntityRef collectorEntity, DroppableType energyCellType);
    // 0x7c45d30 - Positive charges go to the collector's ultimate, with an event.
    static void TryGiveEnergyCell(Frame* frame, EntityRef collectorEntity, std::int32_t charge);
    // 0x7c45cdc - The charge of one cell type; 0 for other droppables.
    static std::int32_t GetCharge(DroppableType energyCellType, const EnergyCellManagerSpec* spec);

    // 0x7c45ba8
    std::int32_t GetHashCode() const;
    // 0x7c45bc4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(EnergyCellManager) == EnergyCellManager::SIZE);

struct alignas(8) EnforcerFirstShotBonusEnhancement {
    static constexpr const char* ComponentTypeName = "EnforcerFirstShotBonusEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefEnforcerFirstShotBonusEnhancementSpec Spec;

    // 0x7c45e98
    std::int32_t GetHashCode() const;
    // 0x7c45eb4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(EnforcerFirstShotBonusEnhancement) == EnforcerFirstShotBonusEnhancement::SIZE);

struct alignas(8) EntityDropper {
    static constexpr const char* ComponentTypeName = "EntityDropper";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    Ptr SpecsPtr;
    std::uint8_t _pad0[4];
    FP lastPushDropTime;

    // 0x7c45ec8
    Collections::QListPtr<AssetRefEntityDropperSpec> get_Specs() const { return Collections::QListPtr<AssetRefEntityDropperSpec>(SpecsPtr); }
    // 0x7c45f04
    void set_Specs(Collections::QListPtr<AssetRefEntityDropperSpec> value) { SpecsPtr = value.Ptr; }

    // 0x7c45f0c
    std::int32_t GetHashCode() const;
    // 0x7c46184
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c4604c - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c45f34 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c46100 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c45fc8 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(EntityDropper) == EntityDropper::SIZE);

struct alignas(4) EntityGroupStats {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::int32_t EntityGroupMaxMembers;
    QBoolean IsInitialised;

    // 0x7b97ba4
    std::int32_t GetHashCode() const;
    // 0x7b97bd0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(EntityGroupStats) == EntityGroupStats::SIZE);

struct alignas(8) EntityGroup {
    static constexpr const char* ComponentTypeName = "EntityGroup";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t LastFrameGroupFull;
    Ptr GroupMembersPtr;
    AssetRefEntityGroupSpec Spec;
    EntityGroupStats Stats;

    // 0x7c46228
    Collections::QListPtr<EntityGroupMember> get_GroupMembers() const { return Collections::QListPtr<EntityGroupMember>(GroupMembersPtr); }
    // 0x7c46264
    void set_GroupMembers(Collections::QListPtr<EntityGroupMember> value) { GroupMembersPtr = value.Ptr; }

    // 0x7c4626c
    std::int32_t GetHashCode() const;
    // 0x7c4652c
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c463f4 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c462dc - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c464a8 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c46370 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(EntityGroup) == EntityGroup::SIZE);

struct alignas(1) EntityGroupMemberGrouped {
    static constexpr const char* ComponentTypeName = "EntityGroupMemberGrouped";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c46668
    std::int32_t GetHashCode() const;
    // 0x7c46670
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(EntityGroupMemberGrouped) == EntityGroupMemberGrouped::SIZE);

// quantum.core has a Quantum.EntityInfo of its own (the entity table record), so this
// component lives in Quantum::Code.
namespace Code {
struct alignas(4) EntityInfo {
    static constexpr const char* ComponentTypeName = "EntityInfo";
    static constexpr int SIZE = 12;
    static constexpr int ALIGNMENT = 4;

    EntityInfoType type;
    std::int32_t id;
    QBoolean isActiveObjective;

    // 0x7c46674
    std::int32_t GetHashCode() const;
    // 0x7c466b4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(EntityInfo) == EntityInfo::SIZE);

} // namespace Code

struct alignas(8) FireReactor {
    static constexpr const char* ComponentTypeName = "FireReactor";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    QBoolean IsEnabled;
    std::uint8_t _pad0[4];
    AssetRefFireReactorSpec Spec;
    FP TimeBeforeNextCycle;

    // 0x7c467e8
    std::int32_t GetHashCode() const;
    // 0x7c4683c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(FireReactor) == FireReactor::SIZE);

struct alignas(4) GOAPState {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    EWorldState Negative;
    EWorldState Positive;

    // 0x7b80574
    std::int32_t GetHashCode() const;
    // 0x7b80958
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GOAPState) == GOAPState::SIZE);

struct alignas(8) GOAPAgent {
    static constexpr const char* ComponentTypeName = "GOAPAgent";
    static constexpr int SIZE = 120;
    static constexpr int ALIGNMENT = 8;

    std::uint8_t CurrentPlanSize;
    std::int8_t CurrentActionIndex;
    std::int8_t LastProcessedActionIndex;
    std::uint8_t _pad0[1];
    Ptr GoalDisableTimesPtr;
    AssetRefAIConfig Config;
    std::array<AssetRefGOAPAction, 6> _Plan_;
    AssetRefGOAPGoal CurrentGoal;
    AssetRefGOAPRoot Root;
    FP CurrentActionTime;
    FP CurrentGoalTime;
    FP InterruptionCheckCooldown;
    GOAPState CurrentState;
    GOAPState GoalState;

    // 0x7b80298
    Collections::QListPtr<FP> get_GoalDisableTimes() const { return Collections::QListPtr<FP>(GoalDisableTimesPtr); }
    // 0x7b802d4
    std::span<AssetRefGOAPAction> get_Plan() { return _Plan_; }
    std::span<const AssetRefGOAPAction> get_Plan() const { return _Plan_; }

    // 0x7b8033c
    std::int32_t GetHashCode() const;
    // 0x7b80630
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b805a8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7b805b0 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(GOAPAgent) == GOAPAgent::SIZE);

struct alignas(8) GOAPData {
    static constexpr const char* ComponentTypeName = "GOAPData";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefGOAPHeuristic HeuristicCost;

    // 0x7c46884
    std::int32_t GetHashCode() const;
    // 0x7c468a0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GOAPData) == GOAPData::SIZE);

struct alignas(8) GadgetStats {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    FP Cooldown;

    // 0x7b97db4
    std::int32_t GetHashCode() const;
    // 0x7b97dd0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GadgetStats) == GadgetStats::SIZE);

struct alignas(8) GadgetUser {
    static constexpr const char* ComponentTypeName = "GadgetUser";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t activationFrame;
    std::int32_t cooldownEndFrame;
    std::int32_t cooldownStartFrame;
    std::uint8_t _pad0[4];
    AssetRefGadgetSpec gadgetSpec;
    GadgetStats Stats;

    // 0x7c468b4
    std::int32_t GetHashCode() const;
    // 0x7c4694c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GadgetUser) == GadgetUser::SIZE);

struct alignas(8) GameManager {
    static constexpr const char* ComponentTypeName = "GameManager";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t nbPlayersSpawned;
    std::uint8_t _pad0[4];
    AssetRefGameManagerConfig config;

    // 0x7c46b5c
    std::int32_t GetHashCode() const;
    // 0x7c46ba0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GameManager) == GameManager::SIZE);

struct alignas(4) GameModeActivator {
    static constexpr const char* ComponentTypeName = "GameModeActivator";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    GAME_MODIFICATOR modificator;
    QBoolean enabled;

    // 0x7c47590
    std::int32_t GetHashCode() const;
    // 0x7c475a8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GameModeActivator) == GameModeActivator::SIZE);

struct alignas(4) GameModeHandlerStats {
    static constexpr int SIZE = 28;
    static constexpr int ALIGNMENT = 4;

    std::array<std::int32_t, 4> leadChangesForMinute;
    std::int32_t mainloopEndFrame;
    std::int32_t mainloopStartFrame;
    std::int32_t totalLeadChanges;

    // 0x7b97de4
    std::int32_t GetHashCode() const;
    // 0x7b97e94
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GameModeHandlerStats) == GameModeHandlerStats::SIZE);

struct alignas(8) GameModeHandler {
    static constexpr const char* ComponentTypeName = "GameModeHandler";
    static constexpr int SIZE = 192;
    static constexpr int ALIGNMENT = 8;

    GAME_MODE_HANDLER_STATE state;
    std::int32_t averageHumanHeroLevel;
    std::int32_t finishFrame;
    std::int32_t introFlags;
    std::int32_t randomBotCharacterSelectionOffset;
    std::int32_t randomBotWeaponSelectionOffset;
    std::int32_t stateStartFrame;
    QBoolean averageHumanHeroLevelIsComputed;
    QBoolean boltMultiplierOn;
    QBoolean debugSkipRequested;
    QBoolean isPaused;
    QBoolean unlimitedTimeCheatActive;
    Ptr playersCharacterIDsPtr;
    Ptr playersSkinsIDsPtr;
    std::uint32_t nPlayersReadyAfterLoadingAddressables;
    std::uint32_t nPlayersReadyForCurrentGameMode;
    std::uint32_t numBotsSpawned;
    std::uint32_t numHumanPlayersSpawned;
    AssetRefGameModeHandlerConfig config;
    AssetRefGameModeHandlerLevelConfig levelConfig;
    AssetRefGameModeSpec gameModeSpecRef;
    AssetRefGameModesTable gameModesTable;
    EntityRef entity;
    EntityRef gameModeEntity;
    FP averageHumanWeaponLevel;
    FP modeEndBotsTimeoutCounter;
    FP pausedTime;
    FP waitExpectedHumanPlayersTimer;
    FP waitUnexpectedPlayersTimer;
    GameModeHandlerStats stats;
    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c4766c
    Collections::QListPtr<std::int32_t> get_playersCharacterIDs() const { return Collections::QListPtr<std::int32_t>(playersCharacterIDsPtr); }
    // 0x7c476a8
    void set_playersCharacterIDs(Collections::QListPtr<std::int32_t> value) { playersCharacterIDsPtr = value.Ptr; }
    // 0x7c476b0
    Collections::QListPtr<std::int32_t> get_playersSkinsIDs() const { return Collections::QListPtr<std::int32_t>(playersSkinsIDsPtr); }
    // 0x7c476ec
    void set_playersSkinsIDs(Collections::QListPtr<std::int32_t> value) { playersSkinsIDsPtr = value.Ptr; }

    // 0x7c4839c - Time since the current state began, minus the time spent paused, in
    // unscaled time (implemented in systems/game_mode_libs.cpp).
    FP GetCurrentStateElapsedTime(Frame* f) const;
    // 0x7c483cc - Same, including paused time.
    FP GetCurrentStateElapsedTimeNoPause(Frame* f) const;

    // 0x7c476f4
    std::int32_t GetHashCode() const;
    // 0x7c47cec
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c47b4c - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c479e0 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c47c68 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c47ac8 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(GameModeHandler) == GameModeHandler::SIZE);

struct alignas(8) GameModifiersManager {
    static constexpr const char* ComponentTypeName = "GameModifiersManager";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefGameModifiersSpec Spec;

    // 0x7c496fc
    std::int32_t GetHashCode() const;
    // 0x7c49718
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GameModifiersManager) == GameModifiersManager::SIZE);

struct alignas(8) GatlingEnhancement {
    static constexpr const char* ComponentTypeName = "GatlingEnhancement";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean ShouldResetAmmoOnGadgetUse;
    std::uint8_t _pad0[4];
    AssetRefGatlingEnhancementSpec Spec;

    // 0x7c4972c
    std::int32_t GetHashCode() const;
    // 0x7c49754
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GatlingEnhancement) == GatlingEnhancement::SIZE);

struct alignas(8) GatlingWeapon {
    static constexpr const char* ComponentTypeName = "GatlingWeapon";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    Ptr fireRateLevelsPtr;
    std::uint8_t _pad0[4];
    FP spinUpValue;

    // 0x7c4978c
    Collections::QListPtr<GatlingFireRateLevel> get_fireRateLevels() const { return Collections::QListPtr<GatlingFireRateLevel>(fireRateLevelsPtr); }
    // 0x7c497c8
    void set_fireRateLevels(Collections::QListPtr<GatlingFireRateLevel> value) { fireRateLevelsPtr = value.Ptr; }

    // 0x7c497d0
    std::int32_t GetHashCode() const;
    // 0x7c49880
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c497f8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c49800 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(GatlingWeapon) == GatlingWeapon::SIZE);

struct alignas(1) GloveOfDoomWeapon {
    static constexpr const char* ComponentTypeName = "GloveOfDoomWeapon";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c49924
    std::int32_t GetHashCode() const;
    // 0x7c4992c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GloveOfDoomWeapon) == GloveOfDoomWeapon::SIZE);

struct alignas(4) GoldenBolt {
    static constexpr const char* ComponentTypeName = "GoldenBolt";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    QBoolean isCollected;

    // 0x7c49930
    std::int32_t GetHashCode() const;
    // 0x7c49940
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GoldenBolt) == GoldenBolt::SIZE);

struct alignas(4) GoldenBoltManager {
    static constexpr const char* ComponentTypeName = "GoldenBoltManager";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    Ptr PendingGoldBoltSpawnsPtr;
    Ptr SpawnedGoldenBoltsPtr;

    // 0x7c49954
    Collections::QListPtr<PendingGoldBoltSpawn> get_PendingGoldBoltSpawns() const { return Collections::QListPtr<PendingGoldBoltSpawn>(PendingGoldBoltSpawnsPtr); }
    // 0x7c49990
    void set_PendingGoldBoltSpawns(Collections::QListPtr<PendingGoldBoltSpawn> value) { PendingGoldBoltSpawnsPtr = value.Ptr; }
    // 0x7c49998
    Collections::QListPtr<SpawnedGoldenBoltData> get_SpawnedGoldenBolts() const { return Collections::QListPtr<SpawnedGoldenBoltData>(SpawnedGoldenBoltsPtr); }
    // 0x7c499d4
    void set_SpawnedGoldenBolts(Collections::QListPtr<SpawnedGoldenBoltData> value) { SpawnedGoldenBoltsPtr = value.Ptr; }

    // 0x7c499dc
    std::int32_t GetHashCode() const;
    // 0x7c49d24
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c49b6c - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c499f4 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c49ca0 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c49ae8 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(GoldenBoltManager) == GoldenBoltManager::SIZE);

struct alignas(4) GrenadeLauncherAftershockEnhancement {
    static constexpr const char* ComponentTypeName = "GrenadeLauncherAftershockEnhancement";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    Ptr SpecsPtr;

    // 0x7c4a8ec
    Collections::QListPtr<AssetRefGrenadeLauncherAftershockEnhancementSpec> get_Specs() const { return Collections::QListPtr<AssetRefGrenadeLauncherAftershockEnhancementSpec>(SpecsPtr); }
    // 0x7c4a928
    void set_Specs(Collections::QListPtr<AssetRefGrenadeLauncherAftershockEnhancementSpec> value) { SpecsPtr = value.Ptr; }

    // 0x7c4a930
    std::int32_t GetHashCode() const;
    // 0x7c4a9c8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c4a940 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c4a948 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(GrenadeLauncherAftershockEnhancement) == GrenadeLauncherAftershockEnhancement::SIZE);

struct alignas(8) GrenadeLauncherStunGrenadeEnhancement {
    static constexpr const char* ComponentTypeName = "GrenadeLauncherStunGrenadeEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefGrenadeLauncherStunGrenadeEnhancementSpec Spec;

    // 0x7c4aa5c
    std::int32_t GetHashCode() const;
    // 0x7c4aa78
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GrenadeLauncherStunGrenadeEnhancement) == GrenadeLauncherStunGrenadeEnhancement::SIZE);

struct alignas(8) GrinderActor {
    static constexpr const char* ComponentTypeName = "GrinderActor";
    static constexpr int SIZE = 160;
    static constexpr int ALIGNMENT = 8;

    std::int32_t currentRailStaticColliderIndex;
    std::int32_t startGrindingFrame;
    QBoolean disabled;
    QBoolean isGrinding;
    QBoolean isOnRail;
    QBoolean isSnapping;
    AssetRefGrinderActorConfig config;
    EntityRef currentRailEntity;
    EntityRef entity;
    FP accelCounter;
    FP currentDistanceOnRail;
    FP disableTimer;
    FP grindSign;
    FP snapCounter;
    FPVector3 grindVelocity;
    FPVector3 snapEndPos;
    FPVector3 snapStartPos;

    // 0x7c4aa8c
    std::int32_t GetHashCode() const;
    // 0x7c4ac40
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GrinderActor) == GrinderActor::SIZE);

struct alignas(8) GroundDashActor {
    static constexpr const char* ComponentTypeName = "GroundDashActor";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t startingLastFrame;
    QBoolean disabled;
    QBoolean isDashing;
    QBoolean isStarting;
    QBoolean shouldDash;
    std::uint8_t _pad0[4];
    AssetRefGroundDashActorConfig Spec;

    // 0x7c4c7dc
    std::int32_t GetHashCode() const;
    // 0x7c4c85c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GroundDashActor) == GroundDashActor::SIZE);

struct alignas(8) HeadHunterWallHackEnhancement {
    static constexpr const char* ComponentTypeName = "HeadHunterWallHackEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefHeadHunterWallHackEnhancementSpec Spec;

    // 0x7c4c990
    std::int32_t GetHashCode() const;
    // 0x7c4c9ac
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HeadHunterWallHackEnhancement) == HeadHunterWallHackEnhancement::SIZE);

struct alignas(8) HealSource {
    static constexpr const char* ComponentTypeName = "HealSource";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefHealSourceSpec Spec;

    // 0x7c4c9c0
    std::int32_t GetHashCode() const;
    // 0x7c4c9dc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HealSource) == HealSource::SIZE);

struct alignas(8) HealthStats {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    QBoolean Initialised;
    std::uint8_t _pad0[4];
    FP InitialMaxHP;
    FP MaxHP;

    // 0x7b98690
    std::int32_t GetHashCode() const;
    // 0x7b986e4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HealthStats) == HealthStats::SIZE);

struct alignas(8) Health {
    static constexpr const char* ComponentTypeName = "Health";
    static constexpr int SIZE = 112;
    static constexpr int ALIGNMENT = 8;

    std::int32_t regenFrameReference;
    Ptr ActiveOverhealthsPtr;
    AssetRefHealthSpec Spec;
    FP CurrentHP;
    FP CurrentOverHP;
    std::array<FP, 5> _MaxHpMultArray_;
    FP MaxOverHP;
    FP StartRegenHP;
    HealthStats Stats;

    // 0x7c4c9f0
    Collections::QListPtr<Overhealth> get_ActiveOverhealths() const { return Collections::QListPtr<Overhealth>(ActiveOverhealthsPtr); }
    // 0x7c4ca2c
    void set_ActiveOverhealths(Collections::QListPtr<Overhealth> value) { ActiveOverhealthsPtr = value.Ptr; }
    // 0x7c4ca34
    std::span<FP> get_MaxHpMultArray() { return _MaxHpMultArray_; }
    std::span<const FP> get_MaxHpMultArray() const { return _MaxHpMultArray_; }

    // 0x7c4d1c8 - Lowers the overhealth cap, trimming current overhealth to fit
    // (implemented in systems/health_systems.cpp).
    void DecreaseOverhealth(FP amount);

    // 0x7c4ca9c
    std::int32_t GetHashCode() const;
    // 0x7c4ce70
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c4cd38 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c4cc20 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c4cdec - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c4ccb4 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Health) == Health::SIZE);

struct alignas(8) HelipackActor {
    static constexpr const char* ComponentTypeName = "HelipackActor";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::int32_t startFrame;
    QBoolean disabled;
    QBoolean impulseIsDone;
    QBoolean isActive;
    QBoolean isExpired;
    std::uint8_t _pad0[4];
    AssetRefHelipackActorSpec Spec;
    FP bonusDuration;
    FP usageTime;

    // 0x7c4d1f8
    std::int32_t GetHashCode() const;
    // 0x7c4d2b0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HelipackActor) == HelipackActor::SIZE);

struct alignas(8) Hierarchy {
    static constexpr const char* ComponentTypeName = "Hierarchy";
    static constexpr int SIZE = 64;
    static constexpr int ALIGNMENT = 8;

    EntityRef Parent;
    FPVector3 Offset;
    FPQuaternion LocalRotation;

    // 0x7c4d798
    std::int32_t GetHashCode() const;
    // 0x7c4d7f4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Hierarchy) == Hierarchy::SIZE);

struct alignas(8) HoloshieldGloveHealZoneEnhancement {
    static constexpr const char* ComponentTypeName = "HoloshieldGloveHealZoneEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefHoloshieldGloveHealZoneEnhancementSpec Spec;

    // 0x7c4d83c
    std::int32_t GetHashCode() const;
    // 0x7c4d858
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HoloshieldGloveHealZoneEnhancement) == HoloshieldGloveHealZoneEnhancement::SIZE);

struct alignas(4) HoloshieldGloveShield {
    static constexpr const char* ComponentTypeName = "HoloshieldGloveShield";
    static constexpr int SIZE = 12;
    static constexpr int ALIGNMENT = 4;

    std::int32_t startFrame;
    QBoolean addedToGroup;
    VoidRepulserState state;

    // 0x7c4d86c
    std::int32_t GetHashCode() const;
    // 0x7c4d8b0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HoloshieldGloveShield) == HoloshieldGloveShield::SIZE);

struct alignas(1) HoloshieldGloveShields {
    static constexpr const char* ComponentTypeName = "HoloshieldGloveShields";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c4d9e4
    std::int32_t GetHashCode() const;
    // 0x7c4d9ec
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HoloshieldGloveShields) == HoloshieldGloveShields::SIZE);

struct alignas(1) HoloshieldGloveShieldsGrouped {
    static constexpr const char* ComponentTypeName = "HoloshieldGloveShieldsGrouped";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c4d9f0
    std::int32_t GetHashCode() const;
    // 0x7c4d9f8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HoloshieldGloveShieldsGrouped) == HoloshieldGloveShieldsGrouped::SIZE);

struct alignas(1) HoloshieldGloveWeapon {
    static constexpr const char* ComponentTypeName = "HoloshieldGloveWeapon";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c4d9fc
    std::int32_t GetHashCode() const;
    // 0x7c4da04
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HoloshieldGloveWeapon) == HoloshieldGloveWeapon::SIZE);

struct alignas(4) HoverBootsWeapon {
    static constexpr const char* ComponentTypeName = "HoverBootsWeapon";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    std::int32_t StartUseFrame;

    // 0x7c4da08
    std::int32_t GetHashCode() const;
    // 0x7c4da24
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(HoverBootsWeapon) == HoverBootsWeapon::SIZE);

struct alignas(8) ItemSpawner {
    static constexpr const char* ComponentTypeName = "ItemSpawner";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t frameAllDestroyed;
    std::int32_t spawnedItemCounter;
    std::int32_t spawnedItemId;
    QBoolean enabled;
    QBoolean spawning;
    Ptr SpawnedEntitiesPtr;
    AssetRefItemSpawnerConfig Config;

    // 0x7c4dad8
    Collections::QListPtr<EntityRef> get_SpawnedEntities() const { return Collections::QListPtr<EntityRef>(SpawnedEntitiesPtr); }
    // 0x7c4db14
    void set_SpawnedEntities(Collections::QListPtr<EntityRef> value) { SpawnedEntitiesPtr = value.Ptr; }

    // 0x7c4db1c
    std::int32_t GetHashCode() const;
    // 0x7c4de18
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c4dce0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c4dbc8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c4dd94 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c4dc5c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(ItemSpawner) == ItemSpawner::SIZE);

struct alignas(8) JumpAttack {
    static constexpr const char* ComponentTypeName = "JumpAttack";
    static constexpr int SIZE = 72;
    static constexpr int ALIGNMENT = 8;

    std::int32_t startFrame;
    QBoolean attackIsActive;
    QBoolean performedSlam;
    std::uint8_t _pad0[4];
    AssetRefJumpAttackSpec Spec;
    FPVector3 endPosition;
    FPVector3 startPosition;

    // 0x7c4e044
    std::int32_t GetHashCode() const;
    // 0x7c4e0e4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(JumpAttack) == JumpAttack::SIZE);

struct alignas(8) KccExtension {
    static constexpr const char* ComponentTypeName = "KccExtension";
    static constexpr int SIZE = 472;
    static constexpr int ALIGNMENT = 8;

    std::int32_t disableFrame;
    JumpState jumpState;
    KccSurfaceSetPriority surfaceSetPriority;
    KccSurfaceType currentSurfaceType;
    QBoolean Impulsed;
    QBoolean cancelUpwardPushCorrection;
    QBoolean enabled;
    QBoolean forceInputSnapOnGround;
    QBoolean forceOrientation;
    QBoolean grounded;
    QBoolean hasImpulseToConsume;
    QBoolean hasPushToConsume;
    QBoolean ignoreMoveSpeedModifier;
    QBoolean jumpRequested;
    QBoolean lockJump;
    QBoolean lockRotation;
    QBoolean lookTowardsCamera;
    QBoolean lookTowardsCameraSight;
    QBoolean manualJumped;
    QBoolean preserveForceLookInput;
    QBoolean resetPush;
    Ptr accelerationModifiersPtr;
    Ptr brakingModifiersPtr;
    Ptr gravityModifiersPtr;
    Ptr maxFallSpeedModifiersPtr;
    Ptr maxSpeedModifiersPtr;
    Ptr speedModifiersPtr;
    Ptr wallCheckExclusionsPtr;
    AssetRefKccExtensionConfig Config;
    EntityRef impulseTarget;
    FP airTime;
    FP autoJumpDownTimeAfterManualJumpCounter;
    FP autoJumpDownTimeCounter;
    FP baseJumpMultiplier;
    FP forceInputBlend;
    FP forceInputCounter;
    FP forceInputFallOff;
    FP forceInputMaxBlend;
    FP groundedTime;
    FP groundedTimeSinceJump;
    FP impulseMaxSpeed;
    FP impulseMaxVerticalSpeed;
    FP impulseTargetSnapLerpStrength;
    FP impulseTimer;
    FP lockJumpTimer;
    FP lockRotationTimer;
    FP maxSpeedModifier;
    FP moveSpeedModifier;
    FP orientationSharpness;
    FP velocityBlend;
    FPVector2 forceInput;
    FPVector3 forceLookInput;
    FPVector3 groundNormal;
    FPVector3 impulse;
    FPVector3 lastGroundedPos;
    FPVector3 push;
    FPVector3 pushVelocity;
    FPVector3 velocity;

    // 0x7c4e45c
    Collections::QListPtr<ModInfo> get_accelerationModifiers() const { return Collections::QListPtr<ModInfo>(accelerationModifiersPtr); }
    // 0x7c4e498
    void set_accelerationModifiers(Collections::QListPtr<ModInfo> value) { accelerationModifiersPtr = value.Ptr; }
    // 0x7c4e4a0
    Collections::QListPtr<ModInfo> get_brakingModifiers() const { return Collections::QListPtr<ModInfo>(brakingModifiersPtr); }
    // 0x7c4e4dc
    void set_brakingModifiers(Collections::QListPtr<ModInfo> value) { brakingModifiersPtr = value.Ptr; }
    // 0x7c4e4e4
    Collections::QListPtr<ModInfo> get_gravityModifiers() const { return Collections::QListPtr<ModInfo>(gravityModifiersPtr); }
    // 0x7c4e520
    void set_gravityModifiers(Collections::QListPtr<ModInfo> value) { gravityModifiersPtr = value.Ptr; }
    // 0x7c4e528
    Collections::QListPtr<ModInfo> get_maxFallSpeedModifiers() const { return Collections::QListPtr<ModInfo>(maxFallSpeedModifiersPtr); }
    // 0x7c4e564
    void set_maxFallSpeedModifiers(Collections::QListPtr<ModInfo> value) { maxFallSpeedModifiersPtr = value.Ptr; }
    // 0x7c4e56c
    Collections::QListPtr<ModInfo> get_maxSpeedModifiers() const { return Collections::QListPtr<ModInfo>(maxSpeedModifiersPtr); }
    // 0x7c4e5a8
    void set_maxSpeedModifiers(Collections::QListPtr<ModInfo> value) { maxSpeedModifiersPtr = value.Ptr; }
    // 0x7c4e5b0
    Collections::QListPtr<ModInfo> get_speedModifiers() const { return Collections::QListPtr<ModInfo>(speedModifiersPtr); }
    // 0x7c4e5ec
    void set_speedModifiers(Collections::QListPtr<ModInfo> value) { speedModifiersPtr = value.Ptr; }
    // 0x7c4e5f4
    Collections::QListPtr<EntityRef> get_wallCheckExclusions() const { return Collections::QListPtr<EntityRef>(wallCheckExclusionsPtr); }
    // 0x7c4e630
    void set_wallCheckExclusions(Collections::QListPtr<EntityRef> value) { wallCheckExclusionsPtr = value.Ptr; }

    // 0x7c4e638
    std::int32_t GetHashCode() const;
    // 0x7c4f25c
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c4ee9c - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c4eb80 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c4f1d8 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c4ee18 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(KccExtension) == KccExtension::SIZE);

struct alignas(8) KccModifier {
    static constexpr const char* ComponentTypeName = "KccModifier";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefKccModifierSpec Spec;

    // 0x7c55a08
    std::int32_t GetHashCode() const;
    // 0x7c55a24
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(KccModifier) == KccModifier::SIZE);

struct alignas(8) KillNextFrame {
    static constexpr const char* ComponentTypeName = "KillNextFrame";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t MarkedForKillFrame;
    KillType killType;
    EntityRef killer;
    FP KillDelay;

    // 0x7c55a38
    std::int32_t GetHashCode() const;
    // 0x7c55aa0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(KillNextFrame) == KillNextFrame::SIZE);

struct alignas(8) KillZone {
    static constexpr const char* ComponentTypeName = "KillZone";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    Ptr areasPtr;
    std::uint8_t _pad0[4];
    FP verticalThreshold;

    // 0x7c55be4
    Collections::QListPtr<KillZoneArea> get_areas() const { return Collections::QListPtr<KillZoneArea>(areasPtr); }
    // 0x7c55c20
    void set_areas(Collections::QListPtr<KillZoneArea> value) { areasPtr = value.Ptr; }

    // 0x7c55c28
    std::int32_t GetHashCode() const;
    // 0x7c55cd8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c55c50 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c55c58 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(KillZone) == KillZone::SIZE);

struct alignas(8) KilledByData {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t frame;
    KillType killType;
    QBoolean isOwnerUltimate;
    std::uint8_t _pad0[4];
    EntityRef owner;
    EntityRef source;
    EntityRef weapon;

    // 0x7b99748
    std::int32_t GetHashCode() const;
    // 0x7b997b4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(KilledByData) == KilledByData::SIZE);

struct alignas(8) Killable {
    static constexpr const char* ComponentTypeName = "Killable";
    static constexpr int SIZE = 88;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t reviveTotemId;
    std::uint8_t _pad0[2];
    DeathAnimationState deathAnimationState;
    std::int32_t deathCount;
    std::int32_t maxDeath;
    KillZoneSurfaceType lastKillzoneSurfaceType;
    QBoolean IsAlive;
    AssetRefDeathAnimationSpec deathAnimationSpec;
    ComponentPrototypeRef Prototype;
    KilledByData killedBy;

    // 0x7c560fc - The Killable prototype the component was made from (implemented in
    // systems/combat_libs.cpp; see systems/asset_decls.h).
    Prototypes::Killable_Prototype* GetPrototype(Frame* frame) const;

    // 0x7c55d7c
    std::int32_t GetHashCode() const;
    // 0x7c55e60
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Killable) == Killable::SIZE);

struct alignas(8) LedgeGrabbingActor {
    static constexpr const char* ComponentTypeName = "LedgeGrabbingActor";
    static constexpr int SIZE = 152;
    static constexpr int ALIGNMENT = 8;

    std::int32_t LedgeGrabbingStartFrame;
    QBoolean IsLedgeGrabbing;
    AssetRefLedgeGrabbingSpec Spec;
    FPVector3 Destination;
    FPVector3 GrabStartPosition;
    FPVector3 SnapPosition;
    FPQuaternion GrabStartRotation;
    FPQuaternion SnapRotation;

    // 0x7c56154
    std::int32_t GetHashCode() const;
    // 0x7c56238
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(LedgeGrabbingActor) == LedgeGrabbingActor::SIZE);

struct alignas(8) LevelButton {
    static constexpr const char* ComponentTypeName = "LevelButton";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t lastActivationFrame;
    QBoolean hasBeenHit;
    QBoolean isOnCooldown;
    std::uint8_t _pad0[4];
    AssetRefLevelButtonConfig Config;
    EntityRef entity;
    EntityRef gameplayElementEntity;

    // 0x7c5635c
    std::int32_t GetHashCode() const;
    // 0x7c563f4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(LevelButton) == LevelButton::SIZE);

struct alignas(8) LevelButtonGameplayElement {
    static constexpr const char* ComponentTypeName = "LevelButtonGameplayElement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefLevelButtonGameplayElementConfig Config;

    // 0x7c567c8
    std::int32_t GetHashCode() const;
    // 0x7c567e4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(LevelButtonGameplayElement) == LevelButtonGameplayElement::SIZE);

struct alignas(2) LevelHolder {
    static constexpr const char* ComponentTypeName = "LevelHolder";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 2;

    std::uint16_t Level;
    std::array<std::uint8_t, 2> _alignment_padding_;

    // 0x7c567f8
    std::int32_t GetHashCode() const;
    // 0x7c56814
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(LevelHolder) == LevelHolder::SIZE);

struct alignas(8) LevelUpActor {
    static constexpr const char* ComponentTypeName = "LevelUpActor";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefLevelUpActorSpec Spec;

    // 0x7c568c8
    std::int32_t GetHashCode() const;
    // 0x7c568e4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(LevelUpActor) == LevelUpActor::SIZE);

struct alignas(8) LifetimeStats {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean initialised;
    QBoolean spawnEntitiesOnDeath;
    FP lifetimeDuration;

    // 0x7b99a38
    std::int32_t GetHashCode() const;
    // 0x7b99a7c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(LifetimeStats) == LifetimeStats::SIZE);

struct alignas(8) Lifetime {
    static constexpr const char* ComponentTypeName = "Lifetime";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t StartFrame;
    Ptr EntitiesToSpawnOnDeathPtr;
    AssetRefLifetimeSpec Spec;
    LifetimeStats Stats;

    // 0x7c568f8
    Collections::QListPtr<AssetRefEntityPrototype> get_EntitiesToSpawnOnDeath() const { return Collections::QListPtr<AssetRefEntityPrototype>(EntitiesToSpawnOnDeathPtr); }
    // 0x7c56934
    void set_EntitiesToSpawnOnDeath(Collections::QListPtr<AssetRefEntityPrototype> value) { EntitiesToSpawnOnDeathPtr = value.Ptr; }

    // 0x7c5693c
    std::int32_t GetHashCode() const;
    // 0x7c56a38
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c569b0 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c569b8 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Lifetime) == Lifetime::SIZE);

struct alignas(8) MeleeAttack {
    static constexpr const char* ComponentTypeName = "MeleeAttack";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t startFrame;
    QBoolean attackHit;
    QBoolean attackIsActive;
    QBoolean attackMissed;
    AssetRefMeleeAttackSpec Spec;
    EntityRef spawnedAreaOfEffect;
    FP meleeDamageMultiplier;

    // 0x7c56b74
    std::int32_t GetHashCode() const;
    // 0x7c56c18
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(MeleeAttack) == MeleeAttack::SIZE);

struct alignas(8) MeleeJumpAttack {
    static constexpr const char* ComponentTypeName = "MeleeJumpAttack";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t stateStartFrame;
    MeleeJumpAttackState state;
    AssetRefMeleeJumpAttackSpec spec;

    // 0x7c56e28
    std::int32_t GetHashCode() const;
    // 0x7c56e7c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(MeleeJumpAttack) == MeleeJumpAttack::SIZE);

struct alignas(8) MineLauncherEnhancement {
    static constexpr const char* ComponentTypeName = "MineLauncherEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefWeaponEnhancementSpec Spec;

    // 0x7c56fb0
    std::int32_t GetHashCode() const;
    // 0x7c56fcc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(MineLauncherEnhancement) == MineLauncherEnhancement::SIZE);

struct alignas(4) MineMerger {
    static constexpr const char* ComponentTypeName = "MineMerger";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::int32_t mergeCount;
    QBoolean wasMergeChecked;

    // 0x7c56fe0
    std::int32_t GetHashCode() const;
    // 0x7c5700c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(MineMerger) == MineMerger::SIZE);

struct alignas(8) MobGroup {
    static constexpr const char* ComponentTypeName = "MobGroup";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t NumToSpawn;
    Ptr MembersPtr;
    AssetRefMobGroupConfig SwarmerConfig;
    EntityRef ClosestMemberToTarget;
    EntityRef TargetEntity;

    // 0x7c570d0
    Collections::QListPtr<EntityRef> get_Members() const { return Collections::QListPtr<EntityRef>(MembersPtr); }
    // 0x7c5710c
    void set_Members(Collections::QListPtr<EntityRef> value) { MembersPtr = value.Ptr; }

    // 0x7c57114
    std::int32_t GetHashCode() const;
    // 0x7c573e0
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c572a8 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c57190 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5735c - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c57224 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(MobGroup) == MobGroup::SIZE);

struct alignas(8) MobGroupMember {
    static constexpr const char* ComponentTypeName = "MobGroupMember";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    EntityRef groupEntity;

    // 0x7c57888
    std::int32_t GetHashCode() const;
    // 0x7c578a0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(MobGroupMember) == MobGroupMember::SIZE);

struct alignas(8) MobInfo {
    static constexpr const char* ComponentTypeName = "MobInfo";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    MobType Type;
    QBoolean UseOwnerLevel;
    AssetRefMobInfoSpec Spec;

    // 0x7c578b4
    std::int32_t GetHashCode() const;
    // 0x7c578f0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(MobInfo) == MobInfo::SIZE);

struct alignas(8) MoveTo {
    static constexpr const char* ComponentTypeName = "MoveTo";
    static constexpr int SIZE = 144;
    static constexpr int ALIGNMENT = 8;

    EasingType easing;
    std::int32_t moveStartFrame;
    std::int32_t waitStartFrame;
    MoveMode moveMode;
    MoveOffsetOrientation offsetOrientation;
    MoveState moveState;
    QBoolean followTarget;
    QBoolean isInitialized;
    Ptr onMoveFinishedPtr;
    std::uint8_t _pad0[4];
    AssetRefMoveToSpec moveToSpec;
    EntityRef targetEntity;
    FP delay;
    FP duration;
    FPVector3 moveStartPosition;
    FPVector3 offset;
    FPVector3 targetPosition;

    // 0x7c579c4
    Collections::QListPtr<AssetRefCallbackAction> get_onMoveFinished() const { return Collections::QListPtr<AssetRefCallbackAction>(onMoveFinishedPtr); }
    // 0x7c57a00
    void set_onMoveFinished(Collections::QListPtr<AssetRefCallbackAction> value) { onMoveFinishedPtr = value.Ptr; }

    // 0x7c57a08
    std::int32_t GetHashCode() const;
    // 0x7c57dd8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c57ca0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c57b88 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c57d54 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c57c1c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(MoveTo) == MoveTo::SIZE);

struct alignas(8) MovementPackCache {
    static constexpr const char* ComponentTypeName = "MovementPackCache";
    static constexpr int SIZE = 56;
    static constexpr int ALIGNMENT = 8;

    std::int32_t updateFrequency;
    std::uint8_t _pad0[4];
    FPVector3 slopeTangent;
    FPVector3 tangent;

    // 0x7c58c34
    std::int32_t GetHashCode() const;
    // 0x7c58c94
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(MovementPackCache) == MovementPackCache::SIZE);

struct alignas(8) MovingWall {
    static constexpr const char* ComponentTypeName = "MovingWall";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::int32_t navmeshRegionID;
    std::int32_t offsetIndex;
    std::int32_t stoppedFrame;
    QBoolean stopped;
    AssetRefMovingWallConfig Config;
    FPVector3 initialPosition;

    // 0x7c58d68
    std::int32_t GetHashCode() const;
    // 0x7c58e14
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(MovingWall) == MovingWall::SIZE);

struct alignas(4) OnKillCallbacks {
    static constexpr const char* ComponentTypeName = "OnKillCallbacks";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    Ptr onPlayerKilledPtr;

    // 0x7c59158
    Collections::QListPtr<AssetRefCallbackAction> get_onPlayerKilled() const { return Collections::QListPtr<AssetRefCallbackAction>(onPlayerKilledPtr); }
    // 0x7c59194
    void set_onPlayerKilled(Collections::QListPtr<AssetRefCallbackAction> value) { onPlayerKilledPtr = value.Ptr; }

    // 0x7c5919c
    std::int32_t GetHashCode() const;
    // 0x7c593fc
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c592c4 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c591ac - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c59378 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c59240 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(OnKillCallbacks) == OnKillCallbacks::SIZE);

struct alignas(8) OnTriggerEnter {
    static constexpr const char* ComponentTypeName = "OnTriggerEnter";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t ActivationFrame;
    Ptr onTriggerEnterPtr;
    ComponentPrototypeRef Prototype;

    // 0x7c59490
    Collections::QListPtr<AssetRefCallbackAction> get_onTriggerEnter() const { return Collections::QListPtr<AssetRefCallbackAction>(onTriggerEnterPtr); }
    // 0x7c594cc
    void set_onTriggerEnter(Collections::QListPtr<AssetRefCallbackAction> value) { onTriggerEnterPtr = value.Ptr; }

    // 0x7c594d4
    std::int32_t GetHashCode() const;
    // 0x7c59774
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5963c - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c59524 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c596f0 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c595b8 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(OnTriggerEnter) == OnTriggerEnter::SIZE);

struct alignas(8) Ownership {
    static constexpr const char* ComponentTypeName = "Ownership";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    QBoolean IsOwnerUltimate;
    std::uint8_t _pad0[4];
    EntityRef Owner;
    EntityRef Source;
    EntityRef Weapon;

    // 0x7c5999c - Adds or updates the entity's ownership; owner, weapon and source are
    // only overwritten by valid entities (implemented in systems/combat_libs.cpp).
    static void SetOwnerShip(Frame* frame, EntityRef entity, EntityRef owner, EntityRef weapon,
                             EntityRef directSource, bool isOwnerUltimate);
    // 0x7c59ab0
    static bool TryGetOwnership(Frame* frame, EntityRef entity, Ownership*& ownership);

    // 0x7c598f8
    std::int32_t GetHashCode() const;
    // 0x7c59944
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Ownership) == Ownership::SIZE);

struct alignas(8) ParabolaFollowerParams {
    static constexpr int SIZE = 64;
    static constexpr int ALIGNMENT = 8;

    QBoolean WillStopAtEndPos;
    std::uint8_t _pad0[4];
    FP MaxHeight;
    FPVector3 End;
    FPVector3 Start;

    // 0x7b9a138
    std::int32_t GetHashCode() const;
    // 0x7b9a1a4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ParabolaFollowerParams) == ParabolaFollowerParams::SIZE);

struct alignas(8) ParabolaFollower {
    static constexpr const char* ComponentTypeName = "ParabolaFollower";
    static constexpr int SIZE = 136;
    static constexpr int ALIGNMENT = 8;

    std::int32_t RemoveRequestTick;
    std::int32_t StartFrame;
    REMOVE_REASON RemoveRequestReason;
    std::uint8_t _pad0[4];
    AssetRefParabolaFollowerSpec Spec;
    EntityRef HomingTarget;
    FP LastUsedT;
    FP TotalDuration;
    FPVector3 LastPosition;
    ParabolaFollowerParams Params;

    // 0x7c59eac
    std::int32_t GetHashCode() const;
    // 0x7c59fa4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ParabolaFollower) == ParabolaFollower::SIZE);

struct alignas(8) PendingPhysicsImpulse {
    static constexpr const char* ComponentTypeName = "PendingPhysicsImpulse";
    static constexpr int SIZE = 64;
    static constexpr int ALIGNMENT = 8;

    std::int32_t impulseTime;
    QBoolean processed;
    FP impulseDelay;
    FPVector3 hitpoint;
    FPVector3 impulse;

    // 0x7c5a190
    std::int32_t GetHashCode() const;
    // 0x7c5a21c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PendingPhysicsImpulse) == PendingPhysicsImpulse::SIZE);

struct alignas(8) PushData {
    static constexpr int SIZE = 136;
    static constexpr int ALIGNMENT = 8;

    BumpType bumpType;
    PushOnShieldBehavior onShieldBehaviour;
    QBoolean bypassPrePushMultiplier;
    QBoolean bypassPushStop;
    QBoolean canBump;
    QBoolean forcePushStop;
    QBoolean isContinuous;
    std::uint8_t _pad0[4];
    EntityRef pushed;
    EntityRef pusher;
    EntityRef pusherOwner;
    FP bumpPercentage;
    FP flatBumpStrength;
    FP forcePushStopDuration;
    FP forcePushStopStrength;
    FPVector3 hitpoint;
    FPVector3 impulse;

    // 0x7b9ac24
    std::int32_t GetHashCode() const;
    // 0x7b9ad70
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PushData) == PushData::SIZE);

struct alignas(8) PendingPushStop {
    static constexpr const char* ComponentTypeName = "PendingPushStop";
    static constexpr int SIZE = 152;
    static constexpr int ALIGNMENT = 8;

    QBoolean isContinuous;
    std::uint8_t _pad0[4];
    FP pushStopDuration;
    PushData pendingData;

    // 0x7c5a310
    std::int32_t GetHashCode() const;
    // 0x7c5a364
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PendingPushStop) == PendingPushStop::SIZE);

struct alignas(8) PlayerActor {
    static constexpr const char* ComponentTypeName = "PlayerActor";
    static constexpr int SIZE = 144;
    static constexpr int ALIGNMENT = 8;

    std::int16_t killCount;
    std::uint16_t introTrackId;
    std::uint16_t winStreak;
    std::uint8_t _pad0[2];
    std::int32_t actorID;
    std::int32_t characterIndex;
    std::int32_t combatStartFrame;
    std::int32_t eliminationFrame;
    std::int32_t heroLevel;
    std::int32_t heroTalentLevel;
    std::int32_t killStreak;
    std::int32_t lastKillFrame;
    std::int32_t rngSeed;
    std::int32_t secondarySkinIndex;
    std::int32_t skinIndex;
    PlayerRef playerRef;
    QBoolean gameModeFinished;
    QBoolean hasDisconnected;
    QBoolean isBot;
    QBoolean isInactive;
    EntityRef inCombatWith;
    EntityRef rivalEntity;
    FP durationBeforeCombatReset;
    FP maxNoActionTimeAfterPush;
    FP noActionCounter;
    FP noActionMaxPushForce;
    FP noActionPushForceThreshold;
    FP noActionTimeAfterPush;
    FP timeWithoutCombatActivity;

    // 0x7c5ad84 - Not a bot (implemented in systems/combat_libs.cpp).
    bool get_IsHuman() const;

    // 0x7c5a3ac
    std::int32_t GetHashCode() const;
    // 0x7c5a684
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PlayerActor) == PlayerActor::SIZE);

struct alignas(1) PlayerActorInitialized {
    static constexpr const char* ComponentTypeName = "PlayerActorInitialized";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c5aecc
    std::int32_t GetHashCode() const;
    // 0x7c5aed4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PlayerActorInitialized) == PlayerActorInitialized::SIZE);

struct alignas(8) PowerUp {
    static constexpr const char* ComponentTypeName = "PowerUp";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean IsPickedUp;
    std::uint8_t _pad0[4];
    AssetRefPowerUpSpec Spec;

    // 0x7c5aed8
    std::int32_t GetHashCode() const;
    // 0x7c5af00
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PowerUp) == PowerUp::SIZE);

struct alignas(4) PowerUpUser {
    static constexpr const char* ComponentTypeName = "PowerUpUser";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    Ptr ActivePowerUpsPtr;

    // 0x7c5af38
    Collections::QListPtr<ActivePowerUp> get_ActivePowerUps() const { return Collections::QListPtr<ActivePowerUp>(ActivePowerUpsPtr); }
    // 0x7c5af74
    void set_ActivePowerUps(Collections::QListPtr<ActivePowerUp> value) { ActivePowerUpsPtr = value.Ptr; }

    // 0x7c5af7c
    std::int32_t GetHashCode() const;
    // 0x7c5b1dc
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5b0a4 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c5af8c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5b158 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c5b020 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(PowerUpUser) == PowerUpUser::SIZE);

struct alignas(8) PreciseHitReceiver {
    static constexpr const char* ComponentTypeName = "PreciseHitReceiver";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefPreciseHitReceiverSpec Spec;

    // 0x7c5b270
    std::int32_t GetHashCode() const;
    // 0x7c5b28c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PreciseHitReceiver) == PreciseHitReceiver::SIZE);

struct alignas(8) PrioritisedTarget {
    static constexpr int SIZE = 56;
    static constexpr int ALIGNMENT = 8;

    std::int32_t acquiredFrame;
    std::int32_t currentWaypointIndex;
    std::int32_t lostFrame;
    Ptr waypointsPtr;
    EntityRef entity;
    FP waypointRadius;
    FPVector3 entityPosition;

    // 0x7b9a7e4
    Collections::QListPtr<FPVector3> get_waypoints() const { return Collections::QListPtr<FPVector3>(waypointsPtr); }
    // 0x7b9a820
    void set_waypoints(Collections::QListPtr<FPVector3> value) { waypointsPtr = value.Ptr; }

    // 0x7b9a828
    std::int32_t GetHashCode() const;
    // 0x7b9a8ec
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b9a8e4 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
};
static_assert(sizeof(PrioritisedTarget) == PrioritisedTarget::SIZE);

struct alignas(8) PrioritisedTargetHandler {
    static constexpr const char* ComponentTypeName = "PrioritisedTargetHandler";
    static constexpr int SIZE = 288;
    static constexpr int ALIGNMENT = 8;

    AssetRefPrioritisedTargetHandlerSpec spec;
    std::array<PrioritisedTarget, 5> _targets_;

    // 0x7c5b2a0
    std::span<PrioritisedTarget> get_targets() { return _targets_; }
    std::span<const PrioritisedTarget> get_targets() const { return _targets_; }

    // 0x7c5b308
    std::int32_t GetHashCode() const;
    // 0x7c5b50c
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5b3e0 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5b478 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(PrioritisedTargetHandler) == PrioritisedTargetHandler::SIZE);

struct alignas(8) Projectile {
    static constexpr const char* ComponentTypeName = "Projectile";
    static constexpr int SIZE = 384;
    static constexpr int ALIGNMENT = 8;

    std::int32_t ImpactFrame;
    std::int32_t LastEntitySpawnFrame;
    std::int32_t RaycastQueryIndex;
    std::int32_t ReachedTargetFrame;
    std::int32_t SpawnedFrame;
    ProjectileDeathType DeathType;
    ProjectileTargetedPart TargetedPart;
    QBoolean HasValidCollisionImpact;
    QBoolean ReachedTarget;
    QBoolean TargetPosIsRangedRaycastHit;
    QBoolean forceDisableHoming;
    Ptr entitiesToSpawnOnImpactPtr;
    AssetRefProjectileSpec Spec;
    EntityRef OwnerActor;
    EntityRef OwnerWeapon;
    EntityRef TargetEntity;
    FP CurrentHomingLerpFactor;
    FP CurveRemainingDistanceToTarget;
    FP ToLookPointRaycastLength;
    FP WeaponRange;
    FPVector3 CurrVelocity;
    FPVector3 NextPosition;
    FPVector3 OwnerWeaponHierarchyOffset;
    FPVector3 PreviousPosition;
    FPVector3 ReachedTargetPosition;
    FPVector3 SpawnOffset;
    FPVector3 SpawnedPosition;
    FPVector3 TargetPosition;
    Physics3D::Hit3D Impact;

    // 0x7c5c2cc
    Collections::QListPtr<AssetRefEntityPrototype> get_entitiesToSpawnOnImpact() const { return Collections::QListPtr<AssetRefEntityPrototype>(entitiesToSpawnOnImpactPtr); }
    // 0x7c5c308
    void set_entitiesToSpawnOnImpact(Collections::QListPtr<AssetRefEntityPrototype> value) { entitiesToSpawnOnImpactPtr = value.Ptr; }

    // 0x7c5c310
    std::int32_t GetHashCode() const;
    // TODO(0x7c5c834, Quantum.Projectile.Serialize): all fields in layout order; waits
    // on Hit3D serialisation in quantum_core (throws until then).
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5c6fc - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c5c5e4 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5c7b0 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c5c678 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Projectile) == Projectile::SIZE);

struct alignas(8) ProjectileDebugInfo {
    static constexpr const char* ComponentTypeName = "ProjectileDebugInfo";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    FPVector3 raycastEndPosition;
    FPVector3 raycastStartPosition;

    // 0x7c5ce60
    std::int32_t GetHashCode() const;
    // 0x7c5cea0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ProjectileDebugInfo) == ProjectileDebugInfo::SIZE);

struct alignas(8) ProjectileHitInfo {
    static constexpr const char* ComponentTypeName = "ProjectileHitInfo";
    static constexpr int SIZE = 152;
    static constexpr int ALIGNMENT = 8;

    ProjectileDeathType projectileDeathType;
    QBoolean HasValidCollisionImpact;
    EntityRef projectileOwner;
    EntityRef projectileWeapon;
    FPVector3 spawnPosition;
    FPVector3 velocity;
    Physics3D::Hit3D hit;

    // 0x7c5ced8
    std::int32_t GetHashCode() const;
    // TODO(0x7c5cf84, Quantum.ProjectileHitInfo.Serialize): all fields in layout order; waits
    // on Hit3D serialisation in quantum_core (throws until then).
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ProjectileHitInfo) == ProjectileHitInfo::SIZE);

struct alignas(8) ProjectileSpawner {
    static constexpr const char* ComponentTypeName = "ProjectileSpawner";
    static constexpr int SIZE = 56;
    static constexpr int ALIGNMENT = 8;

    std::int32_t CurrSpawnIndex;
    std::int32_t LastSpawnFrame;
    std::int32_t StartFrame;
    std::uint8_t _pad0[4];
    AssetRefProjectileSpawnerSpec Spec;
    FP RandomRad;
    FPVector3 Normal;

    // 0x7c5d098
    std::int32_t GetHashCode() const;
    // 0x7c5d14c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ProjectileSpawner) == ProjectileSpawner::SIZE);

struct alignas(8) PushedByData {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t frame;
    std::uint8_t _pad0[4];
    EntityRef owner;
    EntityRef source;
    EntityRef weapon;

    // 0x7b9af74
    std::int32_t GetHashCode() const;
    // 0x7b9afd0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PushedByData) == PushedByData::SIZE);

struct alignas(8) Pushable {
    static constexpr const char* ComponentTypeName = "Pushable";
    static constexpr int SIZE = 88;
    static constexpr int ALIGNMENT = 8;

    std::int32_t consecutivePushCount;
    std::int32_t pushModifierStatusEffectLastFrame;
    std::int32_t pushModifierStatusEffectStartFrame;
    std::int32_t pushStopStartFrame;
    QBoolean isInsideContinuousPush;
    QBoolean pushModifierStatusEffectRevertAfterPush;
    Ptr PendingPushRequestsPtr;
    std::uint8_t _pad0[4];
    AssetRefPushableSpec Spec;
    FP prePhysicsPushForceMultiplier;
    FP pushModifierStatusEffect;
    PushedByData pushedBy;

    // 0x7c5d308
    Collections::QListPtr<PushData> get_PendingPushRequests() const { return Collections::QListPtr<PushData>(PendingPushRequestsPtr); }
    // 0x7c5d344
    void set_PendingPushRequests(Collections::QListPtr<PushData> value) { PendingPushRequestsPtr = value.Ptr; }

    // 0x7c5d34c
    std::int32_t GetHashCode() const;
    // 0x7c5d6b8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5d580 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c5d468 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5d634 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c5d4fc - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Pushable) == Pushable::SIZE);

struct alignas(1) PushableTrigger {
    static constexpr const char* ComponentTypeName = "PushableTrigger";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c5e2c0
    std::int32_t GetHashCode() const;
    // 0x7c5e2c8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PushableTrigger) == PushableTrigger::SIZE);

struct alignas(8) Pusher {
    static constexpr const char* ComponentTypeName = "Pusher";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    AssetRefPusherSpec Spec;
    FP CurrentPusherRange;

    // 0x7c5e2cc
    std::int32_t GetHashCode() const;
    // 0x7c5e310
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Pusher) == Pusher::SIZE);

struct alignas(8) Rail {
    static constexpr const char* ComponentTypeName = "Rail";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t bakedRailIndex;
    Ptr grindersPtr;
    EntityRef entity;

    // 0x7c5e348
    Collections::QListPtr<EntityRef> get_grinders() const { return Collections::QListPtr<EntityRef>(grindersPtr); }
    // 0x7c5e384
    void set_grinders(Collections::QListPtr<EntityRef> value) { grindersPtr = value.Ptr; }

    // 0x7c5e38c
    std::int32_t GetHashCode() const;
    // 0x7c5e620
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5e4e8 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c5e3d0 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5e59c - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c5e464 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Rail) == Rail::SIZE);

struct alignas(8) RailCollider {
    static constexpr const char* ComponentTypeName = "RailCollider";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    EntityRef entity;
    EntityRef nearbyRailColLeft;
    EntityRef nearbyRailColRight;
    EntityRef railEntity;
    FP distanceOnRail;

    // 0x7c5e74c
    std::int32_t GetHashCode() const;
    // 0x7c5e7b8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RailCollider) == RailCollider::SIZE);

struct alignas(4) RandomWeaponsActor {
    static constexpr const char* ComponentTypeName = "RandomWeaponsActor";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    std::int32_t nextRandomWeaponFrame;

    // 0x7c5e828
    std::int32_t GetHashCode() const;
    // 0x7c5e844
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RandomWeaponsActor) == RandomWeaponsActor::SIZE);

struct alignas(4) RankingManager {
    static constexpr const char* ComponentTypeName = "RankingManager";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 4;

    std::int32_t lastUpdateFrame;
    QBoolean closeToWinSlowMotionDone;
    QBoolean needUpdate;
    Ptr rankingPtr;

    // 0x7c5e8f8
    Collections::QListPtr<RankingItem> get_ranking() const { return Collections::QListPtr<RankingItem>(rankingPtr); }
    // 0x7c5e934
    void set_ranking(Collections::QListPtr<RankingItem> value) { rankingPtr = value.Ptr; }

    // 0x7c5e93c
    std::int32_t GetHashCode() const;
    // 0x7c5ebd8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5eaa0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c5e988 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5eb54 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c5ea1c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(RankingManager) == RankingManager::SIZE);

struct alignas(8) RecoverAmmoOnDashEnhancement {
    static constexpr const char* ComponentTypeName = "RecoverAmmoOnDashEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefRecoverAmmoOnDashEnhancementSpec Spec;

    // 0x7c5ed14
    std::int32_t GetHashCode() const;
    // 0x7c5ed30
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RecoverAmmoOnDashEnhancement) == RecoverAmmoOnDashEnhancement::SIZE);

struct alignas(8) ReplaceMultiShotConfigsEnhancement {
    static constexpr const char* ComponentTypeName = "ReplaceMultiShotConfigsEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefReplaceMultiShotConfigsEnhancementSpec Spec;

    // 0x7c5ed44
    std::int32_t GetHashCode() const;
    // 0x7c5ed60
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ReplaceMultiShotConfigsEnhancement) == ReplaceMultiShotConfigsEnhancement::SIZE);

struct alignas(8) RewardReceiver {
    static constexpr const char* ComponentTypeName = "RewardReceiver";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    Ptr pendingRewardsPtr;
    std::uint8_t _pad0[4];
    AssetRefRewardReceiverSpec Spec;

    // 0x7c5ed74
    Collections::QListPtr<Reward> get_pendingRewards() const { return Collections::QListPtr<Reward>(pendingRewardsPtr); }
    // 0x7c5edb0
    void set_pendingRewards(Collections::QListPtr<Reward> value) { pendingRewardsPtr = value.Ptr; }

    // 0x7c5edb8
    std::int32_t GetHashCode() const;
    // 0x7c5f038
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5ef00 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c5ede8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5efb4 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c5ee7c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(RewardReceiver) == RewardReceiver::SIZE);

struct alignas(8) Roll {
    static constexpr const char* ComponentTypeName = "Roll";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    Ptr hitEntitiesPtr;
    std::uint32_t bounceCount;
    AssetRefRollSpec Spec;
    FPVector3 direction;

    // 0x7c5f0dc
    Collections::QListPtr<CollidingEntity> get_hitEntities() const { return Collections::QListPtr<CollidingEntity>(hitEntitiesPtr); }
    // 0x7c5f118
    void set_hitEntities(Collections::QListPtr<CollidingEntity> value) { hitEntitiesPtr = value.Ptr; }

    // 0x7c5f120
    std::int32_t GetHashCode() const;
    // 0x7c5f3dc
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5f2a4 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c5f18c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5f358 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c5f220 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Roll) == Roll::SIZE);

struct alignas(1) RunnableWall {
    static constexpr const char* ComponentTypeName = "RunnableWall";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c5f514
    std::int32_t GetHashCode() const;
    // 0x7c5f51c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RunnableWall) == RunnableWall::SIZE);

struct alignas(1) RunnableWallTriggerZone {
    static constexpr const char* ComponentTypeName = "RunnableWallTriggerZone";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c5f520
    std::int32_t GetHashCode() const;
    // 0x7c5f528
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RunnableWallTriggerZone) == RunnableWallTriggerZone::SIZE);

struct alignas(4) RynoStats {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    QBoolean initialised;
    std::uint32_t maxMissilesPerFire;

    // 0x7b9bbf8
    std::int32_t GetHashCode() const;
    // 0x7b9bc20
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RynoStats) == RynoStats::SIZE);

struct alignas(8) RynoWeapon {
    static constexpr const char* ComponentTypeName = "RynoWeapon";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t LastFiredTargetIndex;
    std::int32_t LastFiredTime;
    std::int32_t StartLockingFrame;
    Ptr LockedTargetsPtr;
    std::uint32_t RemainingMissiles;
    std::uint8_t _pad0[4];
    EntityRef StartLockingWeaponUser;
    RynoStats Stats;

    // 0x7c5fa3c
    Collections::QListPtr<LockedTarget> get_LockedTargets() const { return Collections::QListPtr<LockedTarget>(LockedTargetsPtr); }
    // 0x7c5fa78
    void set_LockedTargets(Collections::QListPtr<LockedTarget> value) { LockedTargetsPtr = value.Ptr; }

    // 0x7c5fa80
    std::int32_t GetHashCode() const;
    // 0x7c5fd8c
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c5fc54 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c5fb3c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c5fd08 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c5fbd0 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(RynoWeapon) == RynoWeapon::SIZE);

struct alignas(8) ShapeOverlapper {
    static constexpr const char* ComponentTypeName = "ShapeOverlapper";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t lastCastFrame;
    Ptr hitsPtr;
    AssetRefShapeOverlapperSpec Spec;

    // 0x7c60000
    Collections::QListPtr<CollidingEntity> get_hits() const { return Collections::QListPtr<CollidingEntity>(hitsPtr); }
    // 0x7c6003c
    void set_hits(Collections::QListPtr<CollidingEntity> value) { hitsPtr = value.Ptr; }

    // 0x7c60044
    std::int32_t GetHashCode() const;
    // 0x7c602e8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c601b0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c60098 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c60264 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c6012c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(ShapeOverlapper) == ShapeOverlapper::SIZE);

struct alignas(8) ShieldedStats {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    QBoolean statsInitialised;
    std::uint8_t _pad0[4];
    FP initialShieldPoints;
    FP shieldMaxPoints;

    // 0x7b9bff0
    std::int32_t GetHashCode() const;
    // 0x7b9c044
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShieldedStats) == ShieldedStats::SIZE);

struct alignas(8) ShieldedActor {
    static constexpr const char* ComponentTypeName = "ShieldedActor";
    static constexpr int SIZE = 80;
    static constexpr int ALIGNMENT = 8;

    std::int32_t lastFrameShieldHitSpeedModWasApplied;
    std::int32_t startRegenFrame;
    QBoolean brokenThisFrame;
    QBoolean canRegen;
    QBoolean shieldBypassedThisFrame;
    QBoolean shieldIsBroken;
    ShieldState state;
    std::uint8_t _pad0[4];
    AssetRefShieldedActorSpec spec;
    FP DamageMultiplier_Shieldless;
    FP shieldPoints;
    ShieldedStats stats;

    // 0x7c60414
    std::int32_t GetHashCode() const;
    // 0x7c60510
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShieldedActor) == ShieldedActor::SIZE);

struct alignas(4) ShieldGroupStats {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 4;

    std::int32_t AutoShootMaxMembers;
    std::int32_t LastMaxedOutFrame;
    std::int32_t ShieldGroupMaxMembers;
    QBoolean Initialised;

    // 0x7b9bdd8
    std::int32_t GetHashCode() const;
    // 0x7b9be48
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShieldGroupStats) == ShieldGroupStats::SIZE);

struct alignas(4) ShieldsGroup {
    static constexpr const char* ComponentTypeName = "ShieldsGroup";
    static constexpr int SIZE = 20;
    static constexpr int ALIGNMENT = 4;

    Ptr membersPtr;
    ShieldGroupStats stats;

    // 0x7c6071c
    Collections::QListPtr<ShieldGroupMember> get_members() const { return Collections::QListPtr<ShieldGroupMember>(membersPtr); }
    // 0x7c60758
    void set_members(Collections::QListPtr<ShieldGroupMember> value) { membersPtr = value.Ptr; }

    // 0x7c60760
    std::int32_t GetHashCode() const;
    // 0x7c609d8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c608a0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c60788 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c60954 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c6081c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(ShieldsGroup) == ShieldsGroup::SIZE);

struct alignas(8) ShotgunBerserkerEnhancement {
    static constexpr const char* ComponentTypeName = "ShotgunBerserkerEnhancement";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    AssetRefEntityPrototype OriginalAmmoAsset;
    AssetRefShotgunBerserkerEnhancementSpec Spec;

    // 0x7c60a7c
    std::int32_t GetHashCode() const;
    // 0x7c60abc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShotgunBerserkerEnhancement) == ShotgunBerserkerEnhancement::SIZE);

struct alignas(1) ShotgunBerserkerEnhancementEffect {
    static constexpr const char* ComponentTypeName = "ShotgunBerserkerEnhancementEffect";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c60af4
    std::int32_t GetHashCode() const;
    // 0x7c60afc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShotgunBerserkerEnhancementEffect) == ShotgunBerserkerEnhancementEffect::SIZE);

struct alignas(8) ShotgunEnhancement {
    static constexpr const char* ComponentTypeName = "ShotgunEnhancement";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    AssetRefEntityPrototype OriginalAmmoAsset;
    AssetRefShotgunEnhancementSpec Spec;

    // 0x7c60b00
    std::int32_t GetHashCode() const;
    // 0x7c60b40
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShotgunEnhancement) == ShotgunEnhancement::SIZE);

struct alignas(1) ShotgunEnhancementEffect {
    static constexpr const char* ComponentTypeName = "ShotgunEnhancementEffect";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c60b78
    std::int32_t GetHashCode() const;
    // 0x7c60b80
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShotgunEnhancementEffect) == ShotgunEnhancementEffect::SIZE);

struct alignas(8) ShrinkingZone {
    static constexpr const char* ComponentTypeName = "ShrinkingZone";
    static constexpr int SIZE = 160;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t closingPointIndex;
    std::uint16_t closingZoneIndex;
    QBoolean initialized;
    QBoolean isShrinking;
    QBoolean updating;
    AssetRefShrinkingZoneSpec Spec;
    FP endRadius;
    FP endTime;
    FP radius;
    FP startRadius;
    FP startTime;
    FPVector3 currentCenterPositonForBots;
    FPVector3 endPosition;
    FPVector3 position;
    FPVector3 startPosition;

    // 0x7c60b84
    std::int32_t GetHashCode() const;
    // 0x7c60d0c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShrinkingZone) == ShrinkingZone::SIZE);

struct alignas(4) ShrinkingZoneActor {
    static constexpr const char* ComponentTypeName = "ShrinkingZoneActor";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 4;

    std::int32_t enteredZoneFrame;
    std::int32_t lastHitFrame;
    QBoolean IgnoreShrinkingZone;
    QBoolean IsInsideZone;

    // 0x7c61678
    std::int32_t GetHashCode() const;
    // 0x7c616d8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShrinkingZoneActor) == ShrinkingZoneActor::SIZE);

struct alignas(4) SpawnProjectilesOnDestroy {
    static constexpr const char* ComponentTypeName = "SpawnProjectilesOnDestroy";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    Ptr SpecsPtr;

    // 0x7c6181c
    Collections::QListPtr<AssetRefProjectileSpawnerSpec> get_Specs() const { return Collections::QListPtr<AssetRefProjectileSpawnerSpec>(SpecsPtr); }
    // 0x7c61858
    void set_Specs(Collections::QListPtr<AssetRefProjectileSpawnerSpec> value) { SpecsPtr = value.Ptr; }

    // 0x7c61860
    std::int32_t GetHashCode() const;
    // 0x7c618f8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c61870 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c61878 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(SpawnProjectilesOnDestroy) == SpawnProjectilesOnDestroy::SIZE);

struct alignas(8) Spawned {
    static constexpr const char* ComponentTypeName = "Spawned";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    EntityRef spawnerEntity;

    // 0x7c6198c
    std::int32_t GetHashCode() const;
    // 0x7c619a4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Spawned) == Spawned::SIZE);

struct alignas(8) SpawnedGoldenBoltData {
    static constexpr const char* ComponentTypeName = "SpawnedGoldenBoltData";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    GoldenBoltState State;
    std::int32_t SpawnFrame;
    std::int32_t SpawnPointIndex;
    std::uint8_t _pad0[4];
    EntityRef Entity;
    FPVector3 ArrivalPoint;

    // 0x7c619b8
    std::int32_t GetHashCode() const;
    // 0x7c61a48
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(SpawnedGoldenBoltData) == SpawnedGoldenBoltData::SIZE);

struct alignas(8) Speeder {
    static constexpr const char* ComponentTypeName = "Speeder";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t UsedFrame;
    TeamFlag Team;
    AssetRefSpeederSpec Spec;
    FP Duration;
    FP SpeedModifier;

    // 0x7c61bf4
    std::int32_t GetHashCode() const;
    // 0x7c61c80
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Speeder) == Speeder::SIZE);

struct alignas(4) StatsManager {
    static constexpr const char* ComponentTypeName = "StatsManager";
    static constexpr int SIZE = 12;
    static constexpr int ALIGNMENT = 4;

    Ptr allFrameStatsPtr;
    Ptr curFramestatsPtr;
    Ptr summaryPtr;

    // 0x7c61dd4
    Collections::QListPtr<Stat> get_allFrameStats() const { return Collections::QListPtr<Stat>(allFrameStatsPtr); }
    // 0x7c61e10
    void set_allFrameStats(Collections::QListPtr<Stat> value) { allFrameStatsPtr = value.Ptr; }
    // 0x7c61e18
    Collections::QListPtr<Stat> get_curFramestats() const { return Collections::QListPtr<Stat>(curFramestatsPtr); }
    // 0x7c61e54
    void set_curFramestats(Collections::QListPtr<Stat> value) { curFramestatsPtr = value.Ptr; }
    // 0x7c61e5c
    Collections::QDictionaryPtr<STAT_ID, StatSummary> get_summary() const { return Collections::QDictionaryPtr<STAT_ID, StatSummary>(summaryPtr); }
    // 0x7c61e98
    void set_summary(Collections::QDictionaryPtr<STAT_ID, StatSummary> value) { summaryPtr = value.Ptr; }

    // 0x7c61ea0
    std::int32_t GetHashCode() const;
    // 0x7c622b8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c62094 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c61ec8 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c62234 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c62010 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(StatsManager) == StatsManager::SIZE);

struct alignas(8) StreamEffect {
    static constexpr const char* ComponentTypeName = "StreamEffect";
    static constexpr int SIZE = 168;
    static constexpr int ALIGNMENT = 8;

    Ptr pointsPtr;
    std::uint8_t _pad0[4];
    AssetRefStreamEffectSpec Spec;
    Transform3D overlapTransform;
    Shape3D compoundShape;

    // 0x7c623dc
    Collections::QListPtr<StreamPoint> get_points() const { return Collections::QListPtr<StreamPoint>(pointsPtr); }
    // 0x7c62418
    void set_points(Collections::QListPtr<StreamPoint> value) { pointsPtr = value.Ptr; }

    // 0x7c62420
    std::int32_t GetHashCode() const;
    // TODO(0x7c626dc, Quantum.StreamEffect.Serialize): all fields in layout order; waits
    // on Shape3D serialisation in quantum_core (throws until then).
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c625a4 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c6248c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c62658 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c62520 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(StreamEffect) == StreamEffect::SIZE);

struct alignas(8) StreamEffectSpawner {
    static constexpr const char* ComponentTypeName = "StreamEffectSpawner";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    AssetRefStreamEffectSpawnerSpec Spec;
    EntityRef streamEffect;

    // 0x7c627a0
    std::int32_t GetHashCode() const;
    // 0x7c627d4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(StreamEffectSpawner) == StreamEffectSpawner::SIZE);

struct alignas(8) SweepAttack {
    static constexpr const char* ComponentTypeName = "SweepAttack";
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t changeStateFrame;
    SweepAttackState currentState;
    AssetRefSweepAttackSpec Spec;
    FPVector3 initialForward;

    // 0x7c6280c
    std::int32_t GetHashCode() const;
    // 0x7c62880
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(SweepAttack) == SweepAttack::SIZE);

struct alignas(8) PotentialTarget {
    static constexpr int SIZE = 104;
    static constexpr int ALIGNMENT = 8;

    QBoolean bypassCameraAiming;
    QBoolean inBack;
    QBoolean inHeightDiff;
    QBoolean inSight;
    QBoolean inWeaponRange;
    QBoolean inWeaponRangePreCheck;
    QBoolean matchesWeaponTargetFilter;
    Ptr LineOfSightQueryIndicesPtr;
    EntityRef Entity;
    FP WeaponScore;
    FP overShootDistance;
    FPVector3 Position;
    FPVector3 Velocity;

    // 0x7b9a360
    Collections::QListPtr<std::int32_t> get_LineOfSightQueryIndices() const { return Collections::QListPtr<std::int32_t>(LineOfSightQueryIndicesPtr); }
    // 0x7b9a39c
    void set_LineOfSightQueryIndices(Collections::QListPtr<std::int32_t> value) { LineOfSightQueryIndicesPtr = value.Ptr; }

    // 0x7b9a3a4
    std::int32_t GetHashCode() const;
    // 0x7b9a4bc
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b9a4b4 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
};
static_assert(sizeof(PotentialTarget) == PotentialTarget::SIZE);

struct alignas(8) TargetFinder {
    static constexpr const char* ComponentTypeName = "TargetFinder";
    static constexpr int SIZE = 288;
    static constexpr int ALIGNMENT = 8;

    DamageType EquippedWeaponDamageType;
    QBoolean HasShootableTarget;
    QBoolean LineOfSightCheckRequested;
    QBoolean disabled;
    QBoolean forceSetAutoTarget;
    QBoolean keepAllPotentialTargets;
    QBoolean useContinuousLineOfSightChecks;
    Ptr PotentialTargetsPtr;
    AssetRefTargetFinderSpec EquippedWeaponFinderSpecOverride;
    AssetRefTargetFinderSpec Spec;
    EntityRef BotCustomShootTarget;
    FP EquippedWeaponOverShootDistance;
    FP EquippedWeaponRange;
    FP overridenMaxRange;
    PotentialTarget AutoTargetInfo;
    PotentialTarget WrenchAutoTargetInfo;

    // 0x7c62ad8
    Collections::QListPtr<PotentialTarget> get_PotentialTargets() const { return Collections::QListPtr<PotentialTarget>(PotentialTargetsPtr); }
    // 0x7c62b14
    void set_PotentialTargets(Collections::QListPtr<PotentialTarget> value) { PotentialTargetsPtr = value.Ptr; }

    // 0x7c62b1c
    std::int32_t GetHashCode() const;
    // 0x7c62f18
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c62de0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c62c88 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c62e94 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c62d4c - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(TargetFinder) == TargetFinder::SIZE);

struct alignas(4) TargetHolder {
    static constexpr const char* ComponentTypeName = "TargetHolder";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    Ptr targetsPtr;

    // 0x7c63614
    Collections::QListPtr<TargetingData> get_targets() const { return Collections::QListPtr<TargetingData>(targetsPtr); }
    // 0x7c63650
    void set_targets(Collections::QListPtr<TargetingData> value) { targetsPtr = value.Ptr; }

    // 0x7c63658
    std::int32_t GetHashCode() const;
    // 0x7c63780
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c63668 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c636fc - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(TargetHolder) == TargetHolder::SIZE);

struct alignas(8) Targetable {
    static constexpr const char* ComponentTypeName = "Targetable";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    QBoolean IgnoreAutoShoot;
    QBoolean IgnoreAutoTargetInsideBounds;
    QBoolean IgnoreCameraAiming;
    std::uint8_t _pad0[4];
    FP BoundsSizeMultiplier;

    // 0x7c63814
    std::int32_t GetHashCode() const;
    // 0x7c6385c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Targetable) == Targetable::SIZE);

struct alignas(8) TargetedEffect {
    static constexpr const char* ComponentTypeName = "TargetedEffect";
    static constexpr int SIZE = 56;
    static constexpr int ALIGNMENT = 8;

    std::int32_t maxTickCount;
    std::int32_t startFrame;
    std::int32_t tickCount;
    Ptr tickedEntitiesPtr;
    AssetRefTargetedEffectSpec spec;
    EntityRef target;
    FPVector3 startPosition;

    // 0x7c638b4
    Collections::QListPtr<EntityRef> get_tickedEntities() const { return Collections::QListPtr<EntityRef>(tickedEntitiesPtr); }
    // 0x7c638f0
    void set_tickedEntities(Collections::QListPtr<EntityRef> value) { tickedEntitiesPtr = value.Ptr; }

    // 0x7c638f8
    std::int32_t GetHashCode() const;
    // 0x7c63c04
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c63acc - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c639b4 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c63b80 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c63a48 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(TargetedEffect) == TargetedEffect::SIZE);

struct alignas(4) Team {
    static constexpr const char* ComponentTypeName = "Team";
    static constexpr int SIZE = 12;
    static constexpr int ALIGNMENT = 4;

    std::int32_t TeamID;
    std::int32_t eliminationFrame;
    QBoolean isLeader;

    // The team id the systems give neutral entities (a literal -2 in the original).
    static constexpr std::int32_t NeutralTeamID = -2;

    // 0x7c642cc - Gives the entity a Team (adding the component if needed) with `teamID`;
    // it becomes the team's leader if no player of that team leads it yet. Implemented in
    // systems/team.cpp.
    static void JoinTeam(Frame* f, EntityRef newTeamMember, std::int32_t teamID);
    // 0x7c643f4 - The first player (PlayerActor with a Team) of `teamId` that leads it.
    static bool TryGetPlayerTeamLeader(Frame* f, std::int32_t teamId, EntityRef& leader);

    // 0x7c6454c - -1 without a Team.
    static std::int32_t GetTeamID(Frame* f, EntityRef teamMember);

    // 0x7c6404c
    std::int32_t GetHashCode() const;
    // 0x7c6409c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Team) == Team::SIZE);

struct alignas(8) TeslaClawMoreBeam {
    static constexpr const char* ComponentTypeName = "TeslaClawMoreBeam";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefTeslaClawMoreBeamEnhancementSpec Spec;

    // 0x7c65284
    std::int32_t GetHashCode() const;
    // 0x7c652a0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(TeslaClawMoreBeam) == TeslaClawMoreBeam::SIZE);

struct alignas(8) TimeScale {
    static constexpr const char* ComponentTypeName = "TimeScale";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t endFrame;
    std::int32_t startFrame;
    QBoolean autoReverse;
    std::uint8_t _pad0[4];
    FP endTimeScale;
    FP startTimeScale;

    // 0x7c652b4
    std::int32_t GetHashCode() const;
    // 0x7c6533c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(TimeScale) == TimeScale::SIZE);

struct alignas(8) TrailSpawner {
    static constexpr const char* ComponentTypeName = "TrailSpawner";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    AssetRefTrailSpawnerSpec Spec;
    EntityRef aoeTrail;

    // 0x7c65490
    std::int32_t GetHashCode() const;
    // 0x7c654c4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(TrailSpawner) == TrailSpawner::SIZE);

struct alignas(8) TravolatorActor {
    static constexpr const char* ComponentTypeName = "TravolatorActor";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    AssetRefTravolatorStaticTriggerConfig Spec;
    FPVector3 Direction;

    // 0x7c654fc
    std::int32_t GetHashCode() const;
    // 0x7c65540
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(TravolatorActor) == TravolatorActor::SIZE);

struct alignas(8) TurnTo {
    static constexpr const char* ComponentTypeName = "TurnTo";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    FP turnSpeed;
    FPVector3 lookAtDirection;

    // 0x7c65578
    std::int32_t GetHashCode() const;
    // 0x7c655bc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(TurnTo) == TurnTo::SIZE);

struct alignas(8) TutorialGameMode {
    static constexpr const char* ComponentTypeName = "TutorialGameMode";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean isPVETuto;
    TutorialGameModeStep currentStep;
    AssetRefTutorialGameModeSpec Spec;

    // 0x7c655f4
    std::int32_t GetHashCode() const;
    // 0x7c65630
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(TutorialGameMode) == TutorialGameMode::SIZE);

struct alignas(8) UtilityReasoner {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    Ptr ConsiderationsPtr;
    Ptr CooldownsDictPtr;
    Ptr HighRankConsiderationsPtr;
    Ptr MomentumListPtr;
    Ptr PreviousExecutionPtr;
    std::uint8_t _pad0[4];
    AssetRefUTRoot UTRoot;
    FP TimeToTick;

    // 0x7b87000
    Collections::QListPtr<AssetRefConsideration> get_Considerations() const { return Collections::QListPtr<AssetRefConsideration>(ConsiderationsPtr); }
    // 0x7b85fb0
    void set_Considerations(Collections::QListPtr<AssetRefConsideration> value) { ConsiderationsPtr = value.Ptr; }
    // 0x7b86634
    Collections::QDictionaryPtr<AssetRefConsideration, FP> get_CooldownsDict() const { return Collections::QDictionaryPtr<AssetRefConsideration, FP>(CooldownsDictPtr); }
    // 0x7b85fc0
    void set_CooldownsDict(Collections::QDictionaryPtr<AssetRefConsideration, FP> value) { CooldownsDictPtr = value.Ptr; }
    // 0x7b8703c
    Collections::QListPtr<AssetRefConsideration> get_HighRankConsiderations() const { return Collections::QListPtr<AssetRefConsideration>(HighRankConsiderationsPtr); }
    // 0x7b85fb8
    void set_HighRankConsiderations(Collections::QListPtr<AssetRefConsideration> value) { HighRankConsiderationsPtr = value.Ptr; }
    // 0x7b86ab0
    Collections::QListPtr<UTMomentumPack> get_MomentumList() const { return Collections::QListPtr<UTMomentumPack>(MomentumListPtr); }
    // 0x7b85fd0
    void set_MomentumList(Collections::QListPtr<UTMomentumPack> value) { MomentumListPtr = value.Ptr; }
    // 0x7b86de0
    Collections::QListPtr<AssetRefConsideration> get_PreviousExecution() const { return Collections::QListPtr<AssetRefConsideration>(PreviousExecutionPtr); }
    // 0x7b85fc8
    void set_PreviousExecution(Collections::QListPtr<AssetRefConsideration> value) { PreviousExecutionPtr = value.Ptr; }

    // 0x7b85748
    std::int32_t GetHashCode() const;
    // 0x7b858d8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b857e0 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
};
static_assert(sizeof(UtilityReasoner) == UtilityReasoner::SIZE);

struct alignas(8) UTAgent {
    static constexpr const char* ComponentTypeName = "UTAgent";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    AssetRefAIConfig Config;
    ::Quantum::UtilityReasoner UtilityReasoner;

    // 0x7b8570c
    std::int32_t GetHashCode() const;
    // 0x7b858a4
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b857d4 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7b857ec - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(UTAgent) == UTAgent::SIZE);

struct alignas(4) UltimateStats {
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    std::int32_t FullCharge;

    // 0x7b9c930
    std::int32_t GetHashCode() const;
    // 0x7b9c94c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(UltimateStats) == UltimateStats::SIZE);

struct alignas(8) UltimateAbilityUser {
    static constexpr const char* ComponentTypeName = "UltimateAbilityUser";
    static constexpr int SIZE = 72;
    static constexpr int ALIGNMENT = 8;

    std::int32_t activationFrame;
    std::int32_t deactivationFrame;
    std::int32_t lastHitChargeFrame;
    QBoolean chargeWithTimeDisabled;
    QBoolean disabled;
    UltimateStats Stats;
    AssetRefUltimateAbilitySpec abilitySpec;
    AssetRefUltimateAbilityUserSpec userSpec;
    FP bonusChargePerSecond;
    FP chargePerPushReceivedMultiplier;
    FP chargeWithTimeMultiplier;
    FP chargedValue;

    // 0x7c65704
    std::int32_t GetHashCode() const;
    // 0x7c65850
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(UltimateAbilityUser) == UltimateAbilityUser::SIZE);

struct alignas(4) UltimateEvolution {
    static constexpr const char* ComponentTypeName = "UltimateEvolution";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::int32_t CurrentEvolutionLevel;
    QBoolean AoESpawned;

    // 0x7c65b74
    std::int32_t GetHashCode() const;
    // 0x7c65b9c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(UltimateEvolution) == UltimateEvolution::SIZE);

struct alignas(8) UltimateSlam {
    static constexpr const char* ComponentTypeName = "UltimateSlam";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t endFrame;
    std::uint8_t _pad0[4];
    FPVector3 startPosition;

    // 0x7c65c60
    std::int32_t GetHashCode() const;
    // 0x7c65ca0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(UltimateSlam) == UltimateSlam::SIZE);

struct alignas(8) UltimateTank {
    static constexpr const char* ComponentTypeName = "UltimateTank";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t spawnFrame;
    std::uint8_t _pad0[4];
    FP healthDecreasePerSecond;
    FP playerInitialMaxHP;
    FP playerMaxHP;

    // 0x7c65d64
    std::int32_t GetHashCode() const;
    // 0x7c65de4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(UltimateTank) == UltimateTank::SIZE);

struct alignas(4) UnlockedFeaturesManager {
    static constexpr const char* ComponentTypeName = "UnlockedFeaturesManager";
    static constexpr int SIZE = 76;
    static constexpr int ALIGNMENT = 4;

    std::array<QBoolean, 19> _unlockedFeatures_;

    // 0x7c65ec8
    std::span<QBoolean> get_unlockedFeatures() { return _unlockedFeatures_; }
    std::span<const QBoolean> get_unlockedFeatures() const { return _unlockedFeatures_; }

    // 0x7c65f30
    std::int32_t GetHashCode() const;
    // 0x7c65fd0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(UnlockedFeaturesManager) == UnlockedFeaturesManager::SIZE);

struct alignas(8) VoidRepulserBulldozerEnhancement {
    static constexpr const char* ComponentTypeName = "VoidRepulserBulldozerEnhancement";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t UsageStartFrame;
    QBoolean IsMoveSpeedModifierApplied;
    AssetRefVoidRepulserBulldozerEnhancementSpec Spec;

    // 0x7c66698
    std::int32_t GetHashCode() const;
    // 0x7c666ec
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(VoidRepulserBulldozerEnhancement) == VoidRepulserBulldozerEnhancement::SIZE);

struct alignas(4) VoidRepulserShield {
    static constexpr const char* ComponentTypeName = "VoidRepulserShield";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    VoidRepulserState State;

    // 0x7c667c0
    std::int32_t GetHashCode() const;
    // 0x7c667d0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(VoidRepulserShield) == VoidRepulserShield::SIZE);

struct alignas(8) VoidRepulserSpeedBoostEnhancement {
    static constexpr const char* ComponentTypeName = "VoidRepulserSpeedBoostEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefVoidRepulserSpeedBoostEnhancementSpec Spec;

    // 0x7c66884
    std::int32_t GetHashCode() const;
    // 0x7c668a0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(VoidRepulserSpeedBoostEnhancement) == VoidRepulserSpeedBoostEnhancement::SIZE);

struct alignas(8) VoidRepulserVoidWaveEnhancement {
    static constexpr const char* ComponentTypeName = "VoidRepulserVoidWaveEnhancement";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean ChargeStarted;
    std::uint8_t _pad0[4];
    AssetRefVoidRepulserVoidWaveEnhancementSpec Spec;

    // 0x7c668b4
    std::int32_t GetHashCode() const;
    // 0x7c668dc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(VoidRepulserVoidWaveEnhancement) == VoidRepulserVoidWaveEnhancement::SIZE);

struct alignas(8) VoidRepulserData {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t regenFrameReference;
    QBoolean shouldRestore;
    VoidRepulserState state;
    std::uint8_t _pad0[4];
    FP hp;
    FP startRegenHP;

    // 0x7b9d2b0
    std::int32_t GetHashCode() const;
    // 0x7b9d330
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(VoidRepulserData) == VoidRepulserData::SIZE);

struct alignas(8) VoidRepulserWeapon {
    static constexpr const char* ComponentTypeName = "VoidRepulserWeapon";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    VoidRepulserData data;

    // 0x7c66914
    std::int32_t GetHashCode() const;
    // 0x7c66930
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(VoidRepulserWeapon) == VoidRepulserWeapon::SIZE);

struct alignas(8) WakeUpSurroundingsOnPushOrDestroy {
    static constexpr const char* ComponentTypeName = "WakeUpSurroundingsOnPushOrDestroy";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    EntityRef spawner;
    FP radius;

    // 0x7c66944
    std::int32_t GetHashCode() const;
    // 0x7c6697c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WakeUpSurroundingsOnPushOrDestroy) == WakeUpSurroundingsOnPushOrDestroy::SIZE);

struct alignas(8) WallRunningActor {
    static constexpr const char* ComponentTypeName = "WallRunningActor";
    static constexpr int SIZE = 88;
    static constexpr int ALIGNMENT = 8;

    QBoolean CanWallRun;
    QBoolean IsInitialized;
    QBoolean IsWallRunning;
    QBoolean RunningLeft;
    AssetRefWallRunningSpec Spec;
    EntityRef RunnableWallEntity;
    FP RunningSpeed;
    FPVector3 PreviousPosition;
    FPVector3 RunningDirection;

    // 0x7c669b4
    std::int32_t GetHashCode() const;
    // 0x7c66a84
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WallRunningActor) == WallRunningActor::SIZE);

struct alignas(8) WarmongerMiniRocketsEnhancement {
    static constexpr const char* ComponentTypeName = "WarmongerMiniRocketsEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefWarmongerMiniRocketsEnhancementSpec Spec;

    // 0x7c675b4
    std::int32_t GetHashCode() const;
    // 0x7c675d0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WarmongerMiniRocketsEnhancement) == WarmongerMiniRocketsEnhancement::SIZE);

struct alignas(8) WaveElement {
    static constexpr const char* ComponentTypeName = "WaveElement";
    static constexpr int SIZE = 104;
    static constexpr int ALIGNMENT = 8;

    EntityRef spawner;
    FP stuckDuration;
    FPVector3 previousPosition;
    QString64 dropperID;

    // 0x7c675e4
    std::int32_t GetHashCode() const;
    // 0x7c67660
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WaveElement) == WaveElement::SIZE);

struct alignas(8) WaveManager {
    static constexpr const char* ComponentTypeName = "WaveManager";
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t waveSpawnRequestIndex;
    std::uint8_t _pad0[2];
    std::int32_t currentWaveIndex;
    std::int32_t currentWaveStartFrame;
    std::int32_t nextWaveStartFrame;
    QBoolean allWavesCompleted;
    Ptr waveSpawnersPtr;
    AssetRefSquadSiegeConfig SquadSiegeConfig;
    AssetRefWaveManagerSpec Spec;
    AssetRefWaveReferences referencesAsset;

    // 0x7c676b8
    Collections::QDictionaryPtr<std::uint16_t, EntityRef> get_waveSpawners() const { return Collections::QDictionaryPtr<std::uint16_t, EntityRef>(waveSpawnersPtr); }
    // 0x7c676f4
    void set_waveSpawners(Collections::QDictionaryPtr<std::uint16_t, EntityRef> value) { waveSpawnersPtr = value.Ptr; }

    // 0x7c676fc
    std::int32_t GetHashCode() const;
    // 0x7c67a3c
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c67904 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c677ec - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c679b8 - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c67880 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(WaveManager) == WaveManager::SIZE);

struct alignas(8) SpawnConfig {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t findPositionAttempts;
    QBoolean useRandomSpawnTransform;
    FP fallbackYOffset;
    FP freePositionRadius;
    FP spawnPositionOffsetDistance;

    // 0x7b9c08c
    std::int32_t GetHashCode() const;
    // 0x7b9c118
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(SpawnConfig) == SpawnConfig::SIZE);

struct alignas(8) WaveSpawner {
    static constexpr const char* ComponentTypeName = "WaveSpawner";
    static constexpr int SIZE = 88;
    static constexpr int ALIGNMENT = 8;

    std::int32_t currentEntityIndex;
    std::int32_t lastSpawnFrame;
    Ptr waypointsPtr;
    std::uint32_t index;
    BitSet16 droppers;
    EntityRef currentGroup;
    FP spawnInterval;
    FP wanderRadius;
    FP waypointRadius;
    SpawnConfig spawnConfig;

    // 0x7c67cd8
    Collections::QListPtr<FPVector3> get_waypoints() const { return Collections::QListPtr<FPVector3>(waypointsPtr); }
    // 0x7c67d14
    void set_waypoints(Collections::QListPtr<FPVector3> value) { waypointsPtr = value.Ptr; }

    // 0x7c67d1c
    std::int32_t GetHashCode() const;
    // 0x7c67eb8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c67e30 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c67e38 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(WaveSpawner) == WaveSpawner::SIZE);

struct alignas(8) WeaponStats {
    static constexpr int SIZE = 88;
    static constexpr int ALIGNMENT = 8;

    std::int32_t AmmoPerBurst;
    std::int32_t CurrentMultiShotConfig;
    std::int32_t MaxAmmo;
    QBoolean ConsumeOneAmmoPerMultiShot;
    QBoolean ModifiersWereApplied;
    QBoolean SetKccLookTowardsCamera;
    QBoolean UseProceduralCameraOffsetsOnUse;
    Ptr MultiShotConfigsPtr;
    WeaponStartUseType StartUseType;
    std::uint8_t _pad0[4];
    FP CooldownDurationSeconds;
    FP FireRate;
    FP MoveSpeedModifierOnUse;
    FP OverShootDistance;
    FP Range;
    FP ReloadDurationSeconds;

    // 0x7b9db84
    Collections::QListPtr<QMultiShotConfig> get_MultiShotConfigs() const { return Collections::QListPtr<QMultiShotConfig>(MultiShotConfigsPtr); }
    // 0x7b9dbc0
    void set_MultiShotConfigs(Collections::QListPtr<QMultiShotConfig> value) { MultiShotConfigsPtr = value.Ptr; }

    // 0x7b9dbc8
    std::int32_t GetHashCode() const;
    // 0x7b9dde4
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b9dd30 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7b9dd28 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
};
static_assert(sizeof(WeaponStats) == WeaponStats::SIZE);

struct alignas(8) Weapon {
    static constexpr const char* ComponentTypeName = "Weapon";
    static constexpr int SIZE = 264;
    static constexpr int ALIGNMENT = 8;

    std::int32_t BufferedInputFrame;
    std::int32_t CooldownStartFrame;
    std::int32_t CurrentBurstShot;
    std::int32_t EquippedFrame;
    std::int32_t FiredFrame;
    std::int32_t NextAllowedFireFrame;
    std::int32_t RawInputPressedFrame;
    std::int32_t ReloadStartFrame;
    std::int32_t RemainingAmmo;
    std::int32_t RemainingAmmoWhenStartAutoLoad;
    std::int32_t UsageModifersAppliedFrame;
    std::int32_t _TriggerPressedFrame;
    std::int32_t _TriggerReleasedFrame;
    QBoolean IsFiring;
    QBoolean ReleaseTriggerHandledExternally;
    QBoolean ShouldCancelNextFire;
    QBoolean _ChangedBeingUsed;
    QBoolean _IsBeingUsed;
    QBoolean _TriggerIsPressed;
    QBoolean _TriggerWasPressed;
    AssetRefEntityPrototype AmmunitionAsset;
    AssetRefProjectileSpec LobProjectileSpec;
    AssetRefWeaponSpec WeaponData;
    EntityRef AmmoInstance;
    EntityRef WeaponUser;
    FP BonusAmmoMultiplier;
    FP BonusFirerate;
    FP CooldownMultiplier;
    FP CurrentBurstFireRate;
    FP CurrentCooldownDuration;
    FP CurrentFireRate;
    FP IgnoreEntitiesMultiplier;
    WeaponStats Stats;

    // 0x7c68104
    std::int32_t GetHashCode() const;
    // 0x7c68594
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c684f0 - Allocates the collections that start allocated.
    void AllocatePointers(Frame* f, EntityRef entityRef);
    // 0x7c6844c - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c684fc - Component callback: AllocatePointers.
    static void OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
    // 0x7c68458 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(Weapon) == Weapon::SIZE);

struct alignas(8) ChargingStats {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t NumberOfChargeLevel;
    QBoolean AutoReleaseOnFullCharge;
    FP FullChargeDuration;

    // 0x7b95bb8
    std::int32_t GetHashCode() const;
    // 0x7b95c0c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ChargingStats) == ChargingStats::SIZE);

struct alignas(8) WeaponBehaviour_Charging {
    static constexpr const char* ComponentTypeName = "WeaponBehaviour_Charging";
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t ChargingStartFrame;
    QBoolean NeedReset;
    AssetRefWeaponBehaviour_ChargingSpec Spec;
    ChargingStats Stats;

    // 0x7c692b4
    std::int32_t GetHashCode() const;
    // 0x7c69324
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponBehaviour_Charging) == WeaponBehaviour_Charging::SIZE);

struct alignas(4) WeaponLoadoutItemData {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::uint8_t enhancementLevel;
    std::uint8_t weaponLevel;
    std::uint8_t _pad0[2];
    GENERATED_WEAPON_ID uniqueWeaponID;

    // 0x7b9d7a4
    std::int32_t GetHashCode() const;
    // 0x7b9d9f8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponLoadoutItemData) == WeaponLoadoutItemData::SIZE);

struct alignas(8) WeaponLoadoutItem {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    std::int32_t equippedFrame;
    std::int32_t totalEquippeDurationInFrames;
    AssetRefEntityPrototype weaponProtoRef;
    EntityRef spawnedWeapon;
    WeaponLoadoutItemData data;

    // 0x7b9d7f4
    std::int32_t GetHashCode() const;
    // 0x7b9d8a8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponLoadoutItem) == WeaponLoadoutItem::SIZE);

struct alignas(8) WeaponLoadout {
    static constexpr int SIZE = 128;
    static constexpr int ALIGNMENT = 8;

    std::array<WeaponLoadoutItem, 4> _slots_;

    // 0x7b9d484
    std::span<WeaponLoadoutItem> get_slots() { return _slots_; }
    std::span<const WeaponLoadoutItem> get_slots() const { return _slots_; }

    // 0x7b9d4ec
    std::int32_t GetHashCode() const;
    // 0x7b9d598
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponLoadout) == WeaponLoadout::SIZE);

struct alignas(8) WeaponLoadoutActor {
    static constexpr const char* ComponentTypeName = "WeaponLoadoutActor";
    static constexpr int SIZE = 168;
    static constexpr int ALIGNMENT = 8;

    std::int32_t switchWeaponStartFrame;
    QBoolean disabled;
    QBoolean infiniteFiringEnabled;
    WeaponLoadoutSlot activeSlot;
    WeaponLoadoutSlot previousSlotBeforeGadget;
    WeaponLoadoutSlot previousSlotBeforeMelee;
    WeaponLoadoutSlot previousSlotBeforeUltimate;
    std::uint8_t _pad0[4];
    AssetRefWeaponLoadoutSpec spec;
    WeaponLoadout loadout;

    // 0x7c69490
    std::int32_t GetHashCode() const;
    // 0x7c69548
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponLoadoutActor) == WeaponLoadoutActor::SIZE);

struct alignas(8) WeaponProjectileSwitcherEnhancement {
    static constexpr const char* ComponentTypeName = "WeaponProjectileSwitcherEnhancement";
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t ShotCounter;
    std::uint8_t _pad0[4];
    AssetRefEntityPrototype OriginalAmmoAsset;
    AssetRefWeaponProjectileSwitcherEnhancementSpec Spec;

    // 0x7c6980c
    std::int32_t GetHashCode() const;
    // 0x7c6986c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponProjectileSwitcherEnhancement) == WeaponProjectileSwitcherEnhancement::SIZE);

struct alignas(8) WeaponShowcaseGameMode {
    static constexpr const char* ComponentTypeName = "WeaponShowcaseGameMode";
    static constexpr int SIZE = 200;
    static constexpr int ALIGNMENT = 8;

    std::int32_t startFrame;
    QBoolean playAnimation;
    QBoolean startTrigger;
    QBoolean stopAnimation;
    QBoolean stopTrigger;
    QBoolean triggerIsPressed;
    EntityRef userEntity;
    EntityRef weaponEntity;
    FP currentDelay;
    FP currentTimer;
    FPVector3 lastRotationAngle;
    FPVector3 lastRotationPositon;
    FPVector3 shootAngle;
    FPVector3 shootPosition;
    FPVector3 spawnAngle;
    FPVector3 spawnPosition;

    // 0x7c69940
    std::int32_t GetHashCode() const;
    // 0x7c69aa8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponShowcaseGameMode) == WeaponShowcaseGameMode::SIZE);

struct alignas(8) WeaponUser {
    static constexpr const char* ComponentTypeName = "WeaponUser";
    static constexpr int SIZE = 240;
    static constexpr int ALIGNMENT = 8;

    std::int32_t LastUsedFrame;
    LookPointHitType WeapontoLookPointType;
    QBoolean Disabled;
    QBoolean LookPointIsRangedRaycastHit;
    EntityRef EquippedWeapon;
    EntityRef ForceMissEntity;
    EntityRef LookPointEntity;
    FP BonusAmmoMultiplier;
    FP DamageMultiplier;
    FP DamageMultiplier_Boss;
    FP DamageMultiplier_Electric;
    FP DamageMultiplier_Melee;
    FP DamageMultiplier_Mobs;
    FP DamageMultiplier_MoreBolts;
    FP DefaultDamageMultiplier;
    FP ForceMissProbability;
    FP WeaponToLookPointRaycastAngle;
    FP WeaponToLookPointRaycastLength;
    ComponentPrototypeRef Prototype;
    FPVector3 LookPoint;
    FPVector3 ProjectedCameraPosition;
    FPVector3 ProjectedCameraSight;
    FPVector3 ToLookPointApproximate;

    // 0x7c69c4c
    std::int32_t GetHashCode() const;
    // 0x7c69eac
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponUser) == WeaponUser::SIZE);

struct alignas(8) WrenchGroundSlamEnhancement {
    static constexpr const char* ComponentTypeName = "WrenchGroundSlamEnhancement";
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetRefWrenchGroundSlamEnhancementSpec Spec;

    // 0x7c6a274
    std::int32_t GetHashCode() const;
    // 0x7c6a290
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WrenchGroundSlamEnhancement) == WrenchGroundSlamEnhancement::SIZE);

struct alignas(8) WrenchThrowEnhancement {
    static constexpr const char* ComponentTypeName = "WrenchThrowEnhancement";
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t FireButtonPressedFrame;
    QBoolean IsArmed;
    AssetRefWrenchThrowEnhancementSpec Spec;

    // 0x7c6a2a4
    std::int32_t GetHashCode() const;
    // 0x7c6a2f4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WrenchThrowEnhancement) == WrenchThrowEnhancement::SIZE);

struct alignas(1) WrenchThrowProjectile {
    static constexpr const char* ComponentTypeName = "WrenchThrowProjectile";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c6a3c8
    std::int32_t GetHashCode() const;
    // 0x7c6a3d0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WrenchThrowProjectile) == WrenchThrowProjectile::SIZE);

struct alignas(1) WrenchThrowWeapon {
    static constexpr const char* ComponentTypeName = "WrenchThrowWeapon";
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 1;

    std::array<std::uint8_t, 4> _alignment_padding_;

    // 0x7c6a3d4
    std::int32_t GetHashCode() const;
    // 0x7c6a3dc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WrenchThrowWeapon) == WrenchThrowWeapon::SIZE);

struct alignas(8) DelayedImpulseInfo {
    static constexpr int SIZE = 80;
    static constexpr int ALIGNMENT = 8;

    std::int32_t comboCounter;
    std::int32_t impulseTime;
    QBoolean pending;
    std::uint8_t _pad0[4];
    EntityRef target;
    FP forceInputDuration;
    FP impulseDelay;
    FP impulseDuration;
    FP inputFalloff;
    FPVector3 impulse;

    // 0x7b969d8
    std::int32_t GetHashCode() const;
    // 0x7b96ac4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DelayedImpulseInfo) == DelayedImpulseInfo::SIZE);

struct alignas(8) WrenchWeapon {
    static constexpr const char* ComponentTypeName = "WrenchWeapon";
    static constexpr int SIZE = 104;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t ComboCounter;
    std::uint8_t _pad0[2];
    QBoolean Disabled;
    FP TimeToRemainAlignedWithDirection;
    FP ignoreMoveSpeedCounter;
    DelayedImpulseInfo DelayedImpulse;

    // 0x7c6a3e0
    std::int32_t GetHashCode() const;
    // 0x7c6a468
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WrenchWeapon) == WrenchWeapon::SIZE);

struct alignas(8) ZoneControlTeamScoring {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t VictoryPoints;
    std::uint8_t _pad0[2];
    std::int32_t LastScoringFrame;
    std::int32_t TeamId;
    std::uint8_t _pad1[4];
    FP ZoneCompletion;

    // 0x7b9e738
    std::int32_t GetHashCode() const;
    // 0x7b9e7e8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ZoneControlTeamScoring) == ZoneControlTeamScoring::SIZE);

struct alignas(8) ZoneControlScoring {
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    ZoneControlTeamScoring TeamA;
    ZoneControlTeamScoring TeamB;

    // 0x7b9e700
    std::int32_t GetHashCode() const;
    // 0x7b9e7b8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ZoneControlScoring) == ZoneControlScoring::SIZE);

struct alignas(8) ZoneControlGameMode {
    static constexpr const char* ComponentTypeName = "ZoneControlGameMode";
    static constexpr int SIZE = 72;
    static constexpr int ALIGNMENT = 8;

    std::int32_t ActiveZoneIndex;
    Ptr ControlZoneStateHoldersPtr;
    Ptr ZonesActivationOrderPtr;
    std::uint8_t _pad0[4];
    AssetRefZoneControlGameModeSpec Spec;
    ZoneControlScoring Scoring;

    // 0x7c6a55c
    Collections::QListPtr<ControlZoneStateHolder> get_ControlZoneStateHolders() const { return Collections::QListPtr<ControlZoneStateHolder>(ControlZoneStateHoldersPtr); }
    // 0x7c6a598
    void set_ControlZoneStateHolders(Collections::QListPtr<ControlZoneStateHolder> value) { ControlZoneStateHoldersPtr = value.Ptr; }
    // 0x7c6a5a0
    Collections::QListPtr<std::int32_t> get_ZonesActivationOrder() const { return Collections::QListPtr<std::int32_t>(ZonesActivationOrderPtr); }
    // 0x7c6a5dc
    void set_ZonesActivationOrder(Collections::QListPtr<std::int32_t> value) { ZonesActivationOrderPtr = value.Ptr; }

    // 0x7c6a5e4
    std::int32_t GetHashCode() const;
    // 0x7c6a6e8
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7c6a660 - Frees the owned collections and clears the references.
    void ClearPointers(Frame* f, EntityRef entityRef);
    // 0x7c6a668 - Component callback: ClearPointers.
    static void OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr);
};
static_assert(sizeof(ZoneControlGameMode) == ZoneControlGameMode::SIZE);

struct alignas(8) HealthHitResult {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    FP damages;
    FP overHealthDamages;

    // 0x7b98640
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(HealthHitResult) == HealthHitResult::SIZE);

struct alignas(8) ShieldHitResult {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    FP damages;

    // 0x7b98d30
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(ShieldHitResult) == ShieldHitResult::SIZE);

struct alignas(8) HitResult {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    ShieldHitResult shield;
    HealthHitResult health;

    // 0x7b98cd4
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(HitResult) == HitResult::SIZE);

struct alignas(8) DamageToImpulseData {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t priority;
    QBoolean resetVelocity;
    FP delay;
    FPVector3 impulse;

    // 0x7b9696c
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(DamageToImpulseData) == DamageToImpulseData::SIZE);

struct alignas(8) OnSpawnEntityOnDeathParams {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    EntityRef dying;
    EntityRef spawning;
    FP raycastCoef;

    // 0x7b99ef8
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(OnSpawnEntityOnDeathParams) == OnSpawnEntityOnDeathParams::SIZE);

struct alignas(8) OnPushedData {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    QBoolean isPushStop;
    std::uint8_t _pad0[4];
    FP pushStopDuration;

    // 0x7b99ed0
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(OnPushedData) == OnPushedData::SIZE);

struct alignas(4) GameModeSettings {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    GAME_MODE gameMode;
    GENERATED_WEAPON_ID showcaseWeaponId;

    // 0x7b9808c
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(GameModeSettings) == GameModeSettings::SIZE);

struct alignas(8) BotWeightedExtraPower {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t extraPowerStats;
    std::uint8_t _pad0[4];
    FP weight;

    // 0x7b95700
    std::int32_t GetHashCode() const;
    // 0x7b95740
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BotWeightedExtraPower) == BotWeightedExtraPower::SIZE);

struct alignas(4) CustomBotInfo {
    static constexpr int SIZE = 44;
    static constexpr int ALIGNMENT = 4;

    std::int32_t skinId;
    std::int32_t teamId;
    QString34 nickname;

    // 0x7b9651c
    std::int32_t GetHashCode() const;
    // 0x7b9657c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(CustomBotInfo) == CustomBotInfo::SIZE);

struct alignas(8) BotsSettings {
    static constexpr int SIZE = 640;
    static constexpr int ALIGNMENT = 8;

    std::array<std::uint8_t, 32> BotLoadoutPresets;
    std::uint8_t BotsDifficulty_0_100;
    std::uint8_t CustomBotCount;
    std::array<std::uint8_t, 22> PossibleBotCharacterIDs;
    std::array<std::uint8_t, 128> PossibleBotWeaponIDs;
    std::uint8_t RoomMasterBucketIndex;
    std::uint8_t RoomMasterBucketIndexForChoosingHeroesAndWeapons;
    std::uint8_t _pad0[2];
    std::int32_t BotsPowerStats;
    std::int32_t NumBotLoadoutPresets;
    QBoolean FixedBotWeapons;
    FP BackpackBracketPlayerPlacement;
    FP BackpackLevelBracketPercentage;
    FP DamageMultiplier;
    std::array<BotWeightedExtraPower, 4> _ExtraPowerLevels_;
    std::array<CustomBotInfo, 8> _CustomBots_;

    // 0x7b9586c
    std::span<BotWeightedExtraPower> get_ExtraPowerLevels() { return _ExtraPowerLevels_; }
    std::span<const BotWeightedExtraPower> get_ExtraPowerLevels() const { return _ExtraPowerLevels_; }
    // 0x7b95804
    std::span<CustomBotInfo> get_CustomBots() { return _CustomBots_; }
    std::span<const CustomBotInfo> get_CustomBots() const { return _CustomBots_; }

    // 0x7b958d4
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(BotsSettings) == BotsSettings::SIZE);

struct alignas(4) UnlockedFeatures {
    static constexpr int SIZE = 76;
    static constexpr int ALIGNMENT = 4;

    QBoolean Gameplay_CameraMovement;
    QBoolean Gameplay_CharacterMovement;
    QBoolean Gameplay_Dash;
    QBoolean Gameplay_DoubleJump;
    QBoolean Gameplay_Fire;
    QBoolean Gameplay_FirstWeapon;
    QBoolean Gameplay_GadgetWeapon;
    QBoolean Gameplay_Helipack;
    QBoolean Gameplay_Jump;
    QBoolean Gameplay_Melee;
    QBoolean Gameplay_SecondWeapon_Deprecated;
    QBoolean Gameplay_ThirdWeapon_Deprecated;
    QBoolean Gameplay_UltimateWeapon;
    QBoolean HUD_BoltCounter;
    QBoolean HUD_BoltLevel;
    QBoolean HUD_Health;
    QBoolean HUD_Ranking;
    QBoolean HUD_Shield;
    QBoolean HUD_Timer;

    // 0x7b9ca00
    std::int32_t GetHashCode() const;
};
static_assert(sizeof(UnlockedFeatures) == UnlockedFeatures::SIZE);

struct alignas(8) BlackboardEntry {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    Ptr ReactiveDecoratorsPtr;
    std::uint8_t _pad0[4];
    BlackboardValue Value;

    // 0x7b87bfc
    Collections::QListPtr<AssetRefBTDecorator> get_ReactiveDecorators() const { return Collections::QListPtr<AssetRefBTDecorator>(ReactiveDecoratorsPtr); }
    // 0x7b89534
    void set_ReactiveDecorators(Collections::QListPtr<AssetRefBTDecorator> value) { ReactiveDecoratorsPtr = value.Ptr; }

    // 0x7b8953c
    std::int32_t GetHashCode() const;
    // 0x7b89658
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BlackboardEntry) == BlackboardEntry::SIZE);

struct alignas(8) NavBlacklistEntry {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t frame;
    std::uint8_t _pad0[4];
    EntityRef entity;
    FPVector3 position;

    // 0x7b99da4
    std::int32_t GetHashCode() const;
    // 0x7b99dfc
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(NavBlacklistEntry) == NavBlacklistEntry::SIZE);

struct alignas(8) AISpawned {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t destroyedAtFrame;
    std::uint8_t _pad0[4];
    EntityRef entity;

    // 0x7b948fc
    std::int32_t GetHashCode() const;
    // 0x7b94930
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(AISpawned) == AISpawned::SIZE);

struct alignas(8) ActiveCheckpoint {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    EntityRef Actor;
    EntityRef Checkpoint;

    // 0x7b949f4
    std::int32_t GetHashCode() const;
    // 0x7b94a1c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActiveCheckpoint) == ActiveCheckpoint::SIZE);

struct alignas(8) RespawnTimers {
    static constexpr int SIZE = 72;
    static constexpr int ALIGNMENT = 8;

    std::int32_t UnspawnedFrame;
    QBoolean IsRespawnPointCalculated;
    EntityRef ToRespawn;
    FPVector3 Position;
    FPQuaternion Rotation;

    // 0x7b9b5f0
    std::int32_t GetHashCode() const;
    // 0x7b9b678
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RespawnTimers) == RespawnTimers::SIZE);

struct alignas(4) SpawnPointDynamicData {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 4;

    std::int32_t lastRespawnFrame;
    std::int32_t numActorsInInnerRange;
    std::int32_t numActorsInOuterRange;
    QBoolean isOutsideShrinkZone;

    // 0x7b9c20c
    std::int32_t GetHashCode() const;
    // 0x7b9c27c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(SpawnPointDynamicData) == SpawnPointDynamicData::SIZE);

struct alignas(8) DoomGroupMember {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t joinedGroupFrame;
    std::uint8_t _pad0[4];
    EntityRef entity;

    // 0x7b96c58
    std::int32_t GetHashCode() const;
    // 0x7b96c88
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DoomGroupMember) == DoomGroupMember::SIZE);

struct alignas(8) TrailPoint {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    QBoolean trailCutFromPoint;
    std::uint8_t _pad0[4];
    FP frameAdded;
    FPVector3 position;

    // 0x7b9c81c
    std::int32_t GetHashCode() const;
    // 0x7b9c870
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(TrailPoint) == TrailPoint::SIZE);

struct alignas(8) EffectHitInfo {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    EntityRef Entity;
    FPVector3 Point;

    // 0x7b975e8
    std::int32_t GetHashCode() const;
    // 0x7b97618
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(EffectHitInfo) == EffectHitInfo::SIZE);

struct alignas(8) EffectedEntity {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t Frame;
    std::uint8_t _pad0[4];
    EntityRef Entity;

    // 0x7b97650
    std::int32_t GetHashCode() const;
    // 0x7b97680
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(EffectedEntity) == EffectedEntity::SIZE);

struct alignas(4) PowerUpStats {
    static constexpr int SIZE = 12;
    static constexpr int ALIGNMENT = 4;

    CollectionSource Source;
    std::int32_t Amount;
    PowerUpType Type;

    // 0x7b9a618
    std::int32_t GetHashCode() const;
    // 0x7b9a658
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PowerUpStats) == PowerUpStats::SIZE);

struct alignas(8) WeaponUsageStats {
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::int32_t numActorKilled;
    std::int32_t numCratesBroken;
    std::int32_t numCriticals_Mobs;
    std::int32_t numCriticals_Players;
    std::int32_t numHeadshots_Players;
    std::int32_t numHits_Env;
    std::int32_t numHits_Mobs;
    std::int32_t numHits_Players;
    std::int32_t numNPCKilled;
    std::int32_t numShots;
    FP totalDamageDealt;

    // 0x7b9e0d8
    std::int32_t GetHashCode() const;
    // 0x7b9e214
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(WeaponUsageStats) == WeaponUsageStats::SIZE);

struct alignas(8) CollidingEntity {
    static constexpr int SIZE = 88;
    static constexpr int ALIGNMENT = 8;

    std::int32_t collisionFrame;
    std::uint8_t _pad0[4];
    EntityRef entity;
    FPVector3 collisionVelocity;
    FPVector3 hitNormal;
    FPVector3 impactPosition;

    // 0x7b95e44
    std::int32_t GetHashCode() const;
    // 0x7b95ed4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(CollidingEntity) == CollidingEntity::SIZE);

struct alignas(8) ResetValues {
    static constexpr int SIZE = 56;
    static constexpr int ALIGNMENT = 8;

    FPVector3 position;
    FPQuaternion rotation;

    // 0x7b9b578
    std::int32_t GetHashCode() const;
    // 0x7b9b5b8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ResetValues) == ResetValues::SIZE);

struct alignas(4) GoalScoreFrame {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::int32_t frame;
    TeamFlag team;

    // 0x7b9819c
    std::int32_t GetHashCode() const;
    // 0x7b981c8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GoalScoreFrame) == GoalScoreFrame::SIZE);

struct alignas(8) BotUTData {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::uint32_t rank;
    std::uint8_t _pad0[4];
    FP score;

    // 0x7b955fc
    std::int32_t GetHashCode() const;
    // 0x7b9563c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BotUTData) == BotUTData::SIZE);

struct alignas(8) RicochetTargetInfo {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t priority;
    std::uint8_t _pad0[4];
    EntityRef entity;

    // 0x7b9ba08
    std::int32_t GetHashCode() const;
    // 0x7b9ba38
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RicochetTargetInfo) == RicochetTargetInfo::SIZE);

struct alignas(8) DamageIncreaseData {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t StartFrame;
    std::uint8_t _pad0[4];
    FP DamagePercentIncrease;
    FP Duration;

    // 0x7b966b0
    std::int32_t GetHashCode() const;
    // 0x7b96710
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DamageIncreaseData) == DamageIncreaseData::SIZE);

struct alignas(8) DamageMultiplierData {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    DAMAGE_MULTIPLIER_ORIGIN origin;
    std::int32_t endFrame;
    FP value;

    // 0x7b967e4
    std::int32_t GetHashCode() const;
    // 0x7b96838
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DamageMultiplierData) == DamageMultiplierData::SIZE);

struct alignas(2) CollectData {
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 2;

    std::uint16_t collectCount;
    std::uint16_t collectStartFrame;

    // 0x7b95ce0
    std::int32_t GetHashCode() const;
    // 0x7b95d20
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(CollectData) == CollectData::SIZE);

struct alignas(8) DropAnimData {
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    FPVector3 dropPos;
    FPVector3 targetPos;

    // 0x7b96ed0
    std::int32_t GetHashCode() const;
    // 0x7b96f10
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(DropAnimData) == DropAnimData::SIZE);

struct alignas(8) Droppable {
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::uint8_t collectorVersion;
    std::uint8_t dropperVersion;
    std::uint8_t teamIndex;
    DroppableState state;
    DroppableType type;
    std::uint8_t _pad0[1];
    std::uint16_t collectorIndex;
    std::uint16_t destroyFrame;
    std::uint16_t dropFrame;
    std::uint16_t dropperIndex;
    std::uint16_t id;
    QBoolean shortRange;
    std::uint8_t _pad1[4];
    FPVector3 position;

    // 0x7b96f48
    std::int32_t GetHashCode() const;
    // 0x7b9707c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Droppable) == Droppable::SIZE);

struct alignas(8) EntityGroupMember {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t JoinedGroupFrame;
    std::uint8_t _pad0[4];
    EntityRef Entity;

    // 0x7b97ab0
    std::int32_t GetHashCode() const;
    // 0x7b97ae0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(EntityGroupMember) == EntityGroupMember::SIZE);

struct alignas(8) GatlingFireRateLevel {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    QBoolean isProgressive;
    std::uint8_t _pad0[4];
    FP fireRate;
    FP spinUpTime;

    // 0x7b98100
    std::int32_t GetHashCode() const;
    // 0x7b98154
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(GatlingFireRateLevel) == GatlingFireRateLevel::SIZE);

struct alignas(4) PendingGoldBoltSpawn {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::int32_t SpawnFrame;
    std::int32_t SpawnPointIndex;

    // 0x7b9a1fc
    std::int32_t GetHashCode() const;
    // 0x7b9a23c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PendingGoldBoltSpawn) == PendingGoldBoltSpawn::SIZE);

struct alignas(8) Overhealth {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    std::int32_t StartFrame;
    std::uint8_t _pad0[4];
    FP DecreaseDuration;
    FP DecreaseTickRate;
    FP Duration;
    FP Value;

    // 0x7b9a0d4 - Seconds between two decay ticks (implemented in systems/health_systems.cpp).
    FP get_DecreaseInterval() const;
    // 0x7b9a108 - HP lost per decay tick.
    FP get_DecreaseByTick() const;

    // Value equality, as the original's default struct comparison (QList.Remove).
    friend bool operator==(const Overhealth& a, const Overhealth& b) {
        return a.StartFrame == b.StartFrame && a.DecreaseDuration == b.DecreaseDuration &&
               a.DecreaseTickRate == b.DecreaseTickRate && a.Duration == b.Duration && a.Value == b.Value;
    }

    // 0x7b99f48
    std::int32_t GetHashCode() const;
    // 0x7b99fe0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Overhealth) == Overhealth::SIZE);

struct alignas(8) ModInfo {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    MOD_ORIGIN origin;
    std::uint8_t _pad0[4];
    FP date;
    FP duration;
    FP falloffDuration;
    FP value;

    // 0x7b99be0
    std::int32_t GetHashCode() const;
    // 0x7b99c6c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ModInfo) == ModInfo::SIZE);

struct alignas(8) KillZoneArea {
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    KillZoneSurfaceType surfaceType;
    std::uint8_t _pad0[4];
    FPVector2 size;
    FPVector3 corner;

    // 0x7b99620
    std::int32_t GetHashCode() const;
    // 0x7b99674
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(KillZoneArea) == KillZoneArea::SIZE);

struct alignas(8) ActivePowerUp {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t StartFrame;
    ::Quantum::PowerUpType PowerUpType;
    AssetRefPowerUpSpec Spec;

    // 0x7b94a54
    std::int32_t GetHashCode() const;
    // 0x7b94aa8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ActivePowerUp) == ActivePowerUp::SIZE);

struct alignas(8) RankingItem {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::uint16_t rank;
    std::uint16_t score;
    std::int32_t eliminationFrame;
    std::int32_t scoreFrame;
    std::int32_t teamId;
    EntityRef entity;

    // 0x7b9b250
    std::int32_t GetHashCode() const;
    // 0x7b9b30c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(RankingItem) == RankingItem::SIZE);

struct alignas(8) Reward {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    DroppableType type;
    std::uint8_t _pad0[1];
    std::uint16_t amount;
    std::uint16_t collectFrame;
    std::uint8_t _pad1[2];
    QBoolean isRival;
    RewardReason reason;
    FPVector3 sourceEntityPosition;

    // 0x7b9b76c
    std::int32_t GetHashCode() const;
    // 0x7b9b7f4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Reward) == Reward::SIZE);

struct alignas(8) LockedTarget {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t startLockFrame;
    QBoolean isLocked;
    EntityRef entity;

    // 0x7b99ac4
    std::int32_t GetHashCode() const;
    // 0x7b99b0c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(LockedTarget) == LockedTarget::SIZE);

struct alignas(8) ShieldGroupMember {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    std::int32_t joinedGroupFrame;
    std::uint8_t _pad0[4];
    EntityRef entity;

    // 0x7b9bce4
    std::int32_t GetHashCode() const;
    // 0x7b9bd14
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ShieldGroupMember) == ShieldGroupMember::SIZE);

struct alignas(8) Stat {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    STAT_ID id;
    std::uint8_t _pad0[4];
    FP value;

    // 0x7b9c43c
    std::int32_t GetHashCode() const;
    // 0x7b9c464
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Stat) == Stat::SIZE);

struct alignas(8) StatSummary {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    FP avg;
    FP lastValue;
    FP max;
    FP min;

    // 0x7b9c528
    std::int32_t GetHashCode() const;
    // 0x7b9c5a4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(StatSummary) == StatSummary::SIZE);

struct alignas(8) StreamPoint {
    static constexpr int SIZE = 48;
    static constexpr int ALIGNMENT = 8;

    std::int32_t creationFrame;
    std::uint8_t _pad0[4];
    EntityRef entity;
    EntityRef previousPointEntity;
    FPVector3 position;

    // 0x7b9c5fc
    std::int32_t GetHashCode() const;
    // 0x7b9c66c
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(StreamPoint) == StreamPoint::SIZE);

struct alignas(8) TargetingData {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;

    EntityRef target;
    FPVector3 targetPoint;

    // 0x7b9c7b4
    std::int32_t GetHashCode() const;
    // 0x7b9c7e4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(TargetingData) == TargetingData::SIZE);

struct alignas(8) ControlZoneStateHolder {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    CONTROL_ZONE_STATE State;
    std::int32_t LastControllerTeamId;
    std::int32_t LastFullControlFrame;
    std::int32_t MapCustomDataIndex;
    std::int32_t NbOfControlSwitches;
    std::int32_t ScoringFrame;
    std::int32_t StateStartFrame;
    std::uint8_t _pad0[4];
    FP ControlPercent;

    // 0x7b96064
    std::int32_t GetHashCode() const;
    // 0x7b96148
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(ControlZoneStateHolder) == ControlZoneStateHolder::SIZE);

struct alignas(8) BotSDKData {
    static constexpr int SIZE = 24;
    static constexpr int ALIGNMENT = 8;

    std::int32_t ElapsedTicks;
    std::uint8_t _pad0[4];
    FP ElapsedPartialTicks;
    FP OriginalDeltaTime;

    // 0x7b952e0
    std::int32_t GetHashCode() const;
    // 0x7b95340
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BotSDKData) == BotSDKData::SIZE);

struct alignas(4) UTMomentumData {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::uint8_t DecayAmount;
    std::uint8_t _pad0[3];
    std::int32_t Value;

    // 0x7b871a4
    std::int32_t GetHashCode() const;
    // 0x7b871e4
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(UTMomentumData) == UTMomentumData::SIZE);

struct alignas(8) UTMomentumPack {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;

    AssetRefConsideration ConsiderationRef;
    UTMomentumData MomentumData;

    // 0x7b9c8b8
    std::int32_t GetHashCode() const;
    // 0x7b9c8f8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(UTMomentumPack) == UTMomentumPack::SIZE);

struct alignas(4) QMultiShotConfig {
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 4;

    Ptr DataPtr;

    // 0x7b9b0b4
    Collections::QListPtr<QMultiShotData> get_Data() const { return Collections::QListPtr<QMultiShotData>(DataPtr); }
    // 0x7b9b0f0
    void set_Data(Collections::QListPtr<QMultiShotData> value) { DataPtr = value.Ptr; }

    // 0x7b9b0f8
    std::int32_t GetHashCode() const;
    // 0x7b9b108
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(QMultiShotConfig) == QMultiShotConfig::SIZE);

struct alignas(8) QMultiShotData {
    static constexpr int SIZE = 40;
    static constexpr int ALIGNMENT = 8;

    QBoolean UseRandomOffset;
    QBoolean UseSpecialAmmunition;
    AssetRefEntityPrototype ShotAmmunition;
    FPVector3 SpawnOffset;

    // 0x7b9b19c
    std::int32_t GetHashCode() const;
    // 0x7b9b1f8
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(QMultiShotData) == QMultiShotData::SIZE);

} // namespace Quantum
