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
