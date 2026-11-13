// Tests for static triangle meshes: the map's baked triangle data (TriangleCCW / Map
// serialisation), TriangleMesh cells and the mesh paths of the 3D scene.
#include "test_framework.h"
#include "math_tables.h"

#include "quantum_core/byte_stream.h"
#include "quantum_core/map.h"
#include "quantum_core/triangle_ccw.h"

#include <vector>

using namespace Quantum;

namespace {

FP F(int v) { return FP::FromInt(v); }
FPVector3 V3(int x, int y, int z) { return {F(x), F(y), F(z)}; }

TriangleCCW MakeTriangle(FPVector3 a, FPVector3 b, FPVector3 c, std::int32_t staticIndex) {
    TriangleCCW tri;
    tri.A = a;
    tri.B = b;
    tri.C = c;
    tri.StaticDataIndex = staticIndex;
    tri.ComputeNormal();
    return tri;
}

bool SameTriangle(const TriangleCCW& x, const TriangleCCW& y) {
    return x.A == y.A && x.B == y.B && x.C == y.C && x.Normal == y.Normal && x.Center == y.Center &&
           x.EdgeA == y.EdgeA && x.LengthEdgeC == y.LengthEdgeC && x.BroadRadius == y.BroadRadius &&
           x.MaxY == y.MaxY && x.MinY == y.MinY && x.StaticDataIndex == y.StaticDataIndex;
}

} // namespace

TEST_CASE("TriangleCCW.Serialize writes vertices, optional metadata and the static index") {
    rr::test::InstallMathDerivedTables();
    const TriangleCCW tri = MakeTriangle(V3(0, 0, 0), V3(0, 0, 2), V3(2, 0, 0), 7);

    for (const bool metadata : {false, true}) {
        // The stream writes into a buffer of fixed size.
        std::vector<std::uint8_t> bytes(1024);
        ByteStream out(&bytes);
        TriangleCCW copy = tri;
        copy.Serialize(out, true, metadata);
        // Vertices 72 bytes; metadata 6 FPs and 9 vectors (264 bytes); static index 4 bytes.
        CHECK(out.Position() == (metadata ? 72 + 264 + 4 : 76));

        ByteStream in(&bytes);
        TriangleCCW read;
        read.Serialize(in, false, metadata);
        CHECK(SameTriangle(read, tri));
    }
}

TEST_CASE("Map.SerializeStaticColliderTriangles round-trips the triangle dictionary") {
    rr::test::InstallMathDerivedTables();
    Map map;
    map.StaticColliders3DTriangles[3] = {MakeTriangle(V3(0, 0, 0), V3(0, 0, 1), V3(1, 0, 0), 3),
                                         MakeTriangle(V3(1, 0, 0), V3(0, 0, 1), V3(1, 0, 1), 3)};
    map.StaticColliders3DTriangles[1] = {MakeTriangle(V3(0, 1, 0), V3(0, 1, 4), V3(4, 1, 0), 1)};
    map.SerializeTrianglesMetadata = false;

    std::vector<std::uint8_t> bytes(4096);
    ByteStream out(&bytes);
    map.SerializeStaticColliderTriangles(out, true);
    CHECK(map.OptionalTriangleMetadataSerializationEnabled);
    // count, then (key 1: count, 1 triangle), (key 3: count, 2 triangles)
    CHECK(out.Position() == 4 + 2 * 8 + 3 * 76);
    CHECK(bytes[0] == 2 && bytes[4] == 1); // little-endian count, then the smallest key

    Map loaded;
    loaded.OptionalTriangleMetadataSerializationEnabled = true;
    loaded.SerializeTrianglesMetadata = false;
    ByteStream in(&bytes);
    loaded.SerializeStaticColliderTriangles(in, false);
    CHECK(loaded.StaticColliders3DTriangles.size() == 2u);
    CHECK(loaded.StaticColliders3DTriangles[3].size() == 2u);
    CHECK(SameTriangle(loaded.StaticColliders3DTriangles[3][1], map.StaticColliders3DTriangles[3][1]));
    CHECK(SameTriangle(loaded.StaticColliders3DTriangles[1][0], map.StaticColliders3DTriangles[1][0]));

    // Without the optional flag the reader expects the metadata: written with it, reads back.
    map.SerializeTrianglesMetadata = true;
    std::vector<std::uint8_t> full(4096);
    ByteStream out2(&full);
    map.SerializeStaticColliderTriangles(out2, true);
    Map legacy;
    ByteStream in2(&full);
    legacy.SerializeStaticColliderTriangles(in2, false);
    CHECK(SameTriangle(legacy.StaticColliders3DTriangles[3][0], map.StaticColliders3DTriangles[3][0]));
}

#include "quantum_core/physics3d/collision_manifold3d.h"
#include "quantum_core/triangle_mesh.h"

TEST_CASE("TriangleMesh: cells, hashes, bounds and insertion order") {
    rr::test::InstallMathDerivedTables();
    CHECK(TriangleMesh::CellHash(0, 0, 0) == 0x8000800080008000ull);
    CHECK(TriangleMesh::CellHash(-1, 1, 0) == 0x800080008001'7fffull);
    CHECK(TriangleMesh::BitsRequiredForNumber(0u) == 0 && TriangleMesh::BitsRequiredForNumber(5u) == 3 &&
          TriangleMesh::BitsRequiredForNumber(-1) == 32);

    TriangleMesh mesh;
    CHECK(mesh.Predicted());
    mesh.Initialize(4, 1, 1, 2);
    CHECK(mesh.CellExtent() == 2 && mesh.TriangleCount() == 1 && mesh.TriangleCapacity() == 2);
    // sqrt(3) / 2 * 4, about 3.46
    CHECK(mesh.CellBroadRadius() > FP::FromRaw(226000) && mesh.CellBroadRadius() < FP::FromRaw(228000));
    CHECK(mesh.InsertMany({MakeTriangle(V3(1, 0, 1), V3(1, 0, 2), V3(2, 0, 1), 0)}) == TriangleReference{});
    mesh.SetPredicted(false);

    // A floor triangle spanning cells x -1..1, z -1..1 (y 0), and a small one in cell (0,0,0).
    const TriangleReference big = mesh.InsertMany({MakeTriangle(V3(-3, 0, -3), V3(-3, 0, 3), V3(3, 0, -3), 0)});
    const TriangleReference small = mesh.InsertMany({MakeTriangle(V3(1, 1, 1), V3(1, 1, 2), V3(2, 1, 1), 1),
                                                     MakeTriangle(V3(1, 2, 1), V3(1, 2, 2), V3(2, 2, 1), 1)});
    CHECK(big == TriangleReference(1, true) && small == TriangleReference(3, true));
    CHECK(mesh.TriangleCount() == 4 && mesh.TriangleCapacity() == 4);
    CHECK(mesh.GetTriangle(3)->Next == 2 && mesh.GetTriangle(2)->Next == 0 && mesh.GetTriangle(3)->Index == 3);

    const std::int32_t* tris = nullptr;
    std::int32_t count = 0;
    CHECK(mesh.TryGetTriangles(0, 0, 0, tris, count) && count == 3);
    CHECK(tris[0] == 1 && tris[1] == 2 && tris[2] == 3);
    CHECK(mesh.TryGetTriangles(-1, 0, -1, tris, count) && count == 1 && tris[0] == 1);
    CHECK(!mesh.TryGetTriangles(5, 0, 5, tris, count) && count == 0 && tris == nullptr);

    std::int32_t x = 0, y = 0, z = 0;
    // The raw value is divided (truncating) before the floor: a tiny negative value is cell 0.
    mesh.ToCell(FPVector3{FP::FromRaw(-1), F(5), -F(4)}, x, y, z);
    CHECK(x == 0 && y == 1 && z == -1);

    // Bounds clip to the cells in use (x -1..0, y 0..0, z -1..0).
    Physics3D::IntAABB box;
    box.MinX = static_cast<std::int32_t>(-F(100).RawValue());
    box.MaxX = static_cast<std::int32_t>(F(100).RawValue());
    box.MinY = 0;
    box.MaxY = static_cast<std::int32_t>(F(1).RawValue());
    box.MinZ = static_cast<std::int32_t>(F(1).RawValue());
    box.MaxZ = static_cast<std::int32_t>(F(2).RawValue());
    const TriangleMesh::CellBounds cells = mesh.GetBounds(box);
    CHECK(cells.MinX == -1 && cells.MaxX == 0 && cells.MinY == 0 && cells.MaxY == 0 && cells.MinZ == 0 &&
          cells.MaxZ == 0);
}

TEST_CASE("TriangleMesh: built from a map with immutable and toggleable colliders") {
    rr::test::InstallMathDerivedTables();
    Map map;
    map.Guid = AssetGuid(5);
    map.TriangleMeshCellSize = 8;
    map.StaticColliders3D.resize(3);
    for (auto& collider : map.StaticColliders3D) collider.ShapeType = Shape3DType::Mesh;
    map.StaticColliders3D[1].MutableMode = StaticColliderMutableMode::ToggleableStartOff;
    map.StaticColliders3D[2].MutableMode = StaticColliderMutableMode::ToggleableStartOn;
    map.StaticColliders3DTriangles[0] = {MakeTriangle(V3(0, 0, 0), V3(0, 0, 1), V3(1, 0, 0), 0)};
    map.StaticColliders3DTriangles[1] = {MakeTriangle(V3(0, 1, 0), V3(0, 1, 1), V3(1, 1, 0), 1),
                                         MakeTriangle(V3(0, 2, 0), V3(0, 2, 1), V3(1, 2, 0), 1)};
    map.StaticColliders3DTriangles[2] = {MakeTriangle(V3(0, 3, 0), V3(0, 3, 1), V3(1, 3, 0), 2)};
    CHECK(TriangleMesh::CalculateDefaultTriangleCapacityForMap(map) == 2);

    TriangleMesh mesh;
    mesh.SetPredicted(false);
    mesh.Initialize(map.TriangleMeshCellSize, 4099, 2, 3);
    mesh.InjectImmutableCollidersFromMap(map);
    mesh.LockImmutable();
    CHECK(mesh.TriangleCount() == 2);
    const std::int32_t* tris = nullptr;
    std::int32_t count = 0;
    CHECK(mesh.TryGetTriangles(0, 0, 0, tris, count) && count == 1 && mesh.GetTriangle(tris[0])->CCWTri.StaticDataIndex == 0);
}
