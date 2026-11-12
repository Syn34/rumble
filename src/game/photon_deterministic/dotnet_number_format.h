// dotnet_number_format.h - text formatting of float values the way the original's .NET
// runtime does it, for the ToString methods of the deterministic maths types.
//
// The original converts each FP to float and formats it with the invariant culture.
// That runtime formats a float from its 7 most significant decimal digits ("G" picks
// fixed or scientific notation, "F<n>" rounds those 7 digits again to n decimals).
// Uncertain: this assumes the pre-.NET Core 3.0 behaviour (7 digits rather than the
// shortest round-trip form); the two only differ for values needing 8+ digits.
// ToString output is for logs and debugging only; the simulation never parses it.
#pragma once

#include <string>

namespace Photon::Deterministic::detail {

// float.ToString(CultureInfo.InvariantCulture), i.e. format "G" with 7 digits.
std::string FormatSingle(float value);

// float.ToString("F<decimals>", CultureInfo.InvariantCulture).
std::string FormatSingleFixed(float value, int decimals);

} // namespace Photon::Deterministic::detail
