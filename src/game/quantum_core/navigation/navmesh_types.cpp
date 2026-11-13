#include "quantum_core/navigation/navmesh_types.h"

#include "photon_deterministic/fp_math.h"
#include "quantum_core/byte_stream.h"
#include "quantum_core/navigation/navmesh.h"

namespace Quantum {

namespace BS = ByteStreamExtension;

// 0x7d2b5d8
void NavMeshPortal::Serialize(ByteStream& stream, bool write) {
    LeftVertex = BS::Serialize(stream, write, LeftVertex);
    RightVertex = BS::Serialize(stream, write, RightVertex);
    Neighbor = BS::Serialize(stream, write, Neighbor);
}

// 0x7d2b888
void NavMeshVertex::Serialize(ByteStream& stream, bool write) {
    Point = BS::Serialize(stream, write, Point);
    Borders = BS::SerializeIntArray(stream, write, std::move(Borders));
}

// 0x7d27548
FPVector3 NavMeshVertex::CalculateNormal(int vertexIndex, const NavMesh& navmesh,
                                         NavMeshRegionMask regionMask) {
    FPVector3 sum = FPVector3::Zero();
    int activeBorders = 0;
    for (const std::int32_t border : navmesh.Vertices.at(static_cast<std::size_t>(vertexIndex)).Borders) {
        if (navmesh.IsBorderActive(border, regionMask)) {
            sum = sum + navmesh.Borders.at(static_cast<std::size_t>(border)).Normal;
            ++activeBorders;
        }
    }
    if (activeBorders < 1) {
        return sum;
    }

    // Average, then normalise unless the result is already (nearly) unit length.
    const FPVector3 average = sum / activeBorders;
    const FP squaredLength = average.SqrMagnitude();
    if (squaredLength <= FP::Zero()) {
        return average;
    }
    if (Photon::Deterministic::FPMath::Abs(squaredLength - FP::One()) <= FP::Epsilon()) {
        return average;
    }
    const FP inverseLength = FP::One() / Photon::Deterministic::FPMath::Sqrt(squaredLength);
    return average * inverseLength;
}

// 0x7d2b640
void NavMeshTriangle::Serialize(ByteStream& stream, bool write) {
    Vertex0 = BS::Serialize(stream, write, Vertex0);
    Vertex1 = BS::Serialize(stream, write, Vertex1);
    Vertex2 = BS::Serialize(stream, write, Vertex2);
    Center = BS::Serialize(stream, write, Center);
    Normal = BS::Serialize(stream, write, Normal);
    Neighbors = BS::SerializeArrayLength(stream, write, std::move(Neighbors));
    for (NavMeshPortal& portal : Neighbors) {
        portal.Serialize(stream, write);
    }
    Links = BS::SerializeIntArray(stream, write, std::move(Links));
    Borders = BS::SerializeIntArray(stream, write, std::move(Borders));
    Regions.Serialize(stream, write);
    Cost = BS::Serialize(stream, write, Cost);
}

// 0x7d2b934
void NavMeshBorder::Serialize(ByteStream& stream, bool write) {
    V0 = BS::Serialize(stream, write, V0);
    V1 = BS::Serialize(stream, write, V1);
    Normal = BS::Serialize(stream, write, Normal);
    RegionsOffmesh.Serialize(stream, write);
    Regions.Serialize(stream, write);
}

// 0x7d2ba54
void NavMeshBorderNode::Serialize(ByteStream& stream, bool write) {
    Borders = BS::SerializeIntArray(stream, write, std::move(Borders));
}

// 0x7d2ba88
void NavMeshTriangleNode::Serialize(ByteStream& stream, bool write) {
    Triangles = BS::SerializeIntArray(stream, write, std::move(Triangles));
}

// 0x7d2bbe4
void NavMeshTriangleCenterGridNode::Serialize(ByteStream& stream, bool write) {
    Triangles = BS::SerializeIntArray(stream, write, std::move(Triangles));
}

// 0x7d2babc
void NavMeshLink::Serialize(ByteStream& stream, bool write) {
    Start = BS::Serialize(stream, write, Start);
    End = BS::Serialize(stream, write, End);
    Triangle = BS::Serialize(stream, write, Triangle);
    CostOverride = BS::Serialize(stream, write, CostOverride);
    Region.Serialize(stream, write);
    Name = BS::SerializeString(stream, write, Name);
}

} // namespace Quantum
