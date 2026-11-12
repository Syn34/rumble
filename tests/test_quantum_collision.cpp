#include "test_framework.h"
#include "math_tables.h"

#include "quantum_core/physics2d/hit.h"
#include "quantum_core/physics2d/narrow_checks.h"
#include "quantum_core/physics3d/hit3d.h"
#include "quantum_core/physics3d/narrow_checks.h"
#include "quantum_core/shape2d.h"
#include "quantum_core/shape3d.h"
#include "quantum_core/transform.h"

#include <cstddef>

using namespace Quantum;
using Photon::Deterministic::FPMath::Abs;

namespace {
FP F(int v) { return FP::FromInt(v); }
FPVector2 V2(int x, int y) { return {F(x), F(y)}; }
FPVector3 V3(int x, int y, int z) { return {F(x), F(y), F(z)}; }
bool Near(FP a, FP b, int tolerance = 16) { return Abs(a - b) <= FP::FromRaw(tolerance); }
bool Near(FPVector2 a, FPVector2 b, int tolerance = 16) {
    return Near(a.X, b.X, tolerance) && Near(a.Y, b.Y, tolerance);
}

Physics2D::NarrowEntry2D Entry(const Shape2D& shape, FPVector2 position, FP rotation = FP::Zero()) {
    Physics2D::NarrowEntry2D entry;
    entry.Shape = &shape;
    entry.Narrow.Position = position;
    Photon::Deterministic::FPMath::SinCos(rotation, entry.Narrow.RotSin, entry.Narrow.RotCos);
    return entry;
}
} // namespace

TEST_CASE("Quantum shape layouts match the original") {
    CHECK(offsetof(Shape2D, LocalTransform) == 8);
    CHECK(offsetof(Shape2D, BroadRadius) == 0x20);
    CHECK(offsetof(Shape3D, LocalTransform) == 8);
    CHECK(offsetof(Shape3D, BroadRadius) == 0x40);
    CHECK(offsetof(Physics2D::CollisionManifold2D, Normal) == 0x48);
    CHECK(offsetof(Physics2D::CollisionManifold2D, _contactPoints) == 0x58);
    CHECK(offsetof(Physics3D::CollisionManifold3D, Normal) == 0x50);
    CHECK(offsetof(Physics3D::CollisionManifold3D, _contactPoints) == 0x68);
    CHECK(offsetof(Physics2D::Hit, Point) == 0x20);
    CHECK(offsetof(Physics3D::Hit3D, Normal) == 0x38);
}

TEST_CASE("Quantum::Transform3D axes and points") {
    rr::test::InstallMathDerivedTables();
    Transform3D t;
    t.Position = V3(1, 2, 3);
    t.Rotation = FPQuaternion::Identity();
    CHECK(t.Right() == FPVector3::Right());
    CHECK(t.Up() == FPVector3::Up());
    CHECK(t.Forward() == FPVector3::Forward());
    CHECK(t.Left() == FPVector3::Left() && t.Back() == FPVector3::Back());
    CHECK(t.TransformPoint(V3(1, 0, 0)) == V3(2, 2, 3));
    CHECK(t.InverseTransformPoint(V3(2, 2, 3)) == V3(1, 0, 0));
    t.Rotate(F(0), F(90), F(0));
    // Yaw by 90 degrees maps forward onto right (within rounding).
    const FPVector3 f = t.Forward();
    CHECK(Near(f.X, F(1), 128) && Near(f.Z, F(0), 128));
    CHECK(f == t.Rotation * FPVector3::Forward());
    CHECK(t.Left() == -t.Right());

    Transform2D t2{V2(0, 0), FP::Zero()};
    CHECK(t2.Up() == FPVector2::Up());
}

TEST_CASE("Quantum::Shape2D creation, area and hash") {
    rr::test::InstallMathDerivedTables();
    const Shape2D circle = Shape2D::CreateCircle(F(2), V2(1, 0));
    CHECK(circle.Type() == Shape2DType::Circle);
    CHECK(circle.BroadRadius == F(2));
    CHECK(circle.LocalTransform.Position == V2(1, 0));
    CHECK(Near(circle.GetArea(), FP::Pi() * 4, 4));

    const Shape2D box = Shape2D::CreateBox(V2(3, 4));
    CHECK(box.BroadRadius == F(5));
    CHECK(box.GetArea() == F(48));

    const Shape2D edge = Shape2D::CreateEdge(FP::Zero());
    CHECK(edge.Edge.Extent == FP::Epsilon());

    // Same data -> same hash; the hash covers the shape data.
    CHECK(Shape2D::CreateBox(V2(3, 4)).GetHashCode() == box.GetHashCode());
    CHECK(Shape2D::CreateBox(V2(3, 5)).GetHashCode() != box.GetHashCode());
}

TEST_CASE("Quantum::Shape3D creation and volume") {
    rr::test::InstallMathDerivedTables();
    const Shape3D box = Shape3D::CreateBox(V3(1, 2, 3));
    CHECK(box.Type() == Shape3DType::Box);
    CHECK(box.GetVolume() == F(48));
    CHECK(box.LocalTransform.Rotation == FPQuaternion::Identity());
    const Shape3D sphere = Shape3D::CreateSphere(F(3), V3(0, 1, 0));
    CHECK(Near(sphere.GetVolume(), FP::FromRaw(7410000), 4000)); // 4/3 pi 27 = 113.097
    CHECK(sphere.LocalTransform.Position == V3(0, 1, 0));
    const Shape3D mesh = Shape3D::CreateMesh(7);
    CHECK(mesh.Mesh.MeshIndex == 7 && mesh.GetVolume() == FP::Zero());
}

TEST_CASE("Physics2D::NarrowChecks circle pairs") {
    rr::test::InstallMathDerivedTables();
    using namespace Physics2D;
    const Shape2D circle = Shape2D::CreateCircle(F(1));
    auto a = Entry(circle, V2(0, 0));
    auto b = Entry(circle, V2(1, 0));
    CollisionManifold2D m;
    CollisionManifoldInfo2D info(&m, &a, &b);
    CHECK(NarrowChecks::CircleCircle::Check(&info, true));
    CHECK(m.Normal == V2(1, 0));
    CHECK(m.Penetration == F(1));
    CHECK(m.ContactPointsCount() == 1 && m._contactPoints[0] == V2(1, 0));

    b.Narrow.Position = V2(3, 0);
    CHECK(!NarrowChecks::CircleCircle::Check(&info, true));

    // Box (2x2 half-extents 1) and a circle touching its right face.
    const Shape2D box = Shape2D::CreateBox(V2(1, 1));
    auto boxEntry = Entry(box, V2(0, 0));
    auto circleEntry = Entry(circle, FPVector2{F(1) + FP::Half(), F(0)});
    CollisionManifold2D m2;
    CollisionManifoldInfo2D boxInfo(&m2, &circleEntry, &boxEntry); // inverted by CheckShapes
    CHECK(NarrowChecks::CheckShapes(&boxInfo, true));
    CHECK(m2.GetNormalInverted());
    CHECK(Near(m2.Normal, V2(1, 0)));
    CHECK(Near(m2.Penetration, FP::Half()));
    CHECK(Near(m2._contactPoints[0], V2(1, 0)));

    // Circle against an edge lying on the X axis from -2 to 2.
    const Shape2D edge = Shape2D::CreateEdge(F(2));
    auto edgeEntry = Entry(edge, V2(0, 0));
    auto above = Entry(circle, FPVector2{F(1), FP::Half()});
    CollisionManifold2D m3;
    CollisionManifoldInfo2D edgeInfo(&m3, &above, &edgeEntry);
    CHECK(NarrowChecks::CircleEdge::Check(&edgeInfo, true));
    CHECK(Near(m3.Normal, V2(0, -1)));
    CHECK(Near(m3.Penetration, FP::Half()));
    CHECK(Near(m3._contactPoints[0], V2(1, 0)));
}

TEST_CASE("Physics2D::NarrowChecks::BoxEdge") {
    rr::test::InstallMathDerivedTables();
    using namespace Physics2D;
    const Shape2D box = Shape2D::CreateBox(V2(1, 1));
    const Shape2D edge = Shape2D::CreateEdge(F(3));
    auto boxEntry = Entry(box, V2(0, 0));
    // Horizontal edge through the box's upper half, wider than the box.
    auto edgeEntry = Entry(edge, FPVector2{F(0), FP::Half()});
    CollisionManifold2D m;
    CollisionManifoldInfo2D info(&m, &boxEntry, &edgeEntry);
    CHECK(NarrowChecks::BoxEdge::Check(&info, true));
    CHECK(m.ContactPointsCount() == 2);
    CHECK(Near(m.Normal, V2(0, 1), 64));
    CHECK(Near(m.Penetration, FP::Half(), 64));

    edgeEntry.Narrow.Position = V2(0, 3);
    CollisionManifold2D miss;
    CollisionManifoldInfo2D missInfo(&miss, &boxEntry, &edgeEntry);
    CHECK(!NarrowChecks::BoxEdge::Check(&missInfo, true));
}

TEST_CASE("Physics2D::NarrowChecks::LineBoxLocalSpace") {
    FPVector2 contact, normal;
    FP t;
    CHECK(Physics2D::NarrowChecks::LineBoxLocalSpace(V2(-4, 0), V2(4, 0), V2(-1, -1), V2(1, 1),
                                                     contact, normal, t));
    CHECK(contact == V2(-1, 0) && normal == FPVector2::Left());
    CHECK(t == FP::FromRaw(24576)); // 3/8
    CHECK(!Physics2D::NarrowChecks::LineBoxLocalSpace(V2(-4, 3), V2(4, 3), V2(-1, -1), V2(1, 1),
                                                      contact, normal, t));
}

TEST_CASE("Physics3D sphere pair and manifold helpers") {
    rr::test::InstallMathDerivedTables();
    using namespace Physics3D;
    const Shape3D sphere = Shape3D::CreateSphere(F(1));
    NarrowEntry3D a, b;
    a.Shape = b.Shape = &sphere;
    a.Narrow.Position = V3(0, 0, 0);
    b.Narrow.Position = V3(0, 1, 0);
    CollisionManifold3D m;
    CollisionManifoldInfo3D info{&m, &a, &b, nullptr};
    CHECK(NarrowChecks::SphereSphere::Check(&info, true));
    CHECK(m.Normal == V3(0, 1, 0) && m.Penetration == F(1));
    CollisionManifold3D::AddContactPoint(&m, V3(0, 3, 0));
    CHECK(CollisionManifold3D::GetAverageContactPoint(&m) == V3(0, 2, 0));

    const FPVector3RawInt packed = FPVector3RawInt::FromFPVector3(V3(-2, 5, 0));
    CHECK(static_cast<FPVector3>(packed) == V3(-2, 5, 0));
}

TEST_CASE("Physics hit collections sort nearest first") {
    Physics2D::Hit hits[4];
    const int xs[4] = {5, -1, 3, 2};
    for (int i = 0; i < 4; ++i) {
        hits[i].Point = V2(xs[i], 0);
    }
    Physics2D::HitCollection collection;
    collection._hits = hits;
    collection._count = 4;
    collection.Sort(V2(0, 0));
    CHECK(hits[0].Point == V2(-1, 0) && hits[1].Point == V2(2, 0) && hits[2].Point == V2(3, 0) &&
          hits[3].Point == V2(5, 0));

    Physics3D::Hit3D hits3[3];
    hits3[0].CastDistanceNormalized = FP::Half();
    hits3[1].CastDistanceNormalized = FP::_0_25();
    hits3[2].CastDistanceNormalized = FP::_0_75();
    Physics3D::HitCollection3D c3;
    c3._hits3D = hits3;
    c3._count = 3;
    c3.SortCastDistance();
    CHECK(c3[0].CastDistanceNormalized == FP::_0_25() && c3[2].CastDistanceNormalized == FP::_0_75());
    CHECK(c3[0].IsDynamic() == false);
}
