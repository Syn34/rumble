// builtin_assets.h - the small asset types quantum.core defines itself (quantum.core.dll):
// Quantum.BinaryData, Quantum.PhysicsMaterial, Quantum.CharacterController2DConfig,
// Quantum.CharacterController3DConfig, Quantum.EntityView, Quantum.EntityPrototype and
// Quantum.TerrainCollider, plus their typed references. (Map, NavMesh, NavMeshAgentConfig
// and PolygonCollider have their own headers.)
//
// Field defaults are those of the original constructors.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "quantum_core/asset_guid.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/byte_stream.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace Quantum {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;

struct TriangleCCW;

// A ByteStream together with the bytes it reads (BinaryData::CreateByteStream).
struct BinaryDataStream {
    std::vector<std::uint8_t> Bytes;
    ByteStream Stream{&Bytes};

    explicit BinaryDataStream(std::vector<std::uint8_t> bytes) : Bytes(std::move(bytes)) {}
    BinaryDataStream(const BinaryDataStream&) = delete;
    BinaryDataStream& operator=(const BinaryDataStream&) = delete;
};

// Opaque bytes baked by the editor (navmesh data, static mesh triangles...), optionally
// gzip-compressed.
class BinaryData final : public AssetObject, public IBuiltInAssetObject {
  public:
    std::vector<std::uint8_t> Data;
    bool IsCompressed = false;

    // 0x7ce0bc8 - empty data.
    BinaryData() = default;

    // 0x7ce0aa0
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }

    // 0x7ce0b44 - a stream over a copy of the data, decompressed first if needed.
    std::unique_ptr<BinaryDataStream> CreateByteStream() const;
};

// How two materials' coefficients combine. Original: Quantum.PhysicsCombineFunction.
enum class PhysicsCombineFunction : std::int32_t { Max = 0, Min = 1, Average = 2, Multiply = 3 };

class PhysicsMaterial final : public AssetObject, public IBuiltInAssetObject {
  public:
    static constexpr std::int64_t DEFAULT_ID = 1;

    FP Restitution;
    PhysicsCombineFunction RestitutionCombineFunction = PhysicsCombineFunction::Min;
    // FP.FromFloat_UNSAFE(0.4f) in the constructor: 0.4f * 65536 truncated.
    FP FrictionStatic = FP::FromRaw(26214);
    FP FrictionDynamic = FP::FromRaw(26214);
    PhysicsCombineFunction FrictionCombineFunction = PhysicsCombineFunction::Min;

    // 0x7ce5514
    PhysicsMaterial() = default;

    // 0x7ce5438
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }
    // 0x7ce54e0 - clamps both friction coefficients into [0, 1].
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class CharacterController2DConfig final : public AssetObject, public IBuiltInAssetObject {
  public:
    static constexpr std::int64_t DEFAULT_ID = 3;

    FPVector2 Offset;
    FP Radius = FP::FromRaw(45875);               // 0.7
    FP MaxPenetration = FP::FromRaw(655);         // 0.01
    FP PenetrationCorrection = FP::FromRaw(32768); // 0.5
    FP Extent = FP::FromRaw(13107);               // 0.2
    std::int32_t MaxContacts = 4;
    std::int32_t LayerMask = -1; // Quantum.LayerMask (all layers)
    bool UseManifoldNormal = false;
    bool AirControl = true;
    FP Acceleration = FP::FromInt(25);
    FP Braking = FP::FromInt(10);
    FP BaseJumpImpulse = FP::FromInt(7);
    FP MaxSpeed = FP::FromInt(3);
    FPVector2 Gravity = FPVector2(FP::FromRaw(0), FP::FromInt(-10));
    FP MaxSlope = FP::FromInt(60);
    FP MaxSlopeSpeed = FP::FromInt(5);

    // 0x7ce0e50
    CharacterController2DConfig() = default;

    // 0x7ce0d54
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }
    // 0x7ce0e0c - caches the gravity's length and direction.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;

    FP GravityStrength() const { return _gravityStrength; }
    // 0x7ce0dfc
    void SetGravityStrength(FP value) { _gravityStrength = value; }
    FPVector2 GravityNormalized() const { return _gravityNormalized; }
    // 0x7ce0e04
    void SetGravityNormalized(FPVector2 value) { _gravityNormalized = value; }

  private:
    FP _gravityStrength;
    FPVector2 _gravityNormalized;
};

class CharacterController3DConfig final : public AssetObject, public IBuiltInAssetObject {
  public:
    static constexpr std::int64_t DEFAULT_ID = 4;

    FPVector3 Offset = FPVector3(FP::FromRaw(0), FP::FromRaw(32768), FP::FromRaw(0)); // y 0.5
    FP Radius = FP::FromRaw(32768);               // 0.5
    FP MaxPenetration = FP::FromRaw(3275);        // 0.05
    FP PenetrationCorrection = FP::FromRaw(32768); // 0.5
    FP Extent = FP::FromRaw(6553);                // 0.1
    std::int32_t MaxContacts = 8;
    std::int32_t LayerMask = -1; // Quantum.LayerMask (all layers)
    bool UseManifoldNormal = false;
    bool AirControl = true;
    FP Acceleration = FP::FromInt(25);
    FP Braking = FP::FromInt(10);
    FP BaseJumpImpulse = FP::FromInt(6);
    FP MaxSpeed = FP::FromInt(3);
    FPVector3 Gravity = FPVector3(FP::FromRaw(0), FP::FromInt(-10), FP::FromRaw(0));
    FP MaxSlope = FP::FromInt(60);
    FP MaxSlopeSpeed = FP::FromInt(5);

    // 0x7ce11a4
    CharacterController3DConfig() = default;

    // 0x7ce1028
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }
    // 0x7ce1108 - caches the gravity's length and direction.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;

    // 0x7ce10d0
    FP GravityStrength() const { return _gravityStrength; }
    // 0x7ce10d8
    void SetGravityStrength(FP value) { _gravityStrength = value; }
    // 0x7ce10e0
    FPVector3 GravityNormalized() const { return _gravityNormalized; }
    // 0x7ce10f4
    void SetGravityNormalized(const FPVector3& value) { _gravityNormalized = value; }

  private:
    FP _gravityStrength;
    FPVector3 _gravityNormalized;
};

// The presentation-side view of an entity; the simulation only passes its GUID around.
class EntityView final : public AssetObject, public IBuiltInAssetObject {
  public:
    // 0x7ce157c
    EntityView() = default;
    // 0x7ce14d4
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }
};

// A baked entity (its component prototypes) that frames instantiate.
// TODO(0x7ce1424, Quantum.EntityPrototype..ctor): the Container field
// (Quantum.EntityPrototypeContainer, the component prototypes) is missing until the
// prototype system exists; the class is a placeholder so references can be resolved.
class EntityPrototype final : public AssetObject, public IBuiltInAssetObject {
  public:
    EntityPrototype() = default;
    // 0x7ce137c
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }
};

// 0x7ce5384 Serialize, 0x7ce5394 ToString, 0x7ce539c / 0x7ce53ac Equals, 0x7ce5424
// GetHashCode, 0x7ce542c !=
using AssetRefPhysicsMaterial = TypedAssetRef<PhysicsMaterial>;

// A height-field collider: Resolution x Resolution heights, an optional hole bit per cell.
class TerrainCollider final : public AssetObject, public IBuiltInAssetObject {
  public:
    static constexpr int HoleMaskBitsPerBlock = 64;

    std::vector<FP> HeightMap;
    std::vector<std::uint64_t> HoleMask; // empty: no holes
    FPVector3 Position;
    FPVector3 Scale;
    std::int32_t Resolution = 0;
    AssetRefPhysicsMaterial PhysicsMaterial;

    // 0x7ce60b4
    TerrainCollider() = default;

    // 0x7ce588c
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }

    // 0x7ce5934 - the triangles Bake produced.
    const std::vector<TriangleCCW>& Triangles() const;

    // 0x7ce593c - the hole bit of cell (x, y) (index Resolution * x + y); false without a
    // hole mask or outside it.
    bool IsHole(int x, int y) const;
    // 0x7ce5994 - sets the hole bit; throws std::out_of_range outside the mask.
    void SetHole(int x, int y);

    // TODO(0x7ce5b64, Quantum.TerrainCollider.Bake): triangulates the height field
    // (skipping holes) into TriangleCCW with the given static collider index; needs the 3D
    // static collider setup.

  private:
    std::shared_ptr<std::vector<TriangleCCW>> _triangles;
};

// Typed references. Addresses per type: op_Implicit, Serialize, ToString, Equals (typed,
// boxed), GetHashCode, ==, != and the Ext.FindAsset extension, where the type has them.
//
// 0x7ce0988 ToString, 0x7ce0a10 / 0x7ce0a20 Equals, 0x7ce0a98 GetHashCode
using AssetRefBinaryData = TypedAssetRef<BinaryData>;
// 0x7ce0c34 op_Implicit, 0x7ce0c48 ToString, 0x7ce0c50 / 0x7ce0c60 Equals, 0x7ce0cd8
// GetHashCode, 0x7ce0ce0 Ext.FindAsset
using AssetRefCharacterController2DConfig = TypedAssetRef<CharacterController2DConfig>;
// 0x7ce0f10 op_Implicit, 0x7ce0f1c ToString, 0x7ce0f24 / 0x7ce0f34 Equals, 0x7ce0fac
// GetHashCode, 0x7ce0fb4 Ext.FindAsset
using AssetRefCharacterController3DConfig = TypedAssetRef<CharacterController3DConfig>;
// 0x7ce142c Serialize, 0x7ce143c ToString, 0x7ce1444 / 0x7ce1454 Equals, 0x7ce14cc GetHashCode
using AssetRefEntityView = TypedAssetRef<EntityView>;
// 0x7ce57f4 ToString, 0x7ce57fc / 0x7ce580c Equals, 0x7ce5884 GetHashCode
using AssetRefTerrainCollider = TypedAssetRef<TerrainCollider>;
// (AssetRefEntityPrototype has its own struct in asset_ref_entity_prototype.h.)

} // namespace Quantum
