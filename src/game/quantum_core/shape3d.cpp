// shape3d.cpp - Quantum.Shape3D and its nested shape structs.
#include "shape3d.h"

#include "photon_deterministic/fp_internal.h"

namespace Quantum {

namespace detail = Photon::Deterministic::detail;
using detail::HashCombine;
using detail::HashSeed;

namespace {

Shape3D MakePlaced(Shape3DType type, const std::optional<FPVector3>& posOffset,
                   const std::optional<FPQuaternion>& rotOffset) {
    Shape3D shape;
    shape._type = type;
    shape.LocalTransform.Position = posOffset.value_or(FPVector3::Zero());
    shape.LocalTransform.Rotation = rotOffset.value_or(FPQuaternion::Identity());
    return shape;
}

} // namespace

// 0x7d3ba18
void Shape3D::BoxShape::SetExtents(const FPVector3& value) {
    _broadRadius = value.Magnitude();
    _extents = value;
}

// 0x7d3b184
std::int32_t Shape3D::CompoundShape3D::GetHashCode(const FPVector3& centroid) const {
    std::int32_t hash = HashCombine(HashSeed, _isPersistent);
    hash = HashCombine(hash, centroid.GetHashCode());
    hash = HashCombine(hash, _volume.GetHashCode());
    hash = HashCombine(hash, _broadRadius.GetHashCode());
    hash = HashCombine(hash, _count);
    hash = HashCombine(hash, _capacity);
    return HashCombine(hash, _shapesBuffer);
}

// 0x7d3a9e8
Shape3D Shape3D::CreateSphere(FP radius, std::optional<FPVector3> posOffset) {
    Shape3D shape = MakePlaced(Shape3DType::Sphere, posOffset, std::nullopt);
    shape.Sphere.Radius = radius;
    return shape;
}

// 0x7d3aae0
Shape3D Shape3D::CreateBox(const FPVector3& extents, std::optional<FPVector3> posOffset,
                           std::optional<FPQuaternion> rotOffset) {
    Shape3D shape = MakePlaced(Shape3DType::Box, posOffset, rotOffset);
    shape.Box.SetExtents(extents);
    return shape;
}

// 0x7d3ac90
Shape3D Shape3D::CreateMesh(std::int32_t meshIndex, std::optional<FPVector3> posOffset,
                            std::optional<FPQuaternion> rotOffset) {
    Shape3D shape = MakePlaced(Shape3DType::Mesh, posOffset, rotOffset);
    shape.Mesh.MeshIndex = meshIndex;
    return shape;
}

// 0x7d3adf0
FP Shape3D::GetVolume() const {
    switch (_type) {
    case Shape3DType::Sphere: {
        // r^3 * 4pi, then divided by 3 * 2^16 in one truncating step (instead of a
        // shift followed by a division).
        const FP radius = Sphere.Radius;
        const FP cube = radius * radius * radius;
        constexpr FP::Raw fourPi = 4 * FP::Pi().RawValue();
        return FP::FromRaw(detail::WrapMul(cube.RawValue(), fourPi) / (FP::Raw{3} << 16));
    }
    case Shape3DType::Box: {
        const FPVector3& e = Box._extents;
        // 8 * x * y * z: the factor 8 folded into the final shift.
        return FP::FromRaw(detail::WrapMul((e.X * e.Y).RawValue(), e.Z.RawValue()) >> 13);
    }
    case Shape3DType::Compound:
        return Compound._volume;
    default:
        return FP::Zero();
    }
}

// 0x7d3b0e8
std::int32_t Shape3D::GetHashCode() const {
    std::int32_t hash = HashCombine(HashSeed, static_cast<std::int32_t>(_type));
    hash = HashCombine(hash, UserTag);
    if (_type == Shape3DType::Compound) {
        return HashCombine(hash, Compound.GetHashCode(Centroid));
    }
    hash = HashCombine(hash, LocalTransform.GetHashCode());
    hash = HashCombine(hash, Box._broadRadius.GetHashCode());
    return HashCombine(hash, Box._extents.GetHashCode());
}

} // namespace Quantum
