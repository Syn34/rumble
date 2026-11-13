// level_assets.h - level data: per-map custom data, static triggers, buttons and moving walls (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_core/physics3d/scene3d.h"

namespace Quantum {

class BumperBarConfig : public AssetObject {
  public:
    FP PushForce{};
    FP forceUpwardCorrection{};

    // 0x7c8e1d8
    BumperBarConfig() = default;
};

class BumperConfig : public AssetObject {
  public:
    FP velocityBlend = FP::FromRaw(52428); // 0.799988
    FP lockedInputBlend = FP::FromRaw(52428); // 0.799988
    FP lockedInputDuration = FP::FromRaw(49152); // 0.75
    FP lockedInputFallOff = FP::FromRaw(16384); // 0.25
    std::uint32_t AK_Wwise_Event_GP_JumpPad_Activate = 3495499727u;

    // 0x7c8e1b0
    BumperConfig() = default;
};

class ItemSpawnerConfig : public AssetObject {
  public:
    ::Quantum::EntityPrefabsAmount EntityPrefabsAmount{};
    AssetRefEntityPrototype EntityToSpawn;
    std::vector<AssetRefEntityPrototype> EntitiesToSpawn;
    std::int32_t AmountToSpawn = 1;
    bool SpawnOnStart = true;
    bool DoesRespawn = true;
    FP RespawnDelay = FP::FromInt(5);
    FP DelayBetweenEachItem = FP::FromRaw(32768); // 0.5
    bool WakeUpPhysicsBody{};
    FP ItemSpawnHeight = FP::FromInt(1);
    FP DropDuration = FP::FromRaw(32768); // 0.5
    std::vector<FPVector3> SpawnPatternOffset;
    FP ItemSpawnMinRotation{};
    FP ItemSpawnMaxRotation{};
    FPVector3 RandomOffset;

    // 0x7c92874
    ItemSpawnerConfig() = default;

    // TODO(0x7c9278c, Quantum.ItemSpawnerConfig.TryGetEntityToSpawn): bool
    //   TryGetEntityToSpawn(Frame frame, AssetRefEntityPrototype entityToSpawn); uses
    //   RNGSession.Next, AssetGuid.op_Equality, AssetGuid.op_Implicit,
    //   AssetRefEntityPrototype.op_Equality, AssetRefEntityPrototype.op_Implicit, Frame.get_RNG
};

class LevelButtonConfig : public AssetObject {
  public:
    bool autoSwitchOffAfterActivation{};
    FP timeBeforeAutoSwitchOff{};
    FP cooldownAfterSwitchOff{};

    // 0x7c92ad8
    LevelButtonConfig() = default;
};

class LevelButtonGameplayElementConfig : public AssetObject {
  public:

    // 0x7c92ae8
    LevelButtonGameplayElementConfig() = default;

    // 0x7c92ae0 - nothing by default.
    virtual void OnSwitchOn(Frame* f, EntityRef entity, EntityRef pusherActor) const { (void)f, (void)entity, (void)pusherActor; }
    // 0x7c92ae4 - nothing by default.
    virtual void OnSwitchOff(Frame* f, EntityRef entity) const { (void)f, (void)entity; }
};

class MovingWallConfig : public AssetObject {
  public:
    bool StartsLockedInPlace = true;
    FP MoveSpeed = FP::FromInt(1);
    std::vector<MovingWallOffset> WallOffsets;

    // 0x7c937c8
    MovingWallConfig() = default;
};

class ShrinkingZoneSpec : public AssetObject {
  public:
    FP hitInterval = FP::FromInt(1);
    AssetRefDamageSpec DamageSpec;
    FPAnimationCurve DamageMultiplierCurve;
    AssetRefPusherSpec PusherSpec;

    // 0x7c94568
    ShrinkingZoneSpec() = default;
};

class StaticColliderConfig : public AssetObject {
  public:
    std::int32_t CustomId{};
    KccSurfaceType SurfaceType{};
    bool IgnoreAutoJump{};
    FP ReboundBonus{};

    // 0x7c928d4
    StaticColliderConfig() = default;
};

class StaticTriggerConfig : public AssetObject {
  public:

    // 0x7c928e8
    StaticTriggerConfig() = default;

    // 0x7c928dc - nothing by default.
    virtual void OnTriggerEnter(Frame* f, const Physics3D::TriggerInfo3D& info) const { (void)f, (void)info; }
    // 0x7c928e0 - nothing by default.
    virtual void OnTrigger(Frame* f, const Physics3D::TriggerInfo3D& info) const { (void)f, (void)info; }
    // 0x7c928e4 - nothing by default.
    virtual void OnTriggerExit(Frame* f, const Physics3D::ExitInfo3D& info) const { (void)f, (void)info; }
};

class ArenaCannonConfig : public LevelButtonGameplayElementConfig {
  public:
    FP FireDuration{};
    FP InitialDelay{};

    // 0x7bd7e94
    ArenaCannonConfig() = default;

    // TODO(0x7bd7d10, Quantum.ArenaCannonConfig.OnSwitchOn): void OnSwitchOn(Frame f, EntityRef
    //   entity, EntityRef pusherActor); uses ArenaCannon.OnSwitchOn,
    //   LevelButtonGameplayElementConfig.OnSwitchOn
    // TODO(0x7bd7ddc, Quantum.ArenaCannonConfig.OnSwitchOff): void OnSwitchOff(Frame f, EntityRef
    //   entity); uses ArenaCannon.OnSwitchOff, LevelButtonGameplayElementConfig.OnSwitchOff
};

class AutoJumperStaticTriggerConfig : public StaticTriggerConfig {
  public:

    // 0x7bd5f78
    AutoJumperStaticTriggerConfig() = default;

    // TODO(0x7bd5ce0, Quantum.AutoJumperStaticTriggerConfig.OnTriggerEnter): void
    //   OnTriggerEnter(Frame frame, TriggerInfo3D info); uses FrameEvents.AutoJumperTriggered,
    //   AutoJumperLib.StartAutoJump, TriggerInfo3D.get_Entity, TriggerInfo3D.get_StaticData
};

class DestroyElementConfig : public LevelButtonGameplayElementConfig {
  public:

    // 0x7bd7ed4
    DestroyElementConfig() = default;

    // TODO(0x7bd7e9c, Quantum.DestroyElementConfig.OnSwitchOn): void OnSwitchOn(Frame f, EntityRef
    //   entity, EntityRef pusherActor); uses Core.FrameBase.Destroy,
    //   LevelButtonGameplayElementConfig.OnSwitchOn
};

class DestroyMobGroupGameplayElement : public LevelButtonGameplayElementConfig {
  public:

    // 0x7cd3138
    DestroyMobGroupGameplayElement() = default;

    // TODO(0x7cd3070, Quantum.DestroyMobGroupGameplayElement.OnSwitchOn): void OnSwitchOn(Frame f,
    //   EntityRef entity, EntityRef pusherActor); uses LevelButtonGameplayElementConfig.OnSwitchOn,
    //   MobGroup.KillAllGroupMembers
};

class MovingWallGameplayElementConfig : public LevelButtonGameplayElementConfig {
  public:
    enum class Mode : std::int32_t {
        Unlock = 0,
        Lock = 1,
    };

    MovingWallGameplayElementConfig::Mode mode{};

    // 0x7bd8330
    MovingWallGameplayElementConfig() = default;

    // TODO(0x7bd8190, Quantum.MovingWallGameplayElementConfig.OnSwitchOn): void OnSwitchOn(Frame f,
    //   EntityRef entity, EntityRef pusherActor); uses FrameEvents.OnMovingWallStart,
    //   MovingWall.GetCurrentOffset, MovingWall.IncrementCurrentOffset,
    //   NavMeshRegionMask.ToggleRegion, QBoolean.op_Implicit
};

class RailStaticTriggerConfig : public StaticTriggerConfig {
  public:

    // 0x7bd9308
    RailStaticTriggerConfig() = default;

    // TODO(0x7bd8f18, Quantum.RailStaticTriggerConfig.OnTriggerEnter): void OnTriggerEnter(Frame
    //   frame, TriggerInfo3D info); uses GrinderActor.get_CanGrind,
    //   MapCustomData.TryGetStaticColliderToGrindRailIndex, QBoolean.op_Implicit,
    //   RailLib.TryGrindOnRail, TriggerInfo3D.get_Entity, TriggerInfo3D.get_StaticData
};

class TravolatorStaticTriggerConfig : public StaticTriggerConfig {
  public:
    FP MoveSpeedModifier = FP::FromInt(1);
    FP InputBlend = FP::FromRaw(29484); // 0.44989

    // 0x7bda150
    TravolatorStaticTriggerConfig() = default;

    // TODO(0x7bd9df0, Quantum.TravolatorStaticTriggerConfig.OnTrigger): void OnTrigger(Frame f,
    //   TriggerInfo3D info); uses TriggerInfo3D.get_Entity, TriggerInfo3D.get_StaticData
    // TODO(0x7bda064, Quantum.TravolatorStaticTriggerConfig.OnTriggerExit): void
    //   OnTriggerExit(Frame f, ExitInfo3D info); uses FPVector3.get_Zero
};

} // namespace Quantum
