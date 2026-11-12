// Headless platform backend: no window or input; real time and real files.
#include "../common/common.h"

namespace rr::platform {
namespace {

class NullPlatform final : public Platform {
public:
    explicit NullPlatform(const PlatformConfig& config)
        : files_(config.contentRoot, config.userDataRoot), frameLimit_(config.frameLimit) {}

    bool CreateMainWindow(const WindowDesc& desc) override {
        // There is nothing to render into, so only "no graphics" can succeed.
        return desc.api == GraphicsApi::None;
    }

    bool PumpEvents() override {
        return frameLimit_ == 0 || framesPumped_++ < frameLimit_;
    }

    void GetDrawableSize(int& width, int& height) const override { width = height = 0; }

    const InputState& GetInput() const override { return input_; }
    const Clock& GetClock() const override { return clock_; }
    const FileSystem& GetFileSystem() const override { return files_; }
    OpenGLContext* GetOpenGLContext() override { return nullptr; }
    const VulkanSurfaceProvider* GetVulkanSurfaceProvider() const override { return nullptr; }

private:
    detail::SteadyClock clock_;
    detail::DirectoryFileSystem files_;
    InputState input_;
    std::uint64_t frameLimit_;
    std::uint64_t framesPumped_ = 0;
};

} // namespace

std::unique_ptr<Platform> CreateNullPlatform(const PlatformConfig& config) {
    return std::make_unique<NullPlatform>(config);
}

} // namespace rr::platform
