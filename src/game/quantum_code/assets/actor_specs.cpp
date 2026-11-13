// Methods of the actor assets (quantum.code.dll).
#include "quantum_code/assets/actor_specs.h"

#include "photon_deterministic/fp_math.h"

#include <limits>

namespace Quantum {

FP ActorManagerConfig::GetTotalRespawnDuration(bool isBot) const {
    const FP pending = isBot && BotsUseSpecificPendingRespawnDuration ? BotsPendingRespawnDuration : PendingRespawnDuration;
    return RespawnDuration + pending;
}

bool KccExtensionConfig::TryGetWalkingPushMultiplier(EntityInfoType type, FP& multiplier) const {
    for (const EntityMultiplierPair& pair : WalkingPushMultiplierByEntity) {
        if (pair.EntityInfoType == type) {
            multiplier = pair.Multiplier;
            return true;
        }
    }
    multiplier = FP{};
    return false;
}

} // namespace Quantum

namespace Quantum {

std::int32_t HeroSpec::GetLevelAtPower(Frame* frame, GENERATED_CHARACTER_ID heroID, FP power) const {
    (void)frame, (void)heroID;
    std::int32_t best = 0;
    FP smallestGap = FP::FromRaw(std::numeric_limits<FP::Raw>::max());
    for (std::int32_t level = 0; level < 10; ++level) {
        const FP levelPower = LeveledPower.at(static_cast<std::size_t>(Photon::Deterministic::FPMath::Clamp(level, 0, 9)));
        const FP gap = Photon::Deterministic::FPMath::Abs(power - levelPower);
        if (gap <= smallestGap) best = level;
        if (gap < smallestGap) smallestGap = gap;
    }
    return best;
}

} // namespace Quantum

namespace Quantum {

bool DroppableCollectorSpec::HasMaxCollectPerSecond(DroppableType droppableType, std::uint16_t& maxAmount) const {
    for (const MaxCollectPerSecond& cap : MaxDroppablesCollectedPerSecond) {
        if (cap.droppableType == droppableType) {
            maxAmount = cap.amount;
            return true;
        }
    }
    maxAmount = 0xFFFF;
    return false;
}

} // namespace Quantum
