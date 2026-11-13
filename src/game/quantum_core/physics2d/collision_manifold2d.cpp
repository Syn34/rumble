// collision_manifold2d.cpp - Quantum.Physics2D manifold helpers.
#include "collision_manifold2d.h"

#include <cassert>
#include <utility>

namespace Quantum::Physics2D {

// 0x7d18efc
FPVector2 CollisionManifold2D::GetContactPoint(const CollisionManifold2D* manifold,
                                               std::int32_t index) {
    assert(manifold != nullptr);
    return manifold->_contactPoints[index];
}

// 0x7d18f14
FPVector2 CollisionManifold2D::GetAverageContactPoint(const CollisionManifold2D* manifold) {
    assert(manifold != nullptr);
    const std::int32_t count = manifold->_contactPointsCount;
    if (count == 1) {
        return manifold->_contactPoints[0];
    }
    if (count <= 0) {
        return FPVector2::Zero(); // 0 / count; the original's divide yields 0 for count 0
    }
    FP::Raw sumX = 0;
    FP::Raw sumY = 0;
    for (std::int32_t i = 0; i < count; ++i) {
        sumX += manifold->_contactPoints[i].X.RawValue();
        sumY += manifold->_contactPoints[i].Y.RawValue();
    }
    return {FP::FromRaw(sumX / count), FP::FromRaw(sumY / count)};
}

// 0x7d18f84
void CollisionManifold2D::AddContactPoint(CollisionManifold2D* manifold, const FPVector2& contact) {
    assert(manifold != nullptr && manifold->_contactPointsCount < MAX_CONTACT_POINTS);
    manifold->_contactPoints[manifold->_contactPointsCount++] = contact;
}

// 0x7d18774
void PolygonData::Init(const PolygonCollider& polygonCollider) {
    Vertices = polygonCollider.Vertices.data();
    Normals = polygonCollider.Normals.data();
    VerticesCount = static_cast<std::int32_t>(polygonCollider.Vertices.size());
    NormalsCount = static_cast<std::int32_t>(polygonCollider.Normals.size());
}

// 0x7d18dcc
void CollisionManifoldInfo2D::InvertEntries() {
    assert(Manifold != nullptr);
    std::swap(Manifold->_m._entryIdxA, Manifold->_m._entryIdxB);
    std::swap(EntryA, EntryB);
    Manifold->_flags ^= CollisionManifold2D::NormalInverted;
}

} // namespace Quantum::Physics2D
