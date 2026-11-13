// gjk3d.h - Quantum.Physics3D.Gjk3D: distance, shape cast and time-of-impact queries
// between two convex entries (or an entry and a mesh triangle) using GJK, with the simplex
// storage and the separation function of the time-of-impact root finder. Same design as
// the 2D version (gjk2d.h); simplex points can carry extra precision bits, which the
// triangle and tetrahedron solvers account for.
#pragma once

#include "collision_manifold3d.h"
#include "hit3d.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>
#include <optional>

namespace Quantum {
struct GJKSettings;
struct CCDSettings;
} // namespace Quantum

namespace Quantum::Physics3D {

enum class TimeOfImpactResult : std::int32_t {
    Collision = 0,
    Ignore = 1,
    Failed = 2,
    Overlapping = 3,
};

struct SimplexVertex3D {
    static constexpr int SIZE = 72;
    FPVector3 Point; // (SupportA - SupportB) << precision bits
    FPVector3 SupportA;
    FPVector3 SupportB;
};
static_assert(sizeof(SimplexVertex3D) == SimplexVertex3D::SIZE);

struct SimplexData3D {
    static constexpr int SIZE = 320;
    SimplexVertex3D Vertex0;
    SimplexVertex3D Vertex1;
    SimplexVertex3D Vertex2;
    SimplexVertex3D Vertex3;
    FP Lambda0;
    FP Lambda1;
    FP Lambda2;
    FP Lambda3;
};
static_assert(sizeof(SimplexData3D) == SimplexData3D::SIZE);

struct SimplexDataPointers3D {
    std::int32_t UsedCount = 0;
    SimplexData3D* Data = nullptr;
    SimplexVertex3D* VertexPointers[4] = {};
    FP* Lambdas = nullptr;

    SimplexDataPointers3D() = default;
    // 0x7d51964
    explicit SimplexDataPointers3D(SimplexData3D* simplex) { ResetPointers(this, simplex); }
    // 0x7d50fe4
    static void ResetPointers(SimplexDataPointers3D* ptrs, SimplexData3D* simplex);
};

// The parts of Quantum.Physics3D.Tls used by GJK.
struct NarrowTls3D {
    SimplexData3D SimplexData{};
    SimplexDataPointers3D SimplexPointers;
};

struct SeparationFunction3D {
    enum ClosestFeatures : std::int32_t {
        PointPoint = 0,
        PointEdge = 1,
        PointFace = 2,
    };
    enum EntriesDynamicState : std::uint8_t {
        PointEntryHasLinearMotion = 1,
        PointEntryHasAngularMotion = 2,
        FaceEntryHasLinearMotion = 4,
        FaceEntryHasAngularMotion = 8,
    };

    ClosestFeatures _closestFeatures = PointPoint;
    std::uint8_t _dynamicState = 0; // EntriesDynamicState flags
    FPVector3 _separationAxis;
    CollisionManifoldInfo3D _mInfo;
    const NarrowEntry3D* _entry = nullptr;     // provides the point
    const NarrowEntry3D* _faceEntry = nullptr; // provides the point, edge or face
    FPVector3 _comPoint;
    FPVector3 _comOffsetPoint;
    FPVector3 _faceComPoint;
    FPVector3 _faceComOffsetPoint;

    // 0x7d51968
    void Initialize(const SimplexDataPointers3D* ptrs, const CollisionManifoldInfo3D* mInfo,
                    FP deltaTime, std::int32_t dtBitshift);
    // 0x7d51f70
    FP GetMinimumSeparation(FP deltaTime, std::int32_t dtBitshift);
    // 0x7d526c4
    FP GetSeparation(FP deltaTime, std::int32_t dtBitshift) const;
};

// 0x7d79e54 Entry3D.GetSupportOffset - offset from the entry's narrow position to its
// support point along `worldDir`; spheres report only the motion of their centre. With a
// time it adds the dynamic body's motion over that time (linear, and the rotation by the
// yaw/pitch/roll angular step about the centre of mass).
FPVector3 GetSupportOffset(const NarrowEntry3D& entry, FPVector3 worldDir,
                           std::optional<FP> deltaTime, std::int32_t dtShift);

namespace Gjk3D {

// 0x7d4eff8
void ComputeDistance(SimplexDataPointers3D* ptrs, const NarrowEntry3D* entryA,
                     const NarrowEntry3D* entryB, const TriangleCCW* tri, FP& distance,
                     FPVector3& separationAxis, std::int32_t iterations,
                     std::optional<FP> deltaTime, std::int32_t dtShift);
// 0x7d4f360 - mesh entries use `tri` (centre + support offset). Negates `worldDir` in place,
// like the original.
void GetSimplexVertex(const NarrowEntry3D* entryA, const NarrowEntry3D* entryB,
                      const TriangleCCW* tri, FPVector3& worldDir, SimplexVertex3D* vertex,
                      std::int32_t bitShift, std::optional<FP> deltaTime, std::int32_t dtShift);
// 0x7d4f7e0
void GetWitnessPoints(const SimplexDataPointers3D* ptrs, FPVector3& pointA, FPVector3& pointB);
// 0x7d4fb50
void GetWitnessPointA(const SimplexDataPointers3D* ptrs, FPVector3& pointA);
// 0x7d4fd68
void GetWitnessPointB(const SimplexDataPointers3D* ptrs, FPVector3& pointB);
// 0x7d4f5ac
bool IsAffinelyDependent(const SimplexDataPointers3D* ptrs);
// 0x7d4f744
void ComputeClosestPoint(SimplexDataPointers3D* ptrs, std::int32_t curBitShift,
                         FPVector3& closestPoint, bool& isDegenerate);
// 0x7d503dc
void SolveLine(SimplexDataPointers3D* ptrs, FP* lambdas, FPVector3& closestPoint);
// 0x7d4ff80
void SolveTriangle(SimplexDataPointers3D* ptrs, FP* lambdas, std::int32_t curBitShift,
                   FPVector3& closestPoint);
// 0x7d504f4
void SolveTetrahedron(SimplexDataPointers3D* ptrs, FP* lambdas, std::int32_t curBitShift,
                      FPVector3& closestPoint, bool& isDegenerate);
// 0x7d50a88 - the origin lies on the other side of plane ABC than `oppositeV`. Otherwise
// returns the barycentric weight of `oppositeV` (0 when outside or degenerate).
bool IsOriginOutsidePlane(const FPVector3& vA, const FPVector3& vB, const FPVector3& vC,
                          const FPVector3& oppositeV, bool& isDegenerate,
                          FP& oppositeBarycentricCoord);
// 0x7d50b74
TimeOfImpactResult ComputeTimeOfImpact(NarrowTls3D& tls, CollisionManifoldInfo3D* mInfo,
                                       const CCDSettings& settings, bool detailedInfo,
                                       FP deltaTime, FP& timeOfImpact);
// 0x7d51020 - contact (B's witness) and normal of a time-of-impact hit; against a mesh
// triangle the normal is the triangle's, and hits from behind it are ignored.
TimeOfImpactResult ComputeTimeOfImpactDetailedInfo(SimplexDataPointers3D* ptrs,
                                                   CollisionManifold3D* m,
                                                   const NarrowEntry3D* entryA,
                                                   const NarrowEntry3D* entryB,
                                                   const TriangleCCW* tri, FP distance);
// 0x7d5122c - moves B along `castDistance` towards A (GJK ray cast); see the 2D version.
// Casts into the back of a mesh triangle never hit.
bool ShapeCastManifold(NarrowTls3D& tls, const NarrowEntry3D* entryA, const NarrowEntry3D* entryB,
                       const TriangleCCW* tri, const GJKSettings& settings,
                       const FPVector3& castDistance, bool computeDetailedInfo, Hit3D& hit);

} // namespace Gjk3D

} // namespace Quantum::Physics3D
