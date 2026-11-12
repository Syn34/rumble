// Headless platform backend: no window, no input, real time and real files.
#include "rr/platform/platform.h"

#include <chrono>
#include <fstream>
#include <iterator>

namespace rr::platform {
namespace {

class SteadyClock final : public Clock {
public:
    double Now() const override {
        const auto elapsed = std::chrono::steady_clock::now() - start_;
        return std::chrono::duration<double>(elapsed).count();
    }

private:
    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();
};

class DirectoryFileSystem final : public FileSystem {
public:
    DirectoryFileSystem(std::filesystem::path contentRoot, std::filesystem::path userRoot)
        : contentRoot_(std::move(contentRoot)), userRoot_(std::move(userRoot)) {}

    std::optional<std::vector<std::uint8_t>> ReadContent(const std::string& path) const override {
        std::ifstream file(contentRoot_ / path, std::ios::binary);
        if (!file) {
            return std::nullopt;
        }
        return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(file), {});
    }

    std::filesystem::path UserDataDirectory() const override { return userRoot_; }

private:
    std::filesystem::path contentRoot_;
    std::filesystem::path userRoot_;
};

class NullPlatform final : public Platform {
public:
    explicit NullPlatform(const PlatformConfig& config)
        : files_(config.contentRoot, config.userDataRoot), frameLimit_(config.headlessFrameLimit) {}

    bool CreateMainWindow(const WindowDesc&) override { return true; }

    bool PumpEvents() override {
        if (frameLimit_ == 0) {
            return true;
        }
        return framesPumped_++ < frameLimit_;
    }

    void Present() override {}

    const Clock& GetClock() const override { return clock_; }
    const FileSystem& GetFileSystem() const override { return files_; }

private:
    SteadyClock clock_;
    DirectoryFileSystem files_;
    std::uint64_t frameLimit_;
    std::uint64_t framesPumped_ = 0;
};

} // namespace

std::unique_ptr<Platform> CreateNullPlatform(const PlatformConfig& config) {
    return std::make_unique<NullPlatform>(config);
}

} // namespace rr::platform
