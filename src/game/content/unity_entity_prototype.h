// unity_entity_prototype.h - the engine-side half of entity prototypes (PhotonQuantum.dll).
//
// In the original, an entity prototype asset is attached to a prefab. The prefab's root
// carries an `EntityPrototype` script that describes the framework components implied by
// the object itself (its transform, a collider, a physics body, navigation agents, a view)
// and any number of entity component scripts, each holding one component prototype in its
// `Prototype` field. When the asset is loaded (EntityPrototypeAsset.PrepareAsset) the
// script turns the implied components into prototypes (SerializeImplicitComponents), the
// component scripts hand out theirs (references to other prefabs' prototypes are turned
// into map entity ids by an EntityPrototypeConverter), and the asset's container gets the
// lot.
//
// Here the scripts are plain structs filled from the user's content files; the prefab's
// engine objects they consult (the transform, a source collider, navigation targets) are
// resolved by the loader beforehand and passed in as values.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "photon_deterministic/nullable_fp.h"
#include "quantum_code/components.h"
#include "quantum_core/asset_guid.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/component_prototype.h"
#include "quantum_core/map_entity_id.h"
#include "quantum_core/navigation/navmesh.h"
#include "quantum_core/navigation/navmesh_agent_config.h"
#include "quantum_core/shape3d_config.h"
#include "rr/assets/prefab.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Quantum {
class EntityPrototype;
class Map;
} // namespace Quantum

namespace Quantum::Prototypes {
class FlatEntityPrototypeContainer;
class AIBlackboardComponent_Prototype;
class BlackboardEntry_Prototype;
class BlackboardValue_Prototype;
class DictionaryEntry_Int32_BlackboardEntry_Prototype;
class BlastBallGoal_Prototype;
class Hierarchy_Prototype;
class LevelButton_Prototype;
class RailCollider_Prototype;
class Joint2D_Prototype;
class Joint3D_Prototype;
class PhysicsJoints2D_Prototype;
class PhysicsJoints3D_Prototype;
} // namespace Quantum::Prototypes

namespace rr::content::unity {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;
using Photon::Deterministic::NullableFP;
using ComponentPrototypeList = std::vector<std::shared_ptr<Quantum::ComponentPrototype>>;

// --- FPMathUtils: engine values to fixed point -----------------------------------------------

// 0x7ae8c28 - (x, z) of an engine position: the 2D plane is the horizontal one.
FPVector2 ToFPVector2(rr::assets::Vector3f v);
// 0x7ae8cb0
FPVector3 ToFPVector3(rr::assets::Vector3f v);
// 0x7ae8ca4 - the height (y) of an engine position.
FP ToFPVerticalPosition(rr::assets::Vector3f v);
// 0x7ae8af0 - the 2D rotation in degrees of an engine rotation: minus its yaw.
FP ToFPRotation2DDegrees(rr::assets::Quaternionf q);
// 0x7ae88e4
FP ToFP(float value);

// --- an object a prototype refers to ---------------------------------------------------------

// Identifies one `EntityPrototype` script in the content files (file + object), standing in
// for the engine object references the original compares.
struct ScriptKey {
    std::string cab;
    std::int64_t pathId = 0;

    friend bool operator==(const ScriptKey&, const ScriptKey&) = default;
};

// Quantum.EntityPrototypeConverter: maps references to prototype scripts onto map entity
// ids. For an asset, only the asset's own prototype has an id (0); for a map, the index in
// the map's ordered prototype list.
class EntityPrototypeConverter {
  public:
    std::vector<ScriptKey> OrderedMapPrototypes;
    std::optional<ScriptKey> AssetPrototype;

    // 0x7aedc1c - for a map's prototypes.
    explicit EntityPrototypeConverter(std::vector<ScriptKey> orderedMapPrototypes)
        : OrderedMapPrototypes(std::move(orderedMapPrototypes)) {}
    // 0x7aedc60 - for one asset prototype.
    explicit EntityPrototypeConverter(ScriptKey assetPrototype)
        : AssetPrototype(std::move(assetPrototype)) {}

    // 0x7aedc90 - The map entity id of a referenced prototype (Invalid for none, and for any
    // prototype other than the asset's own when converting an asset).
    void Convert(const std::optional<ScriptKey>& prototype, Quantum::MapEntityId& result) const;
};

// --- the EntityPrototype script --------------------------------------------------------------

enum class EntityPrototypeTransformMode : std::int32_t {
    Transform2D = 0,
    Transform3D = 1,
    None = 2
};

struct Transform2DVerticalInfo {
    bool IsEnabled = false;
    FP Height;
    FP PositionOffset;
};

// What the loader found for PhysicsColliderGeneric.SourceCollider (an engine collider whose
// shape the prototype copies).
struct SourceColliderInfo {
    enum class Kind { Sphere, Box, Other } kind = Kind::Other;
    std::string className;            // engine class, for messages
    float radius = 0;                 // sphere
    rr::assets::Vector3f size;        // box
    rr::assets::Vector3f worldCenter; // centre of its bounds
    rr::assets::Quaternionf worldRotation;
    rr::assets::Vector3f lossyScale{1, 1, 1};
    bool isTrigger = false;
    std::int32_t layer = 0; // of its game object
};

struct PhysicsColliderGeneric {
    bool IsTrigger = false;
    Quantum::AssetRefPhysicsMaterial Material;
    std::optional<SourceColliderInfo> SourceCollider;
    bool IsEnabled = true;
    // Shape2D: no C++ type yet (the game's prototypes are 3D); 2D colliders get no shape.
    Quantum::Shape3DConfig Shape3D;
    std::int32_t Layer = 0;
    std::int32_t CallbackFlags = 0; // CallbackFlags_Wrapper
};

struct PhysicsBodyGeneric {
    bool IsEnabled = false;
    std::int32_t Version2D = 0;
    std::int32_t Version3D = 0;
    std::uint8_t Config2D = 0x74; // PhysicsBody2D.ConfigFlags
    std::uint8_t Config3D = 0x3A; // PhysicsBody3D.ConfigFlags
    std::uint8_t RotationFreeze = 0;
    FP Mass = FP::_1();
    FP Drag = FP::FromRaw(FP::Raw{1} << 15);        // 0.5
    FP AngularDrag = FP::FromRaw(FP::Raw{1} << 15); // 0.5
    FPVector2 CenterOfMass2D;
    FPVector3 CenterOfMass3D;
    NullableFP GravityScale;

    // (Defaults as set by the script's constructor, 0x7abf364.)

    // 0x7abee70 - Upgrades configs saved by older versions: version 1 added a flag to each
    // config (2D: 0x40, 3D: 0x20); version 2 removed one (2D: 0x80, 3D: 0x40).
    void EnsureVersionUpdated();
};

struct NavMeshSpec {
    std::optional<std::string> ReferenceName; // name of the referenced navmesh definition, if any
    Quantum::AssetRefNavMesh Asset;
    std::string Name;
};

struct InitialNavMeshTargetInfo {
    bool IsEnabled = false;
    std::optional<rr::assets::Vector3f> TargetPosition; // world position of Target, if set
    FPVector3 Position;
    NavMeshSpec NavMesh;
};

struct NavMeshPathfinderInfo {
    bool IsEnabled = false;
    Quantum::AssetRefNavMeshAgentConfig NavMeshAgentConfig;
    InitialNavMeshTargetInfo InitialTarget;
};

struct OverrideFP {
    bool IsEnabled = false;
    FP Value;
};

struct NavMeshSteeringAgentInfo {
    bool IsEnabled = false;
    OverrideFP MaxSpeed;
    OverrideFP Acceleration;
};

struct NavMeshAvoidanceAgentInfo {
    bool IsEnabled = false;
};

// The EntityPrototype script on a prefab root.
class EntityPrototype {
  public:
    EntityPrototypeTransformMode TransformMode = EntityPrototypeTransformMode::Transform3D;
    Transform2DVerticalInfo Transform2DVertical;
    PhysicsColliderGeneric PhysicsCollider;
    PhysicsBodyGeneric PhysicsBody;
    NavMeshPathfinderInfo NavMeshPathfinder;
    NavMeshSteeringAgentInfo NavMeshSteeringAgent;
    NavMeshAvoidanceAgentInfo NavMeshAvoidanceAgent;
    Quantum::AssetRefEntityView View;

    // The script's game object: world position and rotation of its transform, and whether
    // it also has an entity view script (GetComponent<EntityView>).
    rr::assets::Vector3f Position;
    rr::assets::Quaternionf Rotation;
    bool HasEntityView = false;

    // 0x7ab6178 - Brings derived values up to date before serialising: the collider shape
    // from a source collider (EntityPrototypeUtils.TrySetShapeConfigFromSourceCollider2D/3D,
    // then the trigger flag and layer from it), the body's config version (and for 2D the
    // rotation freeze from the config), the navigation target's position and navmesh name
    // from their references.
    void PreSerialize();
    // 0x7ab635c - The framework component prototypes the script implies, emitted in order
    // (transform, vertical transform, collider and callbacks, body, pathfinder, steering,
    // avoidance, view). `hasSelfView` reports an entity view script on the object; without
    // one, a valid View asset becomes a View prototype.
    void SerializeImplicitComponents(ComponentPrototypeList& prototypes, bool& hasSelfView) const;
};

// 0x7abeaf4 EntityPrototypeUtils.TrySetShapeConfigFromSourceCollider3D - Copies a sphere's
// radius or a box's scaled half size into `shape` and returns true; with no source
// collider, clears IsSetFromSourceCollider and returns false. The collider must sit at the
// prototype's position (and a box at its rotation): std::logic_error otherwise, and
// std::invalid_argument for other collider kinds (the original throws
// InvalidOperationException / NotSupportedException).
bool TrySetShapeConfigFromSourceCollider3D(Quantum::Shape3DConfig& shape,
                                           const EntityPrototype& reference,
                                           const std::optional<SourceColliderInfo>& source);

// --- prototypes that refer to other prototype scripts (Quantum.Prototypes.Unity) ------------
//
// A component prototype that links to another entity stores a MapEntityId; its editor-side
// twin stores the referenced prototype script instead, and Convert maps it.

struct BlackboardValue_Prototype {
    std::string _field_used_;
    Quantum::QBoolean BooleanValue;
    std::uint8_t ByteValue = 0;
    std::int32_t IntegerValue = 0;
    FP FPValue;
    FPVector2 FPVector2Value;
    FPVector3 FPVector3Value;
    std::optional<ScriptKey> EntityRefValue;
    Quantum::AssetRef AssetRefValue;

    // 0x7afead0
    void Convert(const EntityPrototypeConverter& converter,
                 Quantum::Prototypes::BlackboardValue_Prototype& result) const;
};

struct BlackboardEntry_Prototype {
    BlackboardValue_Prototype Value;
    std::vector<Quantum::AssetRefBTDecorator> ReactiveDecorators;

    // 0x7afe96c
    void Convert(const EntityPrototypeConverter& converter,
                 Quantum::Prototypes::BlackboardEntry_Prototype& result) const;
};

struct DictionaryEntry_Int32_BlackboardEntry_Prototype {
    std::int32_t Key = 0;
    BlackboardEntry_Prototype Value;

    // 0x7afe888
    void
    Convert(const EntityPrototypeConverter& converter,
            Quantum::Prototypes::DictionaryEntry_Int32_BlackboardEntry_Prototype& result) const;
};

struct AIBlackboardComponent_Prototype {
    Quantum::AssetRefAIBlackboard Board;
    std::vector<DictionaryEntry_Int32_BlackboardEntry_Prototype> Entries;

    // 0x7afe670
    std::shared_ptr<Quantum::Prototypes::AIBlackboardComponent_Prototype>
    Convert(const EntityPrototypeConverter& converter) const;
};

struct BlastBallGoal_Prototype {
    Quantum::TeamFlag OwnerTeam{};
    std::optional<ScriptKey> ballEntity;
    Quantum::QBoolean ballInside;

    // 0x7afebf0
    std::shared_ptr<Quantum::Prototypes::BlastBallGoal_Prototype>
    Convert(const EntityPrototypeConverter& converter) const;
};

struct Hierarchy_Prototype {
    std::optional<ScriptKey> Parent;
    FPVector3 Offset;

    // 0x7afeccc
    std::shared_ptr<Quantum::Prototypes::Hierarchy_Prototype>
    Convert(const EntityPrototypeConverter& converter) const;
};

struct LevelButton_Prototype {
    std::optional<ScriptKey> gameplayElementEntity;
    Quantum::AssetRefLevelButtonConfig Config;

    // 0x7afeda8
    std::shared_ptr<Quantum::Prototypes::LevelButton_Prototype>
    Convert(const EntityPrototypeConverter& converter) const;
};

struct RailCollider_Prototype {
    std::optional<ScriptKey> railEntity;
    FP distanceOnRail;
    std::optional<ScriptKey> nearbyRailColLeft;
    std::optional<ScriptKey> nearbyRailColRight;

    // 0x7afee7c
    std::shared_ptr<Quantum::Prototypes::RailCollider_Prototype>
    Convert(const EntityPrototypeConverter& converter) const;
};

// --- entity component scripts and the assets -------------------------------------------------

// An entity component script (EntityComponentBase): carries one component prototype.
class EntityComponentBase {
  public:
    virtual ~EntityComponentBase() = default;
    // 0x7ab56e0 - Nothing to bring up to date by default (the framework's transform,
    // collider and joint component scripts override it; the game's prefabs use none).
    virtual void Refresh() {}
    // EntityComponentBase<TPrototype>.CreatePrototype (shared generic code) hands out the
    // stored prototype; EntityComponentBase<TPrototype, TEditorPrototype> converts the
    // editor-side twin.
    virtual std::shared_ptr<Quantum::ComponentPrototype>
    CreatePrototype(const EntityPrototypeConverter& converter) const = 0;
};

// 0x7ab598c - EntityPrototypeAsset.PrepareAsset (0x7ab5980 Loaded only calls it): fills
// `settings.Container` from the prefab's EntityPrototype script `parent` (identified by
// `parentKey`; nothing happens without one) and its entity component scripts. If the prefab
// has an entity view script, the view asset nested next to the prototype asset (same path,
// "EntityView" instead of "EntityPrototype") becomes a View prototype; `assetGuidOfPath`
// finds it (UnityDB.GetAssetGuid). Failures to find it are logged.
void PrepareEntityPrototypeAsset(
    Quantum::EntityPrototype& settings, EntityPrototype* parent, const ScriptKey& parentKey,
    const std::vector<std::unique_ptr<EntityComponentBase>>& components,
    const std::function<Quantum::AssetGuid(const std::string&)>& assetGuidOfPath);

// 0x7ab75a8 - MapAsset.PrepareAsset: one entity prototype container per baked map entity,
// each with the prototypes of its flat container (Collect order). Nothing happens without
// a prototype list.
void PrepareMapAsset(
    Quantum::Map& settings,
    const std::vector<Quantum::Prototypes::FlatEntityPrototypeContainer>* prototypes);

} // namespace rr::content::unity
