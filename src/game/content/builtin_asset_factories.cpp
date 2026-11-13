// builtin_asset_factories.cpp - builds quantum.core's built-in assets and SimulationConfig
// from the decoded `Settings` data of the game's content files (PC-side glue; new code).
//
// Field names are the original serialised names (see settings_reader.h for the value
// shapes). Each Loaded() then derives the runtime data as in the original.
#include "content/quantum_code_factories.h"

#include "content/quantum_content_provider.h"
#include "content/settings_reader.h"
#include "quantum_code/components.h"
#include "quantum_code/simulation_config.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/component_prototype.h"
#include "quantum_core/map.h"
#include "quantum_core/navigation/navmesh.h"
#include "quantum_core/navigation/navmesh_agent_config.h"
#include "quantum_core/polygon_collider.h"

#include <memory>

namespace rr::content {

void ReadFields(const Value& value, Quantum::FPAnimationCurve::Keyframe& out, ReaderTag) {
    Read(value, "Time", out.Time);
    Read(value, "Value", out.Value);
    Read(value, "InTangent", out.InTangent);
    Read(value, "OutTangent", out.OutTangent);
    Read(value, "TangentMode", out.TangentMode);
    Read(value, "TangentModeLeft", out.TangentModeLeft);
    Read(value, "TangentModeRight", out.TangentModeRight);
    Read(value, "WeightedMode", out.WeightedMode);
}

void ReadFields(const Value& value, Quantum::FPAnimationCurve& out, ReaderTag) {
    Read(value, "Samples", out.Samples);
    Read(value, "PreWrapMode", out.PreWrapMode);
    Read(value, "PostWrapMode", out.PostWrapMode);
    Read(value, "StartTime", out.StartTime);
    Read(value, "EndTime", out.EndTime);
    Read(value, "Resolution", out.Resolution);
    Read(value, "OriginalPreWrapMode", out.OriginalPreWrapMode);
    Read(value, "OriginalPostWrapMode", out.OriginalPostWrapMode);
    Read(value, "Keys", out.Keys);
}

void ReadFields(const Value& value, Quantum::QBoolean& out, ReaderTag) { Read(value, "Value", out.Value); }

void ReadFields(const Value& value, Quantum::EntityPrototypeRef& out, ReaderTag) {
    Read(value, "Asset", out.Asset);
    Read(value, "InnerAssetId", out.InnerAssetId);
}

void ReadFields(const Value& value, Quantum::AutoJumperStartParams& out, ReaderTag) {
    Read(value, "airDrag", out.airDrag);
    Read(value, "gravity", out.gravity);
    Read(value, "launchVelocity", out.launchVelocity);
    Read(value, "travelTimeTotal", out.travelTimeTotal);
    Read(value, "travelTimeUntilAirControlRestored", out.travelTimeUntilAirControlRestored);
}

void ReadFields(const Value& value, Quantum::ChargingStats& out, ReaderTag) {
    Read(value, "AutoReleaseOnFullCharge", out.AutoReleaseOnFullCharge);
    Read(value, "FullChargeDuration", out.FullChargeDuration);
    Read(value, "NumberOfChargeLevel", out.NumberOfChargeLevel);
}

void ReadFields(const Value& value, Quantum::GOAPState& out, ReaderTag) {
    Read(value, "Negative", out.Negative);
    Read(value, "Positive", out.Positive);
}

void ReadFields(const Value& value, Quantum::UTMomentumData& out, ReaderTag) {
    Read(value, "DecayAmount", out.DecayAmount);
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::WeaponLoadoutItemData& out, ReaderTag) {
    Read(value, "enhancementLevel", out.enhancementLevel);
    Read(value, "uniqueWeaponID", out.uniqueWeaponID);
    Read(value, "weaponLevel", out.weaponLevel);
}

void ReadFields(const Value& value, Quantum::Shape3DConfig::CompoundShapeData3D& out, ReaderTag) {
    Read(value, "ShapeType", out.ShapeType);
    Read(value, "SphereRadius", out.SphereRadius);
    Read(value, "PositionOffset", out.PositionOffset);
    Read(value, "BoxExtents", out.BoxExtents);
    Read(value, "RotationOffset", out.RotationOffset);
    Read(value, "UserTag", out.UserTag);
    Read(value, "IsSetFromSourceCollider", out.IsSetFromSourceCollider);
}

void ReadFields(const Value& value, Quantum::Shape3DConfig& out, ReaderTag) {
    Read(value, "ShapeType", out.ShapeType);
    Read(value, "SphereRadius", out.SphereRadius);
    Read(value, "PositionOffset", out.PositionOffset);
    Read(value, "BoxExtents", out.BoxExtents);
    Read(value, "RotationOffset", out.RotationOffset);
    Read(value, "UserTag", out.UserTag);
    Read(value, "IsPersistent", out.IsPersistent);
    Read(value, "CompoundShapes", out.CompoundShapes);
    Read(value, "IsSetFromSourceCollider", out.IsSetFromSourceCollider);
}

void ReadFields(const Value& value, Quantum::StaticColliderData& out, ReaderTag) {
    Read(value, "Name", out.Name);
    Read(value, "Tag", out.Tag);
    Read(value, "Layer", out.Layer);
    Read(value, "Asset", out.Asset);
    Read(value, "IsTrigger", out.IsTrigger);
    Read(value, "ColliderIndex", out.ColliderIndex);
    Read(value, "MutableMode", out.MutableMode);
}

void ReadFields(const Value& value, Quantum::MapStaticCollider3D& out, ReaderTag) {
    Read(value, "Rotation", out.Rotation);
    Read(value, "Position", out.Position);
    Read(value, "PhysicsMaterial", out.PhysicsMaterial);
    Read(value, "ShapeType", out.ShapeType);
    Read(value, "SphereRadius", out.SphereRadius);
    Read(value, "BoxExtents", out.BoxExtents);
    Read(value, "SmoothSphereMeshCollisions", out.SmoothSphereMeshCollisions);
    Read(value, "StaticData", out.StaticData);
    Read(value, "MutableMode", out.MutableMode);
}

namespace {

using namespace Quantum;

void ReadMap(const Value& v, Map& out) {
    Read(v, "UserAsset", out.UserAsset);
    Read(v, "Scene", out.Scene);
    Read(v, "ScenePath", out.ScenePath);
    Read(v, "SceneGuid", out.SceneGuid);
    Read(v, "WorldSize", out.WorldSize);
    Read(v, "BucketsCount", out.BucketsCount);
    Read(v, "BucketsSubdivisions", out.BucketsSubdivisions);
    Read(v, "BucketingAxis", out.BucketingAxis);
    Read(v, "SortingAxis", out.SortingAxis);
    Read(v, "TriangleMeshCellSize", out.TriangleMeshCellSize);
    Read(v, "StaticColliders3DTrianglesData", out.StaticColliders3DTrianglesData);
    Read(v, "OptionalTriangleMetadataSerializationEnabled", out.OptionalTriangleMetadataSerializationEnabled);
    Read(v, "SerializeTrianglesMetadata", out.SerializeTrianglesMetadata);
    Read(v, "StaticColliders3DTrianglesBinaryFile", out.StaticColliders3DTrianglesBinaryFile);
    Read(v, "GridSizeX", out.GridSizeX);
    Read(v, "GridSizeY", out.GridSizeY);
    Read(v, "GridNodeSize", out.GridNodeSize);
    Read(v, "NavMeshLinks", out.NavMeshLinks);
    Read(v, "Regions", out.Regions);
    Read(v, "StaticColliders3D", out.StaticColliders3D);
    // StaticColliders2D: not reproduced (the game's maps have none). MapEntities: the
    // editor bakes them from the scene's entity prototypes; built by the scene loader.
}

void ReadNavMesh(const Value& v, NavMesh& out) {
    Read(v, "Name", out.Name);
    Read(v, "DataAsset", out.DataAsset); // an AssetRefBinaryData, kept as its GUID
    Read(v, "DataFilepath", out.DataFilepath);
    Read(v, "GridSizeX", out.GridSizeX);
    Read(v, "GridSizeY", out.GridSizeY);
    Read(v, "GridNodeSize", out.GridNodeSize);
    Read(v, "WorldOffset", out.WorldOffset);
    Read(v, "MinAgentRadius", out.MinAgentRadius);
}

void ReadNavMeshAgentConfig(const Value& v, NavMeshAgentConfig& out) {
    Read(v, "UpdateInterval", out.UpdateInterval);
    Read(v, "PathQuality", out.PathQuality);
    Read(v, "CachedWaypointCount", out.CachedWaypointCount);
    Read(v, "MaxRepathTimeout", out.MaxRepathTimeout);
    Read(v, "LineOfSightFunneling", out.LineOfSightFunneling);
    Read(v, "DynamicLineOfSight", out.DynamicLineOfSight);
    Read(v, "DynamicLineOfSightWaypointRange", out.DynamicLineOfSightWaypointRange);
    Read(v, "AutomaticTargetCorrection", out.AutomaticTargetCorrection);
    Read(v, "AutomaticTargetCorrectionRadius", out.AutomaticTargetCorrectionRadius);
    Read(v, "EnableWaypointDetectionAxis", out.EnableWaypointDetectionAxis);
    Read(v, "WaypointDetectionAxisExtend", out.WaypointDetectionAxisExtend);
    Read(v, "WaypointDetectionAxisOffset", out.WaypointDetectionAxisOffset);
    Read(v, "DefaultWaypointDetectionDistance", out.DefaultWaypointDetectionDistance);
    Read(v, "MovementType", out.MovementType);
    Read(v, "VerticalPositioning", out.VerticalPositioning);
    Read(v, "Speed", out.Speed);
    Read(v, "AngularSpeed", out.AngularSpeed);
    Read(v, "Acceleration", out.Acceleration);
    Read(v, "StoppingDistance", out.StoppingDistance);
    Read(v, "AutoBraking", out.AutoBraking);
    Read(v, "AutoBrakingDistance", out.AutoBrakingDistance);
    Read(v, "ClampAgentToNavmesh", out.ClampAgentToNavmesh);
    Read(v, "ClampAgentToNavmeshCorrection", out.ClampAgentToNavmeshCorrection);
    Read(v, "AvoidanceType", out.AvoidanceType);
    Read(v, "AvoidanceQuality", out.AvoidanceQuality);
    Read(v, "Priority", out.Priority);
    Read(v, "AvoidanceRadius", out.AvoidanceRadius);
    Read(v, "AvoidanceLayer", out.AvoidanceLayer);
    LayerMask mask{out.AvoidanceMask};
    Read(v, "AvoidanceMask", mask);
    out.AvoidanceMask = mask.BitMask;
    Read(v, "MaxAvoidanceCandidates", out.MaxAvoidanceCandidates);
    Read(v, "ReduceAvoidanceAtWaypoints", out.ReduceAvoidanceAtWaypoints);
    Read(v, "ReduceAvoidanceFactor", out.ReduceAvoidanceFactor);
    Read(v, "AvoidanceCanReduceSpeed", out.AvoidanceCanReduceSpeed);
    Read(v, "ShowDebugSteering", out.ShowDebugSteering);
    Read(v, "ShowDebugAvoidance", out.ShowDebugAvoidance);
}

void ReadPhysicsMaterial(const Value& v, PhysicsMaterial& out) {
    Read(v, "Restitution", out.Restitution);
    Read(v, "RestitutionCombineFunction", out.RestitutionCombineFunction);
    Read(v, "FrictionStatic", out.FrictionStatic);
    Read(v, "FrictionDynamic", out.FrictionDynamic);
    Read(v, "FrictionCombineFunction", out.FrictionCombineFunction);
}

template <class Config> void ReadCharacterControllerConfig(const Value& v, Config& out) {
    Read(v, "Offset", out.Offset);
    Read(v, "Radius", out.Radius);
    Read(v, "MaxPenetration", out.MaxPenetration);
    Read(v, "PenetrationCorrection", out.PenetrationCorrection);
    Read(v, "Extent", out.Extent);
    Read(v, "MaxContacts", out.MaxContacts);
    LayerMask layers{out.LayerMask};
    Read(v, "LayerMask", layers);
    out.LayerMask = layers.BitMask;
    Read(v, "UseManifoldNormal", out.UseManifoldNormal);
    Read(v, "AirControl", out.AirControl);
    Read(v, "Acceleration", out.Acceleration);
    Read(v, "Braking", out.Braking);
    Read(v, "BaseJumpImpulse", out.BaseJumpImpulse);
    Read(v, "MaxSpeed", out.MaxSpeed);
    Read(v, "Gravity", out.Gravity);
    Read(v, "MaxSlope", out.MaxSlope);
    Read(v, "MaxSlopeSpeed", out.MaxSlopeSpeed);
}

void ReadTerrainCollider(const Value& v, TerrainCollider& out) {
    Read(v, "HeightMap", out.HeightMap);
    Read(v, "HoleMask", out.HoleMask);
    Read(v, "Position", out.Position);
    Read(v, "Scale", out.Scale);
    Read(v, "Resolution", out.Resolution);
    Read(v, "PhysicsMaterial", out.PhysicsMaterial);
}

void ReadPolygonCollider(const Value& v, PolygonCollider& out) {
    Read(v, "Vertices", out.Vertices);
    Read(v, "Normals", out.Normals);
    Read(v, "Radius", out.Radius);
    Read(v, "Centroid", out.Centroid);
}

void ReadSimulationConfig(const Value& v, SimulationConfig& out) {
    // Navigation and Physics hold the engines' configurations, which aren't reproduced.
    if (const auto* entities = Child(v, "Entities")) {
        Read(*entities, "CommitCommandsMode", out.Entities.CommitCommandsMode);
        Read(*entities, "DefaultEntityCapacity", out.Entities.DefaultEntityCapacity);
        Read(*entities, "DefaultComponentBlockCapacity", out.Entities.DefaultComponentBlockCapacity);
        if (const auto* list = Child(*entities, "ComponentTypeConfiguration"); list && list->AsArray()) {
            out.Entities.ComponentTypeConfiguration.clear();
            for (const auto& item : *list->AsArray()) {
                auto& entry = out.Entities.ComponentTypeConfiguration.emplace_back();
                Read(*item, "Name", entry.Name);
                Read(*item, "ComponentBlockCapacity", entry.ComponentBlockCapacity);
            }
        }
    }
    Read(v, "AutoLoadSceneFromMap", out.AutoLoadSceneFromMap);
    Read(v, "DeltaTimeType", out.DeltaTimeType);
    Read(v, "ThreadCount", out.ThreadCount);
    Read(v, "ChecksumSnapshotHistoryLengthSeconds", out.ChecksumSnapshotHistoryLengthSeconds);
    Read(v, "ChecksumErrorDumpOptions", out.ChecksumErrorDumpOptions);
    Read(v, "HeapTrackingMode", out.HeapTrackingMode);
    Read(v, "HeapPageShift", out.HeapPageShift);
    Read(v, "HeapPageCount", out.HeapPageCount);
    Read(v, "HeapExtraCount", out.HeapExtraCount);
}

template <class T, void (*Reader)(const Value&, T&)>
std::shared_ptr<AssetObject> Make(const Value& settings) {
    auto asset = std::make_shared<T>();
    Reader(settings, *asset);
    return asset;
}

template <class T> std::shared_ptr<AssetObject> MakeEmpty(const Value&) { return std::make_shared<T>(); }

void ReadBinaryData(const Value& v, BinaryData& out) {
    Read(v, "Data", out.Data);
    Read(v, "IsCompressed", out.IsCompressed);
}

// EntityPrototype's stored container is empty: the editor fills prototypes from the
// components of the prefab the asset belongs to. Building them needs the engine-side
// prefab reader; until then the asset has an empty container.
std::shared_ptr<AssetObject> MakeEntityPrototype(const Value&) {
    auto asset = std::make_shared<EntityPrototype>();
    asset->Container = std::make_shared<EntityPrototypeContainer>();
    return asset;
}

} // namespace

void RegisterBuiltInAssetFactories() {
    RegisterAssetFactory("BinaryDataAsset", &Make<BinaryData, ReadBinaryData>);
    RegisterAssetFactory("CharacterController2DConfigAsset",
                         &Make<CharacterController2DConfig, ReadCharacterControllerConfig<CharacterController2DConfig>>);
    RegisterAssetFactory("CharacterController3DConfigAsset",
                         &Make<CharacterController3DConfig, ReadCharacterControllerConfig<CharacterController3DConfig>>);
    RegisterAssetFactory("EntityPrototypeAsset", &MakeEntityPrototype);
    RegisterAssetFactory("EntityViewAsset", &MakeEmpty<EntityView>);
    RegisterAssetFactory("MapAsset", &Make<Map, ReadMap>);
    RegisterAssetFactory("NavMeshAsset", &Make<NavMesh, ReadNavMesh>);
    RegisterAssetFactory("NavMeshAgentConfigAsset", &Make<NavMeshAgentConfig, ReadNavMeshAgentConfig>);
    RegisterAssetFactory("PhysicsMaterialAsset", &Make<PhysicsMaterial, ReadPhysicsMaterial>);
    RegisterAssetFactory("PolygonColliderAsset", &Make<PolygonCollider, ReadPolygonCollider>);
    RegisterAssetFactory("TerrainColliderAsset", &Make<TerrainCollider, ReadTerrainCollider>);
    RegisterAssetFactory("SimulationConfigAsset", &Make<SimulationConfig, ReadSimulationConfig>);
}

} // namespace rr::content
