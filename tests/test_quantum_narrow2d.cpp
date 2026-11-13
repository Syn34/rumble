// Tests for the 2D narrow phase: polygon, edge and box checks, GJK distance, shape cast and
// time of impact. Expected values are worked out by hand from the algorithms.
#include "test_framework.h"
#include "math_tables.h"

#include "quantum_core/gjk_settings.h"
#include "quantum_core/physics2d/gjk2d.h"
#include "quantum_core/physics2d/narrow_checks.h"
#include "quantum_core/polygon_collider.h"
#include "quantum_core/shape2d.h"

using namespace Quantum;
using namespace Quantum::Physics2D;
using Photon::Deterministic::FPMath::Abs;

namespace {
FP F(int v) { return FP::FromInt(v); }
FP Q(int numerator, int denominator) { return FP::FromRaw(FP::Raw{65536} * numerator / denominator); }
FPVector2 V2(FP x, FP y) { return {x, y}; }
FPVector2 V2(int x, int y) { return {F(x), F(y)}; }
bool Near(FP a, FP b, int tolerance = 16) { return Abs(a - b) <= FP::FromRaw(tolerance); }
bool Near(FPVector2 a, FPVector2 b, int tolerance = 16) {
    return Near(a.X, b.X, tolerance) && Near(a.Y, b.Y, tolerance);
}

// Unrotated entry (sin 0, cos 1) unless a quarter turn is asked for.
NarrowEntry2D Entry(const Shape2D& shape, FPVector2 position, bool quarterTurn = false) {
    NarrowEntry2D entry;
    entry.Shape = &shape;
    entry.Narrow.Position = position;
    entry.TransformPosition = position;
    entry.Narrow.RotSin = quarterTurn ? FP::One() : FP::Zero();
    entry.Narrow.RotCos = quarterTurn ? FP::Zero() : FP::One();
    return entry;
}

// Axis-aligned square polygon of half-size 1 (counter-clockwise from the lower left).
PolygonCollider UnitSquare() {
    PolygonCollider collider;
    collider.Vertices = {V2(-1, -1), V2(1, -1), V2(1, 1), V2(-1, 1)};
    collider.Normals = {V2(0, -1), V2(1, 0), V2(0, 1), V2(-1, 0)};
    collider.Radius = Photon::Deterministic::FPMath::Sqrt(F(2));
    return collider;
}

NarrowEntry2D PolygonEntry(const Shape2D& shape, const PolygonCollider& collider, FPVector2 position) {
    NarrowEntry2D entry = Entry(shape, position);
    entry.Polygon.Init(collider);
    return entry;
}
} // namespace

TEST_CASE("Physics2D OOBB corners, containment and nearest face") {
    NarrowChecks::OOBB box;
    box.Set(V2(2, 1));
    CHECK(box.UL == V2(-2, 1) && box.LL == V2(-2, -1) && box.UR == V2(2, 1) && box.LR == V2(2, -1));
    CHECK(box.Min() == V2(-2, -1) && box.Max() == V2(2, 1));
    CHECK(box.InsideLocal(V2(2, 1)) && !box.InsideLocal(V2(0, 2)));
    CHECK(box.NormalFromClosestFace(V2(0, 0)) == FPVector2::Up()); // tie: top wins
    CHECK(box.NormalFromClosestFace(V2(Q(19, 10), F(0))) == FPVector2::Right());
    CHECK(box.NormalFromClosestFace(V2(F(0), Q(-9, 10))) == FPVector2::Down());
}

TEST_CASE("Physics2D PolygonPolygon.Clip keeps the inside part of a segment") {
    FPVector2 face[2] = {V2(0, 0), V2(4, 0)};
    // Half-plane x <= 1: one end inside plus the crossing point.
    CHECK(NarrowChecks::PolygonPolygon::Clip(V2(1, 0), F(1), face) == 2);
    CHECK(face[0] == V2(0, 0) && face[1] == V2(1, 0));
    FPVector2 outside[2] = {V2(2, 0), V2(3, 0)};
    CHECK(NarrowChecks::PolygonPolygon::Clip(V2(1, 0), F(1), outside) == 0);
}

TEST_CASE("Physics2D CirclePolygon face, vertex and inside regions") {
    rr::test::InstallMathDerivedTables();
    const PolygonCollider square = UnitSquare();
    const Shape2D polygonShape = Shape2D::CreatePolygon(square);
    const NarrowEntry2D polygon = PolygonEntry(polygonShape, square, V2(0, 0));
    NarrowTls2D tls;

    // Face region: circle of radius 1.5 at (2, 0), the right face is 1 away.
    const Shape2D bigCircle = Shape2D::CreateCircle(Q(3, 2));
    const NarrowEntry2D right = Entry(bigCircle, V2(2, 0));
    CollisionManifold2D m{};
    CollisionManifoldInfo2D info(&m, &right, &polygon);
    CHECK(NarrowChecks::CirclePolygon::Check(tls, &info, true));
    CHECK(m.Normal == V2(-1, 0));
    CHECK(m.Penetration == Q(1, 2));
    CHECK(m._contactPointsCount == 1 && m._contactPoints[0] == V2(1, 0));

    // Vertex region at (2, 2): the contact is the corner (1, 1); the penetration still uses
    // the face separation (1.5 - 1), as in the original.
    const NarrowEntry2D corner = Entry(bigCircle, V2(2, 2));
    CollisionManifold2D m2{};
    CollisionManifoldInfo2D info2(&m2, &corner, &polygon);
    CHECK(NarrowChecks::CirclePolygon::Check(tls, &info2, true));
    CHECK(m2._contactPoints[0] == V2(1, 1));
    CHECK(Near(m2.Normal, V2(Q(-7071, 10000), Q(-7071, 10000)), 64));
    CHECK(m2.Penetration == Q(1, 2));

    // Box shape as polygon, circle above it; centre inside -> push out through the top.
    const Shape2D box = Shape2D::CreateBox(V2(1, 1));
    const NarrowEntry2D boxEntry = Entry(box, V2(0, 0));
    const Shape2D unitCircle = Shape2D::CreateCircle(F(1));
    const NarrowEntry2D above = Entry(unitCircle, V2(F(0), Q(3, 2)));
    CollisionManifold2D m3{};
    CollisionManifoldInfo2D info3(&m3, &above, &boxEntry);
    CHECK(NarrowChecks::CirclePolygon::Check(tls, &info3, true));
    CHECK(m3.Normal == V2(0, -1) && m3.Penetration == Q(1, 2) && m3._contactPoints[0] == V2(0, 1));

    const NarrowEntry2D inside = Entry(unitCircle, V2(F(0), Q(1, 2)));
    CollisionManifold2D m4{};
    CollisionManifoldInfo2D info4(&m4, &inside, &boxEntry);
    CHECK(NarrowChecks::CirclePolygon::Check(tls, &info4, true));
    CHECK(m4.Normal == V2(0, -1) && m4.Penetration == F(1));
    CHECK(m4._contactPoints[0] == V2(F(0), Q(-1, 2)));

    const NarrowEntry2D far = Entry(unitCircle, V2(3, 0));
    CollisionManifold2D m5{};
    CollisionManifoldInfo2D info5(&m5, &far, &boxEntry);
    CHECK(!NarrowChecks::CirclePolygon::Check(tls, &info5, true));
}

TEST_CASE("Physics2D PolygonPolygon box against polygon") {
    rr::test::InstallMathDerivedTables();
    const PolygonCollider square = UnitSquare();
    const Shape2D polygonShape = Shape2D::CreatePolygon(square);
    const Shape2D box = Shape2D::CreateBox(V2(1, 1));
    const NarrowEntry2D boxEntry = Entry(box, V2(0, 0));
    const NarrowEntry2D polygon = PolygonEntry(polygonShape, square, V2(Q(3, 2), F(0)));

    // Overlap of 0.5 along x. Both SAT depths are -0.5, so the bias picks B's left face as
    // reference; the clipped incident face is A's right side.
    CollisionManifold2D m{};
    CollisionManifoldInfo2D info(&m, &boxEntry, &polygon);
    CHECK(NarrowChecks::CheckShapes(&info, true));
    CHECK(m.Normal == V2(1, 0));
    CHECK(m.Penetration == Q(1, 2));
    CHECK(m._contactPointsCount == 2);
    CHECK(m._contactPoints[0] == V2(1, -1) && m._contactPoints[1] == V2(1, 1));

    std::int32_t face = -1;
    NarrowTls2D tls;
    NarrowChecks::Polygon a;
    a.Set(tls, boxEntry);
    NarrowChecks::Polygon b;
    b.Set(tls, polygon);
    CHECK(NarrowChecks::PolygonPolygon::FindAxisLeastPenetration(face, a, b) == Q(-1, 2));
    CHECK(face == 2); // the box's right face

    const NarrowEntry2D apart = PolygonEntry(polygonShape, square, V2(3, 0));
    CollisionManifold2D m2{};
    CollisionManifoldInfo2D info2(&m2, &boxEntry, &apart);
    CHECK(!NarrowChecks::CheckShapes(&info2, true));
}

TEST_CASE("Physics2D EdgeEdge crossing edges") {
    const Shape2D longEdge = Shape2D::CreateEdge(F(2));
    const Shape2D shortEdge = Shape2D::CreateEdge(F(1));
    const NarrowEntry2D a = Entry(longEdge, V2(0, 0));
    // Vertical edge from (0.5, -0.5) to (0.5, 1.5).
    const NarrowEntry2D b = Entry(shortEdge, V2(Q(1, 2), Q(1, 2)), true);
    CollisionManifold2D m{};
    CollisionManifoldInfo2D info(&m, &a, &b);
    CHECK(NarrowChecks::CheckShapes(&info, true));
    CHECK(m.Penetration == Q(1, 2));
    CHECK(m.Normal == V2(0, 1));
    CHECK(m._contactPointsCount == 1 && m._contactPoints[0] == V2(Q(1, 2), F(0)));

    // Entirely above A: no crossing.
    const NarrowEntry2D high = Entry(shortEdge, V2(Q(1, 2), F(2)), true);
    CollisionManifold2D m2{};
    CollisionManifoldInfo2D info2(&m2, &a, &high);
    CHECK(!NarrowChecks::CheckShapes(&info2, true));
}

TEST_CASE("Physics2D PolygonEdge edge crossing a box") {
    rr::test::InstallMathDerivedTables();
    const Shape2D box = Shape2D::CreateBox(V2(1, 1));
    const Shape2D edge = Shape2D::CreateEdge(F(2));
    const NarrowEntry2D boxEntry = Entry(box, V2(0, 0));
    // Horizontal edge 0.25 below the box's top, crossing both sides.
    const NarrowEntry2D edgeEntry = Entry(edge, V2(F(0), Q(3, 4)));
    CollisionManifold2D m{};
    CollisionManifoldInfo2D info(&m, &boxEntry, &edgeEntry);
    NarrowTls2D tls;
    CHECK(NarrowChecks::PolygonEdge::Check(tls, &info, true));
    CHECK(m._contactPointsCount == 2);
    CHECK(m._contactPoints[0] == V2(F(-1), Q(3, 4)) && m._contactPoints[1] == V2(F(1), Q(3, 4)));
    CHECK(Near(m.Penetration, Q(1, 4)));
    CHECK(Near(m.Normal, V2(0, 1)));

    const NarrowEntry2D below = Entry(edge, V2(F(0), F(-3)));
    CollisionManifold2D m2{};
    CollisionManifoldInfo2D info2(&m2, &boxEntry, &below);
    CHECK(!NarrowChecks::PolygonEdge::Check(tls, &info2, true));
}

TEST_CASE("Physics2D BoxBox corners and axis") {
    rr::test::InstallMathDerivedTables();
    const Shape2D box = Shape2D::CreateBox(V2(1, 1));
    const NarrowEntry2D a = Entry(box, V2(0, 0));
    const NarrowEntry2D b = Entry(box, V2(Q(3, 2), Q(1, 2)));
    CollisionManifold2D m{};
    CollisionManifoldInfo2D info(&m, &a, &b);
    CHECK(NarrowChecks::CheckShapes(&info, true));
    CHECK(m.Penetration == Q(1, 2));
    CHECK(m.Normal == V2(1, 0));
    CHECK(m._contactPointsCount == 2);
    CHECK(m._contactPoints[0] == V2(Q(1, 2), Q(-1, 2))); // B's lower-left corner
    CHECK(m._contactPoints[1] == V2(1, 1));              // A's upper-right corner

    const NarrowEntry2D apart = Entry(box, V2(F(3), F(0)));
    CollisionManifold2D m2{};
    CollisionManifoldInfo2D info2(&m2, &a, &apart);
    CHECK(!NarrowChecks::CheckShapes(&info2, true));
}

TEST_CASE("Physics2D GJK distance between circles and boxes") {
    rr::test::InstallMathDerivedTables();
    const Shape2D circle = Shape2D::CreateCircle(F(1));
    const NarrowEntry2D a = Entry(circle, V2(0, 0));
    const NarrowEntry2D b = Entry(circle, V2(5, 0));
    SimplexData2D simplex{};
    SimplexDataPointers2D ptrs(&simplex);
    FP distance;
    FPVector2 axis;
    Gjk2D::ComputeDistance(&ptrs, &a, &b, distance, axis, 16, std::nullopt, 0);
    CHECK(distance == F(3));
    CHECK(axis == V2(-1, 0)); // from B towards A

    const Shape2D box = Shape2D::CreateBox(V2(1, 1));
    const NarrowEntry2D boxA = Entry(box, V2(0, 0));
    const NarrowEntry2D boxB = Entry(box, V2(F(4), Q(1, 2)));
    Gjk2D::ComputeDistance(&ptrs, &boxA, &boxB, distance, axis, 16, std::nullopt, 0);
    CHECK(distance == F(2));
    CHECK(axis == V2(-1, 0));
    CHECK(ptrs.UsedCount == 2);

    const NarrowEntry2D overlapping = Entry(box, V2(1, 0));
    Gjk2D::ComputeDistance(&ptrs, &boxA, &overlapping, distance, axis, 16, std::nullopt, 0);
    CHECK(distance == FP::Zero() && axis == FPVector2::Zero());
}

TEST_CASE("Physics2D GJK simplex solvers") {
    SimplexData2D simplex{};
    SimplexDataPointers2D ptrs(&simplex);
    simplex.Vertex0.Point = V2(-2, 1);
    simplex.Vertex1.Point = V2(2, 1);
    simplex.Vertex2.Point = V2(0, -1);
    ptrs.UsedCount = 3;
    FPVector2 closest;
    Gjk2D::ComputeClosestPoint(&ptrs, closest);
    CHECK(ptrs.UsedCount == 3 && closest == FPVector2::Zero()); // origin inside

    SimplexDataPointers2D line(&simplex);
    simplex.Vertex0.Point = V2(-2, 1);
    simplex.Vertex1.Point = V2(2, 1);
    line.UsedCount = 2;
    Gjk2D::ComputeClosestPoint(&line, closest);
    CHECK(closest == V2(0, 1));
    CHECK(simplex.Lambda0 == Q(1, 2) && simplex.Lambda1 == Q(1, 2));
    CHECK(!Gjk2D::IsAffinelyDependent(&line));
    simplex.Vertex1.Point = V2(-2, 1);
    CHECK(Gjk2D::IsAffinelyDependent(&line));
}

TEST_CASE("Physics2D GJK shape cast and settings") {
    rr::test::InstallMathDerivedTables();
    PhysicsQueryConfig config;
    config.ShapeCastMaxIterations = 2; // raised to the minimum of 4
    config.ShapeCastHardTolerance = FP::FromRaw(1);
    config.ShapeCastSoftTolerance = FP::FromRaw(3);
    config.SimplexMinBitShift = -3;
    config.SimplexMaxBitShift = 4;
    const GJKSettings settings(config);
    CHECK(settings.ShapeCastMaxIterations() == 4);
    CHECK(settings.ShapeCastHardTolerance() == FP::FromRaw(6));
    CHECK(settings.ShapeCastSoftTolerance() == FP::FromRaw(6));
    CHECK(settings.SimplexMinBitShift() == 0 && settings.SimplexMaxBitShift() == 4);

    // B (radius 1 at x = 5) moved by (-10, 0) touches A (radius 1 at the origin) after 0.3
    // of the cast. (With the minimal hard tolerance above, the truncated fraction leaves
    // the simplex 8 raw units outside it and the cast gives up, as the original would.)
    config.ShapeCastHardTolerance = Q(1, 1000);
    config.ShapeCastSoftTolerance = Q(1, 100);
    const GJKSettings castSettings(config);
    const Shape2D circle = Shape2D::CreateCircle(F(1));
    const NarrowEntry2D a = Entry(circle, V2(0, 0));
    const NarrowEntry2D b = Entry(circle, V2(5, 0));
    NarrowTls2D tls;
    Hit hit;
    CHECK(!Gjk2D::ShapeCastManifold(tls, &a, &b, settings, V2(-10, 0), true, hit));
    CHECK(Gjk2D::ShapeCastManifold(tls, &a, &b, castSettings, V2(-10, 0), true, hit));
    CHECK(hit.CastDistanceNormalized == FP::FromRaw(19660)); // 3 / 10 truncated
    CHECK(hit.Normal == V2(1, 0));
    // B's moved centre (5 - 10 * 0.29999) minus its radius along the normal.
    CHECK(hit.Point == V2(FP::FromRaw(65544), F(0)));

    // Moving away: no hit.
    CHECK(!Gjk2D::ShapeCastManifold(tls, &a, &b, castSettings, V2(10, 0), true, hit));
}

TEST_CASE("Physics2D time of impact of a fast circle") {
    rr::test::InstallMathDerivedTables();
    const Shape2D circle = Shape2D::CreateCircle(Q(1, 2));
    NarrowBody2D body;
    body.Velocity = V2(10, 0);
    NarrowEntry2D a = Entry(circle, V2(0, 0));
    a.Body = &body;
    a.HasDynamicBody = true;
    const NarrowEntry2D b = Entry(circle, V2(3, 0));

    PhysicsQueryConfig config;
    config.MaxTimeOfImpactIterations = 8;
    config.MaxRootFindingIterations = 20;
    config.CCDDistanceTolerance = Q(1, 100);
    const CCDSettings settings(config);

    CollisionManifold2D m{};
    CollisionManifoldInfo2D info(&m, &a, &b);
    NarrowTls2D tls;
    FP toi;
    // Touches when A's centre reaches x = 2: one secant step lands there.
    CHECK(Gjk2D::ComputeTimeOfImpact(tls, &info, settings, true, F(1), toi) ==
          TimeOfImpactResult::Collision);
    CHECK(toi == FP::FromRaw(13107));
    CHECK(m._m.TimeOfImpact == toi);
    CHECK(m.Normal == V2(1, 0));
    // A's witness (its centre at the impact) moved by the radius along the B-to-A axis.
    CHECK(m._contactPointsCount == 1 && m._contactPoints[0] == V2(FP::FromRaw(98303), F(0)));

    // Too slow to reach B within the step.
    body.Velocity = V2(1, 0);
    CollisionManifold2D m2{};
    CollisionManifoldInfo2D info2(&m2, &a, &b);
    CHECK(Gjk2D::ComputeTimeOfImpact(tls, &info2, settings, true, F(1), toi) ==
          TimeOfImpactResult::Ignore);
    CHECK(toi == F(1));

    // CCD fallback of NarrowChecks.Check.
    body.Velocity = V2(10, 0);
    CollisionManifold2D m3{};
    m3._m.SetUsesCcd(true);
    CollisionManifoldInfo2D info3(&m3, &a, &b);
    CHECK(NarrowChecks::Check(tls, settings, F(1), &info3, false));
}

TEST_CASE("Physics2D LinecastBody against every primitive") {
    rr::test::InstallMathDerivedTables();
    NarrowTls2D tls;
    Hit hit;
    const FPVector2 start = V2(0, 0);
    const FPVector2 end = V2(10, 0);

    const Shape2D circle = Shape2D::CreateCircle(F(1));
    const NarrowEntry2D ball = Entry(circle, V2(5, 0));
    CHECK(NarrowChecks::LinecastBody::Check(tls, AssetGuid{}, &ball, start, end, true, hit));
    CHECK(Near(hit.Point, V2(4, 0), 64) && Near(hit.CastDistanceNormalized, Q(2, 5), 64));
    CHECK(Near(hit.Normal, V2(-1, 0), 64));

    const Shape2D box = Shape2D::CreateBox(V2(1, 1));
    const NarrowEntry2D crate = Entry(box, V2(5, 0));
    CHECK(NarrowChecks::LinecastBody::Check(tls, AssetGuid{}, &crate, start, end, true, hit));
    CHECK(hit.Point == V2(FP::FromRaw(262140), F(0)) && hit.Normal == V2(-1, 0)); // 4 minus the truncation of 0.4
    CHECK(hit.CastDistanceNormalized == FP::FromRaw(26214));

    const PolygonCollider square = UnitSquare();
    const Shape2D polygonShape = Shape2D::CreatePolygon(square);
    NarrowEntry2D polygon = PolygonEntry(polygonShape, square, V2(5, 0));
    polygon.StaticColliderIndex = 2;
    CHECK(NarrowChecks::LinecastBody::Check(tls, AssetGuid{}, &polygon, start, end, true, hit));
    CHECK(hit.CastDistanceNormalized == FP::FromRaw(26214)); // enters the left side at 0.4
    CHECK(hit.Point == V2(FP::FromRaw(262140), F(0)) && hit.Normal == V2(-1, 0));
    CHECK(hit._staticColliderIndex == 2);

    const Shape2D edge = Shape2D::CreateEdge(F(2));
    const NarrowEntry2D wall = Entry(edge, V2(5, 0), true); // vertical, (5, -2) to (5, 2)
    CHECK(NarrowChecks::LinecastBody::Check(tls, AssetGuid{}, &wall, start, end, true, hit));
    CHECK(hit.Point == V2(5, 0) && hit.CastDistanceNormalized == Q(1, 2));
    CHECK(hit.Normal == V2(-1, 0));

    // Starting inside the box misses; passing above everything misses.
    CHECK(!NarrowChecks::LinecastBody::Check(tls, AssetGuid{}, &crate, V2(5, 0), end, true, hit));
    CHECK(!NarrowChecks::LinecastBody::Check(tls, AssetGuid{}, &polygon, V2(0, 3), V2(10, 3), true, hit));
}
