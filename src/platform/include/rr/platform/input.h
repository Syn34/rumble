// input.h - raw device state, refreshed once per frame by the platform.
//
// This is the lowest input layer: physical keys, buttons and axes. Mapping
// physical inputs to game actions belongs to the input-mapping layer above it.
#pragma once

#include <array>
#include <cstdint>

namespace rr::platform {

enum class Key : std::uint8_t {
    Unknown,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Escape, Enter, Space, Tab, Backspace,
    Left, Right, Up, Down,
    LeftShift, RightShift, LeftCtrl, RightCtrl, LeftAlt, RightAlt,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Count
};

enum class MouseButton : std::uint8_t { Left, Right, Middle, Count };

enum class GamepadButton : std::uint8_t {
    South, East, West, North, // A/B/X/Y on an Xbox-style pad
    LeftShoulder, RightShoulder,
    LeftStick, RightStick,
    Back, Start,
    DPadUp, DPadDown, DPadLeft, DPadRight,
    Count
};

enum class GamepadAxis : std::uint8_t {
    LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger, Count
};

struct ButtonState {
    bool down = false;
    bool pressed = false;  // went down this frame
    bool released = false; // went up this frame

    void Set(bool isDown) {
        pressed = isDown && !down;
        released = !isDown && down;
        down = isDown;
    }
    void ClearEdges() { pressed = released = false; }
};

struct InputState {
    std::array<ButtonState, static_cast<std::size_t>(Key::Count)> keys{};
    std::array<ButtonState, static_cast<std::size_t>(MouseButton::Count)> mouseButtons{};
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    float mouseWheel = 0.0f; // notches scrolled this frame

    bool gamepadConnected = false;
    std::array<ButtonState, static_cast<std::size_t>(GamepadButton::Count)> gamepadButtons{};
    // Sticks in [-1, 1] (Y down is positive), triggers in [0, 1].
    std::array<float, static_cast<std::size_t>(GamepadAxis::Count)> gamepadAxes{};

    const ButtonState& Get(Key k) const { return keys[static_cast<std::size_t>(k)]; }
    const ButtonState& Get(MouseButton b) const { return mouseButtons[static_cast<std::size_t>(b)]; }
    const ButtonState& Get(GamepadButton b) const {
        return gamepadButtons[static_cast<std::size_t>(b)];
    }
    float Get(GamepadAxis a) const { return gamepadAxes[static_cast<std::size_t>(a)]; }
};

} // namespace rr::platform
