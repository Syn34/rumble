// event_types.h - the simulation's events (quantum.code.dll): one EventBase subclass per
// event, raised through Frame::Events (frame_events.h) and handed to the presentation.
//
// Each event carries the values it was raised with. Events are pooled per id by the frame
// context, so an event object is reused: every field is overwritten when it is raised.
#pragma once

#include "quantum_code/components.h"
#include "quantum_core/events.h"

#include <cstdint>

namespace Quantum {

class PowerUpSpec;

// Base of the events that concern one entity.
class EventBaseEntityEvent : public EventBase {
  public:
    // 0x7c8a9a0
    EventBaseEntityEvent(int id, EventFlags flags) : EventBase(id, flags) {}

    EntityRef entity;

    // 0x7c8adb4
    virtual std::int32_t GetHashCode() const;
};

class EventAISpawner_OnSpawned final : public EventBase {
  public:
    static constexpr int ID = 0;

    // 0x7c89b40
    EventAISpawner_OnSpawned() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef spawnerEntity{};
    EntityRef spawnedEntity{};

    // 0x7c89bc8
    std::int32_t GetHashCode() const;
};

class EventAISpawner_OnDespawned final : public EventBase {
  public:
    static constexpr int ID = 1;

    // 0x7c89bf0
    EventAISpawner_OnDespawned() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef spawnerEntity{};
    EntityRef despawnedEntity{};

    // 0x7c89c78
    std::int32_t GetHashCode() const;
};

class EventActorIcedChange final : public EventBase {
  public:
    static constexpr int ID = 2;

    // 0x7c89ca0
    EventActorIcedChange() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    QBoolean iced{};

    // 0x7c89d28
    std::int32_t GetHashCode() const;
};

class EventRespawnTimerCreated final : public EventBase {
  public:
    static constexpr int ID = 3;

    // 0x7c89d4c
    EventRespawnTimerCreated() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    FP respawnPointCalculationDelay{};

    // 0x7c89dd4
    std::int32_t GetHashCode() const;
};

class EventRespawnPointSet final : public EventBase {
  public:
    static constexpr int ID = 4;

    // 0x7c89e08
    EventRespawnPointSet() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    FPVector3 spawnPosition{};
    FPQuaternion spawnRotation{};

    // 0x7c89e90
    std::int32_t GetHashCode() const;
};

class EventRespawnTimerDestroyed final : public EventBase {
  public:
    static constexpr int ID = 5;

    // 0x7c89ee8
    EventRespawnTimerDestroyed() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c89f70
    std::int32_t GetHashCode() const;
};

class EventActorSheepedChange final : public EventBase {
  public:
    static constexpr int ID = 7;

    // 0x7c89f84
    EventActorSheepedChange() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    QBoolean sheeped{};

    // 0x7c89f94
    std::int32_t GetHashCode() const;
};

class EventActorStunnedChange final : public EventBase {
  public:
    static constexpr int ID = 8;

    // 0x7c89fb8
    EventActorStunnedChange() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    QBoolean stunned{};

    // 0x7c89fc8
    std::int32_t GetHashCode() const;
};

class EventAgentOfDoomEvent final : public EventBase {
  public:
    static constexpr int ID = 9;

    // 0x7c89fec
    EventAgentOfDoomEvent() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    AgentOfDoomEventType eventType{};

    // 0x7c89ffc
    std::int32_t GetHashCode() const;
};

class EventAreaOfEffect_OnHit final : public EventBase {
  public:
    static constexpr int ID = 10;

    // 0x7c8a030
    EventAreaOfEffect_OnHit() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8a0b8
    std::int32_t GetHashCode() const;
};

class EventAreaOfEffect_OnMiss final : public EventBase {
  public:
    static constexpr int ID = 11;

    // 0x7c8a0cc
    EventAreaOfEffect_OnMiss() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8a0dc
    std::int32_t GetHashCode() const;
};

class EventAreaOfEffect_OnHitInstigator final : public EventBase {
  public:
    static constexpr int ID = 12;

    // 0x7c8a0f0
    EventAreaOfEffect_OnHitInstigator() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef instigator{};

    // 0x7c8a178
    std::int32_t GetHashCode() const;
};

class EventAreaOfEffect_OnEnterArea final : public EventBase {
  public:
    static constexpr int ID = 13;

    // 0x7c8a18c
    EventAreaOfEffect_OnEnterArea() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef inAreaEntity{};
    EntityRef aoeEntity{};

    // 0x7c8a214
    std::int32_t GetHashCode() const;
};

class EventAreaOfEffect_OnExitArea final : public EventBase {
  public:
    static constexpr int ID = 14;

    // 0x7c8a23c
    EventAreaOfEffect_OnExitArea() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef outOfAreaEntity{};
    EntityRef aoeEntity{};

    // 0x7c8a2c4
    std::int32_t GetHashCode() const;
};

class EventAutoJumperTriggered final : public EventBase {
  public:
    static constexpr int ID = 15;

    // 0x7c8a2ec
    EventAutoJumperTriggered() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef jumpingEntity{};
    std::int32_t staticColliderIndex{};

    // 0x7c8a2fc
    std::int32_t GetHashCode() const;
};

class EventBigBoomMine_Triggered final : public EventBase {
  public:
    static constexpr int ID = 16;

    // 0x7c8a330
    EventBigBoomMine_Triggered() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef Owner{};

    // 0x7c8a340
    std::int32_t GetHashCode() const;
};

class EventBlastBallGame_StateChanged final : public EventBase {
  public:
    static constexpr int ID = 17;

    // 0x7c8a354
    EventBlastBallGame_StateChanged() : EventBase(ID, static_cast<EventFlags>(7)) {}

    BlastBallGameState newState{};

    // 0x7c8a3dc
    std::int32_t GetHashCode() const;
};

class EventBlastBallGame_StartOverTime final : public EventBase {
  public:
    static constexpr int ID = 18;

    // 0x7c8a3f8
    EventBlastBallGame_StartOverTime() : EventBase(ID, static_cast<EventFlags>(7)) {}

    // 0x7c8a408
    std::int32_t GetHashCode() const;
};

class EventBlastBallGame_TeamScored final : public EventBase {
  public:
    static constexpr int ID = 19;

    // 0x7c8a410
    EventBlastBallGame_TeamScored() : EventBase(ID, static_cast<EventFlags>(7)) {}

    TeamFlag scoringTeam{};
    std::uint16_t score{};
    EntityRef scoringPlayer{};

    // 0x7c8a498
    std::int32_t GetHashCode() const;
};

class EventBlastBallGame_BallHit final : public EventBase {
  public:
    static constexpr int ID = 20;

    // 0x7c8a4f0
    EventBlastBallGame_BallHit() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef player{};
    FPVector3 position{};
    FPVector3 direction{};

    // 0x7c8a500
    std::int32_t GetHashCode() const;
};

class EventOnBoltCollected final : public EventBase {
  public:
    static constexpr int ID = 21;

    // 0x7c8a558
    EventOnBoltCollected() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    std::int32_t amount{};

    // 0x7c8a5e0
    std::int32_t GetHashCode() const;
};

class EventOnBoltLost final : public EventBase {
  public:
    static constexpr int ID = 22;

    // 0x7c8a614
    EventOnBoltLost() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    std::int32_t amount{};

    // 0x7c8a69c
    std::int32_t GetHashCode() const;
};

class EventBoltCounter_OnUpdateBolts final : public EventBase {
  public:
    static constexpr int ID = 23;

    // 0x7c8a6d0
    EventBoltCounter_OnUpdateBolts() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef counterEntity{};
    std::int32_t currentBolts{};

    // 0x7c8a6e0
    std::int32_t GetHashCode() const;
};

class EventBumperTriggered final : public EventBase {
  public:
    static constexpr int ID = 24;

    // 0x7c8a714
    EventBumperTriggered() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef bumper{};
    EntityRef player{};

    // 0x7c8a79c
    std::int32_t GetHashCode() const;
};

class EventCharacterColliderScaler_OnScaleChanged final : public EventBase {
  public:
    static constexpr int ID = 25;

    // 0x7c8a7c4
    EventCharacterColliderScaler_OnScaleChanged() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8a84c
    std::int32_t GetHashCode() const;
};

class EventColliderResize_OnResizeStarted final : public EventBase {
  public:
    static constexpr int ID = 26;

    // 0x7c8a864
    EventColliderResize_OnResizeStarted() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8a874
    std::int32_t GetHashCode() const;
};

class EventCollisionForce_OnCollision final : public EventBase {
  public:
    static constexpr int ID = 27;

    // 0x7c8a88c
    EventCollisionForce_OnCollision() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    EntityRef otherEntity{};
    FPVector3 hitPoint{};
    FPVector3 hitNormal{};

    // 0x7c8a914
    std::int32_t GetHashCode() const;
};

class EventDamageable_HitReceived final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 28;

    // 0x7c8a990
    EventDamageable_HitReceived() : EventBaseEntityEvent(ID, static_cast<EventFlags>(6)) {}

    HitData hitData{};
    HitResult hitResult{};

    // 0x7c8a9a8
    std::int32_t GetHashCode() const override;
};

class EventOnDamageChargeableReset final : public EventBase {
  public:
    static constexpr int ID = 29;

    // 0x7c8aa00
    EventOnDamageChargeableReset() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef chargeableEntity{};

    // 0x7c8aa88
    std::int32_t GetHashCode() const;
};

class EventDashStarted final : public EventBase {
  public:
    static constexpr int ID = 30;

    // 0x7c8aaa0
    EventDashStarted() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8ab28
    std::int32_t GetHashCode() const;
};

class EventDashComponentDisabled final : public EventBase {
  public:
    static constexpr int ID = 31;

    // 0x7c8ab40
    EventDashComponentDisabled() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8ab50
    std::int32_t GetHashCode() const;
};

class EventDrillDash_OnStateChanged final : public EventBase {
  public:
    static constexpr int ID = 32;

    // 0x7c8ab68
    EventDrillDash_OnStateChanged() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    EntityRef weaponEntity{};
    DrillDashState newState{};

    // 0x7c8ab78
    std::int32_t GetHashCode() const;
};

class EventOnDroppableDropped final : public EventBase {
  public:
    static constexpr int ID = 33;

    // 0x7c8abcc
    EventOnDroppableDropped() : EventBase(ID, static_cast<EventFlags>(7)) {}

    EntityRef dropperEntity{};
    DroppableType type{};
    FPVector3 position{};
    std::uint16_t amount{};

    // 0x7c8ac54
    std::int32_t GetHashCode() const;
};

class EventEliminationGameMode_OnCloseToWin final : public EventBase {
  public:
    static constexpr int ID = 34;

    // 0x7c8acd0
    EventEliminationGameMode_OnCloseToWin() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8ace0
    std::int32_t GetHashCode() const;
};

class EventEnergyCell_OnCollected final : public EventBase {
  public:
    static constexpr int ID = 35;

    // 0x7c8acf8
    EventEnergyCell_OnCollected() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef collector{};
    std::int32_t value{};

    // 0x7c8ad08
    std::int32_t GetHashCode() const;
};

class EventFireReactor_OnStateChanged final : public EventBase {
  public:
    static constexpr int ID = 37;

    // 0x7c8adcc
    EventFireReactor_OnStateChanged() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    QBoolean enabled{};

    // 0x7c8addc
    std::int32_t GetHashCode() const;
};

class EventGadgetUser_OnGadgetActivated final : public EventBase {
  public:
    static constexpr int ID = 38;

    // 0x7c8ae00
    EventGadgetUser_OnGadgetActivated() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8ae10
    std::int32_t GetHashCode() const;
};

class EventGadgetUser_OnGadgetDeactivated final : public EventBase {
  public:
    static constexpr int ID = 39;

    // 0x7c8ae28
    EventGadgetUser_OnGadgetDeactivated() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8ae38
    std::int32_t GetHashCode() const;
};

class EventIntroEnded final : public EventBase {
  public:
    static constexpr int ID = 40;

    // 0x7c8ae50
    EventIntroEnded() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8ae60
    std::int32_t GetHashCode() const;
};

class EventBoltMultiplierOn final : public EventBase {
  public:
    static constexpr int ID = 42;

    // 0x7c8ae78
    EventBoltMultiplierOn() : EventBase(ID, static_cast<EventFlags>(6)) {}

    // 0x7c8ae88
    std::int32_t GetHashCode() const;
};

class EventGameModeHandlerStateChanged final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 43;

    // 0x7c8ae90
    EventGameModeHandlerStateChanged() : EventBaseEntityEvent(ID, static_cast<EventFlags>(7)) {}

    GAME_MODE_HANDLER_STATE prevState{};
    GAME_MODE_HANDLER_STATE newState{};

    // 0x7c8aea0
    std::int32_t GetHashCode() const override;
};

class EventGameMode_OnPlayerEliminated final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 46;

    // 0x7c8af70
    EventGameMode_OnPlayerEliminated() : EventBaseEntityEvent(ID, static_cast<EventFlags>(6)) {}

    // 0x7c8af80
    std::int32_t GetHashCode() const override;
};

class EventGameMode_OnPlayerFinished final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 47;

    // 0x7c8af98
    EventGameMode_OnPlayerFinished() : EventBaseEntityEvent(ID, static_cast<EventFlags>(6)) {}

    // 0x7c8afa8
    std::int32_t GetHashCode() const override;
};

class EventPreloadAddressablePrefabs final : public EventBase {
  public:
    static constexpr int ID = 48;

    // 0x7c8afc0
    EventPreloadAddressablePrefabs() : EventBase(ID, static_cast<EventFlags>(7)) {}

    // 0x7c8b048
    std::int32_t GetHashCode() const;
};

class EventGoldenBoltSpawned final : public EventBase {
  public:
    static constexpr int ID = 49;

    // 0x7c8b050
    EventGoldenBoltSpawned() : EventBase(ID, static_cast<EventFlags>(7)) {}

    QBoolean isUnique{};

    // 0x7c8b0d8
    std::int32_t GetHashCode() const;
};

class EventGoldenBoltCollected final : public EventBase {
  public:
    static constexpr int ID = 50;

    // 0x7c8b0e8
    EventGoldenBoltCollected() : EventBase(ID, static_cast<EventFlags>(7)) {}

    EntityRef boltEntity{};
    EntityRef collector{};

    // 0x7c8b170
    std::int32_t GetHashCode() const;
};

class EventStartGrind final : public EventBase {
  public:
    static constexpr int ID = 51;

    // 0x7c8b198
    EventStartGrind() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    QBoolean jumpAnim{};

    // 0x7c8b220
    std::int32_t GetHashCode() const;
};

class EventSnappedToRail final : public EventBase {
  public:
    static constexpr int ID = 52;

    // 0x7c8b244
    EventSnappedToRail() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8b2cc
    std::int32_t GetHashCode() const;
};

class EventExitGrind final : public EventBase {
  public:
    static constexpr int ID = 53;

    // 0x7c8b2e4
    EventExitGrind() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    QBoolean jumpAnim{};

    // 0x7c8b36c
    std::int32_t GetHashCode() const;
};

class EventSwitchRail final : public EventBase {
  public:
    static constexpr int ID = 54;

    // 0x7c8b390
    EventSwitchRail() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    QBoolean isLeft{};

    // 0x7c8b3a0
    std::int32_t GetHashCode() const;
};

class EventGroundDashStarting final : public EventBase {
  public:
    static constexpr int ID = 55;

    // 0x7c8b3c4
    EventGroundDashStarting() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef userEntity{};

    // 0x7c8b44c
    std::int32_t GetHashCode() const;
};

class EventGroundDashStarted final : public EventBase {
  public:
    static constexpr int ID = 56;

    // 0x7c8b464
    EventGroundDashStarted() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef userEntity{};

    // 0x7c8b4ec
    std::int32_t GetHashCode() const;
};

class EventGroundDashEnded final : public EventBase {
  public:
    static constexpr int ID = 57;

    // 0x7c8b504
    EventGroundDashEnded() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef userEntity{};
    GroundDashEndReason endReason{};

    // 0x7c8b58c
    std::int32_t GetHashCode() const;
};

class EventHealSource_OnHeal final : public EventBase {
  public:
    static constexpr int ID = 58;

    // 0x7c8b5c0
    EventHealSource_OnHeal() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8b5d0
    std::int32_t GetHashCode() const;
};

class EventOnHealthDecreased final : public EventBase {
  public:
    static constexpr int ID = 59;

    // 0x7c8b5e8
    EventOnHealthDecreased() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    FP amount{};
    HitData hitData{};

    // 0x7c8b670
    std::int32_t GetHashCode() const;
};

class EventHelipackStart final : public EventBase {
  public:
    static constexpr int ID = 60;

    // 0x7c8b6c8
    EventHelipackStart() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    FP verticalVelocity{};

    // 0x7c8b750
    std::int32_t GetHashCode() const;
};

class EventHelipackStop final : public EventBase {
  public:
    static constexpr int ID = 61;

    // 0x7c8b784
    EventHelipackStop() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8b80c
    std::int32_t GetHashCode() const;
};

class EventItemSpawnerStatus final : public EventBase {
  public:
    static constexpr int ID = 62;

    // 0x7c8b824
    EventItemSpawnerStatus() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    ItemSpawnerStatusType status{};
    QBoolean enabled{};

    // 0x7c8b8ac
    std::int32_t GetHashCode() const;
};

class EventItemSpawnerEntitySpawn final : public EventBase {
  public:
    static constexpr int ID = 63;

    // 0x7c8b8f8
    EventItemSpawnerEntitySpawn() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8b908
    std::int32_t GetHashCode() const;
};

class EventKccExtension_OnJumpStateChanged final : public EventBase {
  public:
    static constexpr int ID = 64;

    // 0x7c8b920
    EventKccExtension_OnJumpStateChanged() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    JumpState jumpState{};

    // 0x7c8b9a8
    std::int32_t GetHashCode() const;
};

class EventKccExtensions_OnMoveSpeedModifierAdded final : public EventBase {
  public:
    static constexpr int ID = 65;

    // 0x7c8b9dc
    EventKccExtensions_OnMoveSpeedModifierAdded() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    FP value{};
    FP duration{};

    // 0x7c8b9ec
    std::int32_t GetHashCode() const;
};

class EventKillEvent final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 66;

    // 0x7c8ba44
    EventKillEvent() : EventBaseEntityEvent(ID, static_cast<EventFlags>(7)) {}

    KillType killType{};
    KillZoneSurfaceType killZoneSurfaceType{};

    // 0x7c8ba54
    std::int32_t GetHashCode() const override;
};

class EventKillableSpawnEvent final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 67;

    // 0x7c8baac
    EventKillableSpawnEvent() : EventBaseEntityEvent(ID, static_cast<EventFlags>(6)) {}

    // 0x7c8babc
    std::int32_t GetHashCode() const override;
};

class EventReviveEvent final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 68;

    // 0x7c8bad4
    EventReviveEvent() : EventBaseEntityEvent(ID, static_cast<EventFlags>(6)) {}

    FPVector3 orientation{};

    // 0x7c8bae4
    std::int32_t GetHashCode() const override;
};

class EventOnLedgeGrabStart final : public EventBase {
  public:
    static constexpr int ID = 69;

    // 0x7c8bb18
    EventOnLedgeGrabStart() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef character{};
    FP cameraDistanceOffset{};
    FP cameraVerticalAngle{};

    // 0x7c8bba0
    std::int32_t GetHashCode() const;
};

class EventOnLedgeGrabEnd final : public EventBase {
  public:
    static constexpr int ID = 70;

    // 0x7c8bbf8
    EventOnLedgeGrabEnd() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef character{};

    // 0x7c8bc80
    std::int32_t GetHashCode() const;
};

class EventOnLevelButtonPressed final : public EventBase {
  public:
    static constexpr int ID = 71;

    // 0x7c8bc98
    EventOnLevelButtonPressed() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8bca8
    std::int32_t GetHashCode() const;
};

class EventOnLevelButtonReleased final : public EventBase {
  public:
    static constexpr int ID = 72;

    // 0x7c8bcc0
    EventOnLevelButtonReleased() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8bcd0
    std::int32_t GetHashCode() const;
};

class EventOnArenaCannonActivated final : public EventBase {
  public:
    static constexpr int ID = 73;

    // 0x7c8bce8
    EventOnArenaCannonActivated() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8bd70
    std::int32_t GetHashCode() const;
};

class EventMeleeAttack_Start final : public EventBase {
  public:
    static constexpr int ID = 74;

    // 0x7c8bd88
    EventMeleeAttack_Start() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef attackOwner{};

    // 0x7c8bd98
    std::int32_t GetHashCode() const;
};

class EventMeleeAttack_HitOrMissed final : public EventBase {
  public:
    static constexpr int ID = 75;

    // 0x7c8bdb0
    EventMeleeAttack_HitOrMissed() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef attackOwner{};
    QBoolean hit{};

    // 0x7c8bdc0
    std::int32_t GetHashCode() const;
};

class EventMeleeJumpAttack_OnPerformed final : public EventBase {
  public:
    static constexpr int ID = 76;

    // 0x7c8bde4
    EventMeleeJumpAttack_OnPerformed() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8bdf4
    std::int32_t GetHashCode() const;
};

class EventMobSlamAttackEvent final : public EventBase {
  public:
    static constexpr int ID = 77;

    // 0x7c8be0c
    EventMobSlamAttackEvent() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    MobSlamAttackState attackState{};

    // 0x7c8be1c
    std::int32_t GetHashCode() const;
};

class EventMobsEvent final : public EventBase {
  public:
    static constexpr int ID = 78;

    // 0x7c8be50
    EventMobsEvent() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    MobsEventType eventType{};

    // 0x7c8bed8
    std::int32_t GetHashCode() const;
};

class EventMoveTo_OnMoveStarted final : public EventBase {
  public:
    static constexpr int ID = 79;

    // 0x7c8bf0c
    EventMoveTo_OnMoveStarted() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8bf1c
    std::int32_t GetHashCode() const;
};

class EventMoveTo_OnMoveFinished final : public EventBase {
  public:
    static constexpr int ID = 80;

    // 0x7c8bf34
    EventMoveTo_OnMoveFinished() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8bfbc
    std::int32_t GetHashCode() const;
};

class EventOnMovingWallStart final : public EventBase {
  public:
    static constexpr int ID = 81;

    // 0x7c8bfd4
    EventOnMovingWallStart() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8bfe4
    std::int32_t GetHashCode() const;
};

class EventPowerUp_OnStarted final : public EventBase {
  public:
    static constexpr int ID = 82;

    // 0x7c8bffc
    EventPowerUp_OnStarted() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef PowerUpUserEntity{};
    ::Quantum::PowerUpSpec* PowerUpSpec{};

    // TODO(0x7c8c084, Quantum.EventPowerUp_OnStarted.GetHashCode): hashes an asset object by
    // reference identity, which has no deterministic equivalent here.
    std::int32_t GetHashCode() const;
};

class EventPowerUp_OnFinished final : public EventBase {
  public:
    static constexpr int ID = 83;

    // 0x7c8c0c8
    EventPowerUp_OnFinished() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef PowerUpUserEntity{};
    ::Quantum::PowerUpSpec* PowerUpSpec{};
    std::int32_t stackIndex{};

    // TODO(0x7c8c0d8, Quantum.EventPowerUp_OnFinished.GetHashCode): hashes an asset object by
    // reference identity, which has no deterministic equivalent here.
    std::int32_t GetHashCode() const;
};

class EventProjectile_OnHit final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 84;

    // 0x7c8c13c
    EventProjectile_OnHit() : EventBaseEntityEvent(ID, static_cast<EventFlags>(6)) {}

    Physics3D::Hit3D hit{};

    // 0x7c8c14c
    std::int32_t GetHashCode() const override;
};

class EventOnPushed final : public EventBase {
  public:
    static constexpr int ID = 85;

    // 0x7c8c180
    EventOnPushed() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    PushData pushData{};
    FPVector3 force{};
    FP pushModifierStatusEffect{};

    // 0x7c8c208
    std::int32_t GetHashCode() const;
};

class EventOnPushContinued final : public EventBase {
  public:
    static constexpr int ID = 86;

    // 0x7c8c284
    EventOnPushContinued() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    EntityRef pusher{};
    FPVector3 force{};

    // 0x7c8c294
    std::int32_t GetHashCode() const;
};

class EventOnStopPushBegin final : public EventBase {
  public:
    static constexpr int ID = 87;

    // 0x7c8c2e8
    EventOnStopPushBegin() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    EntityRef pusher{};
    FPVector3 force{};

    // 0x7c8c370
    std::int32_t GetHashCode() const;
};

class EventOnPushResistanceChanged final : public EventBase {
  public:
    static constexpr int ID = 88;

    // 0x7c8c3c4
    EventOnPushResistanceChanged() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    FP newResistance{};

    // 0x7c8c3d4
    std::int32_t GetHashCode() const;
};

class EventPushStoppedEnemy final : public EventBase {
  public:
    static constexpr int ID = 89;

    // 0x7c8c408
    EventPushStoppedEnemy() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponUser{};
    FPVector3 UserVelocity{};
    FP PushStopDuration{};

    // 0x7c8c490
    std::int32_t GetHashCode() const;
};

class EventReward_OnPending final : public EventBase {
  public:
    static constexpr int ID = 90;

    // 0x7c8c4e8
    EventReward_OnPending() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    Reward reward{};

    // 0x7c8c570
    std::int32_t GetHashCode() const;
};

class EventReward_OnCollected final : public EventBase {
  public:
    static constexpr int ID = 91;

    // 0x7c8c5a4
    EventReward_OnCollected() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    DroppableType type{};
    std::uint16_t amount{};

    // 0x7c8c5b4
    std::int32_t GetHashCode() const;
};

class EventBounceEvent final : public EventBaseEntityEvent {
  public:
    static constexpr int ID = 92;

    // 0x7c8c60c
    EventBounceEvent() : EventBaseEntityEvent(ID, static_cast<EventFlags>(6)) {}

    FPVector3 position{};
    FPVector3 normal{};
    std::uint32_t bounceCount{};

    // 0x7c8c61c
    std::int32_t GetHashCode() const override;
};

class EventShieldHit final : public EventBase {
  public:
    static constexpr int ID = 93;

    // 0x7c8c698
    EventShieldHit() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    HitData hitData{};
    FP amount{};
    FP remainingShieldPoints{};

    // 0x7c8c720
    std::int32_t GetHashCode() const;
};

class EventShieldFull final : public EventBase {
  public:
    static constexpr int ID = 94;

    // 0x7c8c79c
    EventShieldFull() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8c824
    std::int32_t GetHashCode() const;
};

class EventShieldBreaks final : public EventBase {
  public:
    static constexpr int ID = 96;

    // 0x7c8c83c
    EventShieldBreaks() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8c8c4
    std::int32_t GetHashCode() const;
};

class EventShootingEnemyEvent final : public EventBase {
  public:
    static constexpr int ID = 97;

    // 0x7c8c8dc
    EventShootingEnemyEvent() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    ShootingEnemyEventType eventType{};

    // 0x7c8c8ec
    std::int32_t GetHashCode() const;
};

class EventShrinkingZone_MoveStart final : public EventBase {
  public:
    static constexpr int ID = 98;

    // 0x7c8c920
    EventShrinkingZone_MoveStart() : EventBase(ID, static_cast<EventFlags>(6)) {}

    std::uint16_t zoneIndex{};

    // 0x7c8c930
    std::int32_t GetHashCode() const;
};

class EventSpinningEnemyEvent final : public EventBase {
  public:
    static constexpr int ID = 99;

    // 0x7c8c950
    EventSpinningEnemyEvent() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    SpinningEnemyEventType eventType{};

    // 0x7c8c9d8
    std::int32_t GetHashCode() const;
};

class EventTutorialGameMode_StepChanged final : public EventBase {
  public:
    static constexpr int ID = 100;

    // 0x7c8ca0c
    EventTutorialGameMode_StepChanged() : EventBase(ID, static_cast<EventFlags>(6)) {}

    TutorialGameModeStep newStep{};

    // 0x7c8ca1c
    std::int32_t GetHashCode() const;
};

class EventUltimateAbilityUser_OnStateChanged final : public EventBase {
  public:
    static constexpr int ID = 101;

    // 0x7c8ca3c
    EventUltimateAbilityUser_OnStateChanged() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    UltimateAbilityState state{};

    // 0x7c8cac4
    std::int32_t GetHashCode() const;
};

class EventUltimateAbilityUser_OnRecast final : public EventBase {
  public:
    static constexpr int ID = 102;

    // 0x7c8caf8
    EventUltimateAbilityUser_OnRecast() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8cb08
    std::int32_t GetHashCode() const;
};

class EventUltimateEvolution_OnEvolve final : public EventBase {
  public:
    static constexpr int ID = 103;

    // 0x7c8cb20
    EventUltimateEvolution_OnEvolve() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    std::int32_t level{};

    // 0x7c8cba8
    std::int32_t GetHashCode() const;
};

class EventUltimateTank_OnLedgeGrabStart final : public EventBase {
  public:
    static constexpr int ID = 104;

    // 0x7c8cbdc
    EventUltimateTank_OnLedgeGrabStart() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef tankEntity{};

    // 0x7c8cbec
    std::int32_t GetHashCode() const;
};

class EventOnFeatureUnlocked final : public EventBase {
  public:
    static constexpr int ID = 105;

    // 0x7c8cc04
    EventOnFeatureUnlocked() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    UnlockedFeature unlockedFeature{};

    // 0x7c8cc14
    std::int32_t GetHashCode() const;
};

class EventOnFeatureLocked final : public EventBase {
  public:
    static constexpr int ID = 106;

    // 0x7c8cc48
    EventOnFeatureLocked() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    UnlockedFeature lockedFeature{};

    // 0x7c8cc58
    std::int32_t GetHashCode() const;
};

class EventVoidRepulserShieldStateChange final : public EventBase {
  public:
    static constexpr int ID = 107;

    // 0x7c8cc8c
    EventVoidRepulserShieldStateChange() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    VoidRepulserState state{};

    // 0x7c8cd14
    std::int32_t GetHashCode() const;
};

class EventOnWallRunningStart final : public EventBase {
  public:
    static constexpr int ID = 108;

    // 0x7c8cd48
    EventOnWallRunningStart() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};
    QBoolean left{};

    // 0x7c8cd58
    std::int32_t GetHashCode() const;
};

class EventOnWallRunningStop final : public EventBase {
  public:
    static constexpr int ID = 109;

    // 0x7c8cd7c
    EventOnWallRunningStop() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef entity{};

    // 0x7c8cd8c
    std::int32_t GetHashCode() const;
};

class EventWaveManager_OnWaveStarted final : public EventBase {
  public:
    static constexpr int ID = 110;

    // 0x7c8cda4
    EventWaveManager_OnWaveStarted() : EventBase(ID, static_cast<EventFlags>(7)) {}

    std::int32_t waveIndex{};
    std::int32_t wavesCount{};

    // 0x7c8ce2c
    std::int32_t GetHashCode() const;
};

class EventWaveManager_OnWaveStep final : public EventBase {
  public:
    static constexpr int ID = 111;

    // 0x7c8ce70
    EventWaveManager_OnWaveStep() : EventBase(ID, static_cast<EventFlags>(7)) {}

    std::int32_t waveIndex{};
    std::int32_t wavesCount{};
    std::int32_t stepIndex{};

    // 0x7c8cef8
    std::int32_t GetHashCode() const;
};

class EventWaveManager_OnWaveComplete final : public EventBase {
  public:
    static constexpr int ID = 112;

    // 0x7c8cf58
    EventWaveManager_OnWaveComplete() : EventBase(ID, static_cast<EventFlags>(7)) {}

    std::int32_t waveIndex{};
    std::int32_t wavesCount{};
    WaveCompletionReason reason{};

    // 0x7c8cf68
    std::int32_t GetHashCode() const;
};

class EventWaveSpawner_OnSpawned final : public EventBase {
  public:
    static constexpr int ID = 114;

    // 0x7c8cfc8
    EventWaveSpawner_OnSpawned() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef spawner{};
    QBoolean isBoss{};

    // 0x7c8cfd8
    std::int32_t GetHashCode() const;
};

class EventWeaponBehaviour_Charging_OnStartCharging final : public EventBase {
  public:
    static constexpr int ID = 115;

    // 0x7c8cffc
    EventWeaponBehaviour_Charging_OnStartCharging() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef weaponEntity{};
    EntityRef weaponUser{};

    // 0x7c8d00c
    std::int32_t GetHashCode() const;
};

class EventWeaponUser_Activation final : public EventBase {
  public:
    static constexpr int ID = 116;

    // 0x7c8d034
    EventWeaponUser_Activation() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponUser{};
    QBoolean Disabled{};

    // 0x7c8d0bc
    std::int32_t GetHashCode() const;
};

class EventWeaponUser_Equipped final : public EventBase {
  public:
    static constexpr int ID = 117;

    // 0x7c8d0e0
    EventWeaponUser_Equipped() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponUser{};
    EntityRef Weapon{};

    // 0x7c8d168
    std::int32_t GetHashCode() const;
};

class EventWeaponUser_PrepareUse final : public EventBase {
  public:
    static constexpr int ID = 118;

    // 0x7c8d190
    EventWeaponUser_PrepareUse() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponUser{};
    EntityRef Weapon{};

    // 0x7c8d218
    std::int32_t GetHashCode() const;
};

class EventWeaponUser_Use final : public EventBase {
  public:
    static constexpr int ID = 119;

    // 0x7c8d240
    EventWeaponUser_Use() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponUser{};
    EntityRef Weapon{};

    // 0x7c8d2c8
    std::int32_t GetHashCode() const;
};

class EventWeaponUser_StopUse final : public EventBase {
  public:
    static constexpr int ID = 120;

    // 0x7c8d2f0
    EventWeaponUser_StopUse() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponUser{};

    // 0x7c8d378
    std::int32_t GetHashCode() const;
};

class EventWeaponUser_StartReload final : public EventBase {
  public:
    static constexpr int ID = 121;

    // 0x7c8d390
    EventWeaponUser_StartReload() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponUser{};

    // 0x7c8d418
    std::int32_t GetHashCode() const;
};

class EventWeaponUser_EndReload final : public EventBase {
  public:
    static constexpr int ID = 122;

    // 0x7c8d430
    EventWeaponUser_EndReload() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponUser{};

    // 0x7c8d440
    std::int32_t GetHashCode() const;
};

class EventOnArmWrenchThrow final : public EventBase {
  public:
    static constexpr int ID = 123;

    // 0x7c8d458
    EventOnArmWrenchThrow() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponEntity{};

    // 0x7c8d468
    std::int32_t GetHashCode() const;
};

class EventOnStartWrenchThrow final : public EventBase {
  public:
    static constexpr int ID = 124;

    // 0x7c8d480
    EventOnStartWrenchThrow() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponEntity{};

    // 0x7c8d490
    std::int32_t GetHashCode() const;
};

class EventOnEndWrenchThrow final : public EventBase {
  public:
    static constexpr int ID = 125;

    // 0x7c8d4a8
    EventOnEndWrenchThrow() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef WeaponEntity{};

    // 0x7c8d4b8
    std::int32_t GetHashCode() const;
};

class EventWrenchWeapon_UseWrench final : public EventBase {
  public:
    static constexpr int ID = 126;

    // 0x7c8d4d0
    EventWrenchWeapon_UseWrench() : EventBase(ID, static_cast<EventFlags>(6)) {}

    EntityRef weapon{};
    std::uint16_t comboCounter{};

    // 0x7c8d558
    std::int32_t GetHashCode() const;
};

class EventZoneControl_OnZoneStateChanged final : public EventBase {
  public:
    static constexpr int ID = 127;

    // 0x7c8d58c
    EventZoneControl_OnZoneStateChanged() : EventBase(ID, static_cast<EventFlags>(6)) {}

    CONTROL_ZONE_STATE prevState{};
    CONTROL_ZONE_STATE newState{};
    ControlZoneStateHolder stateHolder{};

    // 0x7c8d614
    std::int32_t GetHashCode() const;
};

} // namespace Quantum
