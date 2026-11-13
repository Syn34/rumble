// triangle_ccw.h - Quantum.TriangleCCW: one triangle of a static mesh collider (vertices
// in counter-clockwise order) with the derived data the collision code uses (centre,
// edges, their lengths, normal and bounds).
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>

namespace Quantum {

class ByteStream;

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;

struct TriangleCCW {
    FPVector3 A;
    FPVector3 B;
    FPVector3 C;
    FPVector3 CenterToA;
    FPVector3 CenterToB;
    FPVector3 CenterToC;
    FP MaxY;
    FP MinY;
    FP BroadRadius; // largest centre-to-vertex distance
    FPVector3 EdgeA; // B - A
    FPVector3 EdgeB; // C - B
    FPVector3 EdgeC; // A - C
    FP LengthEdgeA;
    FP LengthEdgeB;
    FP LengthEdgeC;
    FPVector3 Normal;        // -normalize(cross(B - A, C - A)) after normalising both edges
    FPVector3 InverseNormal; // -Normal
    FPVector3 Center;
    std::int32_t StaticDataIndex = 0;

    // 0x7d398d0 - derives everything above from A, B and C.
    void ComputeNormal();
    // 0x7d36fe0 - component-wise minimum of the vertices.
    FPVector3 Min3() const;
    // 0x7d370a8 - component-wise maximum of the vertices.
    FPVector3 Max3() const;
    // 0x7d3a21c - ray from `origin` along `dir` (any length; its length is the ray's) against
    // the triangle's front face. On a hit returns the point and its distance as a fraction
    // of the ray; outputs are zeroed on a miss. Rays nearly parallel to the face, starting
    // closer than raw 6 or ending before the face miss.
    bool RayIntersectsTriangle(const FPVector3& origin, FPVector3 dir, FPVector3& intersectionPoint,
                               FP& normalizedDist) const;
    // 0x7d3a4c8 - the vertex furthest along `worldDir`, relative to the centre (ties keep
    // A over B over C).
    void GetSupportOffset(const FPVector3& worldDir, FPVector3& supportOffset) const;

    // 0x7d39e5c - the vertices, then either the derived data (MaxY, MinY, BroadRadius, the
    // edges, the centre offsets, the edge lengths, Normal, InverseNormal, Center) when
    // `serializeTrianglesMetadata`, or nothing and ComputeNormal derives it again; then
    // StaticDataIndex. Reads overwrite the fields in place.
    void Serialize(ByteStream& stream, bool write, bool serializeTrianglesMetadata);
};

} // namespace Quantum
