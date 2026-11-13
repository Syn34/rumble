// collision_info3d.cpp - the callback info views of the 3D scene.
#include "collision_info3d.h"

#include "quantum_core/physics3d/collision_manifold3d.h"
#include "quantum_core/physics3d/narrow_checks.h"

namespace Quantum {

namespace {
StaticColliderData StaticDataAt(const Map* map, std::int32_t index) {
    // The original indexes the map's collider array (throwing for a bad index).
    return map->StaticColliders3D.at(static_cast<std::size_t>(index)).StaticData;
}
} // namespace

// 0x7d3a788
ContactPointIterator3D::ContactPointIterator3D(Physics3D::CollisionManifold3D* manifold, bool isInverse,
                                               bool isMesh, Physics3D::CollisionManifold3D* triManifolds)
    : _rootManifold(manifold), _currentManifold(manifold), _triManifolds(triManifolds),
      _currentContactIndex(-1), _isMesh(isMesh), _isInverse(isInverse) {}

// 0x7d3a998
FPVector3 ContactPointIterator3D::First() const {
    const Physics3D::CollisionManifold3D* manifold =
        _isMesh ? &_triManifolds[_rootManifold->_m.NextInMesh] : _rootManifold;
    return manifold->_contactPoints[0];
}

// 0x7d3a680
CollisionInfo3D::CollisionInfo3D(Physics3D::CollisionManifold3D* manifold, Physics3D::Entry3D* entryA,
                                 Physics3D::Entry3D* entryB, const TriangleMesh* sceneMesh, const Map* map,
                                 bool isStatic, bool isInverse, bool isMesh,
                                 Physics3D::CollisionManifold3D* triManifolds)
    : _map(map), _sceneMesh(sceneMesh), _isStatic(isStatic), _isMesh(isMesh), _isInverse(isInverse),
      _manifold(manifold), _entryA(entryA), _entryB(entryB),
      ContactPoints(manifold, isInverse, isMesh, triManifolds) {
    if (isMesh) {
        MeshTriangleCollisions = {manifold, triManifolds, manifold, sceneMesh};
    }
}

// 0x7d3a7d4
FPVector3 CollisionInfo3D::ContactNormal() const {
    if (_isMesh) {
        const Physics3D::NarrowChecks::TriangleCollisionIterator triangles(
            MeshTriangleCollisions._sceneMesh, MeshTriangleCollisions._meshManifold,
            MeshTriangleCollisions._triManifolds);
        return triangles.AverageNormal();
    }
    return _isInverse ? _manifold->Normal : -_manifold->Normal;
}

// 0x7d3a92c
EntityRef CollisionInfo3D::Entity() const { return _isInverse ? _entryB->Ref : _entryA->Ref; }

// 0x7d3a958
EntityRef CollisionInfo3D::Other() const {
    if (_isStatic) {
        return EntityRef{};
    }
    return _isInverse ? _entryA->Ref : _entryB->Ref;
}

// 0x7d3c0fc
EntityRef TriggerInfo3D::Entity() const { return _isInverse ? _entryB->Ref : _entryA->Ref; }

// 0x7d3c128
EntityRef TriggerInfo3D::Other() const {
    if (_isStatic) {
        return EntityRef{};
    }
    return _isInverse ? _entryA->Ref : _entryB->Ref;
}

// 0x7d3c168 - the static side is whichever entry is not an entity.
StaticColliderData TriggerInfo3D::StaticData() const {
    if (_entryA->IsDynamic() && _entryB->IsDynamic()) {
        return {};
    }
    const Physics3D::Entry3D* staticEntry = _entryA->IsDynamic() ? _entryB : _entryA;
    return StaticDataAt(_map, staticEntry->StaticColliderIndex);
}

// 0x7d3a5b0
StaticColliderData ExitInfo3D::StaticData() const {
    if (_staticIndex < 0) {
        return {};
    }
    return StaticDataAt(_map, _staticIndex);
}

} // namespace Quantum
