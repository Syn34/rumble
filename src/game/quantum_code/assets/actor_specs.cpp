// Methods of the actor assets (quantum.code.dll).
#include "quantum_code/assets/actor_specs.h"

namespace Quantum {

FP ActorManagerConfig::GetTotalRespawnDuration(bool isBot) const {
    const FP pending = isBot && BotsUseSpecificPendingRespawnDuration ? BotsPendingRespawnDuration : PendingRespawnDuration;
    return RespawnDuration + pending;
}

} // namespace Quantum
