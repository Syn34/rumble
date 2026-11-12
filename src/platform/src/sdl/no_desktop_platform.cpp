// Used when the project is built without the SDL desktop backend.
#include "rr/platform/platform.h"

namespace rr::platform {

std::unique_ptr<Platform> CreateDesktopPlatform(const PlatformConfig&) {
    return nullptr;
}

} // namespace rr::platform
