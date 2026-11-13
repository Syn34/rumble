// match_app.h - `RangerRumble --match ...`: an offline match, headless or in a window.
#pragma once

#include "game_content.h"
#include "match_setup.h"

#include "rr/platform/platform.h"
#include "rr/render/render_device.h"

#include <cstdint>
#include <string>

namespace rr::app {

struct MatchCommand {
    MatchOptions match;
    ContentOptions content;
    std::uint64_t ticks = 600;   // headless: ticks to run
    std::uint64_t frames = 0;    // windowed: quit after this many frames (fixed time step)
    bool quiet = false;          // headless: only the summary lines
    bool checkViews = false;     // headless: also instantiate every EntityView prefab (numbers)
};

// Runs `ticks` ticks without a window and prints numbers only (entities per component,
// systems, events, a checksum every 60 ticks). Returns 0 when the match ran.
int RunMatchHeadless(const MatchCommand& command);

// Plays the match in the window: the user's input drives player slot 0, the camera follows
// that player's actor, entities are drawn with their view prefabs.
int RunMatchWindowed(const MatchCommand& command, platform::Platform& platform, render::RenderDevice& device);

} // namespace rr::app
