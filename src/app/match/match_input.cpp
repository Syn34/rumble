#include "match_input.h"

#include <algorithm>
#include <cmath>

namespace rr::app {

namespace {

using platform::GamepadAxis;
using platform::GamepadButton;
using platform::Key;
using platform::MouseButton;
using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;

constexpr float kDegreesPerPixel = 0.25f;
constexpr float kStickDegreesPerSecond = 150.0f;
constexpr float kStickDeadZone = 0.2f;

float DeadZone(float v) { return std::fabs(v) < kStickDeadZone ? 0.0f : v; }

} // namespace

void InputMapper::UpdateCamera(const platform::InputState& input, float seconds) {
    const bool drag = input.Get(MouseButton::Right).down;
    if (drag && dragging_) {
        camera.yawDegrees += (input.mouseX - lastMouseX_) * kDegreesPerPixel;
        camera.pitchDegrees += (input.mouseY - lastMouseY_) * kDegreesPerPixel;
    }
    dragging_ = drag;
    lastMouseX_ = input.mouseX;
    lastMouseY_ = input.mouseY;
    if (input.gamepadConnected) {
        camera.yawDegrees += DeadZone(input.Get(GamepadAxis::RightX)) * kStickDegreesPerSecond * seconds;
        camera.pitchDegrees += DeadZone(input.Get(GamepadAxis::RightY)) * kStickDegreesPerSecond * seconds;
    }
    camera.distance = std::clamp(camera.distance - input.mouseWheel * 0.75f, 3.0f, 30.0f);
    camera.pitchDegrees = std::clamp(camera.pitchDegrees, -10.0f, 75.0f);
    camera.yawDegrees = std::fmod(camera.yawDegrees, 360.0f);
}

Quantum::PolledInput InputMapper::Poll(const platform::InputState& input) const {
    Quantum::PolledInput polled;
    polled.Flags = Photon::Deterministic::DeterministicInputFlags::Repeatable;
    Quantum::Input& in = polled.Input;

    // Movement in camera space: x right, y forward.
    float x = 0.0f, y = 0.0f;
    if (input.Get(Key::A).down || input.Get(Key::Left).down) x -= 1.0f;
    if (input.Get(Key::D).down || input.Get(Key::Right).down) x += 1.0f;
    if (input.Get(Key::W).down || input.Get(Key::Up).down) y += 1.0f;
    if (input.Get(Key::S).down || input.Get(Key::Down).down) y -= 1.0f;
    if (input.gamepadConnected) {
        x += DeadZone(input.Get(GamepadAxis::LeftX));
        y -= DeadZone(input.Get(GamepadAxis::LeftY)); // stick Y is down-positive
    }
    if (x != 0.0f || y != 0.0f) {
        // Rotate by the camera yaw into the game's world (left-handed, Y up): forward is
        // (sin yaw, cos yaw), right is (cos yaw, -sin yaw) on the XZ plane.
        const float yaw = camera.yawDegrees * 3.14159265f / 180.0f;
        const float wx = x * std::cos(yaw) + y * std::sin(yaw);
        const float wz = y * std::cos(yaw) - x * std::sin(yaw);
        in.set_Direction(FPVector2(FP::FromFloat_UNSAFE(wx), FP::FromFloat_UNSAFE(wz)));
    }
    in.EncodedCameraSight.set_Yaw(FP::FromFloat_UNSAFE(camera.yawDegrees));
    in.EncodedCameraSight.set_Pitch(FP::FromFloat_UNSAFE(camera.pitchDegrees));
    in.EncodedCameraOrbitDirection = in.EncodedCameraSight;
    in.set_CameraOrbitOffset(FP::FromFloat_UNSAFE(std::min(camera.distance, 16.0f)));

    const bool pad = input.gamepadConnected;
    const auto button = [](bool down) { return Quantum::Button::FromBool(down); };
    in.Jump = button(input.Get(Key::Space).down || (pad && input.Get(GamepadButton::South).down));
    in.Dash = button(input.Get(Key::LeftShift).down || (pad && input.Get(GamepadButton::East).down));
    in.Melee = button(input.Get(Key::F).down || (pad && input.Get(GamepadButton::West).down));
    in.Reload = button(input.Get(Key::R).down || (pad && input.Get(GamepadButton::North).down));
    in.Fire = button(input.Get(MouseButton::Left).down || (pad && input.Get(GamepadAxis::RightTrigger) > 0.5f));
    in.Gadget = button(input.Get(Key::Q).down || (pad && input.Get(GamepadAxis::LeftTrigger) > 0.5f));
    in.Ultimate = button(input.Get(Key::E).down || (pad && input.Get(GamepadButton::RightShoulder).down));
    return polled;
}

} // namespace rr::app
