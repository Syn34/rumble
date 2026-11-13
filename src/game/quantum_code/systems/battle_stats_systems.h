// battle_stats_systems.h - per-player battle statistics (BattleStatsActorSystem). Only the
// weapon-usage lookup the damage resolution needs is reimplemented so far.
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

// Quantum.BattleStatsActorSystem (original base: Quantum.SystemSignalsOnly)
class BattleStatsActorSystem final : public SystemSignalsOnly {
  public:
    // TODO(0x7cb2d20, Quantum.BattleStatsActorSystem.PlayerActor_OnInitialized): PlayerActor_OnInitialized(frame, actorEntity, pActor); not reimplemented yet.
    // TODO(0x7cb3068, Quantum.BattleStatsActorSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7cb3440, Quantum.BattleStatsActorSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7cb3860, Quantum.BattleStatsActorSystem.Bolt_OnGoldenBoltCollected): Bolt_OnGoldenBoltCollected(f, collector); not reimplemented yet.
    // TODO(0x7cb3900, Quantum.BattleStatsActorSystem.Bolt_OnCollected): Bolt_OnCollected(f, collector, amount); not reimplemented yet.
    // TODO(0x7cb39a4, Quantum.BattleStatsActorSystem.GrinderActor_OnExitGrind): GrinderActor_OnExitGrind(f, entity, grindingDuration); not reimplemented yet.
    // TODO(0x7cb3a48, Quantum.BattleStatsActorSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(frame, entity, state); not reimplemented yet.
    // TODO(0x7cb3af0, Quantum.BattleStatsActorSystem.GadgetUser_OnGadgetActivated): GadgetUser_OnGadgetActivated(f, entity); not reimplemented yet.
    // TODO(0x7cb3b90, Quantum.BattleStatsActorSystem.PowerUp_OnCollected): PowerUp_OnCollected(f, collector, type, source); not reimplemented yet.
    // 0x7cb2ec0 - The usage record of the loadout weapon `weapon` of `entity`, created
    // (zeroed) on first use.
    static bool TryGetWeaponUsageStats(Frame* f, BattleStatsActor* pBattleStatsActor, EntityRef entity, EntityRef weapon,
                                       WeaponUsageStats*& stats);
    // TODO(0x7cb3d50, Quantum.BattleStatsActorSystem.Weapon_OnReload): Weapon_OnReload(f, weaponEntity, weapon); not reimplemented yet.
    // TODO(0x7cb3e0c, Quantum.BattleStatsActorSystem.WeaponLoadoutActor_OnManualSwap): WeaponLoadoutActor_OnManualSwap(f, loadoutActor, slot); not reimplemented yet.
    // TODO(0x7cb30f8, Quantum.BattleStatsActorSystem.ComputeStatsForDamageTaken): ComputeStatsForDamageTaken(f, entity, hitData); not reimplemented yet.
    // TODO(0x7cb3224, Quantum.BattleStatsActorSystem.ComputeStatsForDamageDealt): ComputeStatsForDamageDealt(f, entity, hitData); not reimplemented yet.
    // TODO(0x7cb34d0, Quantum.BattleStatsActorSystem.ComputeStatsForVictim): ComputeStatsForVictim(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7cb35f8, Quantum.BattleStatsActorSystem.ComputeStatsForKiller): ComputeStatsForKiller(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7cb3ec0, Quantum.BattleStatsActorSystem.IsEntityOwnedByPlayer): IsEntityOwnedByPlayer(f, entity); not reimplemented yet.
    // TODO(0x7cb3f54, Quantum.BattleStatsActorSystem..ctor): .ctor(); not reimplemented yet.
};

} // namespace Quantum
