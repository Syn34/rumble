// collision_manifold3d.cpp - Quantum.Physics3D manifold helpers and raw-int vectors.
#include "collision_manifold3d.h"

#include "photon_deterministic/fp_internal.h"

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
