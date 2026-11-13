// query_options.h - Quantum.QueryOptions (quantum.core.dll): what a physics query (ray,
// line or shape cast, overlap) may hit and whether it computes detailed hit information.
#pragma once

#include <cstdint>

namespace Quantum {

enum class QueryOptions : std::int32_t {
    HitStatics = 1,
    HitKinematics = 2,
    HitDynamics = 4,
    HitTriggers = 8,
    HitAll = 15,
    ComputeDetailedInfo = 32768,
};

constexpr QueryOptions operator|(QueryOptions a, QueryOptions b) {
    return static_cast<QueryOptions>(static_cast<std::int32_t>(a) | static_cast<std::int32_t>(b));
}
constexpr bool HasFlag(QueryOptions options, QueryOptions flag) {
    return (static_cast<std::int32_t>(options) & static_cast<std::int32_t>(flag)) != 0;
}

} // namespace Quantum
