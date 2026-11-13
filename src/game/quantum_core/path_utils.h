// path_utils.h - Quantum.PathUtils (quantum.core.dll): asset path string helpers.
//
// Paths use '/' separators. Comparisons the original makes with
// StringComparison.OrdinalIgnoreCase fold ASCII letters only here (asset paths are ASCII).
#pragma once

#include <string>

namespace Quantum::PathUtils {

// 0x7d3feb0 - the part of `path` below `folder` ("" when path is the folder itself or
// ends with it); false when the folder is not part of the path.
bool MakeRelativeToFolder(const std::string& path, const std::string& folder, std::string& result);

// 0x7d40128 - everything before the last '.'.
std::string GetPathWithoutExtension(const std::string& path);

// 0x7d3ffe4 - backslashes to '/', "//" to '/' (one pass), separators trimmed from both
// ends.
std::string MakeSane(const std::string& path);

} // namespace Quantum::PathUtils
