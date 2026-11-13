// navigation_constants.h - Quantum.Navigation.Constants (quantum.core.dll).
//
// Static (and in the original, writable) sizes and thresholds of the navigation
// system. The values are set by the class constructor.
#pragma once

#include "photon_deterministic/fp.h"

#include <cstdint>

namespace Quantum::NavigationConstants {

// 0x7d26604 Constants..cctor - the initial values below.
inline int PathFinderSize = 1024;         // open-list size of each thread's PathFinder
inline int RawPathSize = 256;             // default raw path capacity
inline int ActiveVertexDefaultSize = 128; // A* vertices per pool block
inline int FunnelBufferSize = 128;        // initial funnel portal buffer size
inline int MaxFunnelPathSize = 256;       // maximum funnelled path length
inline std::uint8_t MaxWaypoints = 32;
inline Photon::Deterministic::FP MinStoppingDistance = Photon::Deterministic::FP::EN3();
inline bool GenerateFunnelPathNormals = false;

} // namespace Quantum::NavigationConstants
