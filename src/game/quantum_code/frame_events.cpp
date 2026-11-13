// frame_events.cpp - Quantum.Frame.FrameEvents.
#include "quantum_code/frame_events.h"

#include "quantum_code/frame.h"

namespace Quantum {

EventAISpawner_OnSpawned* FrameEvents::AISpawner_OnSpawned(EntityRef spawnerEntity, EntityRef spawnedEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAISpawner_OnSpawned>(EventAISpawner_OnSpawned::ID);
    ev->spawnerEntity = spawnerEntity;
    ev->spawnedEntity = spawnedEntity;
    _f->AddEvent(ev);
    return ev;
}

EventAISpawner_OnDespawned* FrameEvents::AISpawner_OnDespawned(EntityRef spawnerEntity, EntityRef despawnedEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAISpawner_OnDespawned>(EventAISpawner_OnDespawned::ID);
    ev->spawnerEntity = spawnerEntity;
    ev->despawnedEntity = despawnedEntity;
    _f->AddEvent(ev);
    return ev;
}

EventActorIcedChange* FrameEvents::ActorIcedChange(EntityRef entity, QBoolean iced) {
    auto* ev = _f->get_Context()->AcquireEvent<EventActorIcedChange>(EventActorIcedChange::ID);
    ev->entity = entity;
    ev->iced = iced;
    _f->AddEvent(ev);
    return ev;
}

EventRespawnTimerCreated* FrameEvents::RespawnTimerCreated(EntityRef entity, FP respawnPointCalculationDelay) {
    auto* ev = _f->get_Context()->AcquireEvent<EventRespawnTimerCreated>(EventRespawnTimerCreated::ID);
    ev->entity = entity;
    ev->respawnPointCalculationDelay = respawnPointCalculationDelay;
    _f->AddEvent(ev);
    return ev;
}

EventRespawnPointSet* FrameEvents::RespawnPointSet(EntityRef entity, const FPVector3& spawnPosition, const FPQuaternion& spawnRotation) {
    auto* ev = _f->get_Context()->AcquireEvent<EventRespawnPointSet>(EventRespawnPointSet::ID);
    ev->entity = entity;
    ev->spawnPosition = spawnPosition;
    ev->spawnRotation = spawnRotation;
    _f->AddEvent(ev);
    return ev;
}

EventRespawnTimerDestroyed* FrameEvents::RespawnTimerDestroyed(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventRespawnTimerDestroyed>(EventRespawnTimerDestroyed::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventActorSheepedChange* FrameEvents::ActorSheepedChange(EntityRef entity, QBoolean sheeped) {
    auto* ev = _f->get_Context()->AcquireEvent<EventActorSheepedChange>(EventActorSheepedChange::ID);
    ev->entity = entity;
    ev->sheeped = sheeped;
    _f->AddEvent(ev);
    return ev;
}

EventActorStunnedChange* FrameEvents::ActorStunnedChange(EntityRef entity, QBoolean stunned) {
    auto* ev = _f->get_Context()->AcquireEvent<EventActorStunnedChange>(EventActorStunnedChange::ID);
    ev->entity = entity;
    ev->stunned = stunned;
    _f->AddEvent(ev);
    return ev;
}

EventAgentOfDoomEvent* FrameEvents::AgentOfDoomEvent(EntityRef entity, AgentOfDoomEventType eventType) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAgentOfDoomEvent>(EventAgentOfDoomEvent::ID);
    ev->entity = entity;
    ev->eventType = eventType;
    _f->AddEvent(ev);
    return ev;
}

EventAreaOfEffect_OnHit* FrameEvents::AreaOfEffect_OnHit(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAreaOfEffect_OnHit>(EventAreaOfEffect_OnHit::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventAreaOfEffect_OnMiss* FrameEvents::AreaOfEffect_OnMiss(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAreaOfEffect_OnMiss>(EventAreaOfEffect_OnMiss::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventAreaOfEffect_OnHitInstigator* FrameEvents::AreaOfEffect_OnHitInstigator(EntityRef instigator) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAreaOfEffect_OnHitInstigator>(EventAreaOfEffect_OnHitInstigator::ID);
    ev->instigator = instigator;
    _f->AddEvent(ev);
    return ev;
}

EventAreaOfEffect_OnEnterArea* FrameEvents::AreaOfEffect_OnEnterArea(EntityRef inAreaEntity, EntityRef aoeEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAreaOfEffect_OnEnterArea>(EventAreaOfEffect_OnEnterArea::ID);
    ev->inAreaEntity = inAreaEntity;
    ev->aoeEntity = aoeEntity;
    _f->AddEvent(ev);
    return ev;
}

EventAreaOfEffect_OnExitArea* FrameEvents::AreaOfEffect_OnExitArea(EntityRef outOfAreaEntity, EntityRef aoeEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAreaOfEffect_OnExitArea>(EventAreaOfEffect_OnExitArea::ID);
    ev->outOfAreaEntity = outOfAreaEntity;
    ev->aoeEntity = aoeEntity;
    _f->AddEvent(ev);
    return ev;
}

EventAutoJumperTriggered* FrameEvents::AutoJumperTriggered(EntityRef jumpingEntity, std::int32_t staticColliderIndex) {
    auto* ev = _f->get_Context()->AcquireEvent<EventAutoJumperTriggered>(EventAutoJumperTriggered::ID);
    ev->jumpingEntity = jumpingEntity;
    ev->staticColliderIndex = staticColliderIndex;
    _f->AddEvent(ev);
    return ev;
}

EventBigBoomMine_Triggered* FrameEvents::BigBoomMine_Triggered(EntityRef Owner) {
    auto* ev = _f->get_Context()->AcquireEvent<EventBigBoomMine_Triggered>(EventBigBoomMine_Triggered::ID);
    ev->Owner = Owner;
    _f->AddEvent(ev);
    return ev;
}

EventBlastBallGame_StateChanged* FrameEvents::BlastBallGame_StateChanged(BlastBallGameState newState) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventBlastBallGame_StateChanged>(EventBlastBallGame_StateChanged::ID);
    ev->newState = newState;
    _f->AddEvent(ev);
    return ev;
}

EventBlastBallGame_StartOverTime* FrameEvents::BlastBallGame_StartOverTime() {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventBlastBallGame_StartOverTime>(EventBlastBallGame_StartOverTime::ID);
    _f->AddEvent(ev);
    return ev;
}

EventBlastBallGame_TeamScored* FrameEvents::BlastBallGame_TeamScored(TeamFlag scoringTeam, std::uint16_t score, EntityRef scoringPlayer) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventBlastBallGame_TeamScored>(EventBlastBallGame_TeamScored::ID);
    ev->scoringTeam = scoringTeam;
    ev->score = score;
    ev->scoringPlayer = scoringPlayer;
    _f->AddEvent(ev);
    return ev;
}

EventBlastBallGame_BallHit* FrameEvents::BlastBallGame_BallHit(EntityRef player, const FPVector3& position, const FPVector3& direction) {
    auto* ev = _f->get_Context()->AcquireEvent<EventBlastBallGame_BallHit>(EventBlastBallGame_BallHit::ID);
    ev->player = player;
    ev->position = position;
    ev->direction = direction;
    _f->AddEvent(ev);
    return ev;
}

EventOnBoltCollected* FrameEvents::OnBoltCollected(EntityRef entity, std::int32_t amount) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnBoltCollected>(EventOnBoltCollected::ID);
    ev->entity = entity;
    ev->amount = amount;
    _f->AddEvent(ev);
    return ev;
}

EventOnBoltLost* FrameEvents::OnBoltLost(EntityRef entity, std::int32_t amount) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnBoltLost>(EventOnBoltLost::ID);
    ev->entity = entity;
    ev->amount = amount;
    _f->AddEvent(ev);
    return ev;
}

EventBoltCounter_OnUpdateBolts* FrameEvents::BoltCounter_OnUpdateBolts(EntityRef counterEntity, std::int32_t currentBolts) {
    auto* ev = _f->get_Context()->AcquireEvent<EventBoltCounter_OnUpdateBolts>(EventBoltCounter_OnUpdateBolts::ID);
    ev->counterEntity = counterEntity;
    ev->currentBolts = currentBolts;
    _f->AddEvent(ev);
    return ev;
}

EventBumperTriggered* FrameEvents::BumperTriggered(EntityRef bumper, EntityRef player) {
    auto* ev = _f->get_Context()->AcquireEvent<EventBumperTriggered>(EventBumperTriggered::ID);
    ev->bumper = bumper;
    ev->player = player;
    _f->AddEvent(ev);
    return ev;
}

EventCharacterColliderScaler_OnScaleChanged* FrameEvents::CharacterColliderScaler_OnScaleChanged(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventCharacterColliderScaler_OnScaleChanged>(EventCharacterColliderScaler_OnScaleChanged::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventColliderResize_OnResizeStarted* FrameEvents::ColliderResize_OnResizeStarted(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventColliderResize_OnResizeStarted>(EventColliderResize_OnResizeStarted::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventCollisionForce_OnCollision* FrameEvents::CollisionForce_OnCollision(EntityRef entity, EntityRef otherEntity, const FPVector3& hitPoint, const FPVector3& hitNormal) {
    auto* ev = _f->get_Context()->AcquireEvent<EventCollisionForce_OnCollision>(EventCollisionForce_OnCollision::ID);
    ev->entity = entity;
    ev->otherEntity = otherEntity;
    ev->hitPoint = hitPoint;
    ev->hitNormal = hitNormal;
    _f->AddEvent(ev);
    return ev;
}

EventDamageable_HitReceived* FrameEvents::Damageable_HitReceived(EntityRef entity, const HitData& hitData, const HitResult& hitResult) {
    auto* ev = _f->get_Context()->AcquireEvent<EventDamageable_HitReceived>(EventDamageable_HitReceived::ID);
    ev->entity = entity;
    ev->hitData = hitData;
    ev->hitResult = hitResult;
    _f->AddEvent(ev);
    return ev;
}

EventOnDamageChargeableReset* FrameEvents::OnDamageChargeableReset(EntityRef chargeableEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnDamageChargeableReset>(EventOnDamageChargeableReset::ID);
    ev->chargeableEntity = chargeableEntity;
    _f->AddEvent(ev);
    return ev;
}

EventDashStarted* FrameEvents::DashStarted(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventDashStarted>(EventDashStarted::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventDashComponentDisabled* FrameEvents::DashComponentDisabled(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventDashComponentDisabled>(EventDashComponentDisabled::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventDrillDash_OnStateChanged* FrameEvents::DrillDash_OnStateChanged(EntityRef entity, EntityRef weaponEntity, DrillDashState newState) {
    auto* ev = _f->get_Context()->AcquireEvent<EventDrillDash_OnStateChanged>(EventDrillDash_OnStateChanged::ID);
    ev->entity = entity;
    ev->weaponEntity = weaponEntity;
    ev->newState = newState;
    _f->AddEvent(ev);
    return ev;
}

EventOnDroppableDropped* FrameEvents::OnDroppableDropped(EntityRef dropperEntity, DroppableType type, const FPVector3& position, std::uint16_t amount) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventOnDroppableDropped>(EventOnDroppableDropped::ID);
    ev->dropperEntity = dropperEntity;
    ev->type = type;
    ev->position = position;
    ev->amount = amount;
    _f->AddEvent(ev);
    return ev;
}

EventEliminationGameMode_OnCloseToWin* FrameEvents::EliminationGameMode_OnCloseToWin(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventEliminationGameMode_OnCloseToWin>(EventEliminationGameMode_OnCloseToWin::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventEnergyCell_OnCollected* FrameEvents::EnergyCell_OnCollected(EntityRef collector, std::int32_t value) {
    auto* ev = _f->get_Context()->AcquireEvent<EventEnergyCell_OnCollected>(EventEnergyCell_OnCollected::ID);
    ev->collector = collector;
    ev->value = value;
    _f->AddEvent(ev);
    return ev;
}

EventFireReactor_OnStateChanged* FrameEvents::FireReactor_OnStateChanged(EntityRef entity, QBoolean enabled) {
    auto* ev = _f->get_Context()->AcquireEvent<EventFireReactor_OnStateChanged>(EventFireReactor_OnStateChanged::ID);
    ev->entity = entity;
    ev->enabled = enabled;
    _f->AddEvent(ev);
    return ev;
}

EventGadgetUser_OnGadgetActivated* FrameEvents::GadgetUser_OnGadgetActivated(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventGadgetUser_OnGadgetActivated>(EventGadgetUser_OnGadgetActivated::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventGadgetUser_OnGadgetDeactivated* FrameEvents::GadgetUser_OnGadgetDeactivated(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventGadgetUser_OnGadgetDeactivated>(EventGadgetUser_OnGadgetDeactivated::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventIntroEnded* FrameEvents::IntroEnded(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventIntroEnded>(EventIntroEnded::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventBoltMultiplierOn* FrameEvents::BoltMultiplierOn() {
    auto* ev = _f->get_Context()->AcquireEvent<EventBoltMultiplierOn>(EventBoltMultiplierOn::ID);
    _f->AddEvent(ev);
    return ev;
}

EventGameModeHandlerStateChanged* FrameEvents::GameModeHandlerStateChanged(EntityRef entity, GAME_MODE_HANDLER_STATE prevState, GAME_MODE_HANDLER_STATE newState) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventGameModeHandlerStateChanged>(EventGameModeHandlerStateChanged::ID);
    ev->entity = entity;
    ev->prevState = prevState;
    ev->newState = newState;
    _f->AddEvent(ev);
    return ev;
}

EventGameMode_OnPlayerEliminated* FrameEvents::GameMode_OnPlayerEliminated(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventGameMode_OnPlayerEliminated>(EventGameMode_OnPlayerEliminated::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventGameMode_OnPlayerFinished* FrameEvents::GameMode_OnPlayerFinished(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventGameMode_OnPlayerFinished>(EventGameMode_OnPlayerFinished::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventPreloadAddressablePrefabs* FrameEvents::PreloadAddressablePrefabs() {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventPreloadAddressablePrefabs>(EventPreloadAddressablePrefabs::ID);
    _f->AddEvent(ev);
    return ev;
}

EventGoldenBoltSpawned* FrameEvents::GoldenBoltSpawned(QBoolean isUnique) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventGoldenBoltSpawned>(EventGoldenBoltSpawned::ID);
    ev->isUnique = isUnique;
    _f->AddEvent(ev);
    return ev;
}

EventGoldenBoltCollected* FrameEvents::GoldenBoltCollected(EntityRef boltEntity, EntityRef collector) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventGoldenBoltCollected>(EventGoldenBoltCollected::ID);
    ev->boltEntity = boltEntity;
    ev->collector = collector;
    _f->AddEvent(ev);
    return ev;
}

EventStartGrind* FrameEvents::StartGrind(EntityRef entity, QBoolean jumpAnim) {
    auto* ev = _f->get_Context()->AcquireEvent<EventStartGrind>(EventStartGrind::ID);
    ev->entity = entity;
    ev->jumpAnim = jumpAnim;
    _f->AddEvent(ev);
    return ev;
}

EventSnappedToRail* FrameEvents::SnappedToRail(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventSnappedToRail>(EventSnappedToRail::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventExitGrind* FrameEvents::ExitGrind(EntityRef entity, QBoolean jumpAnim) {
    auto* ev = _f->get_Context()->AcquireEvent<EventExitGrind>(EventExitGrind::ID);
    ev->entity = entity;
    ev->jumpAnim = jumpAnim;
    _f->AddEvent(ev);
    return ev;
}

EventSwitchRail* FrameEvents::SwitchRail(EntityRef entity, QBoolean isLeft) {
    auto* ev = _f->get_Context()->AcquireEvent<EventSwitchRail>(EventSwitchRail::ID);
    ev->entity = entity;
    ev->isLeft = isLeft;
    _f->AddEvent(ev);
    return ev;
}

EventGroundDashStarting* FrameEvents::GroundDashStarting(EntityRef userEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventGroundDashStarting>(EventGroundDashStarting::ID);
    ev->userEntity = userEntity;
    _f->AddEvent(ev);
    return ev;
}

EventGroundDashStarted* FrameEvents::GroundDashStarted(EntityRef userEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventGroundDashStarted>(EventGroundDashStarted::ID);
    ev->userEntity = userEntity;
    _f->AddEvent(ev);
    return ev;
}

EventGroundDashEnded* FrameEvents::GroundDashEnded(EntityRef userEntity, GroundDashEndReason endReason) {
    auto* ev = _f->get_Context()->AcquireEvent<EventGroundDashEnded>(EventGroundDashEnded::ID);
    ev->userEntity = userEntity;
    ev->endReason = endReason;
    _f->AddEvent(ev);
    return ev;
}

EventHealSource_OnHeal* FrameEvents::HealSource_OnHeal(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventHealSource_OnHeal>(EventHealSource_OnHeal::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventOnHealthDecreased* FrameEvents::OnHealthDecreased(EntityRef entity, FP amount, const HitData& hitData) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnHealthDecreased>(EventOnHealthDecreased::ID);
    ev->entity = entity;
    ev->amount = amount;
    ev->hitData = hitData;
    _f->AddEvent(ev);
    return ev;
}

EventHelipackStart* FrameEvents::HelipackStart(EntityRef entity, FP verticalVelocity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventHelipackStart>(EventHelipackStart::ID);
    ev->entity = entity;
    ev->verticalVelocity = verticalVelocity;
    _f->AddEvent(ev);
    return ev;
}

EventHelipackStop* FrameEvents::HelipackStop(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventHelipackStop>(EventHelipackStop::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventItemSpawnerStatus* FrameEvents::ItemSpawnerStatus(EntityRef entity, ItemSpawnerStatusType status, QBoolean enabled) {
    auto* ev = _f->get_Context()->AcquireEvent<EventItemSpawnerStatus>(EventItemSpawnerStatus::ID);
    ev->entity = entity;
    ev->status = status;
    ev->enabled = enabled;
    _f->AddEvent(ev);
    return ev;
}

EventItemSpawnerEntitySpawn* FrameEvents::ItemSpawnerEntitySpawn(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventItemSpawnerEntitySpawn>(EventItemSpawnerEntitySpawn::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventKccExtension_OnJumpStateChanged* FrameEvents::KccExtension_OnJumpStateChanged(EntityRef entity, JumpState jumpState) {
    auto* ev = _f->get_Context()->AcquireEvent<EventKccExtension_OnJumpStateChanged>(EventKccExtension_OnJumpStateChanged::ID);
    ev->entity = entity;
    ev->jumpState = jumpState;
    _f->AddEvent(ev);
    return ev;
}

EventKccExtensions_OnMoveSpeedModifierAdded* FrameEvents::KccExtensions_OnMoveSpeedModifierAdded(EntityRef entity, FP value, FP duration) {
    auto* ev = _f->get_Context()->AcquireEvent<EventKccExtensions_OnMoveSpeedModifierAdded>(EventKccExtensions_OnMoveSpeedModifierAdded::ID);
    ev->entity = entity;
    ev->value = value;
    ev->duration = duration;
    _f->AddEvent(ev);
    return ev;
}

EventKillEvent* FrameEvents::KillEvent(EntityRef entity, KillType killType, KillZoneSurfaceType killZoneSurfaceType) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventKillEvent>(EventKillEvent::ID);
    ev->entity = entity;
    ev->killType = killType;
    ev->killZoneSurfaceType = killZoneSurfaceType;
    _f->AddEvent(ev);
    return ev;
}

EventKillableSpawnEvent* FrameEvents::KillableSpawnEvent(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventKillableSpawnEvent>(EventKillableSpawnEvent::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventReviveEvent* FrameEvents::ReviveEvent(EntityRef entity, const FPVector3& orientation) {
    auto* ev = _f->get_Context()->AcquireEvent<EventReviveEvent>(EventReviveEvent::ID);
    ev->entity = entity;
    ev->orientation = orientation;
    _f->AddEvent(ev);
    return ev;
}

EventOnLedgeGrabStart* FrameEvents::OnLedgeGrabStart(EntityRef character, FP cameraDistanceOffset, FP cameraVerticalAngle) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnLedgeGrabStart>(EventOnLedgeGrabStart::ID);
    ev->character = character;
    ev->cameraDistanceOffset = cameraDistanceOffset;
    ev->cameraVerticalAngle = cameraVerticalAngle;
    _f->AddEvent(ev);
    return ev;
}

EventOnLedgeGrabEnd* FrameEvents::OnLedgeGrabEnd(EntityRef character) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnLedgeGrabEnd>(EventOnLedgeGrabEnd::ID);
    ev->character = character;
    _f->AddEvent(ev);
    return ev;
}

EventOnLevelButtonPressed* FrameEvents::OnLevelButtonPressed(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnLevelButtonPressed>(EventOnLevelButtonPressed::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventOnLevelButtonReleased* FrameEvents::OnLevelButtonReleased(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnLevelButtonReleased>(EventOnLevelButtonReleased::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventOnArenaCannonActivated* FrameEvents::OnArenaCannonActivated(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnArenaCannonActivated>(EventOnArenaCannonActivated::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventMeleeAttack_Start* FrameEvents::MeleeAttack_Start(EntityRef attackOwner) {
    auto* ev = _f->get_Context()->AcquireEvent<EventMeleeAttack_Start>(EventMeleeAttack_Start::ID);
    ev->attackOwner = attackOwner;
    _f->AddEvent(ev);
    return ev;
}

EventMeleeAttack_HitOrMissed* FrameEvents::MeleeAttack_HitOrMissed(EntityRef attackOwner, QBoolean hit) {
    auto* ev = _f->get_Context()->AcquireEvent<EventMeleeAttack_HitOrMissed>(EventMeleeAttack_HitOrMissed::ID);
    ev->attackOwner = attackOwner;
    ev->hit = hit;
    _f->AddEvent(ev);
    return ev;
}

EventMeleeJumpAttack_OnPerformed* FrameEvents::MeleeJumpAttack_OnPerformed(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventMeleeJumpAttack_OnPerformed>(EventMeleeJumpAttack_OnPerformed::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventMobSlamAttackEvent* FrameEvents::MobSlamAttackEvent(EntityRef entity, MobSlamAttackState attackState) {
    auto* ev = _f->get_Context()->AcquireEvent<EventMobSlamAttackEvent>(EventMobSlamAttackEvent::ID);
    ev->entity = entity;
    ev->attackState = attackState;
    _f->AddEvent(ev);
    return ev;
}

EventMobsEvent* FrameEvents::MobsEvent(EntityRef entity, MobsEventType eventType) {
    auto* ev = _f->get_Context()->AcquireEvent<EventMobsEvent>(EventMobsEvent::ID);
    ev->entity = entity;
    ev->eventType = eventType;
    _f->AddEvent(ev);
    return ev;
}

EventMoveTo_OnMoveStarted* FrameEvents::MoveTo_OnMoveStarted(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventMoveTo_OnMoveStarted>(EventMoveTo_OnMoveStarted::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventMoveTo_OnMoveFinished* FrameEvents::MoveTo_OnMoveFinished(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventMoveTo_OnMoveFinished>(EventMoveTo_OnMoveFinished::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventOnMovingWallStart* FrameEvents::OnMovingWallStart(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnMovingWallStart>(EventOnMovingWallStart::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventPowerUp_OnStarted* FrameEvents::PowerUp_OnStarted(EntityRef PowerUpUserEntity, PowerUpSpec* PowerUpSpec) {
    auto* ev = _f->get_Context()->AcquireEvent<EventPowerUp_OnStarted>(EventPowerUp_OnStarted::ID);
    ev->PowerUpUserEntity = PowerUpUserEntity;
    ev->PowerUpSpec = PowerUpSpec;
    _f->AddEvent(ev);
    return ev;
}

EventPowerUp_OnFinished* FrameEvents::PowerUp_OnFinished(EntityRef PowerUpUserEntity, PowerUpSpec* PowerUpSpec, std::int32_t stackIndex) {
    auto* ev = _f->get_Context()->AcquireEvent<EventPowerUp_OnFinished>(EventPowerUp_OnFinished::ID);
    ev->PowerUpUserEntity = PowerUpUserEntity;
    ev->PowerUpSpec = PowerUpSpec;
    ev->stackIndex = stackIndex;
    _f->AddEvent(ev);
    return ev;
}

EventProjectile_OnHit* FrameEvents::Projectile_OnHit(EntityRef entity, const Physics3D::Hit3D& hit) {
    auto* ev = _f->get_Context()->AcquireEvent<EventProjectile_OnHit>(EventProjectile_OnHit::ID);
    ev->entity = entity;
    ev->hit = hit;
    _f->AddEvent(ev);
    return ev;
}

EventOnPushed* FrameEvents::OnPushed(EntityRef entity, const PushData& pushData, const FPVector3& force, FP pushModifierStatusEffect) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnPushed>(EventOnPushed::ID);
    ev->entity = entity;
    ev->pushData = pushData;
    ev->force = force;
    ev->pushModifierStatusEffect = pushModifierStatusEffect;
    _f->AddEvent(ev);
    return ev;
}

EventOnPushContinued* FrameEvents::OnPushContinued(EntityRef entity, EntityRef pusher, const FPVector3& force) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnPushContinued>(EventOnPushContinued::ID);
    ev->entity = entity;
    ev->pusher = pusher;
    ev->force = force;
    _f->AddEvent(ev);
    return ev;
}

EventOnStopPushBegin* FrameEvents::OnStopPushBegin(EntityRef entity, EntityRef pusher, const FPVector3& force) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnStopPushBegin>(EventOnStopPushBegin::ID);
    ev->entity = entity;
    ev->pusher = pusher;
    ev->force = force;
    _f->AddEvent(ev);
    return ev;
}

EventOnPushResistanceChanged* FrameEvents::OnPushResistanceChanged(EntityRef entity, FP newResistance) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnPushResistanceChanged>(EventOnPushResistanceChanged::ID);
    ev->entity = entity;
    ev->newResistance = newResistance;
    _f->AddEvent(ev);
    return ev;
}

EventPushStoppedEnemy* FrameEvents::PushStoppedEnemy(EntityRef WeaponUser, const FPVector3& UserVelocity, FP PushStopDuration) {
    auto* ev = _f->get_Context()->AcquireEvent<EventPushStoppedEnemy>(EventPushStoppedEnemy::ID);
    ev->WeaponUser = WeaponUser;
    ev->UserVelocity = UserVelocity;
    ev->PushStopDuration = PushStopDuration;
    _f->AddEvent(ev);
    return ev;
}

EventReward_OnPending* FrameEvents::Reward_OnPending(EntityRef entity, const Reward& reward) {
    auto* ev = _f->get_Context()->AcquireEvent<EventReward_OnPending>(EventReward_OnPending::ID);
    ev->entity = entity;
    ev->reward = reward;
    _f->AddEvent(ev);
    return ev;
}

EventReward_OnCollected* FrameEvents::Reward_OnCollected(EntityRef entity, DroppableType type, std::uint16_t amount) {
    auto* ev = _f->get_Context()->AcquireEvent<EventReward_OnCollected>(EventReward_OnCollected::ID);
    ev->entity = entity;
    ev->type = type;
    ev->amount = amount;
    _f->AddEvent(ev);
    return ev;
}

EventBounceEvent* FrameEvents::BounceEvent(EntityRef entity, const FPVector3& position, const FPVector3& normal, std::uint32_t bounceCount) {
    auto* ev = _f->get_Context()->AcquireEvent<EventBounceEvent>(EventBounceEvent::ID);
    ev->entity = entity;
    ev->position = position;
    ev->normal = normal;
    ev->bounceCount = bounceCount;
    _f->AddEvent(ev);
    return ev;
}

EventShieldHit* FrameEvents::ShieldHit(EntityRef entity, const HitData& hitData, FP amount, FP remainingShieldPoints) {
    auto* ev = _f->get_Context()->AcquireEvent<EventShieldHit>(EventShieldHit::ID);
    ev->entity = entity;
    ev->hitData = hitData;
    ev->amount = amount;
    ev->remainingShieldPoints = remainingShieldPoints;
    _f->AddEvent(ev);
    return ev;
}

EventShieldFull* FrameEvents::ShieldFull(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventShieldFull>(EventShieldFull::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventShieldBreaks* FrameEvents::ShieldBreaks(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventShieldBreaks>(EventShieldBreaks::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventShootingEnemyEvent* FrameEvents::ShootingEnemyEvent(EntityRef entity, ShootingEnemyEventType eventType) {
    auto* ev = _f->get_Context()->AcquireEvent<EventShootingEnemyEvent>(EventShootingEnemyEvent::ID);
    ev->entity = entity;
    ev->eventType = eventType;
    _f->AddEvent(ev);
    return ev;
}

EventShrinkingZone_MoveStart* FrameEvents::ShrinkingZone_MoveStart(std::uint16_t zoneIndex) {
    auto* ev = _f->get_Context()->AcquireEvent<EventShrinkingZone_MoveStart>(EventShrinkingZone_MoveStart::ID);
    ev->zoneIndex = zoneIndex;
    _f->AddEvent(ev);
    return ev;
}

EventSpinningEnemyEvent* FrameEvents::SpinningEnemyEvent(EntityRef entity, SpinningEnemyEventType eventType) {
    auto* ev = _f->get_Context()->AcquireEvent<EventSpinningEnemyEvent>(EventSpinningEnemyEvent::ID);
    ev->entity = entity;
    ev->eventType = eventType;
    _f->AddEvent(ev);
    return ev;
}

EventTutorialGameMode_StepChanged* FrameEvents::TutorialGameMode_StepChanged(TutorialGameModeStep newStep) {
    auto* ev = _f->get_Context()->AcquireEvent<EventTutorialGameMode_StepChanged>(EventTutorialGameMode_StepChanged::ID);
    ev->newStep = newStep;
    _f->AddEvent(ev);
    return ev;
}

EventUltimateAbilityUser_OnStateChanged* FrameEvents::UltimateAbilityUser_OnStateChanged(EntityRef entity, UltimateAbilityState state) {
    auto* ev = _f->get_Context()->AcquireEvent<EventUltimateAbilityUser_OnStateChanged>(EventUltimateAbilityUser_OnStateChanged::ID);
    ev->entity = entity;
    ev->state = state;
    _f->AddEvent(ev);
    return ev;
}

EventUltimateAbilityUser_OnRecast* FrameEvents::UltimateAbilityUser_OnRecast(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventUltimateAbilityUser_OnRecast>(EventUltimateAbilityUser_OnRecast::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventUltimateEvolution_OnEvolve* FrameEvents::UltimateEvolution_OnEvolve(EntityRef entity, std::int32_t level) {
    auto* ev = _f->get_Context()->AcquireEvent<EventUltimateEvolution_OnEvolve>(EventUltimateEvolution_OnEvolve::ID);
    ev->entity = entity;
    ev->level = level;
    _f->AddEvent(ev);
    return ev;
}

EventUltimateTank_OnLedgeGrabStart* FrameEvents::UltimateTank_OnLedgeGrabStart(EntityRef tankEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventUltimateTank_OnLedgeGrabStart>(EventUltimateTank_OnLedgeGrabStart::ID);
    ev->tankEntity = tankEntity;
    _f->AddEvent(ev);
    return ev;
}

EventOnFeatureUnlocked* FrameEvents::OnFeatureUnlocked(EntityRef entity, UnlockedFeature unlockedFeature) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnFeatureUnlocked>(EventOnFeatureUnlocked::ID);
    ev->entity = entity;
    ev->unlockedFeature = unlockedFeature;
    _f->AddEvent(ev);
    return ev;
}

EventOnFeatureLocked* FrameEvents::OnFeatureLocked(EntityRef entity, UnlockedFeature lockedFeature) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnFeatureLocked>(EventOnFeatureLocked::ID);
    ev->entity = entity;
    ev->lockedFeature = lockedFeature;
    _f->AddEvent(ev);
    return ev;
}

EventVoidRepulserShieldStateChange* FrameEvents::VoidRepulserShieldStateChange(EntityRef entity, VoidRepulserState state) {
    auto* ev = _f->get_Context()->AcquireEvent<EventVoidRepulserShieldStateChange>(EventVoidRepulserShieldStateChange::ID);
    ev->entity = entity;
    ev->state = state;
    _f->AddEvent(ev);
    return ev;
}

EventOnWallRunningStart* FrameEvents::OnWallRunningStart(EntityRef entity, QBoolean left) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnWallRunningStart>(EventOnWallRunningStart::ID);
    ev->entity = entity;
    ev->left = left;
    _f->AddEvent(ev);
    return ev;
}

EventOnWallRunningStop* FrameEvents::OnWallRunningStop(EntityRef entity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnWallRunningStop>(EventOnWallRunningStop::ID);
    ev->entity = entity;
    _f->AddEvent(ev);
    return ev;
}

EventWaveManager_OnWaveStarted* FrameEvents::WaveManager_OnWaveStarted(std::int32_t waveIndex, std::int32_t wavesCount) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventWaveManager_OnWaveStarted>(EventWaveManager_OnWaveStarted::ID);
    ev->waveIndex = waveIndex;
    ev->wavesCount = wavesCount;
    _f->AddEvent(ev);
    return ev;
}

EventWaveManager_OnWaveStep* FrameEvents::WaveManager_OnWaveStep(std::int32_t waveIndex, std::int32_t wavesCount, std::int32_t stepIndex) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventWaveManager_OnWaveStep>(EventWaveManager_OnWaveStep::ID);
    ev->waveIndex = waveIndex;
    ev->wavesCount = wavesCount;
    ev->stepIndex = stepIndex;
    _f->AddEvent(ev);
    return ev;
}

EventWaveManager_OnWaveComplete* FrameEvents::WaveManager_OnWaveComplete(std::int32_t waveIndex, std::int32_t wavesCount, WaveCompletionReason reason) {
    // Synced event: raised on verified frames only.
    if (_f->IsPredicted()) {
        return nullptr;
    }
    auto* ev = _f->get_Context()->AcquireEvent<EventWaveManager_OnWaveComplete>(EventWaveManager_OnWaveComplete::ID);
    ev->waveIndex = waveIndex;
    ev->wavesCount = wavesCount;
    ev->reason = reason;
    _f->AddEvent(ev);
    return ev;
}

EventWaveSpawner_OnSpawned* FrameEvents::WaveSpawner_OnSpawned(EntityRef spawner, QBoolean isBoss) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWaveSpawner_OnSpawned>(EventWaveSpawner_OnSpawned::ID);
    ev->spawner = spawner;
    ev->isBoss = isBoss;
    _f->AddEvent(ev);
    return ev;
}

EventWeaponBehaviour_Charging_OnStartCharging* FrameEvents::WeaponBehaviour_Charging_OnStartCharging(EntityRef weaponEntity, EntityRef weaponUser) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWeaponBehaviour_Charging_OnStartCharging>(EventWeaponBehaviour_Charging_OnStartCharging::ID);
    ev->weaponEntity = weaponEntity;
    ev->weaponUser = weaponUser;
    _f->AddEvent(ev);
    return ev;
}

EventWeaponUser_Activation* FrameEvents::WeaponUser_Activation(EntityRef WeaponUser, QBoolean Disabled) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWeaponUser_Activation>(EventWeaponUser_Activation::ID);
    ev->WeaponUser = WeaponUser;
    ev->Disabled = Disabled;
    _f->AddEvent(ev);
    return ev;
}

EventWeaponUser_Equipped* FrameEvents::WeaponUser_Equipped(EntityRef WeaponUser, EntityRef Weapon) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWeaponUser_Equipped>(EventWeaponUser_Equipped::ID);
    ev->WeaponUser = WeaponUser;
    ev->Weapon = Weapon;
    _f->AddEvent(ev);
    return ev;
}

EventWeaponUser_PrepareUse* FrameEvents::WeaponUser_PrepareUse(EntityRef WeaponUser, EntityRef Weapon) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWeaponUser_PrepareUse>(EventWeaponUser_PrepareUse::ID);
    ev->WeaponUser = WeaponUser;
    ev->Weapon = Weapon;
    _f->AddEvent(ev);
    return ev;
}

EventWeaponUser_Use* FrameEvents::WeaponUser_Use(EntityRef WeaponUser, EntityRef Weapon) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWeaponUser_Use>(EventWeaponUser_Use::ID);
    ev->WeaponUser = WeaponUser;
    ev->Weapon = Weapon;
    _f->AddEvent(ev);
    return ev;
}

EventWeaponUser_StopUse* FrameEvents::WeaponUser_StopUse(EntityRef WeaponUser) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWeaponUser_StopUse>(EventWeaponUser_StopUse::ID);
    ev->WeaponUser = WeaponUser;
    _f->AddEvent(ev);
    return ev;
}

EventWeaponUser_StartReload* FrameEvents::WeaponUser_StartReload(EntityRef WeaponUser) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWeaponUser_StartReload>(EventWeaponUser_StartReload::ID);
    ev->WeaponUser = WeaponUser;
    _f->AddEvent(ev);
    return ev;
}

EventWeaponUser_EndReload* FrameEvents::WeaponUser_EndReload(EntityRef WeaponUser) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWeaponUser_EndReload>(EventWeaponUser_EndReload::ID);
    ev->WeaponUser = WeaponUser;
    _f->AddEvent(ev);
    return ev;
}

EventOnArmWrenchThrow* FrameEvents::OnArmWrenchThrow(EntityRef WeaponEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnArmWrenchThrow>(EventOnArmWrenchThrow::ID);
    ev->WeaponEntity = WeaponEntity;
    _f->AddEvent(ev);
    return ev;
}

EventOnStartWrenchThrow* FrameEvents::OnStartWrenchThrow(EntityRef WeaponEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnStartWrenchThrow>(EventOnStartWrenchThrow::ID);
    ev->WeaponEntity = WeaponEntity;
    _f->AddEvent(ev);
    return ev;
}

EventOnEndWrenchThrow* FrameEvents::OnEndWrenchThrow(EntityRef WeaponEntity) {
    auto* ev = _f->get_Context()->AcquireEvent<EventOnEndWrenchThrow>(EventOnEndWrenchThrow::ID);
    ev->WeaponEntity = WeaponEntity;
    _f->AddEvent(ev);
    return ev;
}

EventWrenchWeapon_UseWrench* FrameEvents::WrenchWeapon_UseWrench(EntityRef weapon, std::uint16_t comboCounter) {
    auto* ev = _f->get_Context()->AcquireEvent<EventWrenchWeapon_UseWrench>(EventWrenchWeapon_UseWrench::ID);
    ev->weapon = weapon;
    ev->comboCounter = comboCounter;
    _f->AddEvent(ev);
    return ev;
}

EventZoneControl_OnZoneStateChanged* FrameEvents::ZoneControl_OnZoneStateChanged(CONTROL_ZONE_STATE prevState, CONTROL_ZONE_STATE newState, const ControlZoneStateHolder& stateHolder) {
    auto* ev = _f->get_Context()->AcquireEvent<EventZoneControl_OnZoneStateChanged>(EventZoneControl_OnZoneStateChanged::ID);
    ev->prevState = prevState;
    ev->newState = newState;
    ev->stateHolder = stateHolder;
    _f->AddEvent(ev);
    return ev;
}

int FrameEvents::GetParentEventID(int eventID) {
    switch (eventID) {
    case EventDamageable_HitReceived::ID:
    case EventGameModeHandlerStateChanged::ID:
    case EventGameMode_OnPlayerEliminated::ID:
    case EventGameMode_OnPlayerFinished::ID:
    case EventKillEvent::ID:
    case EventKillableSpawnEvent::ID:
    case EventReviveEvent::ID:
    case EventProjectile_OnHit::ID:
    case EventBounceEvent::ID:
        return EventBaseEntityEvent_ID;
    default:
        return -1;
    }
}

const char* FrameEvents::EventName(int eventID) {
    switch (eventID) {
    case EventAISpawner_OnSpawned::ID:
        return "EventAISpawner_OnSpawned";
    case EventAISpawner_OnDespawned::ID:
        return "EventAISpawner_OnDespawned";
    case EventActorIcedChange::ID:
        return "EventActorIcedChange";
    case EventRespawnTimerCreated::ID:
        return "EventRespawnTimerCreated";
    case EventRespawnPointSet::ID:
        return "EventRespawnPointSet";
    case EventRespawnTimerDestroyed::ID:
        return "EventRespawnTimerDestroyed";
    case EventActorSheepedChange::ID:
        return "EventActorSheepedChange";
    case EventActorStunnedChange::ID:
        return "EventActorStunnedChange";
    case EventAgentOfDoomEvent::ID:
        return "EventAgentOfDoomEvent";
    case EventAreaOfEffect_OnHit::ID:
        return "EventAreaOfEffect_OnHit";
    case EventAreaOfEffect_OnMiss::ID:
        return "EventAreaOfEffect_OnMiss";
    case EventAreaOfEffect_OnHitInstigator::ID:
        return "EventAreaOfEffect_OnHitInstigator";
    case EventAreaOfEffect_OnEnterArea::ID:
        return "EventAreaOfEffect_OnEnterArea";
    case EventAreaOfEffect_OnExitArea::ID:
        return "EventAreaOfEffect_OnExitArea";
    case EventAutoJumperTriggered::ID:
        return "EventAutoJumperTriggered";
    case EventBigBoomMine_Triggered::ID:
        return "EventBigBoomMine_Triggered";
    case EventBlastBallGame_StateChanged::ID:
        return "EventBlastBallGame_StateChanged";
    case EventBlastBallGame_StartOverTime::ID:
        return "EventBlastBallGame_StartOverTime";
    case EventBlastBallGame_TeamScored::ID:
        return "EventBlastBallGame_TeamScored";
    case EventBlastBallGame_BallHit::ID:
        return "EventBlastBallGame_BallHit";
    case EventOnBoltCollected::ID:
        return "EventOnBoltCollected";
    case EventOnBoltLost::ID:
        return "EventOnBoltLost";
    case EventBoltCounter_OnUpdateBolts::ID:
        return "EventBoltCounter_OnUpdateBolts";
    case EventBumperTriggered::ID:
        return "EventBumperTriggered";
    case EventCharacterColliderScaler_OnScaleChanged::ID:
        return "EventCharacterColliderScaler_OnScaleChanged";
    case EventColliderResize_OnResizeStarted::ID:
        return "EventColliderResize_OnResizeStarted";
    case EventCollisionForce_OnCollision::ID:
        return "EventCollisionForce_OnCollision";
    case EventDamageable_HitReceived::ID:
        return "EventDamageable_HitReceived";
    case EventOnDamageChargeableReset::ID:
        return "EventOnDamageChargeableReset";
    case EventDashStarted::ID:
        return "EventDashStarted";
    case EventDashComponentDisabled::ID:
        return "EventDashComponentDisabled";
    case EventDrillDash_OnStateChanged::ID:
        return "EventDrillDash_OnStateChanged";
    case EventOnDroppableDropped::ID:
        return "EventOnDroppableDropped";
    case EventEliminationGameMode_OnCloseToWin::ID:
        return "EventEliminationGameMode_OnCloseToWin";
    case EventEnergyCell_OnCollected::ID:
        return "EventEnergyCell_OnCollected";
    case EventFireReactor_OnStateChanged::ID:
        return "EventFireReactor_OnStateChanged";
    case EventGadgetUser_OnGadgetActivated::ID:
        return "EventGadgetUser_OnGadgetActivated";
    case EventGadgetUser_OnGadgetDeactivated::ID:
        return "EventGadgetUser_OnGadgetDeactivated";
    case EventIntroEnded::ID:
        return "EventIntroEnded";
    case EventBoltMultiplierOn::ID:
        return "EventBoltMultiplierOn";
    case EventGameModeHandlerStateChanged::ID:
        return "EventGameModeHandlerStateChanged";
    case EventGameMode_OnPlayerEliminated::ID:
        return "EventGameMode_OnPlayerEliminated";
    case EventGameMode_OnPlayerFinished::ID:
        return "EventGameMode_OnPlayerFinished";
    case EventPreloadAddressablePrefabs::ID:
        return "EventPreloadAddressablePrefabs";
    case EventGoldenBoltSpawned::ID:
        return "EventGoldenBoltSpawned";
    case EventGoldenBoltCollected::ID:
        return "EventGoldenBoltCollected";
    case EventStartGrind::ID:
        return "EventStartGrind";
    case EventSnappedToRail::ID:
        return "EventSnappedToRail";
    case EventExitGrind::ID:
        return "EventExitGrind";
    case EventSwitchRail::ID:
        return "EventSwitchRail";
    case EventGroundDashStarting::ID:
        return "EventGroundDashStarting";
    case EventGroundDashStarted::ID:
        return "EventGroundDashStarted";
    case EventGroundDashEnded::ID:
        return "EventGroundDashEnded";
    case EventHealSource_OnHeal::ID:
        return "EventHealSource_OnHeal";
    case EventOnHealthDecreased::ID:
        return "EventOnHealthDecreased";
    case EventHelipackStart::ID:
        return "EventHelipackStart";
    case EventHelipackStop::ID:
        return "EventHelipackStop";
    case EventItemSpawnerStatus::ID:
        return "EventItemSpawnerStatus";
    case EventItemSpawnerEntitySpawn::ID:
        return "EventItemSpawnerEntitySpawn";
    case EventKccExtension_OnJumpStateChanged::ID:
        return "EventKccExtension_OnJumpStateChanged";
    case EventKccExtensions_OnMoveSpeedModifierAdded::ID:
        return "EventKccExtensions_OnMoveSpeedModifierAdded";
    case EventKillEvent::ID:
        return "EventKillEvent";
    case EventKillableSpawnEvent::ID:
        return "EventKillableSpawnEvent";
    case EventReviveEvent::ID:
        return "EventReviveEvent";
    case EventOnLedgeGrabStart::ID:
        return "EventOnLedgeGrabStart";
    case EventOnLedgeGrabEnd::ID:
        return "EventOnLedgeGrabEnd";
    case EventOnLevelButtonPressed::ID:
        return "EventOnLevelButtonPressed";
    case EventOnLevelButtonReleased::ID:
        return "EventOnLevelButtonReleased";
    case EventOnArenaCannonActivated::ID:
        return "EventOnArenaCannonActivated";
    case EventMeleeAttack_Start::ID:
        return "EventMeleeAttack_Start";
    case EventMeleeAttack_HitOrMissed::ID:
        return "EventMeleeAttack_HitOrMissed";
    case EventMeleeJumpAttack_OnPerformed::ID:
        return "EventMeleeJumpAttack_OnPerformed";
    case EventMobSlamAttackEvent::ID:
        return "EventMobSlamAttackEvent";
    case EventMobsEvent::ID:
        return "EventMobsEvent";
    case EventMoveTo_OnMoveStarted::ID:
        return "EventMoveTo_OnMoveStarted";
    case EventMoveTo_OnMoveFinished::ID:
        return "EventMoveTo_OnMoveFinished";
    case EventOnMovingWallStart::ID:
        return "EventOnMovingWallStart";
    case EventPowerUp_OnStarted::ID:
        return "EventPowerUp_OnStarted";
    case EventPowerUp_OnFinished::ID:
        return "EventPowerUp_OnFinished";
    case EventProjectile_OnHit::ID:
        return "EventProjectile_OnHit";
    case EventOnPushed::ID:
        return "EventOnPushed";
    case EventOnPushContinued::ID:
        return "EventOnPushContinued";
    case EventOnStopPushBegin::ID:
        return "EventOnStopPushBegin";
    case EventOnPushResistanceChanged::ID:
        return "EventOnPushResistanceChanged";
    case EventPushStoppedEnemy::ID:
        return "EventPushStoppedEnemy";
    case EventReward_OnPending::ID:
        return "EventReward_OnPending";
    case EventReward_OnCollected::ID:
        return "EventReward_OnCollected";
    case EventBounceEvent::ID:
        return "EventBounceEvent";
    case EventShieldHit::ID:
        return "EventShieldHit";
    case EventShieldFull::ID:
        return "EventShieldFull";
    case EventShieldBreaks::ID:
        return "EventShieldBreaks";
    case EventShootingEnemyEvent::ID:
        return "EventShootingEnemyEvent";
    case EventShrinkingZone_MoveStart::ID:
        return "EventShrinkingZone_MoveStart";
    case EventSpinningEnemyEvent::ID:
        return "EventSpinningEnemyEvent";
    case EventTutorialGameMode_StepChanged::ID:
        return "EventTutorialGameMode_StepChanged";
    case EventUltimateAbilityUser_OnStateChanged::ID:
        return "EventUltimateAbilityUser_OnStateChanged";
    case EventUltimateAbilityUser_OnRecast::ID:
        return "EventUltimateAbilityUser_OnRecast";
    case EventUltimateEvolution_OnEvolve::ID:
        return "EventUltimateEvolution_OnEvolve";
    case EventUltimateTank_OnLedgeGrabStart::ID:
        return "EventUltimateTank_OnLedgeGrabStart";
    case EventOnFeatureUnlocked::ID:
        return "EventOnFeatureUnlocked";
    case EventOnFeatureLocked::ID:
        return "EventOnFeatureLocked";
    case EventVoidRepulserShieldStateChange::ID:
        return "EventVoidRepulserShieldStateChange";
    case EventOnWallRunningStart::ID:
        return "EventOnWallRunningStart";
    case EventOnWallRunningStop::ID:
        return "EventOnWallRunningStop";
    case EventWaveManager_OnWaveStarted::ID:
        return "EventWaveManager_OnWaveStarted";
    case EventWaveManager_OnWaveStep::ID:
        return "EventWaveManager_OnWaveStep";
    case EventWaveManager_OnWaveComplete::ID:
        return "EventWaveManager_OnWaveComplete";
    case EventWaveSpawner_OnSpawned::ID:
        return "EventWaveSpawner_OnSpawned";
    case EventWeaponBehaviour_Charging_OnStartCharging::ID:
        return "EventWeaponBehaviour_Charging_OnStartCharging";
    case EventWeaponUser_Activation::ID:
        return "EventWeaponUser_Activation";
    case EventWeaponUser_Equipped::ID:
        return "EventWeaponUser_Equipped";
    case EventWeaponUser_PrepareUse::ID:
        return "EventWeaponUser_PrepareUse";
    case EventWeaponUser_Use::ID:
        return "EventWeaponUser_Use";
    case EventWeaponUser_StopUse::ID:
        return "EventWeaponUser_StopUse";
    case EventWeaponUser_StartReload::ID:
        return "EventWeaponUser_StartReload";
    case EventWeaponUser_EndReload::ID:
        return "EventWeaponUser_EndReload";
    case EventOnArmWrenchThrow::ID:
        return "EventOnArmWrenchThrow";
    case EventOnStartWrenchThrow::ID:
        return "EventOnStartWrenchThrow";
    case EventOnEndWrenchThrow::ID:
        return "EventOnEndWrenchThrow";
    case EventWrenchWeapon_UseWrench::ID:
        return "EventWrenchWeapon_UseWrench";
    case EventZoneControl_OnZoneStateChanged::ID:
        return "EventZoneControl_OnZoneStateChanged";
    default:
        return nullptr;
    }
}

} // namespace Quantum
