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

} // namespace Quantum
