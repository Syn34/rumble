// match_input.h - the local player's input (PC glue): keyboard, mouse and gamepad state from
// the platform layer turned into the game's per-tick Quantum::Input.
//
// TODO: the original maps devices to game actions through the Rewired input library
// (Rewired_Core.dll) with the game's own controller maps and touch controls. Until that is
// reimplemented, these fixed default bindings stand in for it:
//
//   move        W A S D / arrows, left stick      jump      Space, A (south)
//   camera      right mouse drag, right stick     dash      Left Shift, B (east)
//   fire        left mouse, right trigger         melee     F, X (west)
//   gadget      Q, left trigger                   reload    R, Y (north)
//   ultimate    E, right shoulder
#pragma once

#include "quantum_code/quantum_game.h"
#include "rr/platform/input.h"

namespace rr::app {

struct CameraAngles {
    float yawDegrees = 0.0f;    // around the up axis; 0 looks along +Z (game convention)
    float pitchDegrees = 25.0f; // looking down
    float distance = 9.0f;      // orbit distance behind the followed actor
};

class InputMapper {
public:
    // Orbits the camera (mouse drag, right stick, wheel for distance).
    void UpdateCamera(const platform::InputState& input, float seconds);
    // The input of one tick, with movement relative to the camera's yaw.
    Quantum::PolledInput Poll(const platform::InputState& input) const;

    CameraAngles camera;

private:
    float lastMouseX_ = 0.0f, lastMouseY_ = 0.0f;
    bool dragging_ = false;
};

} // namespace rr::app
