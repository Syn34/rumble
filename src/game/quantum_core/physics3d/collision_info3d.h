// collision_info3d.h - what the 3D scene hands to collision and trigger callbacks:
// Quantum.CollisionInfo3D (with its ContactPointIterator3D), Quantum.TriggerInfo3D,
// Quantum.ExitInfo3D and the Quantum.Core.ICollisionCallbacks3D interface.
//
// The infos are views over the scene's entries and manifolds for the duration of one
// callback: the entity whose callbacks fire is `Entity`; the infos made for the second entry
// of a pair are "inverse".
#pragma once

#include "quantum_core/entity_ref.h"
#include "quantum_core/map.h"

#include "photon_deterministic/fp_vector3.h"

#include <cstdint>

namespace Quantum {
namespace Core {
class FrameBase;
}
namespace Physics3D {
struct Entry3D;
struct CollisionManifold3D;
struct ManifoldData3D;
} // namespace Physics3D
class TriangleMesh;

using Photon::Deterministic::FPVector3;

// Quantum.ContactPointIterator3D: walks the contact points of a manifold (or of the triangle
// manifolds of a mesh manifold).
struct ContactPointIterator3D {
    Physics3D::CollisionManifold3D* _rootManifold = nullptr;
    Physics3D::CollisionManifold3D* _currentManifold = nullptr;
    Physics3D::CollisionManifold3D* _triManifolds = nullptr; // bucket triangle manifolds
    std::int32_t _currentContactIndex = -1;
    bool _isMesh = false;
    bool _isInverse = false;

    ContactPointIterator3D() = default;
    // 0x7d3a788
    ContactPointIterator3D(Physics3D::CollisionManifold3D* manifold, bool isInverse, bool isMesh,
                           Physics3D::CollisionManifold3D* triManifolds);
    // 0x7d3a998 - the first contact point (of the first triangle manifold for meshes).
    FPVector3 First() const;
};

// Quantum.CollisionInfo3D
struct CollisionInfo3D {
    const Map* _map = nullptr;
    const TriangleMesh* _sceneMesh = nullptr;
    bool _isStatic = false;
    bool _isMesh = false;
    bool _isInverse = false;
    Physics3D::CollisionManifold3D* _manifold = nullptr;
    Physics3D::Entry3D* _entryA = nullptr;
    Physics3D::Entry3D* _entryB = nullptr;
    ContactPointIterator3D ContactPoints;
    // The triangle manifolds of a mesh collision (all null otherwise); see
    // NarrowChecks::TriangleCollisionIterator.
    struct TriangleCollisions {
        Physics3D::CollisionManifold3D* _meshManifold = nullptr;
        Physics3D::CollisionManifold3D* _triManifolds = nullptr;
        Physics3D::CollisionManifold3D* _currentTri = nullptr;
        const TriangleMesh* _sceneMesh = nullptr;
    } MeshTriangleCollisions;

    CollisionInfo3D() = default;
    // 0x7d3a680
    CollisionInfo3D(Physics3D::CollisionManifold3D* manifold, Physics3D::Entry3D* entryA,
                    Physics3D::Entry3D* entryB, const TriangleMesh* sceneMesh, const Map* map,
                    bool isStatic, bool isInverse, bool isMesh,
                    Physics3D::CollisionManifold3D* triManifolds);

    // 0x7d3a7d4 - the manifold normal as seen from Entity: the stored normal points from A
    // to B, so it is negated for A and kept for B (the inverse info). A mesh collision
    // reports the negated mean of its triangles' normals.
    FPVector3 ContactNormal() const;
    bool IsStatic() const { return _isStatic; } // 0x7d3a924
    // 0x7d3a92c
    EntityRef Entity() const;
    // 0x7d3a958 - none for static colliders.
    EntityRef Other() const;
};

// Quantum.TriggerInfo3D
struct TriggerInfo3D {
    const Map* _map = nullptr;
    bool _isStatic = false;
    bool _isInverse = false;
    Physics3D::ManifoldData3D* _manifold = nullptr;
    Physics3D::Entry3D* _entryA = nullptr;
    Physics3D::Entry3D* _entryB = nullptr;

    TriggerInfo3D() = default;
    // 0x7d3c210
    TriggerInfo3D(Physics3D::ManifoldData3D* manifold, Physics3D::Entry3D* entryA,
                  Physics3D::Entry3D* entryB, const Map* map, bool isStatic, bool isInverse)
        : _map(map), _isStatic(isStatic), _isInverse(isInverse), _manifold(manifold),
          _entryA(entryA), _entryB(entryB) {}

    bool IsStatic() const { return _isStatic; } // 0x7d3c0f4
    // 0x7d3c0fc
    EntityRef Entity() const;
    // 0x7d3c128
    EntityRef Other() const;
    // 0x7d3c168 - the static collider's data from the map (empty for two entities).
    StaticColliderData StaticData() const;
};

// Quantum.ExitInfo3D
struct ExitInfo3D {
    const Map* _map = nullptr;
    std::int32_t _staticIndex = -1;
    EntityRef Entity;
    EntityRef Other;

    ExitInfo3D() = default;
    // 0x7d3a610 - exit from a static collider.
    ExitInfo3D(const Map* map, EntityRef entity, std::int32_t staticIndex)
        : _map(map), _staticIndex(staticIndex), Entity(entity), Other() {}
    // 0x7d3a64c - exit from another entity.
    ExitInfo3D(const Map* map, EntityRef entity, EntityRef other)
        : _map(map), _staticIndex(-1), Entity(entity), Other(other) {}

    EntityRef get_Entity() const { return Entity; } // 0x7d3a590
    EntityRef get_Other() const { return Other; }   // 0x7d3a598
    // 0x7d3a5a0
    bool IsStatic() const { return _staticIndex >= 0; }
    // 0x7d3a5b0 - empty for an exit from an entity.
    StaticColliderData StaticData() const;
};

namespace Core {
// Quantum.Core.ICollisionCallbacks3D (method order = the original's interface slots).
class ICollisionCallbacks3D {
  public:
    virtual ~ICollisionCallbacks3D() = default;
    virtual void OnCollision3D(FrameBase* f, const CollisionInfo3D& info) = 0;
    virtual void OnCollisionEnter3D(FrameBase* f, const CollisionInfo3D& info) = 0;
    virtual void OnCollisionExit3D(FrameBase* f, const ExitInfo3D& info) = 0;
    virtual void OnTrigger3D(FrameBase* f, const TriggerInfo3D& info) = 0;
    virtual void OnTriggerEnter3D(FrameBase* f, const TriggerInfo3D& info) = 0;
    virtual void OnTriggerExit3D(FrameBase* f, const ExitInfo3D& info) = 0;
};
} // namespace Core

namespace Physics3D {
using ::Quantum::CollisionInfo3D;
using ::Quantum::ExitInfo3D;
using ::Quantum::TriggerInfo3D;
using ::Quantum::Core::ICollisionCallbacks3D;
} // namespace Physics3D

} // namespace Quantum
