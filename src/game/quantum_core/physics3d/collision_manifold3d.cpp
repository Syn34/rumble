// collision_manifold3d.cpp - Quantum.Physics3D manifold helpers and raw-int vectors.
#include "collision_manifold3d.h"

#include "quantum_core/physics_components.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_quaternion.h"

#include <cassert>
#include <utility>

namespace Quantum::Physics3D {

namespace detail = Photon::Deterministic::detail;
using detail::HashCombine;
using detail::HashSeed;

namespace {

// The low 32 bits of a raw value, as the original's int cast keeps them.
constexpr std::int32_t LowBits(FP value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value.RawValue()));
}

} // namespace

// 0x7d7b63c
FPVector3 CollisionManifold3D::GetContactPoint(const CollisionManifold3D* manifold,
                                               std::int32_t index) {
    assert(manifold != nullptr);
    return manifold->_contactPoints[index];
}

// 0x7d7b664
FPVector3 CollisionManifold3D::GetAverageContactPoint(const CollisionManifold3D* manifold) {
    assert(manifold != nullptr);
    const std::int32_t count = manifold->_contactPointsCount;
    if (count < 2) {
        return count == 1 ? manifold->_contactPoints[0] : FPVector3::Zero();
    }
    FP::Raw sumX = 0;
    FP::Raw sumY = 0;
    FP::Raw sumZ = 0;
    for (std::int32_t i = 0; i < count; ++i) {
        sumX += manifold->_contactPoints[i].X.RawValue();
        sumY += manifold->_contactPoints[i].Y.RawValue();
        sumZ += manifold->_contactPoints[i].Z.RawValue();
    }
    return {FP::FromRaw(sumX / count), FP::FromRaw(sumY / count), FP::FromRaw(sumZ / count)};
}

// 0x7d79098
void CollisionManifold3D::AddContactPoint(CollisionManifold3D* manifold, const FPVector3& contact) {
    assert(manifold != nullptr && manifold->_contactPointsCount < MAX_CONTACT_POINTS);
    manifold->_contactPoints[manifold->_contactPointsCount++] = contact;
}

// 0x7d7b4d4
void CollisionManifoldInfo3D::InvertEntries() {
    assert(Manifold != nullptr);
    std::swap(Manifold->_m._entryIdxA, Manifold->_m._entryIdxB);
    std::swap(EntryA, EntryB);
    Manifold->_flags ^= CollisionManifold3D::NormalInverted;
}

// 0x7d7c128
void NarrowData3D::Init(NarrowData3D* narrow, const Entry3D* entry, const FPVector3& pos,
                        const FPQuaternion& rot) {
    assert(narrow != nullptr && entry != nullptr && entry->Shape != nullptr);
    narrow->Position = pos;
    const FP inverseLengthSqr = FP::One() / FP::FromRaw(rot.MagnitudeSqrRaw());
    narrow->RotationInverse = {-rot.X * inverseLengthSqr, -rot.Y * inverseLengthSqr,
                               -rot.Z * inverseLengthSqr, rot.W * inverseLengthSqr};
    // The rotation's matrix columns (multiplying by a unit axis is exact).
    narrow->RotatedRight = rot * FPVector3::Right();
    narrow->RotatedUp = rot * FPVector3::Up();
    narrow->RotatedForward = rot * FPVector3::Forward();

    if (entry->Shape->Type() != Shape3DType::Box) {
        return;
    }
    const FPVector3 e = entry->Shape->Box._extents;
    const FP x = e.X;
    const FP y = e.Y;
    const FP z = e.Z;
    const CustomLine edges[CustomEdgesBox::COUNT] = {
        // top face (+y)
        {{-x, y, z}, {x, y, z}},
        {{x, y, z}, {x, y, -z}},
        {{x, y, -z}, {-x, y, -z}},
        {{-x, y, -z}, {-x, y, z}},
        // bottom face (-y)
        {{-x, -y, z}, {x, -y, z}},
        {{x, -y, z}, {x, -y, -z}},
        {{x, -y, -z}, {-x, -y, -z}},
        {{-x, -y, -z}, {-x, -y, z}},
        // vertical edges
        {{-x, -y, z}, {-x, y, z}},
        {{x, -y, z}, {x, y, z}},
        {{x, -y, -z}, {x, y, -z}},
        {{-x, -y, -z}, {-x, y, -z}},
    };
    for (int i = 0; i < CustomEdgesBox::COUNT; ++i) {
        narrow->BoxEdges.Edges[i] = edges[i];
    }

    // Only the single non-zero component of each face normal and face point is written.
    CustomPlane* planes = narrow->BoxPlanes.Planes;
    planes[0].normal.X = FPVector3::Right().X;
    planes[0].pointOnPlane.X = x;
    planes[1].normal.X = FPVector3::Left().X;
    planes[1].pointOnPlane.X = -x;
    planes[2].normal.Y = FPVector3::Up().Y;
    planes[2].pointOnPlane.Y = y;
    planes[3].normal.Y = FPVector3::Down().Y;
    planes[3].pointOnPlane.Y = -y;
    planes[4].normal.Z = FPVector3::Forward().Z;
    planes[4].pointOnPlane.Z = z;
    planes[5].normal.Z = FPVector3::Back().Z;
    planes[5].pointOnPlane.Z = -z;

    // Rotated copies: local vectors expressed with the rotated axes (per-term products).
    const auto rotate = [narrow](const FPVector3& v) {
        return narrow->RotatedRight * v.X + narrow->RotatedUp * v.Y + narrow->RotatedForward * v.Z;
    };
    for (int i = 0; i < CustomEdgesBox::COUNT; ++i) {
        narrow->BoxEdgesRotated.Edges[i] = {rotate(edges[i].start), rotate(edges[i].end)};
    }
    const FPVector3 localNormals[CustomPlanesBox::COUNT] = {
        FPVector3::Right(), FPVector3::Left(), FPVector3::Up(),
        FPVector3::Down(),  FPVector3::Forward(), FPVector3::Back()};
    const FP faceExtents[CustomPlanesBox::COUNT] = {x, -x, y, -y, z, -z};
    for (int i = 0; i < CustomPlanesBox::COUNT; ++i) {
        CustomPlane& plane = narrow->BoxPlanesRotated.Planes[i];
        const FPVector3 normal = rotate(localNormals[i]);
        // The face point is the rotated axis scaled by the (signed) extent.
        const FPVector3 axis = i < 2 ? narrow->RotatedRight
                                     : (i < 4 ? narrow->RotatedUp : narrow->RotatedForward);
        plane.normal = normal;
        plane.pointOnPlane = axis * faceExtents[i];
    }
}

// 0x7d7bdd4
void NarrowData3D::InitFromTransform(NarrowData3D* narrow, const Entry3D* entry,
                                     FPVector3& tPos, FPQuaternion& tRot, bool& useCachedRot,
                                     std::optional<FP> deltaTime) {
    assert(entry != nullptr && entry->Shape != nullptr);
    if (deltaTime.has_value()) {
        assert(entry->Body3D != nullptr);
        const FP dt = *deltaTime;
        const FPVector3 linearStep = entry->Body3D->Velocity * dt;
        const FPVector3 angularStep = entry->Body3D->AngularVelocity * dt;
        const FPQuaternion rotationStep =
            FPQuaternion::CreateFromYawPitchRoll(angularStep.Y, angularStep.X, angularStep.Z);
        // Rotating about the centre of mass moves the origin by com - step * com.
        const FPVector3 com = entry->RotatedCenterOfMass;
        const FPVector3 rotatedCom = com == FPVector3::Zero() ? FPVector3::Zero() : rotationStep * com;
        tPos = (linearStep - rotatedCom) + (tPos + com);
        tRot = FPQuaternion::Product(rotationStep, tRot);
    }
    const Transform3D& local = entry->Shape->LocalTransform;
    if (local.Position != FPVector3::Zero()) {
        tPos = tRot * local.Position + tPos;
    }
    useCachedRot = local.Rotation == FPQuaternion::Identity();
    if (!useCachedRot) {
        tRot = FPQuaternion::Product(tRot, local.Rotation);
    }
    Init(narrow, entry, tPos, tRot);
}

// 0x7d79de4
bool Entry3D::UsesCcd() const {
    if (_compoundRootIndexPlusOne >= 1) {
        assert(_compoundRoot != nullptr);
        return _compoundRoot->_useCCD;
    }
    return _useCCD;
}

// 0x7d79e30
void Entry3D::Init(Entry3D* sceneEntries) {
    if (CompoundRootIndex() >= 0) {
        _compoundRoot = &sceneEntries[CompoundRootIndex()];
    }
}

bool Entry3D::IsTriggerCollider() const {
    return Collider3D != nullptr && Collider3D->IsTrigger();
}

// 0x7d7ac64
void Entry3D::InitNarrowData(Entry3D* entry, const FPVector3* pos, const FPQuaternion* rot) {
    assert(entry != nullptr && pos != nullptr && rot != nullptr);
    FPVector3 position = *pos;
    FPQuaternion rotation = *rot;
    bool useCachedRot = false;
    NarrowData3D::InitFromTransform(&entry->Narrow, entry, position, rotation, useCachedRot, {});
    if (entry->HasDynamicBody && entry->_compoundRootIndexPlusOne < 1) {
        // Without a shape rotation the body's own rotation applies (no explicit rotation).
        CalculateWorldBodySettings(entry, useCachedRot ? std::optional<FPQuaternion>{}
                                                       : std::optional<FPQuaternion>{*rot});
    }
}

void Entry3D::CalculateWorldBodySettings(Entry3D* /*entry*/, std::optional<FPQuaternion> /*rotation*/) {
    // Not reimplemented yet (see the TODO in the header).
}

// 0x7d7b9a8
std::int32_t FPVector3RawInt::GetHashCode() const {
    return HashCombine(HashCombine(HashCombine(HashSeed, RawIntX), RawIntY), RawIntZ);
}

// 0x7d7b9cc
FPVector3RawInt::operator FPVector3() const {
    return {FP::FromRaw(RawIntX), FP::FromRaw(RawIntY), FP::FromRaw(RawIntZ)};
}

// 0x7d7b9e4
FPVector3RawInt FPVector3RawInt::FromFPVector3(const FPVector3& value) {
    return {LowBits(value.X), LowBits(value.Y), LowBits(value.Z)};
}

// 0x7d7bd70
std::int32_t FPQuaternionRawInt::GetHashCode() const {
    std::int32_t hash = HashCombine(HashCombine(HashSeed, RawIntX), RawIntY);
    return HashCombine(HashCombine(hash, RawIntZ), RawIntW);
}

// 0x7d7bd9c
FPQuaternionRawInt::operator FPQuaternion() const {
    return {FP::FromRaw(RawIntX), FP::FromRaw(RawIntY), FP::FromRaw(RawIntZ), FP::FromRaw(RawIntW)};
}

// 0x7d7bdb8
FPQuaternionRawInt FPQuaternionRawInt::FromFPQuaternion(const FPQuaternion& value) {
    return {LowBits(value.X), LowBits(value.Y), LowBits(value.Z), LowBits(value.W)};
}

} // namespace Quantum::Physics3D
