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

} // namespace Quantum
