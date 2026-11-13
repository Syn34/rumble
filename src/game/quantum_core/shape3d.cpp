// shape3d.cpp - Quantum.Shape3D and its nested shape structs.
#include "shape3d.h"
#include "shape3d_config.h"

#include "quantum_core/frame_serializer.h"

#include <stdexcept>

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

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

// 0x7d33ed8
FPVector3 Shape3D::CalculateInertiaTensorFactor(const FPVector3& centerOfMass) const {
    const auto sq = [](FP::Raw v) { return (v * v) >> 16; };
    const FP::Raw dx = sq(LocalTransform.Position.X.RawValue() - centerOfMass.X.RawValue());
    const FP::Raw dy = sq(LocalTransform.Position.Y.RawValue() - centerOfMass.Y.RawValue());
    const FP::Raw dz = sq(LocalTransform.Position.Z.RawValue() - centerOfMass.Z.RawValue());
    FP::Raw fx = dz + dy;
    FP::Raw fy = dz + dx;
    FP::Raw fz = dy + dx;
    if (_type == Shape3DType::Sphere) {
        const FP::Raw r = Sphere.Radius.RawValue();
        // 2 r^2 / 5, the division truncating towards zero.
        const FP::Raw term = ((r * r) << 1) / (FP::Raw{5} << 16);
        fx += term;
        fy += term;
        fz += term;
    } else if (_type == Shape3DType::Box) {
        const FP::Raw ex = sq(Box._extents.X.RawValue());
        const FP::Raw ey = sq(Box._extents.Y.RawValue());
        const FP::Raw ez = sq(Box._extents.Z.RawValue());
        constexpr FP::Raw third = 0x5555;
        fx += ((ez + ey) * third) >> 16;
        fy += ((ez + ex) * third) >> 16;
        fz += ((ey + ex) * third) >> 16;
    }
    return {FP::FromRaw(fx), FP::FromRaw(fy), FP::FromRaw(fz)};
}

// 0x7d35e00
void Shape3D::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* shape = static_cast<Shape3D*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    stream.Serialize(reinterpret_cast<std::uint8_t*>(&shape->_type));
    stream.Serialize(&shape->_padding1);
    stream.Serialize(&shape->UserTag);
    stream.Serialize(&shape->_padding2);
    if (shape->_type == Shape3DType::Compound) {
        // TODO(0x7d3aec0, Quantum.Shape3D.CompoundShape3D.Serialize) is not reimplemented.
        throw std::logic_error("Shape3D.Serialize: compound shapes are not supported yet");
    }
    Transform3D::Serialize(&shape->LocalTransform, serializer);
    // The overlay as four FPs (the box view covers all of it).
    FP::Serialize(&shape->Box._broadRadius, *serializer);
    FP::Serialize(&shape->Box._extents.X, *serializer);
    FP::Serialize(&shape->Box._extents.Y, *serializer);
    FP::Serialize(&shape->Box._extents.Z, *serializer);
}

// 0x7d3ba74
Shape3D Shape3DConfig::CreateShape(Core::FrameBase* /*frame*/) const {
    Shape3D shape;
    switch (ShapeType) {
    case Shape3DType::Sphere: shape = Shape3D::CreateSphere(SphereRadius, PositionOffset); break;
    case Shape3DType::Box: {
        const FP::Raw x = RotationOffset.X.RawValue();
        const FP::Raw y = RotationOffset.Y.RawValue();
        const FP::Raw z = RotationOffset.Z.RawValue();
        const FP::Raw length = Photon::Deterministic::FPMath::SqrtRaw(((y * y) >> 16) + ((x * x) >> 16) + ((z * z) >> 16));
        const FPQuaternion rotation = length == 0 ? FPQuaternion::Identity() : FPQuaternion::Euler(RotationOffset);
        shape = Shape3D::CreateBox(BoxExtents, PositionOffset, rotation);
        break;
    }
    case Shape3DType::Compound:
        throw std::logic_error("Shape3DConfig.CreateShape: compound shapes are not supported yet");
    default: break;
    }
    shape.UserTag = UserTag;
    return shape;
}

} // namespace Quantum
