#include "test_framework.h"
#include "math_tables.h"

#include "photon_deterministic/fp_collision.h"

using namespace Photon::Deterministic;

namespace {
FP F(int v) { return FP::FromInt(v); }
FPVector2 V2(int x, int y) { return {F(x), F(y)}; }
FPVector3 V3(int x, int y, int z) { return {F(x), F(y), F(z)}; }
} // namespace

TEST_CASE("FPCollision::ClosestPointOnSegment") {
    CHECK(FPCollision::ClosestPointOnSegment(V2(5, 3), V2(0, 0), V2(10, 0)) == V2(5, 0));
    CHECK(FPCollision::ClosestPointOnSegment(V2(-4, 3), V2(0, 0), V2(10, 0)) == V2(0, 0));
    CHECK(FPCollision::ClosestPointOnSegment(V2(14, 3), V2(0, 0), V2(10, 0)) == V2(10, 0));
    CHECK(FPCollision::ClosestPointOnSegment(V2(1, 1), V2(2, 2), V2(2, 2)) == V2(2, 2));
    CHECK(FPCollision::ClosestPointOnSegment(V3(2, 2, 1), V3(0, 0, 0), V3(0, 8, 0)) == V3(0, 2, 0));
}

TEST_CASE("FPCollision triangle containment") {
    const FPVector2 a = V2(0, 0), b = V2(4, 0), c = V2(0, 4);
    CHECK(FPCollision::TriangleContainsPointInclusive(V2(1, 1), a, b, c));
    CHECK(FPCollision::TriangleContainsPointInclusive(V2(2, 0), a, b, c)); // on an edge
    CHECK(!FPCollision::TriangleContainsPointInclusive(V2(3, 3), a, b, c));
    // Ground-plane versions ignore Y.
    const FPVector3 a3 = V3(0, 5, 0), b3 = V3(4, -2, 0), c3 = V3(0, 9, 4);
    CHECK(FPCollision::TriangleContainsPointExclusive(V3(1, 100, 1), a3, b3, c3));
    CHECK(!FPCollision::TriangleContainsPointExclusive(V3(2, 0, 0), a3, b3, c3));
    CHECK(FPCollision::TriangleContainsPointInclusive(V3(2, 0, 0), a3, b3, c3));
    CHECK(!FPCollision::TriangleContainsPointInclusive(V3(3, 0, 3), a3, b3, c3));
}

TEST_CASE("FPCollision::CircleIntersectsCircle") {
    CHECK(FPCollision::CircleIntersectsCircle(V2(0, 0), F(1), V2(2, 0), F(1))); // touching
    CHECK(!FPCollision::CircleIntersectsCircle(V2(0, 0), F(1), V2(3, 0), F(1)));
}

TEST_CASE("FPCollision::LineIntersectsLine") {
    rr::test::InstallMathDerivedTables();
    CHECK(FPCollision::LineIntersectsLine(V2(0, 0), V2(4, 4), V2(0, 4), V2(4, 0)));
    CHECK(!FPCollision::LineIntersectsLine(V2(0, 0), V2(4, 0), V2(0, 1), V2(4, 1))); // parallel
    CHECK(!FPCollision::LineIntersectsLine(V2(0, 0), V2(1, 1), V2(0, 4), V2(4, 0)));

    FPVector2 point;
    FP distance, t;
    CHECK(FPCollision::LineIntersectsLine(V2(0, 0), V2(4, 0), V2(1, -2), V2(1, 2), point, distance, t));
    CHECK(point == V2(1, 0));
    CHECK(distance == F(1));
    CHECK(t == FP::_0_25());
    CHECK(!FPCollision::LineIntersectsLine(V2(0, 0), V2(4, 0), V2(5, -2), V2(5, 2), point, distance, t));
    CHECK(point == FPVector2::Zero() && distance == 0 && t == 0);
    CHECK(FPCollision::LineIntersectsLine(V2(0, 0), V2(0, 8), V2(-1, 2), V2(1, 2), point));
    CHECK(point == V2(0, 2));
}

TEST_CASE("FPCollision::LineIntersectsCircle") {
    rr::test::InstallMathDerivedTables();
    FPVector2 point;
    FP t;
    CHECK(FPCollision::LineIntersectsCircle(V2(-10, 0), V2(10, 0), V2(0, 0), F(2), point, t, false));
    CHECK(FPMath::Abs(point.X - F(-2)) <= FP::FromRaw(64) && FPMath::Abs(point.Y) <= FP::FromRaw(4));
    CHECK(FPMath::Abs(t - FP::FromRaw(26214)) <= FP::FromRaw(8)); // 0.4
    CHECK(!FPCollision::LineIntersectsCircle(V2(-10, 5), V2(10, 5), V2(0, 0), F(2), false));
    // Starting inside: the exit point is reported unless ignored.
    CHECK(FPCollision::LineIntersectsCircle(V2(0, 0), V2(10, 0), V2(0, 0), F(2), point, false));
    CHECK(FPMath::Abs(point.X - F(2)) <= FP::FromRaw(64));
    CHECK(!FPCollision::LineIntersectsCircle(V2(0, 0), V2(10, 0), V2(0, 0), F(2), true));
}

TEST_CASE("FPCollision::ClosestDistanceToTriangle") {
    const FPVector3 a = V3(0, 0, 0), b = V3(4, 0, 0), c = V3(0, 0, 4);
    FPVector3 closest, bary;
    // Above the face: projects straight down.
    FP d = FPCollision::ClosestDistanceToTriangle(V3(1, 3, 1), a, b, c, closest, bary);
    CHECK(closest == V3(1, 0, 1));
    CHECK(d == F(9));
    CHECK(bary.X + bary.Y + bary.Z == F(1) || FPMath::Abs(bary.X + bary.Y + bary.Z - F(1)) <= FP::FromRaw(2));
    // Vertex regions.
    d = FPCollision::ClosestDistanceToTriangle(V3(-2, 0, -2), a, b, c, closest, bary);
    CHECK(closest == a && bary == V3(1, 0, 0) && d == F(8));
    d = FPCollision::ClosestDistanceToTriangle(V3(7, 0, -1), a, b, c, closest, bary);
    CHECK(closest == b && bary == V3(0, 1, 0));
    d = FPCollision::ClosestDistanceToTriangle(V3(-1, 0, 9), a, b, c, closest, bary);
    CHECK(closest == c && bary == V3(0, 0, 1));
    // Edge AB.
    d = FPCollision::ClosestDistanceToTriangle(V3(2, 0, -3), a, b, c, closest, bary);
    CHECK(closest == V3(2, 0, 0) && d == F(9));
    CHECK(bary == FPVector3(FP::Half(), FP::Half(), FP::Zero()));
    // Edge AC.
    d = FPCollision::ClosestDistanceToTriangle(V3(-3, 0, 1), a, b, c, closest, bary);
    CHECK(closest == V3(0, 0, 1));
    // Edge BC (hypotenuse).
    d = FPCollision::ClosestDistanceToTriangle(V3(3, 0, 3), a, b, c, closest, bary);
    CHECK(closest == V3(2, 0, 2) && d == F(2));
}
