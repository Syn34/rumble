// log.h - Photon.Deterministic.Log / DeterministicLog: the runtime's logging hook.
//
// The library does not print anything itself; the host installs four callbacks (info,
// warning, error, exception) and every message is formatted and handed to the matching
// callback under a global lock. Messages without an installed callback are dropped.
//
// Formats use .NET composite syntax ("{0}", "{1}", "{{" / "}}" for literal braces).
#pragma once

#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace Photon::Deterministic {

class Log final {
  public:
    using MessageCallback = std::function<void(const std::string&)>;
    using ExceptionCallback = std::function<void(const std::exception&)>;

    Log() = delete;

    // 0x7a88108
    static void Init(MessageCallback info, MessageCallback warn, MessageCallback error,
                     ExceptionCallback exn);

    // 0x7a7dfc4
    template <typename... Args> static void Info(std::string_view format, const Args&... args) {
        Write(Level::Info, format, {ToText(args)...});
    }
    // 0x7a85d50
    template <typename... Args> static void Warn(std::string_view format, const Args&... args) {
        Write(Level::Warn, format, {ToText(args)...});
    }
    // 0x7a79698
    template <typename... Args> static void Error(std::string_view format, const Args&... args) {
        Write(Level::Error, format, {ToText(args)...});
    }
    // 0x7a7dea4
    static void Exception(const std::exception& exn);

    // Expands a composite format string; unknown indices are left as written.
    static std::string FormatComposite(std::string_view format,
                                       const std::vector<std::string>& args);

  private:
    enum class Level { Info, Warn, Error };

    template <typename T> static std::string ToText(const T& value) {
        std::ostringstream out;
        out << value;
        return out.str();
    }

    static void Write(Level level, std::string_view format, const std::vector<std::string>& args);
};

class DeterministicLog final {
  public:
    DeterministicLog() = delete;

    // 0x7a88104 - same as Log::Init.
    static void Init(Log::MessageCallback info, Log::MessageCallback warn,
                     Log::MessageCallback error, Log::ExceptionCallback exn) {
        Log::Init(std::move(info), std::move(warn), std::move(error), std::move(exn));
    }
};

} // namespace Photon::Deterministic
