// event_types.cpp - GetHashCode of the event classes.
#include "quantum_code/event_types.h"

namespace Quantum {

std::int32_t EventBaseEntityEvent::GetHashCode() const { return HashBuilder(223u).Add(entity).Value(); }

std::int32_t EventAISpawner_OnSpawned::GetHashCode() const {
    return HashBuilder(37u).Add(spawnerEntity).Add(spawnedEntity).Value();
}

std::int32_t EventAISpawner_OnDespawned::GetHashCode() const {
    return HashBuilder(41u).Add(spawnerEntity).Add(despawnedEntity).Value();
}

std::int32_t EventActorIcedChange::GetHashCode() const {
    return HashBuilder(43u).Add(entity).Add(iced).Value();
}

std::int32_t EventRespawnTimerCreated::GetHashCode() const {
    return HashBuilder(47u).Add(entity).Add(respawnPointCalculationDelay).Value();
}

std::int32_t EventRespawnPointSet::GetHashCode() const {
    return HashBuilder(53u).Add(entity).Add(spawnPosition).Add(spawnRotation).Value();
}

std::int32_t EventRespawnTimerDestroyed::GetHashCode() const {
    return HashBuilder(59u).Add(entity).Value();
}

std::int32_t EventActorSheepedChange::GetHashCode() const {
    return HashBuilder(67u).Add(entity).Add(sheeped).Value();
}

std::int32_t EventActorStunnedChange::GetHashCode() const {
    return HashBuilder(71u).Add(entity).Add(stunned).Value();
}

std::int32_t EventAgentOfDoomEvent::GetHashCode() const {
    return HashBuilder(73u).Add(entity).Add(eventType).Value();
}

std::int32_t EventAreaOfEffect_OnHit::GetHashCode() const {
    return HashBuilder(79u).Add(entity).Value();
}

std::int32_t EventAreaOfEffect_OnMiss::GetHashCode() const {
    return HashBuilder(83u).Add(entity).Value();
}

std::int32_t EventAreaOfEffect_OnHitInstigator::GetHashCode() const {
    return HashBuilder(89u).Add(instigator).Value();
}

std::int32_t EventAreaOfEffect_OnEnterArea::GetHashCode() const {
    return HashBuilder(97u).Add(inAreaEntity).Add(aoeEntity).Value();
}

std::int32_t EventAreaOfEffect_OnExitArea::GetHashCode() const {
    return HashBuilder(101u).Add(outOfAreaEntity).Add(aoeEntity).Value();
}

std::int32_t EventAutoJumperTriggered::GetHashCode() const {
    return HashBuilder(103u).Add(jumpingEntity).Add(staticColliderIndex).Value();
}

std::int32_t EventBigBoomMine_Triggered::GetHashCode() const {
    return HashBuilder(107u).Add(Owner).Value();
}

std::int32_t EventBlastBallGame_StateChanged::GetHashCode() const {
    return HashBuilder(109u).Add(newState).Value();
}

std::int32_t EventBlastBallGame_StartOverTime::GetHashCode() const { return HashBuilder(113u).Value(); }

std::int32_t EventBlastBallGame_TeamScored::GetHashCode() const {
    return HashBuilder(127u).Add(scoringTeam).Add(score).Add(scoringPlayer).Value();
}

std::int32_t EventBlastBallGame_BallHit::GetHashCode() const {
    return HashBuilder(131u).Add(player).Add(position).Add(direction).Value();
}

std::int32_t EventOnBoltCollected::GetHashCode() const {
    return HashBuilder(137u).Add(entity).Add(amount).Value();
}

std::int32_t EventOnBoltLost::GetHashCode() const {
    return HashBuilder(139u).Add(entity).Add(amount).Value();
}

std::int32_t EventBoltCounter_OnUpdateBolts::GetHashCode() const {
    return HashBuilder(149u).Add(counterEntity).Add(currentBolts).Value();
}

std::int32_t EventBumperTriggered::GetHashCode() const {
    return HashBuilder(151u).Add(bumper).Add(player).Value();
}

std::int32_t EventCharacterColliderScaler_OnScaleChanged::GetHashCode() const {
    return HashBuilder(157u).Add(entity).Value();
}

std::int32_t EventColliderResize_OnResizeStarted::GetHashCode() const {
    return HashBuilder(163u).Add(entity).Value();
}

std::int32_t EventCollisionForce_OnCollision::GetHashCode() const {
    return HashBuilder(167u).Add(entity).Add(otherEntity).Add(hitPoint).Add(hitNormal).Value();
}

std::int32_t EventDamageable_HitReceived::GetHashCode() const {
    return HashBuilder(173u).Add(entity).Add(hitData).Add(hitResult).Value();
}

std::int32_t EventOnDamageChargeableReset::GetHashCode() const {
    return HashBuilder(179u).Add(chargeableEntity).Value();
}

std::int32_t EventDashStarted::GetHashCode() const {
    return HashBuilder(181u).Add(entity).Value();
}

std::int32_t EventDashComponentDisabled::GetHashCode() const {
    return HashBuilder(191u).Add(entity).Value();
}

std::int32_t EventDrillDash_OnStateChanged::GetHashCode() const {
    return HashBuilder(193u).Add(entity).Add(weaponEntity).Add(newState).Value();
}

std::int32_t EventOnDroppableDropped::GetHashCode() const {
    return HashBuilder(197u).Add(dropperEntity).Add(type).Add(position).Add(amount).Value();
}

std::int32_t EventEliminationGameMode_OnCloseToWin::GetHashCode() const {
    return HashBuilder(199u).Add(entity).Value();
}

std::int32_t EventEnergyCell_OnCollected::GetHashCode() const {
    return HashBuilder(211u).Add(collector).Add(value).Value();
}

std::int32_t EventFireReactor_OnStateChanged::GetHashCode() const {
    return HashBuilder(227u).Add(entity).Add(enabled).Value();
}

std::int32_t EventGadgetUser_OnGadgetActivated::GetHashCode() const {
    return HashBuilder(229u).Add(entity).Value();
}

std::int32_t EventGadgetUser_OnGadgetDeactivated::GetHashCode() const {
    return HashBuilder(233u).Add(entity).Value();
}

std::int32_t EventIntroEnded::GetHashCode() const {
    return HashBuilder(239u).Add(entity).Value();
}

std::int32_t EventBoltMultiplierOn::GetHashCode() const { return HashBuilder(251u).Value(); }

std::int32_t EventGameModeHandlerStateChanged::GetHashCode() const {
    return HashBuilder(257u).Add(entity).Add(prevState).Add(newState).Value();
}

std::int32_t EventGameMode_OnPlayerEliminated::GetHashCode() const {
    return HashBuilder(271u).Add(entity).Value();
}

std::int32_t EventGameMode_OnPlayerFinished::GetHashCode() const {
    return HashBuilder(277u).Add(entity).Value();
}

std::int32_t EventPreloadAddressablePrefabs::GetHashCode() const { return HashBuilder(281u).Value(); }

std::int32_t EventGoldenBoltSpawned::GetHashCode() const {
    return HashBuilder(283u).Add(isUnique).Value();
}

std::int32_t EventGoldenBoltCollected::GetHashCode() const {
    return HashBuilder(293u).Add(boltEntity).Add(collector).Value();
}

std::int32_t EventStartGrind::GetHashCode() const {
    return HashBuilder(307u).Add(entity).Add(jumpAnim).Value();
}

std::int32_t EventSnappedToRail::GetHashCode() const {
    return HashBuilder(311u).Add(entity).Value();
}

std::int32_t EventExitGrind::GetHashCode() const {
    return HashBuilder(313u).Add(entity).Add(jumpAnim).Value();
}

std::int32_t EventSwitchRail::GetHashCode() const {
    return HashBuilder(317u).Add(entity).Add(isLeft).Value();
}

std::int32_t EventGroundDashStarting::GetHashCode() const {
    return HashBuilder(331u).Add(userEntity).Value();
}

std::int32_t EventGroundDashStarted::GetHashCode() const {
    return HashBuilder(337u).Add(userEntity).Value();
}

std::int32_t EventGroundDashEnded::GetHashCode() const {
    return HashBuilder(347u).Add(userEntity).Add(endReason).Value();
}

std::int32_t EventHealSource_OnHeal::GetHashCode() const {
    return HashBuilder(349u).Add(entity).Value();
}

std::int32_t EventOnHealthDecreased::GetHashCode() const {
    return HashBuilder(353u).Add(entity).Add(amount).Add(hitData).Value();
}

std::int32_t EventHelipackStart::GetHashCode() const {
    return HashBuilder(359u).Add(entity).Add(verticalVelocity).Value();
}

std::int32_t EventHelipackStop::GetHashCode() const {
    return HashBuilder(367u).Add(entity).Value();
}

std::int32_t EventItemSpawnerStatus::GetHashCode() const {
    return HashBuilder(373u).Add(entity).Add(status).Add(enabled).Value();
}

std::int32_t EventItemSpawnerEntitySpawn::GetHashCode() const {
    return HashBuilder(379u).Add(entity).Value();
}

std::int32_t EventKccExtension_OnJumpStateChanged::GetHashCode() const {
    return HashBuilder(383u).Add(entity).Add(jumpState).Value();
}

std::int32_t EventKccExtensions_OnMoveSpeedModifierAdded::GetHashCode() const {
    return HashBuilder(389u).Add(entity).Add(value).Add(duration).Value();
}

std::int32_t EventKillEvent::GetHashCode() const {
    return HashBuilder(397u).Add(entity).Add(killType).Add(killZoneSurfaceType).Value();
}

std::int32_t EventKillableSpawnEvent::GetHashCode() const {
    return HashBuilder(401u).Add(entity).Value();
}

std::int32_t EventReviveEvent::GetHashCode() const {
    return HashBuilder(409u).Add(entity).Add(orientation).Value();
}

std::int32_t EventOnLedgeGrabStart::GetHashCode() const {
    return HashBuilder(419u).Add(character).Add(cameraDistanceOffset).Add(cameraVerticalAngle).Value();
}

std::int32_t EventOnLedgeGrabEnd::GetHashCode() const {
    return HashBuilder(421u).Add(character).Value();
}

std::int32_t EventOnLevelButtonPressed::GetHashCode() const {
    return HashBuilder(431u).Add(entity).Value();
}

std::int32_t EventOnLevelButtonReleased::GetHashCode() const {
    return HashBuilder(433u).Add(entity).Value();
}

std::int32_t EventOnArenaCannonActivated::GetHashCode() const {
    return HashBuilder(439u).Add(entity).Value();
}

std::int32_t EventMeleeAttack_Start::GetHashCode() const {
    return HashBuilder(443u).Add(attackOwner).Value();
}

std::int32_t EventMeleeAttack_HitOrMissed::GetHashCode() const {
    return HashBuilder(449u).Add(attackOwner).Add(hit).Value();
}

std::int32_t EventMeleeJumpAttack_OnPerformed::GetHashCode() const {
    return HashBuilder(457u).Add(entity).Value();
}

std::int32_t EventMobSlamAttackEvent::GetHashCode() const {
    return HashBuilder(461u).Add(entity).Add(attackState).Value();
}

std::int32_t EventMobsEvent::GetHashCode() const {
    return HashBuilder(463u).Add(entity).Add(eventType).Value();
}

std::int32_t EventMoveTo_OnMoveStarted::GetHashCode() const {
    return HashBuilder(467u).Add(entity).Value();
}

std::int32_t EventMoveTo_OnMoveFinished::GetHashCode() const {
    return HashBuilder(479u).Add(entity).Value();
}

std::int32_t EventOnMovingWallStart::GetHashCode() const {
    return HashBuilder(487u).Add(entity).Value();
}

std::int32_t EventPowerUp_OnStarted::GetHashCode() const {
    throw std::logic_error("event hash over an asset reference is not reproducible");
}

std::int32_t EventPowerUp_OnFinished::GetHashCode() const {
    throw std::logic_error("event hash over an asset reference is not reproducible");
}

std::int32_t EventProjectile_OnHit::GetHashCode() const {
    return HashBuilder(503u).Add(entity).Add(hit).Value();
}

std::int32_t EventOnPushed::GetHashCode() const {
    return HashBuilder(509u).Add(entity).Add(pushData).Add(force).Add(pushModifierStatusEffect).Value();
}

std::int32_t EventOnPushContinued::GetHashCode() const {
    return HashBuilder(521u).Add(entity).Add(pusher).Add(force).Value();
}

std::int32_t EventOnStopPushBegin::GetHashCode() const {
    return HashBuilder(523u).Add(entity).Add(pusher).Add(force).Value();
}

std::int32_t EventOnPushResistanceChanged::GetHashCode() const {
    return HashBuilder(541u).Add(entity).Add(newResistance).Value();
}

std::int32_t EventPushStoppedEnemy::GetHashCode() const {
    return HashBuilder(547u).Add(WeaponUser).Add(UserVelocity).Add(PushStopDuration).Value();
}

std::int32_t EventReward_OnPending::GetHashCode() const {
    return HashBuilder(557u).Add(entity).Add(reward).Value();
}

std::int32_t EventReward_OnCollected::GetHashCode() const {
    return HashBuilder(563u).Add(entity).Add(type).Add(amount).Value();
}

std::int32_t EventBounceEvent::GetHashCode() const {
    return HashBuilder(569u).Add(entity).Add(position).Add(normal).Add(bounceCount).Value();
}

std::int32_t EventShieldHit::GetHashCode() const {
    return HashBuilder(571u).Add(entity).Add(hitData).Add(amount).Add(remainingShieldPoints).Value();
}

std::int32_t EventShieldFull::GetHashCode() const {
    return HashBuilder(577u).Add(entity).Value();
}

std::int32_t EventShieldBreaks::GetHashCode() const {
    return HashBuilder(593u).Add(entity).Value();
}

std::int32_t EventShootingEnemyEvent::GetHashCode() const {
    return HashBuilder(599u).Add(entity).Add(eventType).Value();
}

std::int32_t EventShrinkingZone_MoveStart::GetHashCode() const {
    return HashBuilder(601u).Add(zoneIndex).Value();
}

std::int32_t EventSpinningEnemyEvent::GetHashCode() const {
    return HashBuilder(607u).Add(entity).Add(eventType).Value();
}

std::int32_t EventTutorialGameMode_StepChanged::GetHashCode() const {
    return HashBuilder(613u).Add(newStep).Value();
}

std::int32_t EventUltimateAbilityUser_OnStateChanged::GetHashCode() const {
    return HashBuilder(617u).Add(entity).Add(state).Value();
}

std::int32_t EventUltimateAbilityUser_OnRecast::GetHashCode() const {
    return HashBuilder(619u).Add(entity).Value();
}

std::int32_t EventUltimateEvolution_OnEvolve::GetHashCode() const {
    return HashBuilder(631u).Add(entity).Add(level).Value();
}

std::int32_t EventUltimateTank_OnLedgeGrabStart::GetHashCode() const {
    return HashBuilder(641u).Add(tankEntity).Value();
}

std::int32_t EventOnFeatureUnlocked::GetHashCode() const {
    return HashBuilder(643u).Add(entity).Add(unlockedFeature).Value();
}

std::int32_t EventOnFeatureLocked::GetHashCode() const {
    return HashBuilder(647u).Add(entity).Add(lockedFeature).Value();
}

std::int32_t EventVoidRepulserShieldStateChange::GetHashCode() const {
    return HashBuilder(653u).Add(entity).Add(state).Value();
}

std::int32_t EventOnWallRunningStart::GetHashCode() const {
    return HashBuilder(659u).Add(entity).Add(left).Value();
}

std::int32_t EventOnWallRunningStop::GetHashCode() const {
    return HashBuilder(661u).Add(entity).Value();
}

std::int32_t EventWaveManager_OnWaveStarted::GetHashCode() const {
    return HashBuilder(673u).Add(waveIndex).Add(wavesCount).Value();
}

std::int32_t EventWaveManager_OnWaveStep::GetHashCode() const {
    return HashBuilder(677u).Add(waveIndex).Add(wavesCount).Add(stepIndex).Value();
}

std::int32_t EventWaveManager_OnWaveComplete::GetHashCode() const {
    return HashBuilder(683u).Add(waveIndex).Add(wavesCount).Add(reason).Value();
}

std::int32_t EventWaveSpawner_OnSpawned::GetHashCode() const {
    return HashBuilder(701u).Add(spawner).Add(isBoss).Value();
}

std::int32_t EventWeaponBehaviour_Charging_OnStartCharging::GetHashCode() const {
    return HashBuilder(709u).Add(weaponEntity).Add(weaponUser).Value();
}

std::int32_t EventWeaponUser_Activation::GetHashCode() const {
    return HashBuilder(719u).Add(WeaponUser).Add(Disabled).Value();
}

std::int32_t EventWeaponUser_Equipped::GetHashCode() const {
    return HashBuilder(727u).Add(WeaponUser).Add(Weapon).Value();
}

std::int32_t EventWeaponUser_PrepareUse::GetHashCode() const {
    return HashBuilder(733u).Add(WeaponUser).Add(Weapon).Value();
}

std::int32_t EventWeaponUser_Use::GetHashCode() const {
    return HashBuilder(739u).Add(WeaponUser).Add(Weapon).Value();
}

std::int32_t EventWeaponUser_StopUse::GetHashCode() const {
    return HashBuilder(743u).Add(WeaponUser).Value();
}

std::int32_t EventWeaponUser_StartReload::GetHashCode() const {
    return HashBuilder(751u).Add(WeaponUser).Value();
}

std::int32_t EventWeaponUser_EndReload::GetHashCode() const {
    return HashBuilder(757u).Add(WeaponUser).Value();
}

std::int32_t EventOnArmWrenchThrow::GetHashCode() const {
    return HashBuilder(761u).Add(WeaponEntity).Value();
}

std::int32_t EventOnStartWrenchThrow::GetHashCode() const {
    return HashBuilder(769u).Add(WeaponEntity).Value();
}

std::int32_t EventOnEndWrenchThrow::GetHashCode() const {
    return HashBuilder(773u).Add(WeaponEntity).Value();
}

std::int32_t EventWrenchWeapon_UseWrench::GetHashCode() const {
    return HashBuilder(787u).Add(weapon).Add(comboCounter).Value();
}

std::int32_t EventZoneControl_OnZoneStateChanged::GetHashCode() const {
    return HashBuilder(797u).Add(prevState).Add(newState).Add(stateHolder).Value();
}

} // namespace Quantum
