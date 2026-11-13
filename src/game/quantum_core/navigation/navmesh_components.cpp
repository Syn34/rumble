#include "quantum_core/navigation/navmesh_components.h"

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/deterministic_frame_serializer.h"
#include "photon_deterministic/fp_math.h"
#include "quantum_core/navigation/navmesh_agent_config.h"

namespace Quantum {

using Photon::Deterministic::IDeterministicFrameSerializer;

// 0x7d3099c
FP NavMeshSteeringAgent::CurrentSpeed() const {
    return Photon::Deterministic::FPMath::Sqrt(_velocity.SqrMagnitude());
}

// 0x7d309f8
void NavMeshSteeringAgent::SetConfig(const NavMeshAgentConfig& config, bool overwrite) {
    if (!_maxSpeed.HasValue() || overwrite) {
        _maxSpeed = NullableNonNegativeFP::FromFP(config.Speed);
    }
    if (!_acceleration.HasValue() || overwrite) {
        _acceleration = NullableNonNegativeFP::FromFP(config.Acceleration);
    }
}

// 0x7d30ae0
void NavMeshSteeringAgent::Serialize(void* ptr, IDeterministicFrameSerializer* serializer) {
    auto* agent = static_cast<NavMeshSteeringAgent*>(ptr);
    NullableNonNegativeFP::Serialize(&agent->_maxSpeed, *serializer);
    NullableNonNegativeFP::Serialize(&agent->_acceleration, *serializer);
    FPVector2::Serialize(&agent->_velocity, serializer);
}

// 0x7cf8418
void NavMeshAvoidanceAgent::SetConfig(const NavMeshAgentConfig& config, bool overwrite) {
    if (overwrite || (_membersInitialized & 1) == 0) {
        _membersInitialized |= 1;
        _priority = config.Priority;
    }
    if (overwrite || (_membersInitialized & 2) == 0) {
        _membersInitialized |= 2;
        _avoidanceLayer = static_cast<std::uint8_t>(config.AvoidanceLayer);
    }
    if (overwrite || (_membersInitialized & 4) == 0) {
        _membersInitialized |= 4;
        _avoidanceMask = config.AvoidanceMask;
    }
}

// 0x7cf8cd0
void NavMeshAvoidanceAgent::Serialize(void* ptr, IDeterministicFrameSerializer* serializer) {
    auto* agent = static_cast<NavMeshAvoidanceAgent*>(ptr);
    auto& stream = serializer->Stream();
    stream.Serialize(&agent->_priority);
    stream.Serialize(&agent->_avoidanceLayer);
    stream.Serialize(&agent->_avoidanceMask);
    FPVector2::Serialize(&agent->_desiredDirection, serializer);
}

// 0x7cf7c3c
void NavMeshAvoidanceObstacle::Serialize(void* ptr, IDeterministicFrameSerializer* serializer) {
    auto* obstacle = static_cast<NavMeshAvoidanceObstacle*>(ptr);
    auto& stream = serializer->Stream();
    stream.Serialize(&obstacle->_disabled);
    stream.Serialize(&obstacle->_avoidanceLayer);
    FP::Serialize(&obstacle->_radius, *serializer);
    FPVector2::Serialize(&obstacle->_velocity, serializer);
}

// 0x7cf826c
NavMeshPathfinder::WaypointList NavMeshPathfinder::GetWaypoints(const IFrameHeap& heap) const {
    WaypointList list;
    if (_waypointCapacity == 0) {
        return list;
    }
    auto* block = static_cast<std::uint8_t*>(heap.Void(_waypoints));
    list.Count = _waypointCount;
    list.Capacity = _waypointCapacity;
    list.Points = reinterpret_cast<FPVector3*>(block);
    list.Flags = block + static_cast<std::size_t>(_waypointCapacity) * sizeof(FPVector3);
    list.Links = reinterpret_cast<std::int16_t*>(list.Flags + _waypointCapacity);
    return list;
}

// 0x7cf8114
bool NavMeshPathfinder::CurrentWaypointHasFlag(const IFrameHeap& heap,
                                               WaypointFlag waypointFlag) const {
    if (_active != 1 || _waypointIndex >= _waypointCount) {
        return false;
    }
    const WaypointList list = GetWaypoints(heap);
    const std::uint8_t flags = list.Count > _waypointIndex ? list.Flags[_waypointIndex] : 0;
    return (flags & static_cast<std::uint8_t>(waypointFlag)) != 0;
}

// 0x7cf81c8
FPVector3 NavMeshPathfinder::GetWaypoint(const IFrameHeap& heap, int index) const {
    const WaypointList list = GetWaypoints(heap);
    return list.Count > index ? list.Points[index] : FPVector3::Zero();
}

// 0x7cf818c
std::uint8_t NavMeshPathfinder::GetWaypointFlags(const IFrameHeap& heap, int index) const {
    const WaypointList list = GetWaypoints(heap);
    return list.Count > index ? list.Flags[index] : std::uint8_t{0};
}

// 0x7cf82d4
int NavMeshPathfinder::GetWaypointLink(const IFrameHeap& heap, int index) const {
    const WaypointList list = GetWaypoints(heap);
    return list.Count > index ? list.Links[index] : -1;
}

// 0x7cf87e8
void NavMeshPathfinder::AddWaypoint(const IFrameHeap& heap, const FPVector3& position,
                                    std::uint8_t flags, int link) {
    const WaypointList list = GetWaypoints(heap);
    list.Points[_waypointCount] = position;
    list.Flags[_waypointCount] = flags;
    list.Links[_waypointCount] = static_cast<std::int16_t>(link);
    ++_waypointCount;
}

// 0x7cf8310
bool NavMeshPathfinder::SetWaypointsCapacity(IFrameHeap& heap, std::uint8_t newCapacity) {
    const std::uint8_t oldCapacity = _waypointCapacity;
    if (oldCapacity == newCapacity) {
        return false;
    }
    _waypointCount = 0;
    _waypointCapacity = newCapacity;
    if (_waypoints != Ptr::Null()) {
        heap.Free(_waypoints);
        _waypoints = Ptr::Null();
    }
    if (_waypointCapacity != 0) {
        // Uncertain: the generic allocation helper is assumed to clear the block.
        _waypoints = heap.AllocateAndClearPtr(_waypointCapacity * 27);
    }
    return true;
}

// 0x7cf8868
void NavMeshPathfinder::Clear(const FPVector3& entryPosition, NavMeshSteeringAgent* steeringAgent) {
    _inputTarget = entryPosition;
    _active = 0;
    _waypointIndex = 0;
    _waypointCount = 0;
    _internalTarget = entryPosition;
    if (steeringAgent != nullptr) {
        steeringAgent->Clear();
    }
}

} // namespace Quantum
