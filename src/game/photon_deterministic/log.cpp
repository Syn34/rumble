#include "log.h"

#include <cctype>
#include <mutex>

namespace Photon::Deterministic {

namespace {

struct LogState {
    std::mutex sync;
    Log::MessageCallback info;
    Log::MessageCallback warn;
    Log::MessageCallback error;
    Log::ExceptionCallback exn;
};

LogState& State() {
    static LogState state;
    return state;
}

} // namespace

void Log::Init(MessageCallback info, MessageCallback warn, MessageCallback error,
               ExceptionCallback exn) {
    LogState& state = State();
    std::lock_guard<std::mutex> lock(state.sync);
    state.info = std::move(info);
    state.warn = std::move(warn);
    state.error = std::move(error);
    state.exn = std::move(exn);
}

void Log::Write(Level level, std::string_view format, const std::vector<std::string>& args) {
    LogState& state = State();
    const MessageCallback* callback = nullptr;
    switch (level) {
    case Level::Info:
        callback = &state.info;
        break;
    case Level::Warn:
        callback = &state.warn;
        break;
    case Level::Error:
        callback = &state.error;
        break;
    }
    if (!*callback) {
        return;
    }
    std::lock_guard<std::mutex> lock(state.sync);
    (*callback)(FormatComposite(format, args));
}

void Log::Exception(const std::exception& exn) {
    LogState& state = State();
    if (!state.exn) {
        return;
    }
    std::lock_guard<std::mutex> lock(state.sync);
    state.exn(exn);
}

std::string Log::FormatComposite(std::string_view format, const std::vector<std::string>& args) {
    std::string out;
    out.reserve(format.size());
    for (std::size_t i = 0; i < format.size(); ++i) {
        const char c = format[i];
        if (c == '{' && i + 1 < format.size() && format[i + 1] == '{') {
            out += '{';
            ++i;
            continue;
        }
        if (c == '}' && i + 1 < format.size() && format[i + 1] == '}') {
            out += '}';
            ++i;
            continue;
        }
        if (c == '{') {
            std::size_t end = i + 1;
            std::size_t index = 0;
            bool digits = false;
            while (end < format.size() && std::isdigit(static_cast<unsigned char>(format[end]))) {
                index = index * 10 + static_cast<std::size_t>(format[end] - '0');
                digits = true;
                ++end;
            }
            if (digits && end < format.size() && format[end] == '}' && index < args.size()) {
                out += args[index];
                i = end;
                continue;
            }
        }
        out += c;
    }
    return out;
}

} // namespace Photon::Deterministic
