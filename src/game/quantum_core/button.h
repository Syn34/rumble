// button.h - Quantum.Button: the state of one digital input across frames.
//
// A button remembers the frame it last went down, the frame it last went up and the
// frame of the latest update. It is down while the down frame is newer than the up
// frame; "pressed"/"released" mean the change happened in the latest update.
#pragma once

#include <cstdint>
#include <string>

namespace Quantum {

class FrameSerializer;

struct Button {
    static constexpr int SIZE = 12;

    std::int32_t _frameUp = 0;
    std::int32_t _frameDown = 0;
    std::int32_t _frameCurrent = 0;

    // 0x7cea0c8
    constexpr bool IsDown() const { return _frameDown > _frameUp; }
    // 0x7cea0d8 - Went down in the latest update.
    constexpr bool WasPressed() const { return _frameDown > 0 && _frameDown == _frameCurrent; }
    // 0x7cea0ec - Went up in the latest update (after having been down).
    constexpr bool WasReleased() const {
        return _frameUp == _frameCurrent && _frameUp > _frameDown;
    }

    // 0x7cea114 - Records the state for `currentFrame`: a change of state stamps the frame
    // into the down/up slot (a button that has never been down counts as up).
    // Returns the updated state.
    Button Update(int currentFrame, bool isDown);

    // 0x7cea15c - Button -> bool (IsDown).
    constexpr explicit operator bool() const { return IsDown(); }
    // 0x7cea16c - bool -> Button: a state that is down (or up) since frame 1.
    static constexpr Button FromBool(bool isDown) {
        Button button;
        button._frameUp = isDown ? 0 : 1;
        button._frameDown = isDown ? 1 : 0;
        button._frameCurrent = 1;
        return button;
    }

    // 0x7cea880
    std::int32_t GetHashCode() const;

    // 0x7cea624 - Debug text (wording is this project's own).
    std::string ToString() const;

    // 0x7cea180 - In input mode only IsDown travels (one bool; reading rebuilds the state
    // as FromBool would). Otherwise the three frame numbers are written/read in order.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

static_assert(sizeof(Button) == Button::SIZE);

} // namespace Quantum
