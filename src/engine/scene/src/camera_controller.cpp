#include "rr/scene/camera_controller.h"

#include <algorithm>
#include <cmath>

namespace rr::scene {

namespace {

using platform::Key;
using platform::MouseButton;

constexpr float kMaxPitch = 1.55f;

} // namespace

void CameraController::Frame(const Aabb& bounds) {
    framed_ = bounds.Valid() ? bounds : Aabb::FromCenterExtent({}, {1, 1, 1});
    target_ = framed_.Center();
    radius_ = std::max(Length(framed_.Extent()), 0.05f);
    distance_ = radius_ / std::tan(fovY * 0.5f) * 1.1f;
    yaw_ = 0.6f;
    pitch_ = 0.35f;
    flySpeed_ = std::max(radius_ * 0.5f, 0.5f);
    position_ = Eye();
}

Vec3 CameraController::Forward() const {
    return {-std::cos(pitch_) * std::sin(yaw_), -std::sin(pitch_), -std::cos(pitch_) * std::cos(yaw_)};
}

Vec3 CameraController::Eye() const { return target_ - Forward() * distance_; }

void CameraController::Update(const platform::InputState& input, float dt) {
    const float dx = haveMouse_ ? input.mouseX - lastX_ : 0.0f;
    const float dy = haveMouse_ ? input.mouseY - lastY_ : 0.0f;
    lastX_ = input.mouseX;
    lastY_ = input.mouseY;
    haveMouse_ = true;
    const auto held = [&](MouseButton b) { return input.Get(b).down && !input.Get(b).pressed; };

    if (input.Get(Key::F).pressed) {
        if (mode == Mode::Orbit) {
            position_ = Eye();
            mode = Mode::Fly;
        } else {
            target_ = position_ + Forward() * distance_;
            mode = Mode::Orbit;
        }
    }
    if (input.Get(Key::R).pressed) {
        Frame(framed_);
        mode = Mode::Orbit;
    }

    const Vec3 forward = Forward();
    const Vec3 right = Normalize(Cross(forward, {0, 1, 0}));
    const Vec3 up = Cross(right, forward);

    if (mode == Mode::Orbit) {
        if (held(MouseButton::Left)) {
            yaw_ -= dx * 0.01f;
            pitch_ = std::clamp(pitch_ + dy * 0.01f, -kMaxPitch, kMaxPitch);
        }
        if (held(MouseButton::Right) || held(MouseButton::Middle)) {
            const float scale = distance_ * 0.0015f;
            target_ = target_ - right * (dx * scale) + up * (dy * scale);
        }
        distance_ = std::clamp(distance_ * std::pow(0.88f, input.mouseWheel), radius_ * 0.02f, radius_ * 200.0f);
    } else {
        if (held(MouseButton::Right) || held(MouseButton::Left)) {
            yaw_ -= dx * 0.005f;
            pitch_ = std::clamp(pitch_ - dy * 0.005f, -kMaxPitch, kMaxPitch);
        }
        if (input.mouseWheel != 0.0f) flySpeed_ *= std::pow(1.2f, input.mouseWheel);
        float speed = flySpeed_ * dt;
        if (input.Get(Key::LeftShift).down || input.Get(Key::RightShift).down) speed *= 4.0f;
        Vec3 move;
        if (input.Get(Key::W).down) move = move + forward;
        if (input.Get(Key::S).down) move = move - forward;
        if (input.Get(Key::D).down) move = move + right;
        if (input.Get(Key::A).down) move = move - right;
        if (input.Get(Key::E).down) move = move + Vec3{0, 1, 0};
        if (input.Get(Key::Q).down) move = move - Vec3{0, 1, 0};
        position_ = position_ + move * speed;
    }
}

Camera CameraController::GetCamera() const {
    Camera c;
    c.fovY = fovY;
    const Vec3 eye = mode == Mode::Orbit ? Eye() : position_;
    c.view = LookAt(eye, eye + Forward(), {0, 1, 0});
    // Depth range sized to the scene and the distance to it.
    const float reach = Length(eye - target_) + radius_ * 4.0f;
    c.nearPlane = std::max(reach * 0.0005f, 0.01f);
    c.farPlane = std::max(reach * 4.0f, c.nearPlane * 10.0f);
    return c;
}

} // namespace rr::scene
