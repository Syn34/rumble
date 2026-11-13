#include "quantum_core/navigation/navmesh.h"

#include "photon_deterministic/byte_utils.h"
#include "photon_deterministic/fp_collision.h"
#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/log.h"
#include "photon_deterministic/rng_session.h"
#include "quantum_core/byte_stream.h"
#include "quantum_core/mailbox_set8.h"
#include "quantum_core/navigation/path_finder.h"

#include <algorithm>
#include <exception>
#include <limits>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;
namespace FPCollision = Photon::Deterministic::FPCollision;
namespace BS = ByteStreamExtension;
using Photon::Deterministic::Log;

namespace {

// Integer division as the AArch64 build performs it (no divide-by-zero trap).
constexpr std::int32_t DivideInt(std::int32_t value, std::int32_t divisor) {
    return divisor == 0 ? 0 : value / divisor;
}

// Grid cell along one axis. The half extent is computed in 32-bit integers and shifted
// into FP range as a 32-bit value, exactly like the original (it wraps for huge grids).
std::int32_t GridCell(FP position, std::int32_t gridSize, std::int32_t nodeSize) {
    const auto extent = static_cast<std::int32_t>(static_cast<std::uint32_t>(gridSize) *
                                                  static_cast<std::uint32_t>(nodeSize));
    const auto halfExtentRaw =
        static_cast<std::int32_t>(static_cast<std::uint32_t>(extent / 2) << FP::Precision);
    const auto shifted = static_cast<std::int64_t>(static_cast<std::uint64_t>(position.RawValue()) +
                                                   static_cast<std::uint64_t>(halfExtentRaw));
    return DivideInt(static_cast<std::int32_t>(shifted >> FP::Precision), nodeSize);
}

// Half of the grid extent along one axis, as an FP (from an int, no wrapping concerns).
FP HalfGridExtent(std::int32_t gridSize, std::int32_t nodeSize) {
    const auto extent = static_cast<std::int32_t>(static_cast<std::uint32_t>(gridSize) *
                                                  static_cast<std::uint32_t>(nodeSize));
    return FP(extent / 2);
}

template <class T> const T& Element(const std::vector<T>& items, std::int64_t index) {
    return items.at(static_cast<std::size_t>(index));
}

} // namespace

// 0x7ce2834
void NavMesh::Loaded(const NavMeshBinaryData* dataAsset) {
    if (!DataAsset.IsValid()) {
        return;
    }
    if (dataAsset == nullptr) {
        // Quantum.Log.Warn in the original.
        Log::Warn("NavMesh cannot load data file from {0}", DataAsset.ToString(true));
        return;
    }
    // BinaryData.CreateByteStream: decompress if needed, then read.
    std::vector<std::uint8_t> bytes =
        dataAsset->IsCompressed
            ? Photon::Deterministic::ByteUtils::GZipDecompressBytes(dataAsset->Data)
            : dataAsset->Data;
    try {
        ByteStream stream(&bytes);
        Serialize(stream, false);
    } catch (const std::exception& exception) {
        Log::Error("Serializing the asset with guid {0} for navmesh {1} {2} caused an error",
                   DataAsset.ToString(true), Name, Guid.ToString(true));
        Log::Exception(exception);
    }
}

// 0x7ce3164
int NavMesh::CalculateNodeGridIndex(FP positionX, FP positionZ) const {
    const std::int32_t x = GridCell(positionX, GridSizeX, GridNodeSize);
    const std::int32_t z = GridCell(positionZ, GridSizeY, GridNodeSize);
    return z * GridSizeX + x;
}

// 0x7ce31b4
void NavMesh::CalculateNodeGridPosition(FP positionX, FP positionZ, int& x, int& z) const {
    x = GridCell(positionX, GridSizeX, GridNodeSize);
    z = GridCell(positionZ, GridSizeY, GridNodeSize);
}

// 0x7ce320c
void NavMesh::ClampToGrid(FPVector3& position) const {
    const FPVector2 half(HalfGridExtent(GridSizeX, GridNodeSize),
                         HalfGridExtent(GridSizeY, GridNodeSize));
    position.X = FPMath::Clamp(position.X, -half.X, half.X);
    position.Z = FPMath::Clamp(position.Z, -half.Y, half.Y);
}

// 0x7ce32bc
std::int64_t NavMesh::GetHeightAtTriangleRaw(const FPVector3& p, const FPVector3& normal,
                                             const FPVector3& v0) {
    if (normal.Y.RawValue() == 0) {
        return p.Y.RawValue();
    }
    // Plane n.(x - v0) = 0 solved for y.
    const FP planeOffset = normal.Y * v0.Y + normal.Z * v0.Z + normal.X * v0.X;
    const FP horizontal = p.X * normal.X + p.Z * normal.Z;
    return ((planeOffset - horizontal) / normal.Y).RawValue();
}

// 0x7ce3474
bool NavMesh::FindClosestTriangle(const FPVector3& sourcePosition, FP radius,
                                  NavMeshRegionMask regionMask, int& triangle,
                                  FPVector3& closestPosition, MailboxSet8* mailbox) const {
    triangle = -1;
    closestPosition = FPVector3::Zero();
    if (mailbox != nullptr) {
        mailbox->StartSearch();
    }

    const int centerX =
        FPMath::Clamp(GridCell(sourcePosition.X, GridSizeX, GridNodeSize), 0, GridSizeX - 1);
    const int centerZ =
        FPMath::Clamp(GridCell(sourcePosition.Z, GridSizeY, GridNodeSize), 0, GridSizeY - 1);
    const int rings = FPMath::RoundToInt(radius / (GridNodeSize >> 1));

    // Best "point outside every triangle" candidate (squared distance) and best "point
    // inside a triangle" candidate (height difference). Both start at int.MaxValue raw.
    FP closestDistanceSqr = FP::FromRaw(std::numeric_limits<std::int32_t>::max());
    FP closestHeightDelta = FP::FromRaw(std::numeric_limits<std::int32_t>::max());

    for (int ring = 0; ring <= rings; ++ring) {
        for (int dz = -ring; dz <= ring; ++dz) {
            for (int dx = -ring; dx <= ring; ++dx) {
                // Only the cells on this ring's outline; inner cells were done already.
                if (std::abs(dx) != ring && std::abs(dz) != ring) {
                    continue;
                }
                const int x = centerX + dx;
                const int z = centerZ + dz;
                if (x < 0 || x > GridSizeX - 1 || z < 0 || z > GridSizeY - 1) {
                    continue;
                }
                const NavMeshTriangleNode& node = Element(TrianglesGrid, z * GridSizeX + x);
                for (const std::int32_t candidate : node.Triangles) {
                    if (mailbox != nullptr && mailbox->AlreadyChecked(candidate)) {
                        continue;
                    }
                    const NavMeshTriangle& tri = Element(Triangles, candidate);
                    if (!tri.Regions.IsSubset(regionMask)) {
                        continue;
                    }
                    const FPVector3& v0 = Element(Vertices, tri.Vertex0).Point;
                    const FPVector3& v1 = Element(Vertices, tri.Vertex1).Point;
                    const FPVector3& v2 = Element(Vertices, tri.Vertex2).Point;

                    if (FPCollision::TriangleContainsPointInclusive(sourcePosition, v0, v1, v2)) {
                        const FP height =
                            FP::FromRaw(GetHeightAtTriangleRaw(sourcePosition, tri.Normal, v0));
                        const FP heightDelta = FPMath::Abs(sourcePosition.Y - height);
                        const bool withinReach = heightDelta <= radius || radius.RawValue() == 0;
                        if (withinReach && heightDelta < closestHeightDelta) {
                            closestDistanceSqr = FP::Zero();
                            closestHeightDelta = heightDelta;
                            triangle = candidate;
                            closestPosition = {sourcePosition.X, height, sourcePosition.Z};
                        }
                    } else if (radius > FP::Zero()) {
                        FPVector3 closest;
                        FPVector3 barycentric;
                        const FP distanceSqr = FPCollision::ClosestDistanceToTriangle(
                            sourcePosition, v0, v1, v2, closest, barycentric);
                        if (distanceSqr < closestDistanceSqr) {
                            closestDistanceSqr = distanceSqr;
                            triangle = candidate;
                            closestPosition = closest;
                        }
                    }
                }
            }
        }
    }
    return triangle != -1;
}

// 0x7ce3a78
int NavMesh::FindTriangle2D(const FPVector3& position, NavMeshRegionMask regionMask,
                            bool inclusive) const {
    const int cell = CalculateNodeGridIndex(position.X, position.Z);
    if (cell < 0 || cell >= static_cast<int>(TrianglesGrid.size())) {
        return -1;
    }
    for (const std::int32_t candidate : Element(TrianglesGrid, cell).Triangles) {
        const NavMeshTriangle& tri = Element(Triangles, candidate);
        if (!tri.Regions.IsSubset(regionMask)) {
            continue;
        }
        const FPVector3& v0 = Element(Vertices, tri.Vertex0).Point;
        const FPVector3& v1 = Element(Vertices, tri.Vertex1).Point;
        const FPVector3& v2 = Element(Vertices, tri.Vertex2).Point;
        const bool inside = inclusive
                                ? FPCollision::TriangleContainsPointInclusive(position, v0, v1, v2)
                                : FPCollision::TriangleContainsPointExclusive(position, v0, v1, v2);
        if (inside) {
            return candidate;
        }
    }
    return -1;
}

// 0x7ce3dc0
bool NavMesh::FindRandomPointOnTriangle(int triangle, Photon::Deterministic::RNGSession* rngSession,
                                        FPVector3& result) const {
    if (triangle < 0 || triangle >= static_cast<int>(Triangles.size())) {
        result = FPVector3::Zero();
        return false;
    }
    FP a = rngSession->Next();
    FP b = rngSession->Next();
    // Fold points of the parallelogram's far half back into the triangle.
    if (a + b >= FP::One()) {
        a = FP::One() - a;
        b = FP::One() - b;
    }
    const NavMeshTriangle& tri = Element(Triangles, triangle);
    const FPVector3& v0 = Element(Vertices, tri.Vertex0).Point;
    const FPVector3& v1 = Element(Vertices, tri.Vertex1).Point;
    const FPVector3& v2 = Element(Vertices, tri.Vertex2).Point;
    result.X = v0.X + (v1.X - v0.X) * a + (v2.X - v0.X) * b;
    result.Y = v0.Y + (v1.Y - v0.Y) * a + (v2.Y - v0.Y) * b;
    result.Z = v0.Z + (v1.Z - v0.Z) * a + (v2.Z - v0.Z) * b;
    return true;
}

// 0x7ce3f94
bool NavMesh::FindRandomPointOnNavmesh(const FPVector3& position, FP radius,
                                       Photon::Deterministic::RNGSession* rngSession,
                                       NavMeshRegionMask regionMask, FPVector3& result) const {
    const FP angleSample = rngSession->Next();
    const FP distanceSample = rngSession->Next();
    // sqrt of the sample gives a uniform distribution over the disc's area.
    const FP distance = FPMath::Sqrt(distanceSample) * radius;
    FP sin;
    FP cos;
    FPMath::SinCos(angleSample * FP::PiTimes2(), sin, cos);
    const FPVector3 candidate{position.X + cos * distance, position.Y, position.Z + sin * distance};
    // What is left of the radius is the snapping tolerance.
    const FP snapRadius = FP::FromRaw(std::max<std::int64_t>((radius - distance).RawValue(), 0));

    int triangle = 0;
    FPVector3 closest;
    const bool found =
        FindClosestTriangle(candidate, snapRadius, regionMask, triangle, closest, nullptr);
    result = found ? closest : FPVector3::Zero();
    return found;
}

// 0x7ce41c0
bool NavMesh::IsBorderActive(int borderIndex, NavMeshRegionMask regionMask) const {
    const NavMeshBorder& border = Element(Borders, borderIndex);
    if (!border.Regions.IsMainArea() && !border.Regions.IsSubset(regionMask)) {
        return false;
    }
    if (border.RegionsOffmesh.IsMainArea()) {
        return true;
    }
    return !border.RegionsOffmesh.IsSubset(regionMask);
}

// 0x7ce42c0
bool NavMesh::Contains(const FPVector3& position, NavMeshRegionMask regionMask,
                       bool inclusive) const {
    return FindTriangle2D(position, regionMask, inclusive) >= 0;
}

// 0x7ce4314
bool NavMesh::LineOfSight(const FPVector3& positionStart, const FPVector3& positionDestination,
                          NavMeshRegionMask regionMask, int triangleStart, int triangleDestination,
                          bool considerCost) const {
    if (positionStart == positionDestination) {
        return true;
    }
    FPVector3 snapped;
    if (triangleStart < 0 && !FindClosestTriangle(positionStart, FP::Zero(), regionMask,
                                                  triangleStart, snapped, nullptr)) {
        return false;
    }
    if (triangleDestination < 0 && !FindClosestTriangle(positionDestination, FP::Zero(), regionMask,
                                                        triangleDestination, snapped, nullptr)) {
        return false;
    }

    const FP startCost = Element(Triangles, triangleStart).Cost;
    int previous = -1;
    int current = triangleStart;
    int stepsLeft = 1000;
    while (current != triangleDestination) {
        // Find the portal of `current` the start->destination segment leaves through.
        int next = -1;
        for (const NavMeshPortal& portal : Element(Triangles, current).Neighbors) {
            const int neighbor = portal.Neighbor;
            if (!Element(Triangles, neighbor).Regions.IsSubset(regionMask) ||
                neighbor == previous) {
                continue;
            }
            if (considerCost && Element(Triangles, neighbor).Cost != startCost) {
                continue;
            }
            const FPVector3& left = Element(Vertices, portal.LeftVertex).Point;
            if (positionDestination == left) {
                return true;
            }
            const FPVector3& right = Element(Vertices, portal.RightVertex).Point;
            if (positionDestination == right) {
                return true;
            }
            const std::int64_t leftSide =
                PathFinder::CalculateTriangleArea(positionStart, left, positionDestination);
            const std::int64_t rightSide =
                PathFinder::CalculateTriangleArea(positionStart, right, positionDestination);
            if (leftSide >= 0 && rightSide <= 0) {
                next = neighbor;
                break;
            }
        }
        if (next < 0) {
            return false;
        }
        previous = current;
        current = next;
        if (--stepsLeft == 0) {
            Log::Warn("LineOfSight did not finish: {0} {1} {2} {3}", positionStart.ToString(),
                      positionDestination.ToString(), triangleStart, triangleDestination);
            return false;
        }
    }
    return true;
}

// 0x7ce48f4
bool NavMesh::Raycast2D(FPVector2 origin, FPVector2 direction, FP distance,
                        NavMeshRegionMask regionMask, MailboxSet8* mailbox, FPVector2& hit) const {
    hit = FPVector2::Zero();
    if (mailbox != nullptr) {
        mailbox->StartSearch();
    }
    const FPVector2 end{origin.X + direction.X * distance, origin.Y + direction.Y * distance};

    const auto cellOf = [this](FP value, FP offset) {
        return ((value - offset) / GridNodeSize).AsInt();
    };
    const int startX = cellOf(origin.X, WorldOffset.X);
    const int startY = cellOf(origin.Y, WorldOffset.Y);
    const int endX = cellOf(end.X, WorldOffset.X);
    const int endY = cellOf(end.Y, WorldOffset.Y);
    const int minX = std::max(std::min(startX, endX), 0);
    const int maxX = std::min(std::max(startX, endX), GridSizeX - 1);
    const int minY = std::max(std::min(startY, endY), 0);
    const int maxY = std::min(std::max(startY, endY), GridSizeY - 1);
    if (minY > maxY) {
        return false;
    }

    FP closestDistanceSqr = FP::FromRaw(std::numeric_limits<std::int32_t>::max());
    FPVector2 closest;
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            for (const std::int32_t borderIndex : Element(BorderGrid, y * GridSizeX + x).Borders) {
                if (mailbox != nullptr && mailbox->AlreadyChecked(borderIndex)) {
                    continue;
                }
                if (!IsBorderActive(borderIndex, regionMask)) {
                    continue;
                }
                const NavMeshBorder& border = Element(Borders, borderIndex);
                FP hitDistance;
                FP normalizedDistance;
                // The hit point is written straight into `hit` (as the original does), so a
                // miss leaves it as the last test's output.
                if (FPCollision::LineIntersectsLine(origin, end,
                                                    FPVector2(border.V0.X, border.V0.Z),
                                                    FPVector2(border.V1.X, border.V1.Z), hit,
                                                    hitDistance, normalizedDistance)) {
                    const FP distanceSqr = (hit - origin).SqrMagnitude();
                    if (distanceSqr < closestDistanceSqr) {
                        closestDistanceSqr = distanceSqr;
                        closest = hit;
                    }
                }
            }
        }
    }

    if (closestDistanceSqr.RawValue() > std::numeric_limits<std::int32_t>::max() - 1) {
        return false;
    }
    if (closestDistanceSqr >= distance * distance) {
        return false;
    }
    hit = closest;
    return true;
}

// 0x7ce4c18
FPVector2 NavMesh::MovePositionIntoNavmesh(FPVector2 position, FPVector2 newPosition, FP correction,
                                           NavMeshRegionMask regionMask,
                                           MailboxSet8* mailbox) const {
    // Cells covering the movement's bounding box. Note the upper clamp only triggers
    // above GridSize (not at it), as in the original.
    const auto cellOf = [this](FP value) {
        const std::int64_t whole = value.RawValue() >> FP::Precision;
        return GridNodeSize == 0 ? std::int64_t{0} : whole / GridNodeSize;
    };
    std::int64_t maxX = cellOf(FPMath::Max(position.X, newPosition.X) - WorldOffset.X);
    std::int64_t minX = cellOf(FPMath::Min(position.X, newPosition.X) - WorldOffset.X);
    std::int64_t maxY = cellOf(FPMath::Max(position.Y, newPosition.Y) - WorldOffset.Y);
    std::int64_t minY = cellOf(FPMath::Min(position.Y, newPosition.Y) - WorldOffset.Y);
    if (maxX > GridSizeX) {
        maxX = GridSizeX - 1;
    }
    if (maxY > GridSizeY) {
        maxY = GridSizeY - 1;
    }
    minX = std::max<std::int64_t>(minX, 0);
    minY = std::max<std::int64_t>(minY, 0);

    if (mailbox != nullptr) {
        mailbox->StartSearch();
    }
    if (minX > maxX) {
        return newPosition;
    }

    bool insideChecked = false;
    bool outside = false;
    bool found = false;
    FPVector2 closestOffset;
    FP closestDistanceSqr = FP::FromRaw(std::numeric_limits<std::int32_t>::max());
    for (std::int64_t x = minX; x <= maxX; ++x) {
        for (std::int64_t y = minY; y <= maxY; ++y) {
            // A missing node is a NullReferenceException in the original; the baker always
            // fills every cell.
            for (const std::int32_t borderIndex : Element(BorderGrid, y * GridSizeX + x).Borders) {
                if (mailbox != nullptr && mailbox->AlreadyChecked(borderIndex)) {
                    continue;
                }
                if (!IsBorderActive(borderIndex, regionMask)) {
                    continue;
                }
                const NavMeshBorder& border = Element(Borders, borderIndex);
                // Only positions off the mesh are corrected; checked once, lazily.
                if (!insideChecked) {
                    insideChecked = true;
                    outside = FindTriangle2D(FPVector3(newPosition.X, FP::Zero(), newPosition.Y),
                                             regionMask, false) < 0;
                }
                if (!outside) {
                    continue;
                }
                const FPVector2 onBorder = FPCollision::ClosestPointOnSegment(
                    newPosition, FPVector2(border.V0.X, border.V0.Z),
                    FPVector2(border.V1.X, border.V1.Z));
                const FPVector2 offset = onBorder - newPosition;
                const FP distanceSqr = offset.SqrMagnitude();
                if (distanceSqr < closestDistanceSqr) {
                    closestOffset = offset;
                    closestDistanceSqr = distanceSqr;
                    found = true;
                }
            }
        }
    }

    if (!found) {
        return newPosition;
    }
    const FPVector2 target = closestOffset + newPosition;
    if (correction < FP::One()) {
        return FPVector2::Lerp(newPosition, target, correction);
    }
    return target;
}

// 0x7ce2d30
void NavMesh::Serialize(ByteStream& stream, bool write) {
    Triangles = BS::SerializeArrayLength(stream, write, std::move(Triangles));
    for (NavMeshTriangle& triangle : Triangles) {
        triangle.Serialize(stream, write);
    }
    TrianglesGrid = BS::SerializeArrayLength(stream, write, std::move(TrianglesGrid));
    for (NavMeshTriangleNode& node : TrianglesGrid) {
        node.Serialize(stream, write);
    }
    Vertices = BS::SerializeArrayLength(stream, write, std::move(Vertices));
    for (NavMeshVertex& vertex : Vertices) {
        vertex.Serialize(stream, write);
    }
    BorderGrid = BS::SerializeArrayLength(stream, write, std::move(BorderGrid));
    for (NavMeshBorderNode& node : BorderGrid) {
        node.Serialize(stream, write);
    }
    TrianglesCenterGrid = BS::SerializeArrayLength(stream, write, std::move(TrianglesCenterGrid));
    for (NavMeshTriangleCenterGridNode& node : TrianglesCenterGrid) {
        node.Serialize(stream, write);
    }
    Borders = BS::SerializeArrayLength(stream, write, std::move(Borders));
    for (NavMeshBorder& border : Borders) {
        border.Serialize(stream, write);
    }
    Links = BS::SerializeArrayLength(stream, write, std::move(Links));
    // Quantum.Assert.Always: link indices are stored as shorts in agent waypoints.
    if (Links.size() >= 0x8000) {
        throw std::length_error("NavMesh: too many links");
    }
    for (NavMeshLink& link : Links) {
        link.Serialize(stream, write);
    }
}

} // namespace Quantum
