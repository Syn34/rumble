// camera_controller.h - orbit / fly camera driven by keyboard and mouse, for
// viewers and debugging.
//
//   orbit: left drag rotates around the target, right or middle drag pans,
//          wheel zooms
//   fly:   right (or left) drag looks around, W/A/S/D move, Q/E down/up,
//          Shift moves faster, wheel changes the speed
//   F toggles between the two (keeping the view), R reframes the scene.
#pragma once

#include "rr/platform/input.h"
#include "rr/scene/math.h"
#include "rr/scene/scene_renderer.h"

namespace rr::scene {

class CameraController {
public:
    enum class Mode { Orbit, Fly };

    // Points the orbit camera at `bounds` from a three-quarter view.
    void Frame(const Aabb& bounds);
    void Update(const platform::InputState& input, float dt);
    Camera GetCamera() const;

    Mode mode = Mode::Orbit;
    float fovY = 0.9f;

private:
    Vec3 Eye() const;
    Vec3 Forward() const; // view direction

    Aabb framed_;
    Vec3 target_;
    Vec3 position_; // fly mode
    float yaw_ = 0.6f;
    float pitch_ = 0.35f;
    float distance_ = 5.0f;
    float radius_ = 1.0f;
    float flySpeed_ = 2.0f;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;
    bool haveMouse_ = false;
};

} // namespace rr::scene
