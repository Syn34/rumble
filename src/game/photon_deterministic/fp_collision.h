// fp_collision.h - Photon.Deterministic.FPCollision: small stateless geometry queries
// (closest points, point-in-triangle, segment/segment and segment/circle intersection)
// used by gameplay code and by the Quantum physics engine.
//
// All maths follows FP's rules (products shifted one by one, wrapping adds); the few
// places where the original combines unshifted products before a single shift are
// noted in fp_collision.cpp because they change the low bits.
#pragma once

#include "fp.h"
#include "fp_vector2.h"
#include "fp_vector3.h"

namespace Photon::Deterministic::FPCollision {

// 0x7a74d18 - point on segment p1-p2 closest to `point` (p1 for a zero-length segment).
FPVector3 ClosestPointOnSegment(const FPVector3& point, const FPVector3& p1, const FPVector3& p2);
// 0x7a74e6c
FPVector2 ClosestPointOnSegment(FPVector2 point, FPVector2 p1, FPVector2 p2);

// Point-in-triangle tests. The FPVector3 overloads work in the ground (XZ) plane and
// ignore Y. "Exclusive" rejects points on an edge; "Inclusive" accepts them and accepts
// every point for a degenerate (zero-area) triangle whose edges all pass through it.
// 0x7a74f58
bool TriangleContainsPointExclusive(const FPVector3& pt, const FPVector3& v0, const FPVector3& v1,
                                    const FPVector3& v2);
// 0x7a74ff8
bool TriangleContainsPointInclusive(FPVector2 pt, FPVector2 v0, FPVector2 v1, FPVector2 v2);
// 0x7a75078
bool TriangleContainsPointInclusive(const FPVector3& pt, const FPVector3& v0, const FPVector3& v1,
                                    const FPVector3& v2);

// 0x7a75118 - circles touching or overlapping.
bool CircleIntersectsCircle(FPVector2 a_origin, FP a_radius, FPVector2 b_origin, FP b_radius);

// Segment p1-p2 against segment q1-q2 (endpoints included). `point` is the intersection,
// `distance` its distance from p1 and `normalizedDist` its position along p1-p2 in [0, 1].
// On a miss the outputs are zeroed. Collinear and parallel segments never intersect.
// 0x7a75144
bool LineIntersectsLine(FPVector2 p1, FPVector2 p2, FPVector2 q1, FPVector2 q2);
// 0x7a751c8
bool LineIntersectsLine(FPVector2 p1, FPVector2 p2, FPVector2 q1, FPVector2 q2, FPVector2& point,
                        FP& distance);
// 0x7a751f4
bool LineIntersectsLine(FPVector2 p1, FPVector2 p2, FPVector2 q1, FPVector2 q2, FPVector2& point,
                        FP& distance, FP& normalizedDist);
// 0x7a75304
bool LineIntersectsLine(FPVector2 p1, FPVector2 p2, FPVector2 q1, FPVector2 q2, FPVector2& point);

// Segment p1-p2 against a circle: the first point where the segment enters the circle.
// When the segment starts inside, the exit point is reported instead unless
// `ignoreIfStartPointInside` is set (then it is a miss). On a miss outputs are zeroed.
// 0x7a753bc
bool LineIntersectsCircle(FPVector2 p1, FPVector2 p2, FPVector2 position, FP radius,
                          bool ignoreIfStartPointInside);
// 0x7a75590
bool LineIntersectsCircle(FPVector2 p1, FPVector2 p2, FPVector2 position, FP radius,
                          FPVector2& point, bool ignoreIfStartPointInside);
// 0x7a75410
bool LineIntersectsCircle(FPVector2 p1, FPVector2 p2, FPVector2 position, FP radius,
                          FPVector2& point, FP& normalizedDist, bool ignoreIfStartPointInside);

// 0x7a755bc - closest point to `p` on triangle abc, with its barycentric coordinates
// (weights of a, b, c). Returns the SQUARED distance from p to that point, despite the
// name.
FP ClosestDistanceToTriangle(const FPVector3& p, const FPVector3& a, const FPVector3& b,
                             const FPVector3& c, FPVector3& closestPoint,
                             FPVector3& barycentricCoordinates);

} // namespace Photon::Deterministic::FPCollision
