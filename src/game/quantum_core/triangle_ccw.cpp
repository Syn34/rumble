// triangle_ccw.cpp - Quantum.TriangleCCW.
#include "triangle_ccw.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

namespace Quantum {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;

namespace {
// Rays whose direction has a smaller (more negative) dot product with the normal than
// this are considered to face the triangle.
constexpr FP::Raw FacingLimitRaw = -5;
// Hits closer than this (raw) to the ray origin are ignored.
constexpr FP::Raw MinDistanceRaw = 6;
} // namespace

// 0x7d398d0
void TriangleCCW::ComputeNormal() {
    const FPVector3 ab = FPVector3::Normalize(B - A);
    const FPVector3 ac = FPVector3::Normalize(C - A);
    Normal = FPVector3::Normalize(FPVector3::Cross(ab, ac));
    InverseNormal = Normal;
    Normal = -Normal;
    Center = {fpdetail::DivInt(A.X + B.X + C.X, 3), fpdetail::DivInt(A.Y + B.Y + C.Y, 3),
              fpdetail::DivInt(A.Z + B.Z + C.Z, 3)};
    EdgeA = B - A;
    EdgeB = C - B;
    EdgeC = A - C;
    LengthEdgeA = EdgeA.Magnitude();
    LengthEdgeB = EdgeB.Magnitude();
    LengthEdgeC = EdgeC.Magnitude();
    CenterToA = A - Center;
    CenterToB = B - Center;
    CenterToC = C - Center;
    BroadRadius = FPMath::Max(CenterToA.Magnitude(), CenterToB.Magnitude(), CenterToC.Magnitude());
    MinY = FPMath::Min(A.Y, B.Y, C.Y);
    MaxY = FPMath::Max(A.Y, B.Y, C.Y);
}

// 0x7d36fe0
FPVector3 TriangleCCW::Min3() const {
    return FPVector3::Min(A, FPVector3::Min(B, C));
}

// 0x7d370a8
FPVector3 TriangleCCW::Max3() const {
    return FPVector3::Max(A, FPVector3::Max(B, C));
}

// 0x7d3a21c
bool TriangleCCW::RayIntersectsTriangle(const FPVector3& origin, FPVector3 dir,
                                        FPVector3& intersectionPoint, FP& normalizedDist) const {
    auto miss = [&]() {
        intersectionPoint = FPVector3::Zero();
        normalizedDist = FP::Zero();
        return false;
    };
    const FP lengthSquared = dir.SqrMagnitude();
    const FP length = FPMath::Sqrt(lengthSquared);
    if (length == 0) {
        return miss();
    }
    dir = dir * (FP::One() / length);
    const FP facing = FPVector3::Dot(Normal, dir);
    if (facing.RawValue() >= FacingLimitRaw) {
        return miss();
    }
    const FP t = FPVector3::Dot(A - origin, Normal) / facing;
    if (t.RawValue() < MinDistanceRaw || lengthSquared < t * t) {
        return miss();
    }
    const FPVector3 point = origin + dir * t;
    intersectionPoint = point;
    // Inside all three edges (on the inner side of each, as seen along the normal).
    if (FPVector3::Dot(Normal, FPVector3::Cross(point - A, EdgeA)) < 0 ||
        FPVector3::Dot(Normal, FPVector3::Cross(point - B, EdgeB)) < 0 ||
        FPVector3::Dot(Normal, FPVector3::Cross(point - C, EdgeC)) < 0) {
        return miss();
    }
    normalizedDist = t / length;
    return true;
}

// 0x7d3a4c8
void TriangleCCW::GetSupportOffset(const FPVector3& worldDir, FPVector3& supportOffset) const {
    const FP dotA = FPVector3::Dot(CenterToA, worldDir);
    const FP dotB = FPVector3::Dot(CenterToB, worldDir);
    const FP dotC = FPVector3::Dot(CenterToC, worldDir);
    const FPVector3* best = dotB > dotA ? &CenterToB : &CenterToA;
    const FP bestDot = dotB > dotA ? dotB : dotA;
    supportOffset = dotC > bestDot ? CenterToC : *best;
}

} // namespace Quantum
