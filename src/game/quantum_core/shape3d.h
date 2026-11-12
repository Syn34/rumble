// shape3d.h - Quantum.Shape3D: the collision shape of a 3D collider, with its nested
// per-type structs (SphereShape, BoxShape, MeshShape, TerrainShape, CompoundShape3D).
//
// Like Shape2D this is an explicit-layout struct with overlapping fields, reproduced:
//
//   0x00 type, padding, UserTag, padding
//   0x08 LocalTransform (56 bytes) | Centroid
//   0x40 BroadRadius | Sphere | Box | Mesh | Terrain | Compound (32 bytes)
//
// The first field of every primitive struct is the broad-phase radius (a sphere's radius
// doubles as it). In the original the compound struct starts at 0x08 with its centroid
// (the same bytes as Shape3D.Centroid) and an unused 32-byte slot; here Compound holds
// the part from 0x40 on, whose first field (its broad radius) shares the BroadRadius
// slot.
#pragma once

#include "asset_guid.h"
#include "transform.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>
#include <optional>

namespace Quantum {

enum class Shape3DType : std::uint8_t {
    None = 0,
    Sphere = 1,
    Box = 2,
    Mesh = 4,
    Terrain = 8,
    Compound = 16,
};

struct Shape3D {
    static constexpr int SIZE = 96;
    static constexpr int SHAPE_START = 64;

    struct SphereShape {
        static constexpr int SIZE = 8;
        FP Radius;
    };

    struct BoxShape {
        static constexpr int SIZE = 32;
        FP _broadRadius;
        FPVector3 _extents; // half-size on each axis

        // 0x7d3ba04
        FPVector3 Extents() const { return _extents; }
        // 0x7d3ba18 - also refreshes the broad radius (length of the half-diagonal).
        void SetExtents(const FPVector3& value);
    };

    struct MeshShape {
        static constexpr int SIZE = 16;
        FP _broadRadius; // unused slot shared with BroadRadius (zero for meshes)
        std::int32_t _padding = 0;
        std::int32_t MeshIndex = 0; // index of the static mesh in the scene
    };

    struct TerrainShape {
        static constexpr int SIZE = 16;
        FP _broadRadius;
        ::Quantum::AssetRef TerrainCollider; // AssetRefTerrainCollider
    };

    // A shape made of other shapes stored in the frame heap (_shapesBuffer is a
    // Quantum.Ptr heap offset). The original struct (SIZE 88) begins with _centroid and
    // an unused 32-byte slot; see the layout note above.
    struct CompoundShape3D {
        static constexpr int SIZE = 88;
        static constexpr int DEFAULT_CAPACITY = 4;

        FP _broadRadius;
        FP _volume;
        std::int32_t _shapesBuffer = 0;
        std::uint16_t _count = 0;
        std::uint16_t _capacity = 0;
        std::uint8_t _isPersistent = 0;
        std::uint8_t _padding0[3] = {};
        std::int32_t _padding = 0;

        // 0x7d3b184 - the generated field order starts with _isPersistent. `centroid` is
        // the compound's _centroid (Shape3D::Centroid).
        std::int32_t GetHashCode(const FPVector3& centroid) const;

        // Need the frame heap:
        // TODO(0x7d359f8, Quantum.Shape3D.CompoundShape3D.CopyFrom)
        // TODO(0x7d3b228, Quantum.Shape3D.CompoundShape3D.AddShape): appends a shape and
        // optionally updates the volume-weighted centroid and broad radius.
        // TODO(0x7d3b5b8, Quantum.Shape3D.CompoundShape3D.RemoveShapes)
        // TODO(0x7d3ae70, Quantum.Shape3D.CompoundShape3D.GetShapes)
        // TODO(0x7d3b7ac, Quantum.Shape3D.CompoundShape3D.ResetCompoundMeta)
        // TODO(0x7d36098, Quantum.Shape3D.CompoundShape3D.FreePersistent)
        // TODO(0x7d3b35c, Quantum.Shape3D.CompoundShape3D.EnsureCapacity)
        // TODO(0x7d3aec0, Quantum.Shape3D.CompoundShape3D.Serialize)
    };

    Shape3DType _type = Shape3DType::None;
    std::uint8_t _padding1 = 0;
    std::uint16_t UserTag = 0;
    std::int32_t _padding2 = 0;
    union {
        Transform3D LocalTransform;
        FPVector3 Centroid; // compound shapes
    };
    union {
        FP BroadRadius;
        MeshShape Mesh;
        SphereShape Sphere;
        BoxShape Box;
        TerrainShape Terrain;
        CompoundShape3D Compound;
    };

    Shape3D() : LocalTransform(), Box() {}

    // 0x7d3a9e0 get_Type
    Shape3DType Type() const { return _type; }

    // Offsets are C# nullables: a missing position means zero, a missing rotation means
    // identity.
    // 0x7d3a9e8
    static Shape3D CreateSphere(FP radius, std::optional<FPVector3> posOffset = {});
    // 0x7d3aae0
    static Shape3D CreateBox(const FPVector3& extents, std::optional<FPVector3> posOffset = {},
                             std::optional<FPQuaternion> rotOffset = {});
    // 0x7d3ac90
    static Shape3D CreateMesh(std::int32_t meshIndex, std::optional<FPVector3> posOffset = {},
                              std::optional<FPQuaternion> rotOffset = {});
    // TODO(0x7d35960, Quantum.Shape3D.CreatePersistentCompound): zeroed Compound shape
    // with _isPersistent = 1, created with the frame heap.

    // 0x7d3adf0 - sphere: 4/3 pi r^3; box: 8 * product of extents; compound: stored
    // volume; meshes and terrain: zero.
    FP GetVolume() const;

    // TODO(0x7d33ed8, Quantum.Shape3D.CalculateInertiaTensorFactor): needs the frame for
    // compound children.

    // 0x7d3b0e8 - for primitive shapes the shape data is hashed through the BoxShape view
    // of the overlay, whatever the type.
    std::int32_t GetHashCode() const;

    // TODO(0x7d35e00, Quantum.Shape3D.Serialize): per-type serialisation including the
    // compound heap buffer.
};

static_assert(sizeof(Shape3D::CompoundShape3D) == Shape3D::CompoundShape3D::SIZE - 56);
static_assert(sizeof(Shape3D) == Shape3D::SIZE);

// Quantum.Shape3DConfig: the editable description a 3D collider is built from.
// TODO(0x7d3ba74, Quantum.Shape3DConfig.CreateShape): builds the Shape3D (compound
// children need the frame). Also TODO(0x7d3be00, Quantum.Shape3D.CompoundShapeData3D.CreateShape).

} // namespace Quantum
