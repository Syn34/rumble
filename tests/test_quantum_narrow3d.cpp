// Tests for the 3D narrow phase: TriangleCCW, sphere checks, GJK distance, simplex solvers,
// shape cast and time of impact. Expected values are worked out by hand.
#include "test_framework.h"
#include "math_tables.h"

#include "quantum_core/gjk_settings.h"
#include "quantum_core/physics3d/gjk3d.h"
#include "quantum_core/physics3d/narrow_checks.h"
#include "quantum_core/shape3d.h"
#include "quantum_core/triangle_ccw.h"

using namespace Quantum;
using namespace Quantum::Physics3D;
using Photon::Deterministic::FPMath::Abs;

namespace {
FP F(int v) { return FP::FromInt(v); }
FP Q(int numerator, int denominator) { return FP::FromRaw(FP::Raw{65536} * numerator / denominator); }
FPVector3 V3(FP x, FP y, FP z) { return {x, y, z}; }
FPVector3 V3(int x, int y, int z) { return {F(x), F(y), F(z)}; }
bool Near(FP a, FP b, int tolerance = 16) { return Abs(a - b) <= FP::FromRaw(tolerance); }

NarrowEntry3D Entry(const Shape3D& shape, const FPVector3& position) {
    NarrowEntry3D entry;
    entry.Shape = &shape;
    entry.Narrow.Position = position;
    entry.Narrow.RotatedRight = FPVector3::Right();
    entry.Narrow.RotatedUp = FPVector3::Up();
    entry.Narrow.RotatedForward = FPVector3::Forward();
    entry.Narrow.RotationInverse = FPQuaternion::Identity();
    entry.TransformPosition = position;
    return entry;
}

// Triangle in the y = 0 plane facing down (-Y).
TriangleCCW FloorTriangle() {
    TriangleCCW tri;
    tri.A = V3(0, 0, 0);
    tri.B = V3(0, 0, 1);
    tri.C = V3(1, 0, 0);
    tri.ComputeNormal();
    return tri;
}
} // namespace

TEST_CASE("Quantum::TriangleCCW derived data, ray test and support") {
    rr::test::InstallMathDerivedTables();
    const TriangleCCW tri = FloorTriangle();
    CHECK(tri.Normal == V3(0, -1, 0) && tri.InverseNormal == V3(0, 1, 0));
    CHECK(tri.Center == V3(FP::FromRaw(21845), F(0), FP::FromRaw(21845)));
    CHECK(tri.EdgeA == V3(0, 0, 1) && tri.EdgeB == V3(1, 0, -1) && tri.EdgeC == V3(-1, 0, 0));
    CHECK(tri.LengthEdgeA == F(1) && tri.LengthEdgeC == F(1));
    CHECK(Near(tri.LengthEdgeB, Q(14142, 10000), 8));
    CHECK(tri.MinY == F(0) && tri.MaxY == F(0));
    CHECK(tri.Min3() == V3(0, 0, 0) && tri.Max3() == V3(1, 0, 1));

    // From below along +Y (length 2): hits at (0.25, 0, 0.25), halfway along the ray.
    FPVector3 point;
    FP fraction;
    CHECK(tri.RayIntersectsTriangle(V3(Q(1, 4), F(-1), Q(1, 4)), V3(0, 2, 0), point, fraction));
    CHECK(point == V3(Q(1, 4), F(0), Q(1, 4)) && fraction == Q(1, 2));
    // From above (the back face) and beside the triangle: misses, outputs zeroed.
    CHECK(!tri.RayIntersectsTriangle(V3(Q(1, 4), F(1), Q(1, 4)), V3(0, -2, 0), point, fraction));
    CHECK(!tri.RayIntersectsTriangle(V3(2, -1, 2), V3(0, 2, 0), point, fraction));
    CHECK(point == FPVector3::Zero() && fraction == FP::Zero());

    FPVector3 offset;
    tri.GetSupportOffset(V3(1, 0, 0), offset);
    CHECK(offset == tri.CenterToC);
}

TEST_CASE("Physics3D SphereBox outside and inside") {
    rr::test::InstallMathDerivedTables();
    const Shape3D box = Shape3D::CreateBox(V3(1, 1, 1));
    const Shape3D sphere = Shape3D::CreateSphere(F(1));
    const NarrowEntry3D boxEntry = Entry(box, V3(0, 0, 0));
    const NarrowEntry3D beside = Entry(sphere, V3(Q(3, 2), F(0), F(0)));
    CollisionManifold3D m{};
    CollisionManifoldInfo3D info{&m, &beside, &boxEntry, nullptr};
    CHECK(NarrowChecks::CheckShapes(&info, true));
    CHECK(m.Penetration == Q(1, 2));
    CHECK(m.Normal == V3(-1, 0, 0));
    CHECK(m._contactPointsCount == 1 && m._contactPoints[0] == V3(1, 0, 0));

    // Centre inside, nearest face +X: normal -X, penetration 0.25 + radius, contact the
    // centre nudged 1/256 along the normal.
    const Shape3D small = Shape3D::CreateSphere(Q(1, 2));
    const NarrowEntry3D inside = Entry(small, V3(Q(3, 4), Q(1, 4), F(0)));
    CollisionManifold3D m2{};
    CollisionManifoldInfo3D info2{&m2, &inside, &boxEntry, nullptr};
    CHECK(NarrowChecks::SphereBox::Check(&info2, true));
    CHECK(m2.Normal == V3(-1, 0, 0) && m2.Penetration == Q(3, 4));
    CHECK(m2._contactPoints[0] == V3(Q(3, 4) - FP::FromRaw(256), Q(1, 4), F(0)));

    // Box first: the pair is inverted.
    CollisionManifold3D m3{};
    CollisionManifoldInfo3D info3{&m3, &boxEntry, &beside, nullptr};
    CHECK(NarrowChecks::CheckShapes(&info3, true));
    CHECK(m3.GetNormalInverted() && info3.EntryA == &beside);

    const NarrowEntry3D far = Entry(sphere, V3(3, 0, 0));
    CollisionManifold3D m4{};
    CollisionManifoldInfo3D info4{&m4, &far, &boxEntry, nullptr};
    CHECK(!NarrowChecks::CheckShapes(&info4, true));
}

TEST_CASE("Physics3D SphereTriangle face and edge regions") {
    rr::test::InstallMathDerivedTables();
    const TriangleCCW tri = FloorTriangle();
    const Shape3D sphere = Shape3D::CreateSphere(Q(1, 2));
    const Shape3D mesh = Shape3D::CreateMesh(0);
    const NarrowEntry3D meshEntry = Entry(mesh, V3(0, 0, 0));

    // Below the face (its front side): nearest point is the projection.
    const NarrowEntry3D below = Entry(sphere, V3(Q(1, 4), Q(-1, 4), Q(1, 4)));
    CollisionManifold3D m{};
    CollisionManifoldInfo3D info{&m, &below, &meshEntry, &tri};
    CHECK(NarrowChecks::CheckShapes(&info, true));
    CHECK(m.Penetration == Q(1, 4));
    CHECK(m.Normal == V3(0, 1, 0));
    CHECK(m._contactPoints[0] == V3(Q(1, 4), F(0), Q(1, 4)));

    // Beyond edge AB: nearest point (0, 0, 0.5) on that edge.
    const NarrowEntry3D edge = Entry(sphere, V3(Q(-1, 4), Q(-1, 5), Q(1, 2)));
    CollisionManifold3D m2{};
    CollisionManifoldInfo3D info2{&m2, &edge, &meshEntry, &tri};
    CHECK(NarrowChecks::SphereTriangle::Check(&info2, true, false));
    CHECK(m2._contactPoints[0] == V3(F(0), F(0), Q(1, 2)));
    CHECK(Near(m2.Penetration, Q(1798, 10000), 64));

    // Smooth collision reports the projected centre instead.
    CollisionManifold3D m3{};
    CollisionManifoldInfo3D info3{&m3, &edge, &meshEntry, &tri};
    CHECK(NarrowChecks::SphereTriangle::Check(&info3, true, true));
    CHECK(m3._contactPoints[0] == V3(Q(-1, 4), F(0), Q(1, 2)));

    const NarrowEntry3D high = Entry(sphere, V3(Q(1, 4), F(-1), Q(1, 4)));
    CollisionManifold3D m4{};
    CollisionManifoldInfo3D info4{&m4, &high, &meshEntry, &tri};
    CHECK(!NarrowChecks::CheckShapes(&info4, true));
}

TEST_CASE("Physics3D TriangleCollisionIterator average normal") {
    CollisionManifold3D manifolds[3]{};
    manifolds[0]._m.NextInMesh = 1;
    manifolds[1].Normal = V3(0, 1, 0);
    manifolds[1]._m.NextInMesh = 2;
    manifolds[2].Normal = V3(1, 1, 0);
    const NarrowChecks::TriangleCollisionIterator it(nullptr, &manifolds[0], manifolds);
    CHECK(it.AverageNormal() == V3(Q(-1, 2), F(-1), F(0)));
    CHECK(it._currentTri == &manifolds[0]);
}

TEST_CASE("Physics3D GJK distance and simplex solvers") {
    rr::test::InstallMathDerivedTables();
    const Shape3D sphere = Shape3D::CreateSphere(F(1));
    const NarrowEntry3D a = Entry(sphere, V3(0, 0, 0));
    const NarrowEntry3D b = Entry(sphere, V3(0, 0, 5));
    SimplexData3D simplex{};
    SimplexDataPointers3D ptrs(&simplex);
    FP distance;
    FPVector3 axis;
    Gjk3D::ComputeDistance(&ptrs, &a, &b, nullptr, distance, axis, 16, std::nullopt, 0);
    CHECK(distance == F(3) && axis == V3(0, 0, -1));

    const Shape3D box = Shape3D::CreateBox(V3(1, 1, 1));
    const NarrowEntry3D boxA = Entry(box, V3(0, 0, 0));
    const NarrowEntry3D boxB = Entry(box, V3(F(4), Q(1, 2), F(0)));
    Gjk3D::ComputeDistance(&ptrs, &boxA, &boxB, nullptr, distance, axis, 16, std::nullopt, 0);
    CHECK(distance == F(2) && axis == V3(-1, 0, 0));

    // Tetrahedron around the origin: all four weights, closest point the origin.
    simplex.Vertex0.Point = V3(-1, -1, -1);
    simplex.Vertex1.Point = V3(1, -1, -1);
    simplex.Vertex2.Point = V3(0, 1, -1);
    simplex.Vertex3.Point = V3(0, 0, 1);
    SimplexDataPointers3D tetra(&simplex);
    tetra.UsedCount = 4;
    FPVector3 closest;
    bool degenerate = true;
    Gjk3D::ComputeClosestPoint(&tetra, 0, closest, degenerate);
    CHECK(!degenerate && tetra.UsedCount == 4 && closest == FPVector3::Zero());
    CHECK(Near(simplex.Lambda0, Q(1, 8), 64) && Near(simplex.Lambda1, Q(1, 8), 64));
    CHECK(Near(simplex.Lambda2, Q(1, 4), 64) && Near(simplex.Lambda3, Q(1, 2), 64));

    // Moved up so the origin is below the base: the base face gives the closest point.
    simplex.Vertex0.Point = V3(-1, -1, 1);
    simplex.Vertex1.Point = V3(1, -1, 1);
    simplex.Vertex2.Point = V3(0, 1, 1);
    simplex.Vertex3.Point = V3(0, 0, 3);
    SimplexDataPointers3D above(&simplex);
    above.UsedCount = 4;
    Gjk3D::ComputeClosestPoint(&above, 0, closest, degenerate);
    CHECK(above.UsedCount == 3 && closest == V3(0, 0, 1));

    CHECK(!Gjk3D::IsAffinelyDependent(&above));
    bool flat = false;
    FP weight;
    CHECK(!Gjk3D::IsOriginOutsidePlane(V3(0, 0, 1), V3(1, 0, 1), V3(0, 1, 1), V3(0, 0, 1), flat, weight));
    CHECK(flat);
}

TEST_CASE("Physics3D GJK shape cast") {
    rr::test::InstallMathDerivedTables();
    PhysicsQueryConfig config;
    config.ShapeCastMaxIterations = 8;
    config.ShapeCastHardTolerance = Q(1, 1000);
    config.ShapeCastSoftTolerance = Q(1, 100);
    const GJKSettings settings(config);
    const Shape3D sphere = Shape3D::CreateSphere(F(1));
    const NarrowEntry3D a = Entry(sphere, V3(0, 0, 0));
    const NarrowEntry3D b = Entry(sphere, V3(5, 0, 0));
    NarrowTls3D tls;
    Hit3D hit;
    CHECK(Gjk3D::ShapeCastManifold(tls, &a, &b, nullptr, settings, V3(-10, 0, 0), true, hit));
    CHECK(hit.CastDistanceNormalized == FP::FromRaw(19660));
    CHECK(hit.Normal == V3(1, 0, 0));
    CHECK(hit.Point == V3(FP::FromRaw(65544), F(0), F(0)));
    CHECK(!Gjk3D::ShapeCastManifold(tls, &a, &b, nullptr, settings, V3(10, 0, 0), true, hit));

    // Casting into the back of a triangle never hits.
    const TriangleCCW tri = FloorTriangle();
    CHECK(!Gjk3D::ShapeCastManifold(tls, &a, &b, &tri, settings, V3(0, -1, 0), true, hit));
}

TEST_CASE("Physics3D time of impact of a fast sphere") {
    rr::test::InstallMathDerivedTables();
    const Shape3D sphere = Shape3D::CreateSphere(Q(1, 2));
    NarrowBody3D body;
    body.Velocity = V3(10, 0, 0);
    NarrowEntry3D a = Entry(sphere, V3(0, 0, 0));
    a.Body = &body;
    a.HasDynamicBody = true;
    const NarrowEntry3D b = Entry(sphere, V3(3, 0, 0));

    PhysicsQueryConfig config;
    config.MaxTimeOfImpactIterations = 8;
    config.MaxRootFindingIterations = 20;
    config.CCDDistanceTolerance = Q(1, 100);
    const CCDSettings settings(config);

    CollisionManifold3D m{};
    CollisionManifoldInfo3D info{&m, &a, &b, nullptr};
    NarrowTls3D tls;
    FP toi;
    CHECK(Gjk3D::ComputeTimeOfImpact(tls, &info, settings, true, F(1), toi) == TimeOfImpactResult::Collision);
    CHECK(toi == FP::FromRaw(13107));
    CHECK(m._m.TimeOfImpact == toi);
    CHECK(m.Normal == V3(1, 0, 0));
    // B's witness (its centre) moved by its radius along the B-to-A axis.
    CHECK(m._contactPointsCount == 1 && m._contactPoints[0] == V3(Q(5, 2), F(0), F(0)));

    body.Velocity = V3(1, 0, 0);
    CollisionManifold3D m2{};
    CollisionManifoldInfo3D info2{&m2, &a, &b, nullptr};
    CHECK(Gjk3D::ComputeTimeOfImpact(tls, &info2, settings, false, F(1), toi) == TimeOfImpactResult::Ignore);

    body.Velocity = V3(10, 0, 0);
    CollisionManifold3D m3{};
    m3._m.SetUsesCcd(true);
    CollisionManifoldInfo3D info3{&m3, &a, &b, nullptr};
    CHECK(NarrowChecks::Check(tls, settings, F(1), &info3, false));
}
