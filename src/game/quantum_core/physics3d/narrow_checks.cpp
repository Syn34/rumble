// narrow_checks.cpp - Quantum.Physics3D.NarrowChecks.
#include "narrow_checks.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

namespace Quantum::Physics3D::NarrowChecks {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;

// 0x7d790cc
bool SphereSphere::Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold) {
    const NarrowEntry3D& entryA = *mInfo->EntryA;
    const NarrowEntry3D& entryB = *mInfo->EntryB;
    const FP radiusA = entryA.Shape->Sphere.Radius;
    const FP radii = entryB.Shape->Sphere.Radius + radiusA;
    const FPVector3 posA = entryA.Narrow.Position;
    const FPVector3 delta = entryB.Narrow.Position - posA;
    const FP distanceSquared = FPVector3::Dot(delta, delta);
    const bool overlap = distanceSquared < radii * radii;

    if (overlap && detailedManifold) {
        CollisionManifold3D* manifold = mInfo->Manifold;
        const FP distance = FPMath::Sqrt(distanceSquared);
        FPVector3 contact;
        if (distance > fpdetail::Epsilon) {
            const FPVector3 normal = {delta.X / distance, delta.Y / distance, delta.Z / distance};
            manifold->Penetration = radii - distance;
            manifold->Normal = normal;
            contact = {posA.X + normal.X * radiusA, posA.Y + normal.Y * radiusA,
                       posA.Z + normal.Z * radiusA};
        } else {
            // Concentric: any direction will do.
            manifold->Penetration = radiusA;
            manifold->Normal = FPVector3::Right();
            contact = posA;
        }
        CollisionManifold3D::AddContactPoint(manifold, contact);
    }
    return overlap;
}

} // namespace Quantum::Physics3D::NarrowChecks
