// reward_systems.h - systems that hand out what collectors picked up.
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

class EnergyCellSignalSystem final : public SystemSignalsOnly, public ISignalReward_OnCollected {
  public:
    // 0x7bbd644
    EnergyCellSignalSystem() = default;
    // 0x7bbd61c - Energy cells (low, medium, high) charge the collector's ultimate.
    void Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type, std::uint16_t amount) override;
};

// Bolts: pickups add them; deaths, respawns and eliminations cost some, per the game
// mode's rules.
class BoltSignalSystem final : public SystemSignalsOnly,
                               public ISignalReward_OnCollected,
                               public ISignalOnComponentAdded<BoltGathererActor>,
                               public ISignalKillable_OnKill,
                               public ISignalKillable_OnRevive {
  public:
    // 0x7bb79e8
    BoltSignalSystem() = default;
    // 0x7bb72ec - Bolt pickups.
    void Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type, std::uint16_t amount) override;
    // 0x7bb7310 - A gatherer without a spec is reported and removed.
    void OnAdded(Frame* f, EntityRef entity, BoltGathererActor* component) override;
    // 0x7bb7490 - Death loss.
    void Killable_OnKill(Frame* frame, EntityRef killedEntity, const KilledByData& killedBy) override;
    // TODO(0x7bb770c, Quantum.BoltSignalSystem.GameMode_OnPlayerEliminated): in co-op
    // modes, once the player's team is eliminated the player and a teammate lose the
    // elimination amount; needs Team.IsTeamEliminated and Team.TryGetTeammate.
    // 0x7bb788c - Respawn loss.
    void Killable_OnRevive(Frame* frame, EntityRef toRevive, const FPVector3& position,
                           const FPQuaternion& rotation) override;
    // 0x7bb755c
    static void LoseBoltsFixedAmount(Frame* frame, EntityRef playerEntity, std::int32_t amount);
    // 0x7bb761c - `percentage` of the bolts carried (truncated).
    static void LoseBoltsPercentage(Frame* frame, EntityRef entity, FP percentage);
    // 0x7bb7918 - Never more than carried; reports the loss.
    static void LoseBolts(Frame* frame, EntityRef entity, BoltGathererActor* boltGatherer, std::int32_t amount);
};

} // namespace Quantum
