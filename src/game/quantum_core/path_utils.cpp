#include "quantum_core/path_utils.h"

#include <algorithm>
#include <cctype>

namespace Quantum::PathUtils {

namespace {

// One left-to-right pass of String.Replace.
std::string ReplaceAll(const std::string& text, const std::string& from, const std::string& to) {
    std::string result;
    std::size_t start = 0;
    while (true) {
        const std::size_t found = text.find(from, start);
        if (found == std::string::npos) {
            result.append(text, start, std::string::npos);
            return result;
        }
        result.append(text, start, found - start);
        result += to;
        start = found + from.size();
    }
}

char FoldCase(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

bool EqualsIgnoreCase(const std::string& a, const std::string& b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
               return FoldCase(x) == FoldCase(y);
           });
}

std::size_t FindIgnoreCase(const std::string& text, const std::string& pattern) {
    const auto it = std::search(text.begin(), text.end(), pattern.begin(), pattern.end(),
                                [](char x, char y) { return FoldCase(x) == FoldCase(y); });
    return it == text.end() && !pattern.empty() ? std::string::npos
                                                : static_cast<std::size_t>(it - text.begin());
}

} // namespace

// 0x7d3feb0
bool MakeRelativeToFolder(const std::string& path, const std::string& folder, std::string& result) {
    result.clear();
    const std::string sane = MakeSane(path);
    if (EqualsIgnoreCase(sane, folder)) {
        return true;
    }
    // String.EndsWith(string) is culture-sensitive in .NET; ordinal here.
    const std::string suffix = "/" + folder;
    if (sane.size() >= suffix.size() &&
        sane.compare(sane.size() - suffix.size(), suffix.size(), suffix) == 0) {
        return true;
    }
    const std::size_t index = FindIgnoreCase(sane, folder + "/");
    if (index == std::string::npos || sane.size() < folder.size() + 1) {
        return false;
    }
    const std::size_t start = index + folder.size() + 1;
    result = sane.substr(start, sane.size() - folder.size() - 1 - index);
    return true;
}

// 0x7d40128
std::string GetPathWithoutExtension(const std::string& path) {
    const std::size_t dot = path.rfind('.');
    return dot == std::string::npos ? path : path.substr(0, dot);
}

// 0x7d3ffe4
std::string MakeSane(const std::string& path) {
    std::string result = ReplaceAll(ReplaceAll(path, "\\", "/"), "//", "/");
    const auto isSeparator = [](char c) { return c == '\\' || c == '/'; };
    while (!result.empty() && isSeparator(result.back())) {
        result.pop_back();
    }
    std::size_t first = 0;
    while (first < result.size() && isSeparator(result[first])) {
        ++first;
    }
    return result.substr(first);
}

} // namespace Quantum::PathUtils
