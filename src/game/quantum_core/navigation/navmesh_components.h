// navmesh_components.h - the navigation ECS components of quantum.core.dll:
// NavMeshPathfinder, NavMeshSteeringAgent, NavMeshAvoidanceAgent and
// NavMeshAvoidanceObstacle (and the prototypes that create them).
//
// The structs keep the original field layout (they live in frame memory and are
// serialised field by field). Methods that only touch the component are implemented;
// those that need the frame (heap-allocated waypoint buffers, asset lookups, component
// callbacks) are TODO until the frame/ECS core exists.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "photon_deterministic/nullable_fp.h"
#include "quantum_core/asset_guid.h"
#include "quantum_core/frame_heap.h"
#include "quantum_core/navigation/navmesh_region_mask.h"
#include "quantum_core/ptr.h"

#include <cstdint>

namespace Photon::Deterministic {
class IDeterministicFrameSerializer;
}

namespace Quantum::Core {
class FrameBase;
}

namespace Quantum {

using Allocator::IFrameHeap;
using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;
using Photon::Deterministic::NullableNonNegativeFP;

class NavMeshAgentConfig;

// Navigation.WaypointFlag
enum class WaypointFlag : std::uint8_t {
    Target = 1,
    LinkStart = 2,
    LinkEnd = 4,
    RepathWhenReached = 8,
};

// Steers an entity along a NavMeshPathfinder's waypoints.
struct NavMeshSteeringAgent {
    static constexpr int SIZE = 40;

    NullableNonNegativeFP _maxSpeed;     // unset: taken from the agent config
    NullableNonNegativeFP _acceleration; // unset: taken from the agent config
    std::int64_t _reserved = 0;          // 8 bytes the layout leaves unused
    FPVector2 _velocity;

    // 0x7d30970
    FP Acceleration() const { return _acceleration.ValueOrDefault(); }
    // 0x7d3097c
    void SetAcceleration(FP value) { _acceleration = NullableNonNegativeFP::FromFP(value); }
    // 0x7d3099c
    FP CurrentSpeed() const;
    // 0x7d309b8
    FP MaxSpeed() const { return _maxSpeed.ValueOrDefault(); }
    // 0x7d309c4
    void SetMaxSpeed(FP value) { _maxSpeed = NullableNonNegativeFP::FromFP(value); }
    // 0x7d309e4
    FPVector2 Velocity() const { return _velocity; }
    // 0x7d309f0
    void SetVelocity(FPVector2 value) { _velocity = value; }

    // 0x7d309f8 - copies speed and acceleration from the config where unset (or always
    // with `overwrite`). A null config means the default config asset
    // (NavMeshAgentConfig::DEFAULT_ID), looked up in the frame; if that is missing too and
    // a value has to be copied, std::invalid_argument is thrown (the original dereferences
    // null).
    void SetConfig(Core::FrameBase* frame, const NavMeshAgentConfig* config, bool overwrite);
    // The copying part of SetConfig for a config at hand.
    void SetConfig(const NavMeshAgentConfig& config, bool overwrite);

    // 0x7d30ad8 - stops the agent (the frame and entity are unused).
    void Clear() { _velocity = FPVector2::Zero(); }

    // 0x7d30ae0 - _maxSpeed, _acceleration, _velocity.
    static void Serialize(void* ptr,
                          Photon::Deterministic::IDeterministicFrameSerializer* serializer);

    // TODO(0x7d30b28, Quantum.NavMeshSteeringAgent.OnAdded): component callback; checks the
    // entity has a pathfinder and logs otherwise; needs FrameBase.
    // TODO(0x7d30c6c, Quantum.NavMeshSteeringAgent.OnRemoved): empty component callback;
    // needs the component callback signatures of FrameBase.
};
static_assert(sizeof(NavMeshSteeringAgent) == NavMeshSteeringAgent::SIZE);

// Local avoidance participant (other agents steer around it and it around them).
struct NavMeshAvoidanceAgent {
    static constexpr int SIZE = 40;

    std::uint8_t _priority = 0;
    std::uint8_t _avoidanceLayer = 0;
    std::uint8_t _membersInitialized = 0; // bit 0 priority, 1 layer, 2 mask set explicitly
    std::int32_t _avoidanceMask = 0;
    std::int64_t _reserved = 0; // 8 bytes the layout leaves unused
    FPVector2 _desiredDirection;
    FP _desiredSpeed;

    // 0x7cf8c94
    void SetAvoidanceMask(std::int32_t value) {
        _avoidanceMask = value;
        _membersInitialized |= 4;
    }
    // 0x7cf8ca8
    void SetAvoidanceLayer(std::uint8_t value) {
        _avoidanceLayer = value;
        _membersInitialized |= 2;
    }
    // 0x7cf8cbc
    void SetPriority(std::uint8_t value) {
        _priority = value;
        _membersInitialized |= 1;
    }

    // 0x7cf8418 - copies priority, layer and mask from the config where not set
    // explicitly (or always with `overwrite`). Null config: see NavMeshSteeringAgent.
    void SetConfig(const NavMeshAgentConfig& config, bool overwrite);

    // 0x7cf8cd0 - _priority, _avoidanceLayer, _avoidanceMask, _desiredDirection (the
    // initialised flags and the desired speed are not serialised).
    static void Serialize(void* ptr,
                          Photon::Deterministic::IDeterministicFrameSerializer* serializer);

    // TODO(0x7cf8e6c, Quantum.NavMeshAvoidanceAgent.OnAdded): component callback; applies the
    // pathfinder's config and checks for a steering agent; needs FrameBase.
    // TODO(0x7cf8fdc, Quantum.NavMeshAvoidanceAgent.OnRemoved): empty component callback.
};
static_assert(sizeof(NavMeshAvoidanceAgent) == NavMeshAvoidanceAgent::SIZE);

// A static or moving obstacle other agents avoid (without avoiding anything itself).
struct NavMeshAvoidanceObstacle {
    static constexpr int SIZE = 32;

    std::uint8_t _disabled = 0;
    std::uint8_t _avoidanceLayer = 0;
    FP _radius;
    FPVector2 _velocity;

    // 0x7cf7c18
    void SetIsActive(bool value) { _disabled = value ? 0 : 1; }
    // 0x7cf7c24
    void SetAvoidanceLayer(std::uint8_t value) { _avoidanceLayer = value; }
    // 0x7cf7c2c
    FP Radius() const { return _radius; }
    // 0x7cf7c34
    void SetRadius(FP value) { _radius = value; }

    // 0x7cf7c3c - _disabled, _avoidanceLayer, _radius, _velocity.
    static void Serialize(void* ptr,
                          Photon::Deterministic::IDeterministicFrameSerializer* serializer);

    // TODO(0x7cf7d80, Quantum.NavMeshAvoidanceObstacle.OnAdded): component callback; checks the
    // entity has a transform; needs FrameBase.
    // TODO(0x7cf7e00, Quantum.NavMeshAvoidanceObstacle.OnRemoved): empty component callback.
};
static_assert(sizeof(NavMeshAvoidanceObstacle) == NavMeshAvoidanceObstacle::SIZE);

// Path following state of an agent. Waypoints live in a frame heap block (_waypoints).
struct NavMeshPathfinder {
    static constexpr int SIZE = 96;

    std::uint8_t _active = 0;
    std::uint8_t _waypointIndex = 0;
    std::uint8_t _waypointCount = 0;
    std::uint8_t _waypointCapacity = 0;
    Ptr _waypoints;
    FP _waypointDetectionDistanceSqr;
    FP _ttl;
    AssetGuid _configId;
    AssetGuid _navMeshID;
    NavMeshRegionMask RegionMask;
    FPVector3 _inputTarget;
    FPVector3 _internalTarget;

    // 0x7cf7e04
    AssetGuid ConfigId() const { return _configId; }
    // 0x7cf7e0c
    bool IsActive() const { return _active == 1; }
    // 0x7cf7e1c
    int WaypointIndex() const { return _waypointIndex; }
    // 0x7cf7e24
    int WaypointCount() const { return _waypointCount; }
    // 0x7cf7e2c
    void SetWaypointDetectionDistanceSqr(FP value) { _waypointDetectionDistanceSqr = value; }

    // The waypoint arrays in the frame heap (Navigation.WaypointList). The heap block
    // holds Capacity points, then Capacity flags, then Capacity links (27 bytes each).
    struct WaypointList {
        std::uint8_t Count = 0;
        std::uint8_t Capacity = 0;
        std::uint8_t* Flags = nullptr;
        FPVector3* Points = nullptr;
        std::int16_t* Links = nullptr;
    };

    // Waypoint access goes through the frame's heap; the original takes the frame and
    // uses its heap, callers here pass that heap.

    // 0x7cf826c - empty list (all null) while no buffer is allocated.
    WaypointList GetWaypoints(const IFrameHeap& heap) const;
    // 0x7cf8114 - false when inactive or past the last waypoint.
    bool CurrentWaypointHasFlag(const IFrameHeap& heap, WaypointFlag waypointFlag) const;
    // 0x7cf810c
    bool IsOnLink(const IFrameHeap& heap) const {
        return CurrentWaypointHasFlag(heap, WaypointFlag::LinkEnd);
    }
    // 0x7cf81c8 - zero for an index past the count.
    FPVector3 GetWaypoint(const IFrameHeap& heap, int index) const;
    // 0x7cf818c - 0 for an index past the count.
    std::uint8_t GetWaypointFlags(const IFrameHeap& heap, int index) const;
    // 0x7cf82d4 - -1 for an index past the count.
    int GetWaypointLink(const IFrameHeap& heap, int index) const;
    // 0x7cf8714
    void ResetWaypoints() {
        _waypointIndex = 0;
        _waypointCount = 0;
    }
    // 0x7cf87e8 - appends without a capacity check (as the original).
    void AddWaypoint(const IFrameHeap& heap, const FPVector3& position, std::uint8_t flags,
                     int link);
    // 0x7cf8310 - reallocates the buffer (dropping the waypoints); true if it changed.
    bool SetWaypointsCapacity(IFrameHeap& heap, std::uint8_t newCapacity);
    // 0x7cf8868 - stops at the entry's position (both targets) and clears the steering
    // agent's velocity. The original takes the navigation Entry; the two values it reads
    // from it are passed directly.
    void Clear(const FPVector3& entryPosition, NavMeshSteeringAgent* steeringAgent);

    // TODO(0x7cf7e34, Quantum.NavMeshPathfinder.Create): builds a cleared component with
    // targets at FP.UseableMin, applies SetConfig and the default region mask; waits on
    // SetConfig.
    // TODO(0x7cf7f2c, Quantum.NavMeshPathfinder.SetConfig): stores the config GUID (null:
    // the default config via FrameBase::FindAsset), resizes the waypoint buffer to
    // CachedWaypointCount and forwards to the entity's steering/avoidance agents; needs
    // the navigation components registered as frame component types (GetPointer).
    // TODO(0x7cf8510, Quantum.NavMeshPathfinder.SetTarget): records the target and navmesh and
    // schedules a path search (config via FrameBase::FindAsset); needs the same
    // component access.
    // TODO(0x7cf871c, Quantum.NavMeshPathfinder.Stop): deactivates and optionally zeroes the
    // entity's velocity; needs the physics body components.
    // TODO(0x7cf88b0, Quantum.NavMeshPathfinder.Serialize): byte fields, the heap pointer
    // with its waypoint arrays, then the FP/GUID/mask/vector fields; needs FrameSerializer
    // pointer serialisation of raw blocks.
    // TODO(0x7cf8c80, Quantum.NavMeshPathfinder.OnAdded): component callback.
    // TODO(0x7cf8c84, Quantum.NavMeshPathfinder.OnRemoved): frees the waypoint buffer.
};
static_assert(sizeof(NavMeshPathfinder) == NavMeshPathfinder::SIZE);

// TODO(0x7d7e814, Quantum.Prototypes.NavMeshAvoidanceObstacle_Prototype.AddToEntity): prototype
// materialisation; needs the entity prototype system.
// TODO(0x7d7e8f0, Quantum.Prototypes.NavMeshAvoidanceObstacle_Prototype.Materialize)
// TODO(0x7d7e92c, Quantum.Prototypes.NavMeshAvoidanceObstacle_Prototype.Dispatch)
// TODO(0x7d7e950, Quantum.Prototypes.NavMeshAvoidanceObstacle_Prototype..ctor)
// TODO(0x7d7eb48, Quantum.Prototypes.NavMeshSteeringAgent_Prototype.AddToEntity)
// TODO(0x7d7ec18, Quantum.Prototypes.NavMeshSteeringAgent_Prototype.PostMaterialize)
// TODO(0x7d7ec6c, Quantum.Prototypes.NavMeshSteeringAgent_Prototype.Dispatch)
// TODO(0x7d7ec90, Quantum.Prototypes.NavMeshSteeringAgent_Prototype..ctor)
// TODO(0x7d7ecd8, Quantum.Prototypes.NavMeshPathfinder_Prototype.AddToEntity)
// TODO(0x7d7edf4, Quantum.Prototypes.NavMeshPathfinder_Prototype.Materialize)
// TODO(0x7d7f0b4, Quantum.Prototypes.NavMeshPathfinder_Prototype.Dispatch)
// TODO(0x7d7f0d8, Quantum.Prototypes.NavMeshPathfinder_Prototype..ctor)
// TODO(0x7d8014c, Quantum.Prototypes.NavMeshAvoidanceAgent_Prototype.AddToEntity)
// TODO(0x7d80200, Quantum.Prototypes.NavMeshAvoidanceAgent_Prototype.Dispatch)
// TODO(0x7d80224, Quantum.Prototypes.NavMeshAvoidanceAgent_Prototype..ctor)

} // namespace Quantum
