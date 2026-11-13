// reward_systems.cpp
#include "quantum_code/systems/reward_systems.h"

namespace Quantum {

void EnergyCellSignalSystem::Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type,
                                                std::uint16_t amount) {
    (void)amount;
    if (type == DroppableType::EnergyCellLow || type == DroppableType::EnergyCellMedium ||
        type == DroppableType::EnergyCellHigh) {
        EnergyCellManager::TryGiveEnergyCell(frame, collectorEntity, type);
    }
}

} // namespace Quantum
