// shape2d.h - Quantum.Shape2D: the collision shape of a 2D collider, and its nested
// per-type structs (CircleShape, BoxShape, PolygonShape, EdgeShape, CompoundShape2D).
//
// The original is an explicit-layout struct whose fields overlap; frame snapshots and the
// generated hash functions depend on that overlap, so it is reproduced here:
//
//   0x00 type, padding, UserTag, padding
//   0x08 LocalTransform (24 bytes) | Centroid
//   0x20 BroadRadius | Circle | Box | Polygon | Edge | Compound (32 bytes)
//
// The first field of every primitive shape struct IS the broad-phase radius: a circle's
// radius and an edge's half-length double as BroadRadius. In the original the compound
// struct starts at 0x08 with its centroid (the same bytes as Shape2D.Centroid) followed
// by an unused slot; here Compound holds the part from 0x20 on and the centroid is read
// through Shape2D::Centroid. A compound's area sits in the BroadRadius slot.
#pragma once

#include "asset_guid.h"
#include "transform.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"

#include <cstdint>
#include <span>

namespace Quantum {

struct PolygonCollider;

enum class Shape2DType : std::uint8_t {
    None = 0,
    Circle = 1,
    Polygon = 2,
    Box = 4,
    Edge = 8,
    Compound = 16,
};

struct Shape2D {
    static constexpr int SIZE = 64;
    static constexpr int SHAPE_DATA = 32;

    struct CircleShape {
        static constexpr int SIZE = 8;
        FP Radius;
        // 0x7d33430
        std::int32_t GetHashCode() const;
    };

    struct BoxShape {
        static constexpr int SIZE = 24;
        FP _broadRadius;
        FPVector2 _extents; // half-size on each axis

        // 0x7d33450
        FPVector2 Extents() const { return _extents; }
        // 0x7d3345c - also refreshes the broad radius (length of the half-diagonal).
        void SetExtents(FPVector2 value);
        // 0x7d334a0
        std::int32_t GetHashCode() const;
    };

    struct PolygonShape {
        static constexpr int SIZE = 32;
        FP _broadRadius;
        ::Quantum::AssetRef _assetRef; // AssetRefPolygonCollider (same layout and hash)
        FPVector2 _centroid;

        // 0x7d33448
        ::Quantum::AssetRef AssetRef() const { return _assetRef; }
        // 0x7d33078
        std::int32_t GetHashCode() const;
        // TODO(0x7d32f08, Quantum.Shape2D.PolygonShape.Serialize): FP _broadRadius, then
        // the asset ref (AssetGuid.Serialize, not implemented yet), then _centroid.
    };

    struct EdgeShape {
        static constexpr int SIZE = 8;
        FP Extent; // half-length along the local X axis
        // 0x7d334e0
        std::int32_t GetHashCode() const;
    };

    // A shape made of other shapes, stored in the frame heap (_shapesBuffer is a heap
    // offset; the original type is Quantum.Ptr). The original struct (SIZE 56) begins
    // with _centroid and an unused 8-byte slot; see the layout note above.
    struct CompoundShape2D {
        static constexpr int SIZE = 56;
        static constexpr int DEFAULT_CAPACITY = 4;

        FP _area;
        FP _broadRadius;
        std::int32_t _shapesBuffer = 0;
        std::uint16_t _count = 0;
        std::uint16_t _capacity = 0;
        std::int32_t _isPersistent = 0;
        std::int32_t _padding = 0;

        // 0x7d330cc
        bool IsPersistent() const { return _isPersistent == 1; }
        // 0x7d32fd4 - `centroid` is the compound's _centroid (Shape2D::Centroid).
        std::int32_t GetHashCode(FPVector2 centroid) const;

        // Need the frame heap:
        // TODO(0x7d32230, Quantum.Shape2D.CompoundShape2D.CopyFrom): deep-copies the
        // nested shape buffer of another compound.
        // TODO(0x7d330dc, Quantum.Shape2D.CompoundShape2D.AddShape): appends a shape
        // (EnsureCapacity) and, if asked, updates the area-weighted centroid and the broad
        // radius (max of |child position| + child broad radius).
        // TODO(0x7d32bd8, Quantum.Shape2D.CompoundShape2D.GetShapes): resolves
        // _shapesBuffer through the frame heap; returns false when empty.
        // TODO(0x7d3288c, Quantum.Shape2D.CompoundShape2D.FreePersistent)
        // TODO(0x7d331e0, Quantum.Shape2D.CompoundShape2D.EnsureCapacity)
        // TODO(0x7d32ce4, Quantum.Shape2D.CompoundShape2D.Serialize)
    };

    Shape2DType _type = Shape2DType::None;
    std::uint8_t _padding1 = 0;
    std::uint16_t UserTag = 0;
    std::int32_t _padding2 = 0;
    union {
        Transform2D LocalTransform;
        FPVector2 Centroid; // compound shapes
    };
    union {
        FP BroadRadius;
        CircleShape Circle;
        BoxShape Box;
        PolygonShape Polygon;
        EdgeShape Edge;
        CompoundShape2D Compound;
    };

    Shape2D() : LocalTransform(), Polygon() {}

    // 0x7d32a68 get_Type
    Shape2DType Type() const { return _type; }

    // 0x7d32a70
    static Shape2D CreateCircle(FP radius, FPVector2 posOffset = {});
    // 0x7d32a94 - takes the asset's GUID, radius and centroid.
    static Shape2D CreatePolygon(const PolygonCollider& asset, FPVector2 posOffset = {},
                                 FP rotOffset = {});
    // 0x7d32afc
    static Shape2D CreateBox(FPVector2 extents, FPVector2 posOffset = {}, FP rotOffset = {});
    // 0x7d32b80 - the extent is raised to at least FP.Epsilon.
    static Shape2D CreateEdge(FP extent, FPVector2 posOffset = {}, FP rotOffset = {});
    // TODO(0x7d321a8, Quantum.Shape2D.CreatePersistentCompound): a Compound shape flagged
    // persistent, created with the frame heap.

    // 0x7d32c28 - area of the shape. For a polygon the original looks the PolygonCollider
    // asset up through the frame; here the caller passes its vertices instead (unused for
    // the other types). Edges report a nominal area of 0.2 * Extent.
    FP GetArea(std::span<const FPVector2> polygonVertices = {}) const;

    // TODO(0x7d31208, Quantum.Shape2D.CalculateInertiaFactor): needs the frame for
    // polygon assets and compound children.

    // 0x7d32f50 - for primitive shapes the shape data is hashed through the PolygonShape
    // view of the overlay, whatever the type.
    std::int32_t GetHashCode() const;

    // TODO(0x7d32628, Quantum.Shape2D.Serialize): per-type serialisation including the
    // compound's heap buffer; needs FrameSerializer's heap access and AssetGuid.Serialize.
};

static_assert(sizeof(Shape2D::CompoundShape2D) == Shape2D::CompoundShape2D::SIZE - 24);
static_assert(sizeof(Shape2D) == Shape2D::SIZE);

// Quantum.Shape2DConfig: the editable description a collider is built from.
// TODO(0x7d334f8, Quantum.Shape2DConfig.CreateShape): builds the Shape2D (polygon asset
// lookup and compound children need the frame). Also
// TODO(0x7d33818, Quantum.Shape2D.CompoundShapeData2D.CreateShape).

} // namespace Quantum
