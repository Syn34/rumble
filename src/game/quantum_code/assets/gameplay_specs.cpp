// Methods of the gameplay spec assets (quantum.code.dll).
#include "quantum_code/assets/gameplay_specs.h"

namespace Quantum {

FP BombardierDroneSpec::TakeOffDuration() const { return TakeOffDistance / MoveSpeed; }

FP BombardierDroneSpec::FlightDuration() const { return FlightDistance / MoveSpeed; }

FP BombardierDroneSpec::MaxLifetime() const {
    return TakeOffDistance / MoveSpeed + FlightDistance / MoveSpeed + DroppingMaxDuration;
}

FP BombardierDroneSpec::TotalRange() const {
    return TakeOffDistance + FlightDistance + MoveSpeed * DroppingMaxDuration;
}

bool ProjectileSpec::HasAcceleration() const { return Type == static_cast<ProjectileType>(0) && Accelerates; }

void PusherSpec::SetupPushData(PushData* pushData) const {
    pushData->isContinuous = QBoolean(IsContinuous);
    pushData->bypassPushStop = QBoolean(BypassPushStop);
    pushData->canBump = QBoolean(CanBump);
    pushData->bumpType = BumpType;
    pushData->bumpPercentage = BumpReductionPercentage;
    pushData->flatBumpStrength = FlatBumpStrength;
    pushData->forcePushStop = QBoolean(!BypassPushStop && ForcePushStop);
    pushData->forcePushStopStrength = ForcePushStopStrength;
    pushData->onShieldBehaviour = OnShieldBehaviour;
}

} // namespace Quantum
