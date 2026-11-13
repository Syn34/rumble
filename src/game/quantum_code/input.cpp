// input.cpp - Quantum.Input.
#include "quantum_code/input.h"

#include "photon_deterministic/fp_math.h"

#include <stdexcept>

namespace Quantum {

namespace {

constexpr FP OrbitOffsetMax = FP::FromRaw(std::int64_t{16} << 16);

// FPQuaternion.Euler(pitch, yaw, 0) * Forward.
FPVector3 ViewDirection(const EncodedDirection3& direction) {
    return direction.get_Rotation() * FPVector3::Forward();
}

} // namespace

FPVector2 Input::get_Direction() const {
    if (EncodedDirection == 0) {
        return FPVector2{};
    }
    const std::int64_t degrees = (std::int64_t{EncodedDirection} - 1) * 2;
    return FPVector2::Rotate(FPVector2::Up(), FP::FromRaw(FP::Deg2Rad().RawValue() * degrees));
}

void Input::set_Direction(FPVector2 value) {
    if (value == FPVector2{}) {
        EncodedDirection = 0;
        return;
    }
    const FP fullTurn = FP::FromRaw(std::int64_t{360} << 16);
    const FP degrees = FPVector2::RadiansSigned(FPVector2::Up(), value) * FP::Rad2Deg() + fullTurn;
    const FP wrapped = FP::FromRaw(degrees.RawValue() % fullTurn.RawValue());
    const FP halved = FP::FromRaw(wrapped.RawValue() / 2);
    EncodedDirection = static_cast<std::uint8_t>((halved + FP::One()).RawValue() >> 16);
}

FPVector3 Input::get_CameraSight() const { return ViewDirection(EncodedCameraSight); }

FPVector3 Input::GetCameraPosition(const Transform3D* transform) const {
    if (transform == nullptr) {
        throw std::invalid_argument("Input::GetCameraPosition: null transform");
    }
    const FPVector3 direction = ViewDirection(EncodedCameraOrbitDirection);
    return transform->Position + get_CameraOrbitOffset() * direction;
}

FP Input::get_CameraOrbitOffset() const {
    if (EncodedCameraOrbitOffset == 0) {
        return FP{};
    }
    if (EncodedCameraOrbitOffset == 255) {
        return OrbitOffsetMax;
    }
    // FP(byte) / 255 * 16, in raw integer steps as the original.
    return FP::FromRaw(((std::int64_t{EncodedCameraOrbitOffset} << 16) / 255) * 16);
}

void Input::set_CameraOrbitOffset(FP value) {
    FP fraction{};
    if (value > FP{}) {
        fraction = value < OrbitOffsetMax ? FP::FromRaw(value.RawValue() >> 4) : FP::One();
    }
    fraction = Photon::Deterministic::FPMath::Clamp01(fraction);
    EncodedCameraOrbitOffset = static_cast<std::uint8_t>((fraction.RawValue() * 255) >> 16);
}

FPVector3 Input::get_LookInputVector() const { return ViewDirection(EncodedLookInputVector); }

void Input::set_LookInputVector(const FPVector3& value) {
    EncodedLookInputVector.set_Rotation(FPQuaternion::LookRotation(value, false));
}

std::int32_t Input::GetHashCode() const {
    return HashBuilder(1601u)
        .Add(Dash)
        .Add(EncodedCameraOrbitDirection)
        .Add(EncodedCameraOrbitOffset)
        .Add(EncodedCameraSight)
        .Add(EncodedDirection)
        .Add(EncodedLookInputVector)
        .Add(Fire)
        .Add(Gadget)
        .Add(Jump)
        .Add(Melee)
        .Add(Reload)
        .Add(Ultimate)
        .Value();
}

void Input::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Input*>(ptr);
    SerializeField(&p->EncodedCameraOrbitOffset, serializer);
    SerializeField(&p->EncodedDirection, serializer);
    SerializeField(&p->EncodedCameraOrbitDirection, serializer);
    SerializeField(&p->EncodedCameraSight, serializer);
    SerializeField(&p->EncodedLookInputVector, serializer);
    SerializeField(&p->Dash, serializer);
    SerializeField(&p->Fire, serializer);
    SerializeField(&p->Gadget, serializer);
    SerializeField(&p->Jump, serializer);
    SerializeField(&p->Melee, serializer);
    SerializeField(&p->Reload, serializer);
    SerializeField(&p->Ultimate, serializer);
}

Input Input::Read(FrameSerializer* serializer) {
    Input input{};
    Serialize(&input, serializer);
    return input;
}

void Input::Write(FrameSerializer* serializer, Input input) { Serialize(&input, serializer); }

} // namespace Quantum
