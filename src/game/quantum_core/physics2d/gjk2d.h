// gjk2d.h - Quantum.Physics2D.Gjk2D: distance, shape cast and time-of-impact queries
// between two convex entries using the GJK algorithm, plus the simplex storage and the
// separation function of the time-of-impact root finder.
//
// Support points come from Entry2D.GetSupportOffset, which can extrapolate an entry's pose
// over part of a time step (for continuous collision detection). Times inside the
// time-of-impact search are raw FP values scaled by 2^dtShift (14) for precision.
#pragma once

#include "collision_manifold2d.h"
#include "hit.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"

#include <cstdint>
#include <optional>

namespace Quantum {
struct GJKSettings;
struct CCDSettings;
} // namespace Quantum

namespace Quantum::Physics2D {

enum class TimeOfImpactResult : std::int32_t {
    Collision = 0,
    Ignore = 1, // no impact within the time step
    Failed = 2,
    Overlapping = 3, // already overlapping at the start
};

// A point of the Minkowski difference A - B with the support points it came from.
struct SimplexVertex2D {
    static constexpr int SIZE = 48;
    FPVector2 Point; // (SupportA - SupportB), shifted left by the current precision bits
    FPVector2 SupportA;
    FPVector2 SupportB;
};
static_assert(sizeof(SimplexVertex2D) == SimplexVertex2D::SIZE);

struct SimplexData2D {
    static constexpr int SIZE = 168;
    SimplexVertex2D Vertex0;
    SimplexVertex2D Vertex1;
    SimplexVertex2D Vertex2;
    FP Lambda0; // barycentric weights of the closest point
    FP Lambda1;
    FP Lambda2;
};
static_assert(sizeof(SimplexData2D) == SimplexData2D::SIZE);

// The working view of a simplex: the used vertices in order (the solver reorders the
// pointers instead of the data) and the weights.
struct SimplexDataPointers2D {
    std::int32_t UsedCount = 0;
    SimplexData2D* Data = nullptr;
    SimplexVertex2D* VertexPointers[3] = {};
    FP* Lambdas = nullptr;

    SimplexDataPointers2D() = default;
    // 0x7d83b58
    explicit SimplexDataPointers2D(SimplexData2D* simplex) { ResetPointers(this, simplex); }
    // 0x7d82b0c - empty simplex over `simplex`'s storage.
    static void ResetPointers(SimplexDataPointers2D* ptrs, SimplexData2D* simplex);
};

// The parts of Quantum.Physics2D.Tls (the scene's per-thread scratch data) used by the
// narrow phase and GJK.
struct NarrowTls2D {
    FPVector2 BoxVertices[4]; // Tls.BoxVertices / BoxNormals point at buffers like these
    FPVector2 BoxNormals[4];
    SimplexData2D SimplexData{};
    SimplexDataPointers2D SimplexPointers;
};

// Conservative-advancement separation measure between the closest features found by GJK
// (a point of one entry against a point or an edge of the other), evaluated at times
// within the step.
struct SeparationFunction2D {
    enum ClosestFeatures : std::int32_t {
        PointPoint = 0,
        PointEdge = 1,
    };

    ClosestFeatures _closestFeatures = PointPoint;
    FPVector2 _separationAxis;
    const NarrowEntry2D* _entry = nullptr;     // the entry providing the point
    const NarrowEntry2D* _edgeEntry = nullptr; // the other entry (its edge, if any)
    FPVector2 _comPoint;           // world centre of mass of _entry
    FPVector2 _comOffsetPoint;     // tracked point of _entry relative to it (unrotated)
    FPVector2 _edgeComPoint;       // world centre of mass of _edgeEntry
    FPVector2 _edgeComOffsetPoint; // tracked point / edge midpoint relative to it

    // 0x7d83b5c - from a one-vertex (point-point) or two-vertex (point-edge) simplex at
    // time `deltaTime`.
    void Initialize(const SimplexDataPointers2D* ptrs, const NarrowEntry2D* entryA,
                    const NarrowEntry2D* entryB, FP deltaTime, std::int32_t dtBitshift);
    // 0x7d83f04 - deepest separation at `deltaTime` along the (moved) axis; re-picks the
    // support points and remembers them for GetSeparation.
    FP GetMinimumSeparation(FP deltaTime, std::int32_t dtBitshift);
    // 0x7d845fc - separation of the remembered points at `deltaTime`.
    FP GetSeparation(FP deltaTime, std::int32_t dtBitshift) const;
};

// 0x7d17958 Entry2D.GetSupportOffset - offset from the entry's narrow position to its
// support point along `worldDir` (circles report only the motion of their centre). With a
// time it includes the dynamic body's motion over that time (velocity * dt >> (dtShift + 16)
// and the rotation about the centre of mass).
FPVector2 GetSupportOffset(const NarrowEntry2D& entry, FPVector2 worldDir,
                           std::optional<FP> deltaTime, std::int32_t dtShift);

// 0x7d17d70 Entry2D.ComputeLocalDirectionAndCentroidOffset - rotates sin/cos by the body's
// angular motion over the time (if any), returns the direction in the rotated local frame
// and the resulting displacement of the shape around its centre of mass.
void ComputeLocalDirectionAndCentroidOffset(const NarrowEntry2D& entry, FPVector2 worldDir,
                                            FP& sin, FP& cos, FPVector2& localDir,
                                            FPVector2& centroidOffset,
                                            std::optional<FP> deltaTime, std::int32_t dtShift);

namespace Gjk2D {

// 0x7d82154 - distance between A and B (circle radii subtracted, clamped at zero) and the
// unit axis from B towards A when they are apart.
void ComputeDistance(SimplexDataPointers2D* ptrs, const NarrowEntry2D* entryA,
                     const NarrowEntry2D* entryB, FP& distance, FPVector2& separationAxis,
                     std::int32_t iterations, std::optional<FP> deltaTime, std::int32_t dtShift);
// 0x7d82600 - moves B along `castDistance` towards A (GJK ray cast). On a hit fills the
// fraction of the cast (never 0: shapes overlapping at the start don't count), the normal
// (from A towards B) and, if asked, the contact point on the moved B.
bool ShapeCastManifold(NarrowTls2D& tls, const NarrowEntry2D* entryA, const NarrowEntry2D* entryB,
                       const GJKSettings& settings, FPVector2 castDistance,
                       bool computeDetailedInfo, Hit& hit);
// 0x7d823f4
void GetSimplexVertex(const NarrowEntry2D* entryA, const NarrowEntry2D* entryB, FPVector2 worldDir,
                      SimplexVertex2D* vertex, std::int32_t bitShift, std::optional<FP> deltaTime,
                      std::int32_t dtShift);
// 0x7d82b40 - closest points on A and B (weighted support points).
void GetWitnessPoints(const SimplexDataPointers2D* ptrs, FPVector2& pointA, FPVector2& pointB);
// 0x7d82d10
void GetWitnessPointA(const SimplexDataPointers2D* ptrs, FPVector2& pointA);
// 0x7d82e30
void GetWitnessPointB(const SimplexDataPointers2D* ptrs, FPVector2& pointB);
// 0x7d8250c - the newest vertex adds no dimension (coincident or collinear points).
bool IsAffinelyDependent(const SimplexDataPointers2D* ptrs);
// 0x7d825a8 - closest point of the simplex to the origin; drops unused vertices.
void ComputeClosestPoint(SimplexDataPointers2D* ptrs, FPVector2& closestPoint);
// 0x7d832b8
void SolveLine(SimplexDataPointers2D* ptrs, FP* lambdas, FPVector2& closestPoint);
// 0x7d82f50
void SolveTriangle(SimplexDataPointers2D* ptrs, FP* lambdas, FPVector2& closestPoint);
// 0x7d83394 - earliest time within `deltaTime` at which the moving entries touch.
TimeOfImpactResult ComputeTimeOfImpact(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo,
                                       const CCDSettings& settings, bool detailedInfo,
                                       FP deltaTime, FP& timeOfImpact);
// 0x7d83a00 - contact point and normal for a time-of-impact hit.
TimeOfImpactResult ComputeTimeOfImpactDetailedInfo(SimplexDataPointers2D* ptrs,
                                                   CollisionManifold2D* m,
                                                   const NarrowEntry2D* entryA,
                                                   const NarrowEntry2D* entryB, FP distance);

} // namespace Gjk2D

} // namespace Quantum::Physics2D
