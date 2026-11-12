// shape2d.cpp - Quantum.Shape2D and its nested shape structs.
#include "shape2d.h"

#include "polygon_collider.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

namespace Quantum {

namespace detail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;
using detail::HashCombine;
using detail::HashSeed;

namespace {

Shape2D MakePlaced(Shape2DType type, FPVector2 posOffset, FP rotOffset) {
    Shape2D shape;
    shape._type = type;
    shape.LocalTransform = {posOffset, rotOffset};
    return shape;
}

} // namespace

// --- nested shape structs --------------------------------------------------------------

// 0x7d33430
std::int32_t Shape2D::CircleShape::GetHashCode() const {
    return HashCombine(HashSeed, Radius.GetHashCode());
}

// 0x7d3345c
void Shape2D::BoxShape::SetExtents(FPVector2 value) {
    _broadRadius = value.Magnitude();
    _extents = value;
}

// 0x7d334a0
std::int32_t Shape2D::BoxShape::GetHashCode() const {
    return HashCombine(HashCombine(HashSeed, _broadRadius.GetHashCode()), _extents.GetHashCode());
}

// 0x7d33078
std::int32_t Shape2D::PolygonShape::GetHashCode() const {
    std::int32_t hash = HashCombine(HashSeed, _broadRadius.GetHashCode());
    hash = HashCombine(hash, _assetRef.GetHashCode());
    return HashCombine(hash, _centroid.GetHashCode());
}

// 0x7d334e0
std::int32_t Shape2D::EdgeShape::GetHashCode() const {
    return HashCombine(HashSeed, Extent.GetHashCode());
}

// 0x7d32fd4 - the generated field order starts with _isPersistent.
std::int32_t Shape2D::CompoundShape2D::GetHashCode(FPVector2 centroid) const {
    std::int32_t hash = HashCombine(HashSeed, _isPersistent);
    hash = HashCombine(hash, centroid.GetHashCode());
    hash = HashCombine(hash, _area.GetHashCode());
    hash = HashCombine(hash, _broadRadius.GetHashCode());
    hash = HashCombine(hash, _count);
    hash = HashCombine(hash, _capacity);
    return HashCombine(hash, _shapesBuffer);
}

// --- Shape2D ---------------------------------------------------------------------------

// 0x7d32a70
Shape2D Shape2D::CreateCircle(FP radius, FPVector2 posOffset) {
    Shape2D shape = MakePlaced(Shape2DType::Circle, posOffset, FP::Zero());
    shape.Circle.Radius = radius;
    return shape;
}

// 0x7d32a94
Shape2D Shape2D::CreatePolygon(const PolygonCollider& asset, FPVector2 posOffset, FP rotOffset) {
    Shape2D shape = MakePlaced(Shape2DType::Polygon, posOffset, rotOffset);
    shape.Polygon._broadRadius = asset.Radius;
    shape.Polygon._assetRef = ::Quantum::AssetRef{asset.Guid};
    shape.Polygon._centroid = asset.Centroid;
    return shape;
}

// 0x7d32afc
Shape2D Shape2D::CreateBox(FPVector2 extents, FPVector2 posOffset, FP rotOffset) {
    Shape2D shape = MakePlaced(Shape2DType::Box, posOffset, rotOffset);
    shape.Box.SetExtents(extents);
    return shape;
}

// 0x7d32b80
Shape2D Shape2D::CreateEdge(FP extent, FPVector2 posOffset, FP rotOffset) {
    Shape2D shape = MakePlaced(Shape2DType::Edge, posOffset, rotOffset);
    shape.Edge.Extent = FPMath::Max(extent, FP::Epsilon());
    return shape;
}

// 0x7d32c28
FP Shape2D::GetArea(std::span<const FPVector2> polygonVertices) const {
    switch (_type) {
    case Shape2DType::Circle:
        return Circle.Radius * Circle.Radius * FP::Pi();
    case Shape2DType::Polygon:
        return FPVector2::CalculatePolygonArea(polygonVertices);
    case Shape2DType::Box:
        // (2 * extents.X) * (2 * extents.Y), with one shift for the whole product.
        return FP::FromRaw(
            detail::WrapMul(Box._extents.X.RawValue(), Box._extents.Y.RawValue()) >> 14);
    case Shape2DType::Edge:
        // 2 * Extent * 0.1, again shifted once.
        return FP::FromRaw(detail::WrapMul(Edge.Extent.RawValue(), FP::_0_10().RawValue()) >> 15);
    case Shape2DType::Compound:
        return Compound._area;
    default:
        return FP::Zero();
    }
}

// 0x7d32f50
std::int32_t Shape2D::GetHashCode() const {
    std::int32_t hash = HashCombine(HashSeed, static_cast<std::int32_t>(_type));
    hash = HashCombine(hash, UserTag);
    if (_type == Shape2DType::Compound) {
        return HashCombine(hash, Compound.GetHashCode(Centroid));
    }
    hash = HashCombine(hash, LocalTransform.GetHashCode());
    return HashCombine(hash, Polygon.GetHashCode());
}

} // namespace Quantum
