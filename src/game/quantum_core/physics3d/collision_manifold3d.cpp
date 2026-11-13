// collision_manifold3d.cpp - Quantum.Physics3D manifold helpers and raw-int vectors.
#include "collision_manifold3d.h"

#include "quantum_core/physics_components.h"
#include "quantum_core/physics_scene_settings.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/deterministic_frame_serializer.h"

#include <algorithm>
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

namespace {
// The rotation matrix's columns (Right, Up, Forward) from the quaternion formula; each
// doubled product is formed as (2a) * b and shifted on its own.
void RotationColumns(const FPQuaternion& q, FPVector3 columns[3]) {
    const FP::Raw x = q.X.RawValue(), y = q.Y.RawValue(), z = q.Z.RawValue(), w = q.W.RawValue();
    const FP::Raw x2 = x << 1, y2 = y << 1, z2 = z << 1;
    const FP::Raw xx = (x2 * x) >> 16, yy = (y2 * y) >> 16, zz = (z2 * z) >> 16;
    const FP::Raw xy = (y2 * x) >> 16, xz = (z2 * x) >> 16, yz = (z2 * y) >> 16;
    const FP::Raw wx = (w * x2) >> 16, wy = (w * y2) >> 16, wz = (z2 * w) >> 16;
    constexpr FP::Raw one = FP::Raw{1} << 16;
    columns[0] = {FP::FromRaw(one - (yy + zz)), FP::FromRaw(wz + xy), FP::FromRaw(xz - wy)};
    columns[1] = {FP::FromRaw(xy - wz), FP::FromRaw(one - (xx + zz)), FP::FromRaw(wx + yz)};
    columns[2] = {FP::FromRaw(wy + xz), FP::FromRaw(yz - wx), FP::FromRaw(one - (xx + yy))};
}

FP::Raw Product(FP a, FP b) { return (a.RawValue() * b.RawValue()) >> 16; }
FP::Raw Component(const FPVector3& v, int i) {
    return i == 0 ? v.X.RawValue() : (i == 1 ? v.Y.RawValue() : v.Z.RawValue());
}
} // namespace

// 0x7d7a3dc
void Entry3D::CalculateWorldBodySettings(Entry3D* entry, std::optional<FPQuaternion> rotation) {
    assert(entry != nullptr && entry->Body3D != nullptr);
    FPVector3 columns[3];
    if (rotation.has_value()) {
        RotationColumns(*rotation, columns);
    } else {
        // The narrow phase already holds the rotated axes.
        columns[0] = entry->Narrow.RotatedRight;
        columns[1] = entry->Narrow.RotatedUp;
        columns[2] = entry->Narrow.RotatedForward;
    }
    const PhysicsBody3D* body = entry->Body3D;
    const Physics3D::FPVector3RawInt com = body->_centerOfMass;
    if ((com.RawIntX | com.RawIntY | com.RawIntZ) != 0) {
        const FPVector3 c(com);
        // R * com, every product shifted separately.
        FP::Raw r[3];
        for (int i = 0; i < 3; ++i) {
            r[i] = (Component(columns[1], i) * c.Y.RawValue() >> 16) +
                   (Component(columns[0], i) * c.X.RawValue() >> 16) +
                   (Component(columns[2], i) * c.Z.RawValue() >> 16);
        }
        entry->RotatedCenterOfMass = {FP::FromRaw(r[0]), FP::FromRaw(r[1]), FP::FromRaw(r[2])};
    } else {
        entry->RotatedCenterOfMass = FPVector3::Zero();
    }
    // World inverse inertia: R * diag(I) * R^T. A[i][k] = R[i][k] * I[k]; element (j, i) is
    // sum_k A[i][k] * R[j][k], stored column by column (M00, M10, M20, M01, ...).
    const FPVector3 inertia = body->InertiaTensorInverse();
    const FP::Raw diag[3] = {inertia.X.RawValue(), inertia.Y.RawValue(), inertia.Z.RawValue()};
    FP::Raw a[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int k = 0; k < 3; ++k) {
            a[i][k] = (Component(columns[k], i) * diag[k]) >> 16;
        }
    }
    FP* out = &entry->WorldInertiaTensorInverse.M00;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            const FP::Raw value = (a[i][1] * Component(columns[1], j) >> 16) +
                                  (a[i][0] * Component(columns[0], j) >> 16) +
                                  (a[i][2] * Component(columns[2], j) >> 16);
            out[3 * i + j] = FP::FromRaw(value);
        }
    }
}

// 0x7d7aa88 - velocity components inside (-min, min) snap to zero.
void Entry3D::ApplyMinVelocityIntegration(Entry3D* entry, const PhysicsSceneSettings& settings) {
    PhysicsBody3D* body = entry->Body3D;
    const auto snap = [](FP& value, FP min) {
        if (value < min && value > -min) {
            value = FP::Zero();
        }
    };
    const FP minLinear = settings.MinLinearIntegration;
    snap(body->Velocity.X, minLinear);
    snap(body->Velocity.Y, minLinear);
    snap(body->Velocity.Z, minLinear);
    const FP minAngular = settings.MinAngularIntegration;
    snap(body->AngularVelocity.X, minAngular);
    snap(body->AngularVelocity.Y, minAngular);
    snap(body->AngularVelocity.Z, minAngular);
}

// 0x7d7a724
void Entry3D::IntegrateVelocity(Entry3D* entry, FP dt, const PhysicsSceneSettings& settings,
                                bool applyMinVelThreshold, bool integrateAngVel) {
    if (applyMinVelThreshold) {
        ApplyMinVelocityIntegration(entry, settings);
    }
    ::Quantum::Transform3D* transform = entry->Transform3D;
    PhysicsBody3D* body = entry->Body3D;
    assert(transform != nullptr && body != nullptr);
    transform->Position.X = FP::FromRaw(transform->Position.X.RawValue() + Product(body->Velocity.X, dt));
    transform->Position.Y = FP::FromRaw(transform->Position.Y.RawValue() + Product(body->Velocity.Y, dt));
    transform->Position.Z = FP::FromRaw(transform->Position.Z.RawValue() + Product(body->Velocity.Z, dt));
    // Linear drag: v *= 1 - dt * drag (drag has 10 fractional bits).
    const FP::Raw dt64 = dt.RawValue() << 6;
    const FP linearFactor = FP::FromRaw(FP::Raw{1 << 16} - ((dt64 * body->_drag) >> 16));
    body->Velocity.X = FP::FromRaw(Product(linearFactor, body->Velocity.X));
    body->Velocity.Y = FP::FromRaw(Product(body->Velocity.Y, linearFactor));
    body->Velocity.Z = FP::FromRaw(Product(body->Velocity.Z, linearFactor));
    if (!integrateAngVel) {
        return;
    }
    const auto freeze = static_cast<std::uint8_t>(body->RotationFreeze);
    const FP pitch = (freeze & 1) ? FP::Zero() : FP::FromRaw(Product(body->AngularVelocity.X, dt));
    const FP yaw = (freeze & 2) ? FP::Zero() : FP::FromRaw(Product(body->AngularVelocity.Y, dt));
    const FP roll = (freeze & 4) ? FP::Zero() : FP::FromRaw(Product(body->AngularVelocity.Z, dt));
    const FPQuaternion step = FPQuaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
    const Physics3D::FPVector3RawInt com = body->_centerOfMass;
    if ((com.RawIntX | com.RawIntY | com.RawIntZ) != 0) {
        // Rotation about the centre of mass moves the origin.
        const FPVector3 rotated = step * entry->RotatedCenterOfMass;
        transform->Position = (transform->Position - rotated) + entry->RotatedCenterOfMass;
    }
    transform->Rotation = FPQuaternion::Product(step, transform->Rotation).Normalized();
    const FP angularFactor = FP::FromRaw(FP::Raw{1 << 16} - ((dt64 * body->_angularDrag) >> 16));
    if (!(freeze & 1)) {
        body->AngularVelocity.X = FP::FromRaw(Product(body->AngularVelocity.X, angularFactor));
    }
    if (!(freeze & 2)) {
        body->AngularVelocity.Y = FP::FromRaw(Product(body->AngularVelocity.Y, angularFactor));
    }
    if (!(freeze & 4)) {
        body->AngularVelocity.Z = FP::FromRaw(Product(body->AngularVelocity.Z, angularFactor));
    }
}

// 0x7d7ab5c - box: half-extent per axis = the largest |corner| of the rotated box (from the
// rotated top-face edges); others: the broad radius. Bounds are the low 32 bits of raw.
void Entry3D::ComputeEntryAabb(const Shape3D* shape, const NarrowData3D* narrow, IntAABB* aabb) {
    FP::Raw extent[3];
    if (shape->Type() == Shape3DType::Box) {
        for (int axis = 0; axis < 3; ++axis) {
            FP::Raw best = 0;
            for (int e = 0; e < 4; ++e) {
                const FP::Raw v = Component(narrow->BoxEdgesRotated.Edges[e].start, axis);
                best = std::max(best, v < 0 ? -v : v);
            }
            extent[axis] = best;
        }
    } else {
        extent[0] = extent[1] = extent[2] = shape->BroadRadius.RawValue();
    }
    const auto low = [](FP::Raw v) { return static_cast<std::int32_t>(static_cast<std::uint32_t>(v)); };
    aabb->MinX = low(narrow->Position.X.RawValue() - extent[0]);
    aabb->MaxX = low(narrow->Position.X.RawValue() + extent[0]);
    aabb->MinY = low(narrow->Position.Y.RawValue() - extent[1]);
    aabb->MaxY = low(narrow->Position.Y.RawValue() + extent[1]);
    aabb->MinZ = low(narrow->Position.Z.RawValue() - extent[2]);
    aabb->MaxZ = low(narrow->Position.Z.RawValue() + extent[2]);
}

// 0x7d7adc8 - the entry's bounds; a CCD entry (or part of a CCD compound) with a time step
// also covers where it will be at the end of the step.
void Entry3D::InitBroadAabb(Entry3D* entry, const FPVector3* pos, const FPQuaternion* rot,
                            std::optional<FP> deltaTime) {
    ComputeEntryAabb(entry->Shape, &entry->Narrow, &entry->AABB);
    const Entry3D* root = entry->_compoundRootIndexPlusOne >= 1 ? entry->_compoundRoot : entry;
    if (!root->_useCCD || !deltaTime.has_value()) {
        return;
    }
    NarrowData3D moved{};
    FPVector3 position = *pos;
    FPQuaternion rotation = *rot;
    bool useCachedRot = false;
    NarrowData3D::InitFromTransform(&moved, entry, position, rotation, useCachedRot, deltaTime);
    IntAABB end{};
    ComputeEntryAabb(entry->Shape, &moved, &end);
    entry->AABB.MinX = std::min(entry->AABB.MinX, end.MinX);
    entry->AABB.MaxX = std::max(entry->AABB.MaxX, end.MaxX);
    entry->AABB.MinY = std::min(entry->AABB.MinY, end.MinY);
    entry->AABB.MaxY = std::max(entry->AABB.MaxY, end.MaxY);
    entry->AABB.MinZ = std::min(entry->AABB.MinZ, end.MinZ);
    entry->AABB.MaxZ = std::max(entry->AABB.MaxZ, end.MaxZ);
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

// 0x7d7b6f4
void FPVector3RawInt::Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
    auto* v = static_cast<FPVector3RawInt*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    stream.Serialize(&v->RawIntX);
    stream.Serialize(&v->RawIntY);
    stream.Serialize(&v->RawIntZ);
}

// 0x7d7b9f8
void FPQuaternionRawInt::Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
    auto* q = static_cast<FPQuaternionRawInt*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    stream.Serialize(&q->RawIntX);
    stream.Serialize(&q->RawIntY);
    stream.Serialize(&q->RawIntZ);
    stream.Serialize(&q->RawIntW);
}

} // namespace Quantum::Physics3D
