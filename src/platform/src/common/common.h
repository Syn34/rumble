// Pieces shared by every platform backend.
#pragma once

#include "rr/platform/platform.h"

#include <chrono>

namespace rr::platform::detail {

class SteadyClock final : public Clock {
public:
    double Now() const override {
        const auto elapsed = std::chrono::steady_clock::now() - start_;
        return std::chrono::duration<double>(elapsed).count();
    }

private:
    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();
};

// Content comes from a folder on disk; user data goes to another folder.
class DirectoryFileSystem final : public FileSystem {
public:
    DirectoryFileSystem(std::filesystem::path contentRoot, std::filesystem::path userRoot);

    std::optional<std::vector<std::uint8_t>> ReadContent(const std::string& path) const override;
    std::filesystem::path UserDataDirectory() const override { return userRoot_; }

private:
    std::filesystem::path contentRoot_;
    std::filesystem::path userRoot_;
};

} // namespace rr::platform::detail
