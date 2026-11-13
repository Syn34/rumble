// Tests for the 3D box narrow phase: NarrowData3D box caches, box-box and box-triangle.
// Expected values are worked out by hand.
#include "test_framework.h"
#include "math_tables.h"

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
bool Near(const FPVector3& a, const FPVector3& b, int tolerance = 16) {
    return Near(a.X, b.X, tolerance) && Near(a.Y, b.Y, tolerance) && Near(a.Z, b.Z, tolerance);
}

NarrowEntry3D BoxEntry(const Shape3D& shape, const FPVector3& position,
                       const FPQuaternion& rotation = FPQuaternion::Identity()) {
    NarrowEntry3D entry;
    entry.Shape = &shape;
    NarrowData3D::Init(&entry.Narrow, &entry, position, rotation);
    entry.TransformPosition = position;
    return entry;
}
} // namespace

TEST_CASE("Physics3D NarrowData3D box caches") {
    rr::test::InstallMathDerivedTables();
    const Shape3D box = Shape3D::CreateBox(V3(1, 2, 3));
    const NarrowEntry3D entry = BoxEntry(box, V3(5, 0, 0));
    const NarrowData3D& n = entry.Narrow;
    CHECK(n.RotatedRight == FPVector3::Right() && n.RotatedUp == FPVector3::Up());
    CHECK(n.RotatedForward == FPVector3::Forward());
    CHECK(n.RotationInverse == FPQuaternion::Identity());
    CHECK(n.BoxEdges.Edges[0].start == V3(-1, 2, 3) && n.BoxEdges.Edges[0].end == V3(1, 2, 3));
    CHECK(n.BoxEdges.Edges[11].start == V3(-1, -2, -3) && n.BoxEdges.Edges[11].end == V3(-1, 2, -3));
    CHECK(n.BoxPlanes.Planes[3].normal == V3(0, -1, 0));
    CHECK(n.BoxPlanes.Planes[3].pointOnPlane == V3(0, -2, 0));
    CHECK(n.BoxEdgesRotated.Edges[5].start == n.BoxEdges.Edges[5].start);
    CHECK(n.BoxPlanesRotated.Planes[5].normal == V3(0, 0, -1));
    CHECK(n.BoxPlanesRotated.Planes[5].pointOnPlane == V3(0, 0, -3));

    // A quarter turn about Y: +X maps to -Z (left-handed), the inverse undoes the rotation.
    const FPQuaternion quarter = FPQuaternion::AngleAxis(F(90), FPVector3::Up());
    const NarrowEntry3D turned = BoxEntry(box, V3(0, 0, 0), quarter);
    CHECK(Near(turned.Narrow.RotatedRight, V3(0, 0, -1), 128));
    CHECK(Near(turned.Narrow.RotatedForward, V3(1, 0, 0), 128));
    CHECK(Near(turned.Narrow.RotationInverse * turned.Narrow.RotatedRight, FPVector3::Right(), 16));
    CHECK(Near(turned.Narrow.BoxPlanesRotated.Planes[0].pointOnPlane, V3(0, 0, -1), 128));
}

TEST_CASE("Physics3D NarrowData3D InitFromTransform applies the local offset") {
    rr::test::InstallMathDerivedTables();
    const Shape3D box = Shape3D::CreateBox(V3(1, 1, 1), V3(0, 1, 0));
    NarrowEntry3D entry;
    entry.Shape = &box;
    FPVector3 position = V3(2, 0, 0);
    FPQuaternion rotation = FPQuaternion::Identity();
    bool useCachedRot = false;
    NarrowData3D::InitFromTransform(&entry.Narrow, &entry, position, rotation, useCachedRot, {});
    CHECK(useCachedRot);
    CHECK(entry.Narrow.Position == V3(2, 1, 0));

    // With a time step the body's velocity moves the pose first.
    NarrowBody3D body{V3(4, 0, 0), FPVector3::Zero()};
    entry.Body = &body;
    position = V3(2, 0, 0);
    NarrowData3D::InitFromTransform(&entry.Narrow, &entry, position, rotation, useCachedRot, Q(1, 2));
    CHECK(entry.Narrow.Position == V3(4, 1, 0));
}

TEST_CASE("Physics3D BoxBox stacked boxes") {
    rr::test::InstallMathDerivedTables();
    const Shape3D unit = Shape3D::CreateBox(V3(1, 1, 1));
    const NarrowEntry3D a = BoxEntry(unit, V3(0, 0, 0));
    const NarrowEntry3D b = BoxEntry(unit, V3(F(0), Q(3, 2), F(0)));

    CollisionManifold3D m{};
    CollisionManifoldInfo3D info{&m, &a, &b, nullptr};
    CHECK(NarrowChecks::CheckShapes(&info, true));
    CHECK(m.Normal == V3(0, 1, 0));
    // 0.5 plus the epsilon added to each of the three |cos| terms.
    CHECK(m.Penetration == Q(1, 2) + FP::FromRaw(3 * 65));
    CHECK(m._contactPointsCount == 4);
    // B's bottom corners inside A, found through A's +X and -X faces.
    CHECK(m._contactPoints[0] == V3(F(1), Q(1, 2), F(1)));
    CHECK(m._contactPoints[1] == V3(F(1), Q(1, 2), F(-1)));
    CHECK(m._contactPoints[2] == V3(F(-1), Q(1, 2), F(1)));
    CHECK(m._contactPoints[3] == V3(F(-1), Q(1, 2), F(-1)));

    // Without details: stops at the first contact.
    CollisionManifold3D quick{};
    CollisionManifoldInfo3D quickInfo{&quick, &a, &b, nullptr};
    CHECK(NarrowChecks::BoxBox::Check(&quickInfo, false));
    CHECK(quick._contactPointsCount == 1);

    const NarrowEntry3D apart = BoxEntry(unit, V3(F(0), Q(5, 2), F(0)));
    CollisionManifold3D none{};
    CollisionManifoldInfo3D noneInfo{&none, &a, &apart, nullptr};
    CHECK(!NarrowChecks::BoxBox::Check(&noneInfo, true));
    CHECK(none._contactPointsCount == 0);
}

TEST_CASE("Physics3D BoxBox containment and rotated boxes") {
    rr::test::InstallMathDerivedTables();
    const Shape3D big = Shape3D::CreateBox(V3(2, 2, 2));
    const Shape3D small = Shape3D::CreateBox(V3(Q(1, 2), Q(1, 2), Q(1, 2)));
    const NarrowEntry3D outer = BoxEntry(big, V3(0, 0, 0));
    const NarrowEntry3D inner = BoxEntry(small, V3(Q(1, 10), F(0), F(0)));
    CollisionManifold3D m{};
    CollisionManifoldInfo3D info{&m, &outer, &inner, nullptr};
    CHECK(NarrowChecks::BoxBox::Check(&info, true));
    CHECK(m._contactPointsCount == 1);
    CHECK(m._contactPoints[0] == V3(Q(1, 10), F(0), F(0)));
    CHECK(m.Normal == V3(1, 0, 0));

    // A box turned 45 degrees about Y under an axis-aligned one: separated along Y only.
    const Shape3D unit = Shape3D::CreateBox(V3(1, 1, 1));
    const NarrowEntry3D turned =
        BoxEntry(unit, V3(0, 0, 0), FPQuaternion::AngleAxis(F(45), FPVector3::Up()));
    const NarrowEntry3D above = BoxEntry(unit, V3(F(0), Q(9, 5), F(0)));
    CollisionManifold3D r{};
    CollisionManifoldInfo3D rInfo{&r, &turned, &above, nullptr};
    CHECK(NarrowChecks::BoxBox::Check(&rInfo, true));
    CHECK(Near(r.Normal, V3(0, 1, 0), 8));
    // The edge axis A.right x B.right (length sin 45) wins: edge-axis separations are not
    // normalised, so the reported depth is 0.2 * 0.707 plus the epsilon terms.
    CHECK(r.Penetration > Q(14, 100) && r.Penetration < Q(15, 100));
    CHECK(r._contactPointsCount >= 1);
}

TEST_CASE("Physics3D BoxTriangle against a large floor triangle") {
    rr::test::InstallMathDerivedTables();
    // Front face downwards (normal -Y), like the sphere-triangle tests.
    TriangleCCW tri;
    tri.A = V3(-5, 0, -5);
    tri.B = V3(-5, 0, 5);
    tri.C = V3(5, 0, -5);
    tri.ComputeNormal();
    CHECK(Near(tri.InverseNormal, V3(0, 1, 0), 2));

    const Shape3D unit = Shape3D::CreateBox(V3(1, 1, 1));
    const Shape3D mesh = Shape3D::CreateMesh(0);
    NarrowEntry3D meshEntry;
    meshEntry.Shape = &mesh;
    const NarrowEntry3D box = BoxEntry(unit, V3(F(0), Q(-3, 4), F(0)));

    CollisionManifold3D m{};
    CollisionManifoldInfo3D info{&m, &meshEntry, &box, &tri};
    CHECK(NarrowChecks::CheckShapes(&info, true)); // inverted to (box, mesh)
    CHECK(info.EntryA == &box && m.GetNormalInverted());
    CHECK(m.Penetration == Q(1, 4));
    CHECK(m.Normal == tri.InverseNormal);
    // Triangle edge BC crossing the box's side faces (each crossing found twice).
    CHECK(m._contactPointsCount == 4);
    CHECK(Near(m._contactPoints[0], V3(1, 0, -1), 16));
    CHECK(Near(m._contactPoints[1], V3(-1, 0, 1), 16));
    CHECK(Near(m._contactPoints[2], V3(-1, 0, 1), 16));
    CHECK(Near(m._contactPoints[3], V3(1, 0, -1), 16));

    CollisionManifold3D quick{};
    CollisionManifoldInfo3D quickInfo{&quick, &box, &meshEntry, &tri};
    CHECK(NarrowChecks::BoxTriangle::Check(&quickInfo, false));

    const NarrowEntry3D below = BoxEntry(unit, V3(F(0), Q(-3, 2), F(0)));
    CollisionManifold3D none{};
    CollisionManifoldInfo3D noneInfo{&none, &below, &meshEntry, &tri};
    CHECK(!NarrowChecks::BoxTriangle::Check(&noneInfo, true));
}

TEST_CASE("Physics3D BoxTriangle helpers") {
    rr::test::InstallMathDerivedTables();
    const CustomVertexTriangle tri{V3(0, 0, 0), V3(0, 0, 2), V3(2, 0, 0)};
    CHECK(NarrowChecks::BoxTriangle::PointInTriangle(V3(F(0), F(0), F(0)), tri));
    CHECK(NarrowChecks::BoxTriangle::PointInTriangle(V3(Q(1, 2), F(0), Q(1, 2)), tri));
    CHECK(!NarrowChecks::BoxTriangle::PointInTriangle(V3(2, 0, 2), tri));

    const FP::Raw penetration =
        NarrowChecks::BoxTriangle::PointPenetrationRaw(V3(F(0), Q(1, 2), F(0)), V3(0, 1, 0), V3(1, 1, 1));
    CHECK(penetration == Q(1, 2).RawValue());

    FPVector3 hit;
    const CustomPlane plane(V3(0, 1, 0), V3(0, 1, 0));
    CHECK(NarrowChecks::BoxTriangle::ClipToPlane(plane, CustomLine(V3(0, 0, 0), V3(0, 4, 0)), hit));
    CHECK(hit == V3(0, 1, 0));
    CHECK(!NarrowChecks::BoxTriangle::ClipToPlane(plane, CustomLine(V3(0, 0, 0), V3(4, 0, 0)), hit));
    CHECK(!NarrowChecks::BoxBox::ClipToPlane(plane, CustomLine(V3(0, 2, 0), V3(0, 4, 0)), hit));
}
