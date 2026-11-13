#include "math_tables.h"
#include "test_framework.h"

#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/rng_session.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/byte_stream.h"
#include "quantum_core/mailbox_set8.h"
#include "quantum_core/navigation/navmesh.h"
#include "quantum_core/navigation/navmesh_agent_config.h"
#include "quantum_core/navigation/path_finder.h"

#include <algorithm>
#include <array>
#include <map>
#include <utility>
#include <vector>

using namespace Quantum;
using Photon::Deterministic::FPMath::Abs;

namespace {

FP F(int v) {
    return FP::FromInt(v);
}
FP Half(int v) {
    return FP::FromRaw(static_cast<std::int64_t>(v) * FP::RAW_ONE / 2);
}
FPVector3 V3(FP x, FP y, FP z) {
    return {x, y, z};
}
bool Near(FP a, FP b, int tolerance = 16) {
    return Abs(a - b) <= FP::FromRaw(tolerance);
}
bool Near(const FPVector3& a, const FPVector3& b, int tolerance = 16) {
    return Near(a.X, b.X, tolerance) && Near(a.Y, b.Y, tolerance) && Near(a.Z, b.Z, tolerance);
}

// Builds a navmesh from triangles the way the baker lays the data out: portals with
// left/right as seen when walking into the neighbour (right = clockwise side, X right,
// Z up), borders on unshared edges with inward normals, and grids of `gridSize` x
// `gridSize` unit cells centred on the origin.
NavMesh BuildNavMesh(const std::vector<FPVector3>& points,
                     const std::vector<std::array<int, 3>>& tris, int gridSize) {
    NavMesh mesh;
    mesh.Name = "test";
    mesh.GridSizeX = gridSize;
    mesh.GridSizeY = gridSize;
    mesh.GridNodeSize = 1;
    mesh.WorldOffset = {-F(gridSize / 2), -F(gridSize / 2)};
    for (const FPVector3& p : points) {
        mesh.Vertices.push_back({p, {}});
    }

    std::map<std::pair<int, int>, std::vector<int>> edges;
    for (int t = 0; t < static_cast<int>(tris.size()); ++t) {
        NavMeshTriangle tri;
        tri.Vertex0 = tris[t][0];
        tri.Vertex1 = tris[t][1];
        tri.Vertex2 = tris[t][2];
        const FPVector3& a = points[tri.Vertex0];
        const FPVector3& b = points[tri.Vertex1];
        const FPVector3& c = points[tri.Vertex2];
        tri.Center = (a + b + c) / 3;
        tri.Normal = FPVector3::Normalize(FPVector3::Cross(c - a, b - a));
        if (tri.Normal.Y < FP::Zero()) {
            tri.Normal = -tri.Normal;
        }
        tri.Cost = FP::One();
        mesh.Triangles.push_back(tri);
        for (int e = 0; e < 3; ++e) {
            const int u = tris[t][e];
            const int v = tris[t][(e + 1) % 3];
            edges[{std::min(u, v), std::max(u, v)}].push_back(t);
        }
    }

    for (const auto& [edge, owners] : edges) {
        const FPVector3& p = points[edge.first];
        const FPVector3& q = points[edge.second];
        const FPVector3 mid = (p + q) / 2;
        for (int owner : owners) {
            const FPVector3 dir = mid - mesh.Triangles[owner].Center;
            const FP crossP = dir.X * (p.Z - mid.Z) - dir.Z * (p.X - mid.X);
            const int right = crossP < FP::Zero() ? edge.first : edge.second;
            const int left = right == edge.first ? edge.second : edge.first;
            if (owners.size() == 2) {
                const int other = owners[0] == owner ? owners[1] : owners[0];
                mesh.Triangles[owner].Neighbors.push_back({left, right, other});
            } else {
                NavMeshBorder border;
                border.V0 = p;
                border.V1 = q;
                const FPVector3 inward = mesh.Triangles[owner].Center - mid;
                border.Normal = FPVector3::Normalize({inward.X, FP::Zero(), inward.Z});
                const int index = static_cast<int>(mesh.Borders.size());
                mesh.Borders.push_back(border);
                mesh.Triangles[owner].Borders.push_back(index);
                mesh.Vertices[edge.first].Borders.push_back(index);
                mesh.Vertices[edge.second].Borders.push_back(index);
            }
        }
    }

    // Grids: everything whose bounding box touches a cell is listed in it.
    const int half = gridSize / 2;
    mesh.TrianglesGrid.resize(static_cast<std::size_t>(gridSize * gridSize));
    mesh.BorderGrid.resize(static_cast<std::size_t>(gridSize * gridSize));
    mesh.TrianglesCenterGrid.resize(static_cast<std::size_t>(gridSize * gridSize));
    const auto cellRange = [half, gridSize](FP lo, FP hi, int& first, int& last) {
        first = std::max(lo.AsInt() + half - (lo.RawValue() % FP::RAW_ONE == 0 ? 1 : 0), 0);
        last = std::min(hi.AsInt() + half, gridSize - 1);
    };
    const auto addToGrid = [&](std::vector<NavMeshTriangleNode>* triGrid,
                               std::vector<NavMeshBorderNode>* borderGrid, int index,
                               std::initializer_list<FPVector3> corners) {
        FP minX = corners.begin()->X, maxX = minX, minZ = corners.begin()->Z, maxZ = minZ;
        for (const FPVector3& c : corners) {
            minX = std::min(minX, c.X);
            maxX = std::max(maxX, c.X);
            minZ = std::min(minZ, c.Z);
            maxZ = std::max(maxZ, c.Z);
        }
        int x0, x1, z0, z1;
        cellRange(minX, maxX, x0, x1);
        cellRange(minZ, maxZ, z0, z1);
        for (int z = z0; z <= z1; ++z) {
            for (int x = x0; x <= x1; ++x) {
                const auto cell = static_cast<std::size_t>(z * gridSize + x);
                if (triGrid)
                    (*triGrid)[cell].Triangles.push_back(index);
                if (borderGrid)
                    (*borderGrid)[cell].Borders.push_back(index);
            }
        }
    };
    for (int t = 0; t < static_cast<int>(mesh.Triangles.size()); ++t) {
        const NavMeshTriangle& tri = mesh.Triangles[t];
        addToGrid(&mesh.TrianglesGrid, nullptr, t,
                  {points[tri.Vertex0], points[tri.Vertex1], points[tri.Vertex2]});
    }
    for (int b = 0; b < static_cast<int>(mesh.Borders.size()); ++b) {
        addToGrid(nullptr, &mesh.BorderGrid, b, {mesh.Borders[b].V0, mesh.Borders[b].V1});
    }
    return mesh;
}

// A flat 4x4 square from (-2, -2) to (2, 2), two triangles.
NavMesh SquareMesh() {
    const std::vector<FPVector3> points = {V3(F(-2), F(0), F(-2)), V3(F(2), F(0), F(-2)),
                                           V3(F(2), F(0), F(2)), V3(F(-2), F(0), F(2))};
    return BuildNavMesh(points, {{0, 1, 2}, {0, 2, 3}}, 8);
}

// An L-shaped corridor of unit cells: (0..3, 0..1) and (2..3, 1..3). Its inner corner is
// at (2, 1).
NavMesh CorridorMesh() {
    std::vector<FPVector3> points;
    std::map<std::pair<int, int>, int> index;
    const auto vertex = [&](int x, int z) {
        const auto key = std::make_pair(x, z);
        if (!index.count(key)) {
            index[key] = static_cast<int>(points.size());
            points.push_back(V3(F(x), F(0), F(z)));
        }
        return index[key];
    };
    std::vector<std::array<int, 3>> tris;
    for (const auto& [x, z] :
         std::vector<std::pair<int, int>>{{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}}) {
        const int a = vertex(x, z), b = vertex(x + 1, z), c = vertex(x + 1, z + 1),
                  d = vertex(x, z + 1);
        tris.push_back({a, b, c});
        tris.push_back({a, c, d});
    }
    return BuildNavMesh(points, tris, 8);
}

} // namespace

TEST_CASE("ByteStream stores little-endian values and arrays") {
    std::vector<std::uint8_t> buffer(32);
    ByteStream writer(&buffer);
    writer.WriteInt(0x01020304);
    writer.WriteLong(-2);
    ByteStreamExtension::SerializeIntArray(writer, true, {7, 8});
    CHECK(buffer[0] == 0x04 && buffer[3] == 0x01);
    CHECK(buffer[4] == 0xFE && buffer[11] == 0xFF);
    CHECK(writer.Position() == 24);

    ByteStream reader(&buffer);
    CHECK(reader.ReadInt() == 0x01020304);
    CHECK(reader.ReadLong() == -2);
    const auto values = ByteStreamExtension::SerializeIntArray(reader, false, {});
    CHECK(values.size() == 2 && values[0] == 7 && values[1] == 8);

    bool threw = false;
    try {
        ByteStream shortStream(&buffer);
        for (int i = 0; i < 9; ++i) {
            shortStream.ReadInt();
        }
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("MailboxSet8 marks items once per search") {
    MailboxSet8 mailbox(4);
    mailbox.StartSearch();
    CHECK(!mailbox.AlreadyChecked(2));
    CHECK(mailbox.AlreadyChecked(2));
    CHECK(mailbox.ShouldCheck(1));
    mailbox.StartSearch();
    CHECK(!mailbox.AlreadyChecked(2));
    for (int i = 0; i < 300; ++i) {
        mailbox.StartSearch();
    }
    CHECK(!mailbox.AlreadyChecked(3));
    CHECK(mailbox.AlreadyChecked(3));
}

TEST_CASE("NavMeshRegionMask subset and toggles") {
    NavMeshRegionMask mask;
    CHECK(mask.IsMainArea());
    mask.ToggleRegion(3, true);
    CHECK(mask.IsRegionEnabled(3) && mask.HasValidRegions());
    CHECK(!mask.IsSubset(NavMeshRegionMask(0)));
    CHECK(mask.IsSubset(NavMeshRegionMask::Default()));
    mask.ToggleRegion(3, false);
    CHECK(mask._regions == 0);
    mask.Clear();
    CHECK(mask == NavMeshRegionMask::Default());
    CHECK(NavMeshRegionMask::Combine(NavMeshRegionMask(6), NavMeshRegionMask(3))._regions == 2);
    CHECK(NavMeshRegionMask(5).ToString() == "5");
}

TEST_CASE("NavMeshAgentConfig defaults and Loaded") {
    NavMeshAgentConfig config;
    CHECK(config.Speed == Half(7));
    CHECK(config.AvoidanceType == AvoidanceType::Internal);
    CHECK(config.AvoidanceQuality == AvoidanceQuality::Good);
    config.CachedWaypointCount = 1;
    config.StoppingDistance = FP::Zero();
    config.Loaded();
    CHECK(config.CachedWaypointCount == 3);
    CHECK(config.StoppingDistanceSqrRaw == (FP::EN3() * FP::EN3()).RawValue());
    CHECK(config.AutoBrakingDistanceSqrRaw == FP::_0_25().RawValue());
    config.CachedWaypointCount = 200;
    config.Loaded();
    CHECK(config.CachedWaypointCount == 32);
}

TEST_CASE("NavMesh grid helpers") {
    const NavMesh mesh = SquareMesh();
    int x = 0;
    int z = 0;
    mesh.CalculateNodeGridPosition(Half(1), Half(-3), x, z);
    CHECK(x == 4 && z == 2);
    CHECK(mesh.CalculateNodeGridIndex(Half(1), Half(-3)) == 2 * 8 + 4);
    FPVector3 p = V3(F(10), F(5), F(-10));
    mesh.ClampToGrid(p);
    CHECK(p.X == F(4) && p.Y == F(5) && p.Z == F(-4));
    // Plane y = x / 2 through the origin (normal (-1, 2, 0) / sqrt 5, any length works).
    CHECK(NavMesh::GetHeightAtTriangleRaw(V3(F(2), F(9), F(7)), V3(F(-1), F(2), F(0)),
                                          FPVector3::Zero()) == F(1).RawValue());
    CHECK(NavMesh::GetHeightAtTriangleRaw(V3(F(2), F(9), F(7)), V3(F(1), F(0), F(0)),
                                          FPVector3::Zero()) == F(9).RawValue());
}

TEST_CASE("NavMesh triangle lookups") {
    const NavMesh mesh = SquareMesh();
    const NavMeshRegionMask all = NavMeshRegionMask::Default();
    // Triangle 0 is the lower-right half (x > z), triangle 1 the upper-left.
    CHECK(mesh.FindTriangle2D(V3(F(1), F(0), F(-1)), all, true) == 0);
    CHECK(mesh.FindTriangle2D(V3(F(-1), F(0), F(1)), all, true) == 1);
    CHECK(mesh.FindTriangle2D(V3(F(3), F(0), F(0)), all, true) == -1);
    CHECK(mesh.Contains(V3(F(0), F(0), Half(1)), all, false));
    CHECK(!mesh.Contains(V3(F(5), F(0), F(5)), all, true));

    MailboxSet8 mailbox(mesh.BorderCount());
    int triangle = -1;
    FPVector3 closest;
    CHECK(mesh.FindClosestTriangle(V3(Half(1), F(0), Half(-3)), FP::Zero(), all, triangle, closest,
                                   &mailbox));
    CHECK(triangle == 0 && closest == V3(Half(1), F(0), Half(-3)));
    // Too high above the triangle it is over, so that one is skipped; the other triangle
    // is still taken as "closest" (the distance branch has no radius limit).
    CHECK(mesh.FindClosestTriangle(V3(F(1), F(3), F(-1)), F(1), all, triangle, closest, &mailbox));
    CHECK(triangle == 1);
    // Outside the square: snapped onto the nearest edge.
    CHECK(
        mesh.FindClosestTriangle(V3(Half(5), F(0), F(0)), F(2), all, triangle, closest, &mailbox));
    CHECK(triangle == 0 && Near(closest, V3(F(2), F(0), F(0)), 64));
    CHECK(!mesh.FindClosestTriangle(V3(F(3), F(0), F(0)), FP::Zero(), all, triangle, closest,
                                    &mailbox));
    // Region filtering.
    NavMesh gated = SquareMesh();
    gated.Triangles[0].Regions.ToggleRegion(1, true);
    CHECK(gated.FindTriangle2D(V3(F(1), F(0), F(-1)), NavMeshRegionMask(0), true) == -1);
    CHECK(gated.FindTriangle2D(V3(F(1), F(0), F(-1)), NavMeshRegionMask(2), true) == 0);
}

TEST_CASE("NavMesh random points stay on the mesh") {
    rr::test::InstallMathDerivedTables();
    const NavMesh mesh = SquareMesh();
    Photon::Deterministic::RNGSession rng(1234);
    for (int i = 0; i < 20; ++i) {
        FPVector3 point;
        CHECK(mesh.FindRandomPointOnTriangle(1, &rng, point));
        CHECK(mesh.FindTriangle2D(point, NavMeshRegionMask::Default(), true) >= 0);
        CHECK(mesh.FindRandomPointOnNavmesh(FPVector3::Zero(), Half(3), &rng,
                                            NavMeshRegionMask::Default(), point));
        CHECK(Abs(point.X) <= Half(3) && Abs(point.Z) <= Half(3));
    }
    FPVector3 point = V3(F(1), F(1), F(1));
    CHECK(!mesh.FindRandomPointOnTriangle(5, &rng, point));
    CHECK(point == FPVector3::Zero());
}

TEST_CASE("NavMesh line of sight, raycast and position correction") {
    const NavMesh corridor = CorridorMesh();
    const NavMeshRegionMask all = NavMeshRegionMask::Default();
    CHECK(corridor.LineOfSight(V3(Half(1), F(0), Half(1)), V3(Half(5), F(0), Half(1)), all, -1, -1,
                               false));
    CHECK(!corridor.LineOfSight(V3(Half(1), F(0), Half(1)), V3(Half(5), F(0), Half(5)), all, -1, -1,
                                false));
    CHECK(corridor.LineOfSight(V3(Half(5), F(0), Half(1)), V3(Half(5), F(0), Half(5)), all, -1, -1,
                               true));

    MailboxSet8 mailbox(corridor.BorderCount());
    FPVector2 hit;
    // From inside the corridor towards +Z: hits the top wall at z = 1.
    CHECK(corridor.Raycast2D({Half(1), Half(1)}, {F(0), F(1)}, F(3), all, &mailbox, hit));
    CHECK(Near(hit.X, Half(1)) && Near(hit.Y, F(1)));
    CHECK(!corridor.Raycast2D({Half(1), Half(1)}, {F(1), F(0)}, Half(1), all, &mailbox, hit));

    // Moving through the wall is pulled back onto it.
    const FPVector2 corrected = corridor.MovePositionIntoNavmesh(
        {Half(1), Half(1)}, {Half(1), Half(3)}, F(1), all, &mailbox);
    CHECK(corrected.X == Half(1) && corrected.Y == F(1));
    const FPVector2 inside = corridor.MovePositionIntoNavmesh(
        {Half(1), Half(1)}, {FP::FromRaw(81920), Half(1)}, F(1), all, &mailbox);
    CHECK(inside.X == FP::FromRaw(81920) && inside.Y == Half(1));
    const FPVector2 partial = corridor.MovePositionIntoNavmesh(
        {Half(1), Half(1)}, {Half(1), Half(3)}, Half(1), all, &mailbox);
    CHECK(partial.X == Half(1) && partial.Y == FP::FromRaw(FP::RAW_ONE * 5 / 4));
}

TEST_CASE("NavMesh serialisation round trip") {
    NavMesh mesh = CorridorMesh();
    mesh.Links.push_back({V3(Half(1), F(0), Half(1)), V3(Half(5), F(0), Half(5)), 9, F(-1),
                          NavMeshRegionMask(0), "jump"});
    std::vector<std::uint8_t> buffer(1 << 16);
    ByteStream writer(&buffer);
    mesh.Serialize(writer, true);
    buffer.resize(static_cast<std::size_t>(writer.Position()));

    NavMesh loaded;
    loaded.DataAsset = AssetGuid(42);
    loaded.LoadData(nullptr); // missing data: only a warning
    CHECK(loaded.Triangles.empty());
    BinaryData data;
    data.Data = buffer;
    loaded.LoadData(&data);
    CHECK(loaded.Triangles.size() == mesh.Triangles.size());
    CHECK(loaded.Vertices.size() == mesh.Vertices.size());
    CHECK(loaded.Borders.size() == mesh.Borders.size());
    CHECK(loaded.TrianglesGrid.size() == mesh.TrianglesGrid.size());
    CHECK(loaded.Links.size() == 1 && loaded.Links[0].Name == "jump" &&
          loaded.Links[0].Triangle == 9);
    bool same = true;
    for (std::size_t t = 0; t < mesh.Triangles.size(); ++t) {
        const NavMeshTriangle& a = mesh.Triangles[t];
        const NavMeshTriangle& b = loaded.Triangles[t];
        same = same && a.Vertex0 == b.Vertex0 && a.Vertex2 == b.Vertex2 && a.Center == b.Center &&
               a.Normal == b.Normal && a.Neighbors.size() == b.Neighbors.size() &&
               a.Cost == b.Cost && a.Borders == b.Borders;
    }
    CHECK(same);
    CHECK(loaded.Borders[0].V1 == mesh.Borders[0].V1 &&
          loaded.Borders[0].Normal == mesh.Borders[0].Normal);
}

TEST_CASE("ActiveHeap pops lowest F first") {
    ActiveHeap heap(2);
    std::vector<ActiveVertex> nodes(9);
    const std::array<int, 9> costs = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].F = costs[i];
        heap.Push(&nodes[i]);
    }
    nodes[4].F = 0; // decrease and fix up
    heap.Update(&nodes[4]);
    CHECK(heap.Size() == 9);
    std::vector<std::int64_t> popped;
    while (heap.Size() > 0) {
        popped.push_back(heap.Pop()->F);
    }
    CHECK(std::is_sorted(popped.begin(), popped.end()));
    CHECK(popped.front() == 0 && popped.size() == 9);
}

TEST_CASE("ActiveVertex geometry helpers") {
    const FPVector3 a = V3(F(0), F(0), F(0));
    const FPVector3 b = V3(F(2), F(5), F(0));
    CHECK(ActiveVertex::ClosestPointOnSegment(V3(F(1), F(0), F(3)), a, b) == V3(F(1), F(0), F(0)));
    CHECK(ActiveVertex::ClosestPointOnSegment(V3(F(9), F(0), F(3)), a, b) == b);
    FPVector3 point;
    CHECK(ActiveVertex::LineIntersectsLine(V3(F(1), F(0), F(-1)), V3(F(1), F(0), F(1)), a,
                                           V3(F(2), F(0), F(0)), point));
    CHECK(point == V3(F(1), F(0), F(0)));
    CHECK(!ActiveVertex::LineIntersectsLine(V3(F(3), F(0), F(-1)), V3(F(3), F(0), F(1)), a,
                                            V3(F(2), F(0), F(0)), point));
    CHECK(ActiveVertex::Distance(a, V3(F(1), F(-2), F(3)), PathQuality::Good) == F(6).RawValue());
    CHECK(PathFinder::CalculateTriangleArea(a, V3(F(0), F(0), F(1)), V3(F(1), F(0), F(0))) > 0);
}

TEST_CASE("PathFinder finds and funnels a path around a corner") {
    rr::test::InstallMathDerivedTables();
    const NavMesh corridor = CorridorMesh();
    MailboxSet8 mailbox(
        std::max(corridor.BorderCount(), static_cast<int>(corridor.Triangles.size())));
    for (PathQuality quality : {PathQuality::Fast, PathQuality::Good, PathQuality::Best}) {
        PathFinder finder(1024, 256, RawPathAllocationStrategy::Fixed);
        const FindPathResult result =
            finder.FindPath(&mailbox, V3(Half(1), F(0), Half(1)), V3(Half(5), F(0), Half(5)), false,
                            FP::Zero(), NavMeshRegionMask::Default(), quality, &corridor);
        CHECK(result.Result == FindPathResult::Code::Success);
        CHECK(result.CorrectedStartPosition == V3(Half(1), F(0), Half(1)));
        CHECK(finder.PathSize() == 3);
        if (finder.PathSize() == 3) {
            CHECK(finder.Path()[0].Point == V3(Half(1), F(0), Half(1)));
            CHECK(finder.Path()[1].Point == V3(F(2), F(0), F(1)));
            CHECK(finder.Path()[2].Point == V3(Half(5), F(0), Half(5)));
        }
        CHECK(finder.RawPathSize() >= 4);
    }

    PathFinder finder(1024, 256, RawPathAllocationStrategy::Fixed);
    // Same triangle: straight line, no search.
    FindPathResult result = finder.FindPath(
        &mailbox, V3(Half(1), F(0), Half(1)), V3(FP::FromRaw(40000), F(0), FP::FromRaw(30000)),
        false, FP::Zero(), NavMeshRegionMask::Default(), PathQuality::Good, &corridor);
    CHECK(result.Result == FindPathResult::Code::Success && finder.PathSize() == 2);
    // Off the mesh without target correction.
    result =
        finder.FindPath(&mailbox, V3(Half(1), F(0), Half(1)), V3(F(-3), F(0), F(-3)), false,
                        FP::Zero(), NavMeshRegionMask::Default(), PathQuality::Good, &corridor);
    CHECK(result.Result == FindPathResult::Code::InvalidEndPosition);
    // With correction the end snaps onto the mesh.
    result = finder.FindPath(&mailbox, V3(Half(1), F(0), Half(1)), V3(Half(7), F(0), Half(5)), true,
                             F(1), NavMeshRegionMask::Default(), PathQuality::Good, &corridor);
    CHECK(result.Result == FindPathResult::Code::Success);
    CHECK(Near(result.CorrectedEndPosition, V3(F(3), F(0), Half(5)), 64));
    // Unreachable region.
    NavMesh gated = CorridorMesh();
    for (int t = 6; t < 10; ++t) {
        gated.Triangles[static_cast<std::size_t>(t)].Regions.ToggleRegion(0, true);
    }
    result = finder.FindPath(&mailbox, V3(Half(1), F(0), Half(1)), V3(Half(5), F(0), Half(1)),
                             false, FP::Zero(), NavMeshRegionMask(0), PathQuality::Good, &gated);
    CHECK(result.Result == FindPathResult::Code::Success);
    result = finder.FindPath(&mailbox, V3(Half(1), F(0), Half(1)), V3(Half(5), F(0), Half(5)),
                             false, FP::Zero(), NavMeshRegionMask(0), PathQuality::Good, &gated);
    CHECK(result.Result == FindPathResult::Code::InvalidEndPosition);
}

TEST_CASE("PathFinder uses off-mesh links") {
    rr::test::InstallMathDerivedTables();
    NavMesh mesh = CorridorMesh();
    // A cheap link from the first cell straight into the last one.
    const int startTriangle = mesh.FindTriangle2D(V3(Half(1), F(0), FP::FromRaw(20000)),
                                                  NavMeshRegionMask::Default(), true);
    const int endTriangle =
        mesh.FindTriangle2D(V3(Half(5), F(0), Half(5)), NavMeshRegionMask::Default(), true);
    CHECK(startTriangle >= 0 && endTriangle >= 0);
    mesh.Links.push_back({V3(Half(1), F(0), FP::FromRaw(20000)), V3(Half(5), F(0), Half(5)),
                          endTriangle, FP::_0_10(), NavMeshRegionMask(0), "shortcut"});
    mesh.Triangles[static_cast<std::size_t>(startTriangle)].Links.push_back(0);

    MailboxSet8 mailbox(64);
    PathFinder finder(1024, 256, RawPathAllocationStrategy::Fixed);
    const FindPathResult result = finder.FindPath(
        &mailbox, V3(Half(1), F(0), FP::FromRaw(20000)), V3(Half(5), F(0), FP::FromRaw(170000)),
        false, FP::Zero(), NavMeshRegionMask::Default(), PathQuality::Good, &mesh);
    CHECK(result.Result == FindPathResult::Code::Success);
    bool usesLink = false;
    for (int i = 0; i < finder.PathSize(); ++i) {
        usesLink = usesLink || finder.Path()[static_cast<std::size_t>(i)].Link == 0;
    }
    CHECK(usesLink);
}

TEST_CASE("PathFinder reports a full fixed raw path") {
    rr::test::InstallMathDerivedTables();
    const NavMesh corridor = CorridorMesh();
    MailboxSet8 mailbox(64);
    PathFinder small(1024, 3, RawPathAllocationStrategy::Fixed);
    FindPathResult result =
        small.FindPath(&mailbox, V3(Half(1), F(0), Half(1)), V3(Half(5), F(0), Half(5)), false,
                       FP::Zero(), NavMeshRegionMask::Default(), PathQuality::Good, &corridor);
    CHECK(result.Result == FindPathResult::Code::PathTooLong);
    PathFinder growing(1024, 3, RawPathAllocationStrategy::Allocating);
    result =
        growing.FindPath(&mailbox, V3(Half(1), F(0), Half(1)), V3(Half(5), F(0), Half(5)), false,
                         FP::Zero(), NavMeshRegionMask::Default(), PathQuality::Good, &corridor);
    CHECK(result.Result == FindPathResult::Code::Success && growing.PathSize() == 3);
}
