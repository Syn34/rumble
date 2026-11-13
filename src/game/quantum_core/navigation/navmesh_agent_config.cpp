#include "quantum_core/navigation/navmesh_agent_config.h"

#include "quantum_core/navigation/navigation_constants.h"

namespace Quantum {

// 0x7ce51d0
void NavMeshAgentConfig::Loaded(IResourceManager* /*resourceManager*/, Allocator* /*allocator*/) {
    FP stoppingDistance = StoppingDistance;
    if (stoppingDistance < NavigationConstants::MinStoppingDistance) {
        stoppingDistance = NavigationConstants::MinStoppingDistance;
    }
    StoppingDistanceSqrRaw = (stoppingDistance * stoppingDistance).RawValue();
    AutoBrakingDistanceSqrRaw = (AutoBrakingDistance * AutoBrakingDistance).RawValue();

    if (CachedWaypointCount <= 2) {
        CachedWaypointCount = 3;
    }
    if (CachedWaypointCount > NavigationConstants::MaxWaypoints) {
        CachedWaypointCount = NavigationConstants::MaxWaypoints;
    }
}

} // namespace Quantum
