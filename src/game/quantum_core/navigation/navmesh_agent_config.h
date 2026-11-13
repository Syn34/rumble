// navmesh_agent_config.h - Quantum.NavMeshAgentConfig and Quantum.AssetRefNavMeshAgentConfig
// (quantum.core.dll), plus the enums nested in Quantum.Navigation that the config uses.
//
// An agent config asset bundles every tuning value of a navmesh agent: pathfinding,
// waypoint detection, steering and avoidance.
#pragma once

#include "photon_deterministic/fp.h"
#include "quantum_core/asset_guid.h"

#include <cstdint>
#include <string>

namespace Quantum {

using Photon::Deterministic::FP;

// Quantum.Navigation nested enums.
enum class PathQuality : std::uint8_t { Fast = 0, Good = 1, Best = 2 };
enum class AvoidanceType : std::uint8_t { None = 0, Internal = 2 };
enum class AvoidanceQuality : std::uint8_t { Fast = 1, Good = 3, Best = 4 };
enum class MovementType : std::uint8_t {
    None = 0,
    Transform = 1,
    DynamicBody = 2,
    CharacterController3D = 3,
    Callback = 4
};
enum class VerticalPositioningType : std::uint8_t { None = 0, Navmesh = 1, Physics = 2 };
enum class RawPathAllocationStrategy : std::uint8_t { Fixed = 0, Allocating = 1 };

class NavMeshAgentConfig {
public:
    // GUID of the built-in default config asset.
    static constexpr std::int64_t DEFAULT_ID = 2;

    // AssetObject part: the asset's GUID and path.
    AssetGuid Guid;
    std::string Path;

    std::int32_t UpdateInterval = 1;
    Quantum::PathQuality PathQuality = Quantum::PathQuality::Good;
    std::uint8_t CachedWaypointCount = 6;
    FP MaxRepathTimeout = FP::_5();
    bool LineOfSightFunneling = false;
    bool DynamicLineOfSight = false;
    FP DynamicLineOfSightWaypointRange;
    bool AutomaticTargetCorrection = true;
    FP AutomaticTargetCorrectionRadius = FP::_1();
    bool EnableWaypointDetectionAxis = false;
    FP WaypointDetectionAxisExtend = FP::_0_50();
    FP WaypointDetectionAxisOffset;
    FP DefaultWaypointDetectionDistance = FP::_0_01();
    Quantum::MovementType MovementType = Quantum::MovementType::Transform;
    VerticalPositioningType VerticalPositioning = VerticalPositioningType::Navmesh;
    // FP.FromFloat_UNSAFE(3.5f) in the constructor; 3.5 is exact in FP.
    FP Speed = FP::FromRaw(229376);
    FP AngularSpeed = FP::FromInt(20);
    FP Acceleration = FP::FromInt(20);
    FP StoppingDistance = FP::_0_01();
    bool AutoBraking = false;
    FP AutoBrakingDistance = FP::_0_50();
    bool ClampAgentToNavmesh = false;
    FP ClampAgentToNavmeshCorrection = FP::_0_25();
    Quantum::AvoidanceType AvoidanceType = Quantum::AvoidanceType::Internal;
    Quantum::AvoidanceQuality AvoidanceQuality = Quantum::AvoidanceQuality::Good;
    std::uint8_t Priority = 50;
    FP AvoidanceRadius = FP::_0_50();
    std::int32_t AvoidanceLayer = 0;
    std::int32_t AvoidanceMask = -1; // Quantum.LayerMask (all layers)
    std::int32_t MaxAvoidanceCandidates = 64;
    bool ReduceAvoidanceAtWaypoints = false;
    FP ReduceAvoidanceFactor = FP::_4();
    bool AvoidanceCanReduceSpeed = false;
    bool ShowDebugSteering = false;
    bool ShowDebugAvoidance = false;
    // Filled by Loaded().
    std::int64_t StoppingDistanceSqrRaw = 0;
    std::int64_t AutoBrakingDistanceSqrRaw = 0;

    // 0x7ce52c0 - the defaults above.
    NavMeshAgentConfig() = default;

    // TODO(0x7ce5128, Quantum.NavMeshAgentConfig.Quantum.IBuiltInAssetObject.Dispatch):
    // visitor double dispatch of the asset system; needs IAssetObjectVisitor.

    // 0x7ce51d0 - precomputes the squared distances (stopping distance raised to at least
    // Constants.MinStoppingDistance for that, the field itself is kept) and clamps
    // CachedWaypointCount into [3, Constants.MaxWaypoints]. The resource manager and
    // allocator parameters are unused.
    void Loaded();
};

// Typed asset reference. Original: Quantum.AssetRefNavMeshAgentConfig.
struct AssetRefNavMeshAgentConfig {
    static constexpr int SIZE = 8;

    AssetGuid Id;

    // 0x7ce5024 (Equals(AssetRefNavMeshAgentConfig)); 0x7ce5034 Equals(object) is the
    // boxed form.
    friend constexpr bool operator==(AssetRefNavMeshAgentConfig,
                                     AssetRefNavMeshAgentConfig) = default;
    // 0x7ce50ac
    std::int32_t GetHashCode() const { return Id.GetHashCode(); }
    // 0x7ce501c
    std::string ToString() const { return AssetRef{Id}.ToString(); }

    // TODO(0x7ce50b4, Quantum.AssetRefNavMeshAgentConfigExt.FindAsset): frame.FindAsset;
    // needs FrameBase and the asset database.
};
static_assert(sizeof(AssetRefNavMeshAgentConfig) == AssetRefNavMeshAgentConfig::SIZE);

} // namespace Quantum
