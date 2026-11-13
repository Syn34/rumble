#include "math_tables.h"
#include "test_framework.h"

#include "photon_deterministic/fp_math.h"
#include "quantum_core/navigation/navigation.h"
#include "quantum_core/navigation/navmesh_components.h"
#include "quantum_core/transform.h"

#include <cstddef>
#include <vector>

using namespace Quantum;
using Photon::Deterministic::FPMath::Abs;

namespace {

FP F(int v) {
    return FP::FromInt(v);
}
FP Frac(int numerator, int denominator) {
    return FP::FromRaw(static_cast<std::int64_t>(numerator) * FP::RAW_ONE / denominator);
}

// A bump allocator standing in for the frame heap (Ptr = byte offset, 0 = null).
class TestHeap final : public Allocator::IFrameHeap {
  public:
    TestHeap() : memory_(4096, 0) {}
    void* AllocateAndClear(int size) override { return Void(AllocateAndClearPtr(size)); }
    Ptr AllocateAndClearPtr(int size) override {
        const int offset = next_;
        next_ += (size + 15) & ~15;
        std::fill(memory_.begin() + offset, memory_.begin() + offset + size, std::uint8_t{0});
        ++allocations_;
        return Ptr{offset};
    }
    void Free(void*) override { ++frees_; }
    void Free(Ptr) override { ++frees_; }
    Ptr Void(const void* ptr) const override {
        return Ptr{static_cast<int>(static_cast<const std::uint8_t*>(ptr) - memory_.data())};
    }
    void* Void(Ptr p) const override {
        return const_cast<std::uint8_t*>(memory_.data()) + p.Offset;
    }

    int allocations_ = 0;
    int frees_ = 0;

  private:
    std::vector<std::uint8_t> memory_;
    int next_ = 16;
};

} // namespace

TEST_CASE("Navigation component layouts match the original") {
    CHECK(offsetof(NavMeshSteeringAgent, _velocity) == 0x18);
    CHECK(offsetof(NavMeshAvoidanceAgent, _avoidanceMask) == 4);
    CHECK(offsetof(NavMeshAvoidanceAgent, _desiredDirection) == 0x10);
    CHECK(offsetof(NavMeshAvoidanceObstacle, _velocity) == 0x10);
    CHECK(offsetof(NavMeshPathfinder, RegionMask) == 0x28);
    CHECK(offsetof(NavMeshPathfinder, _internalTarget) == 0x48);
}

TEST_CASE("Navigation agents take config values unless set explicitly") {
    NavMeshAgentConfig config;
    config.Priority = 7;
    config.AvoidanceLayer = 3;
    NavMeshAvoidanceAgent agent;
    agent.SetPriority(90);
    agent.SetConfig(config, false);
    CHECK(agent._priority == 90 && agent._avoidanceLayer == 3 && agent._avoidanceMask == -1);
    agent.SetConfig(config, true);
    CHECK(agent._priority == 7);

    NavMeshSteeringAgent steering;
    steering.SetMaxSpeed(F(9));
    steering.SetConfig(config, false);
    CHECK(steering.MaxSpeed() == F(9) && steering.Acceleration() == F(20));
    steering.SetVelocity({F(3), F(4)});
    CHECK(steering.CurrentSpeed() == F(5));
    steering.Clear();
    CHECK(steering.Velocity() == FPVector2::Zero());

    NavMeshAvoidanceObstacle obstacle;
    obstacle.SetIsActive(false);
    CHECK(obstacle._disabled == 1);
}

TEST_CASE("NavMeshPathfinder waypoint buffer in the frame heap") {
    TestHeap heap;
    NavMeshPathfinder pathfinder;
    CHECK(pathfinder.GetWaypoints(heap).Points == nullptr);
    CHECK(pathfinder.SetWaypointsCapacity(heap, 4));
    CHECK(!pathfinder.SetWaypointsCapacity(heap, 4));
    pathfinder.AddWaypoint(heap, {F(1), F(0), F(2)},
                           static_cast<std::uint8_t>(WaypointFlag::LinkStart), -1);
    pathfinder.AddWaypoint(heap, {F(3), F(0), F(4)},
                           static_cast<std::uint8_t>(WaypointFlag::LinkEnd), 5);
    CHECK(pathfinder.WaypointCount() == 2);
    CHECK(pathfinder.GetWaypoint(heap, 1) == FPVector3(F(3), F(0), F(4)));
    CHECK(pathfinder.GetWaypointLink(heap, 1) == 5 && pathfinder.GetWaypointLink(heap, 2) == -1);
    CHECK(pathfinder.GetWaypointFlags(heap, 0) ==
          static_cast<std::uint8_t>(WaypointFlag::LinkStart));
    pathfinder._active = 1;
    pathfinder._waypointIndex = 1;
    CHECK(pathfinder.IsOnLink(heap));
    pathfinder.ResetWaypoints();
    CHECK(pathfinder.WaypointCount() == 0 && !pathfinder.IsOnLink(heap));
    CHECK(pathfinder.SetWaypointsCapacity(heap, 0));
    CHECK(heap.frees_ == 1 && pathfinder._waypoints == Ptr::Null());

    NavMeshSteeringAgent steering;
    steering.SetVelocity({F(1), F(1)});
    pathfinder.Clear({F(7), F(1), F(8)}, &steering);
    CHECK(pathfinder._inputTarget == FPVector3(F(7), F(1), F(8)) && !pathfinder.IsActive());
    CHECK(steering.Velocity() == FPVector2::Zero());
}

TEST_CASE("Navigation helpers: transforms and desired velocity") {
    rr::test::InstallMathDerivedTables();
    Transform2D transform;
    Transform2DVertical vertical;
    Navigation::ApplyToTransform({F(1), F(2), F(3)}, transform, vertical);
    CHECK(transform.Position == FPVector2(F(1), F(3)) && vertical.Position == F(2));
    CHECK(Navigation::ToVector3({F(5), F(6)}, vertical) == FPVector3(F(5), F(2), F(6)));

    NavMeshSteeringAgent steering;
    steering.SetMaxSpeed(F(2));
    Navigation::Entry entry;
    entry.SteeringAgent = &steering;
    entry.Position = {F(0), F(0), F(0)};
    entry.CurrentWaypoint = {F(3), F(0), F(4)};
    Navigation::UpdateDesiredVelocity(entry);
    CHECK(entry.DesiredSpeed == F(2));
    CHECK(Abs(entry.DesiredDirection.X - Frac(3, 5)) <= FP::FromRaw(2));
    CHECK(Abs(entry.DesiredVelocity.Y - Frac(8, 5)) <= FP::FromRaw(4));
}

TEST_CASE("Navigation quicksort orders candidates by correction") {
    std::vector<Navigation::VelocityCandidate> candidates(6);
    const int costs[] = {0, 5, 2, 9, 2, 1};
    for (int i = 0; i < 6; ++i) {
        candidates[static_cast<std::size_t>(i)].Correction = F(costs[i]);
        candidates[static_cast<std::size_t>(i)].Index1 = i;
    }
    Navigation::Quicksort(candidates, 1, 5);
    CHECK(candidates[0].Index1 == 0);
    for (std::size_t i = 2; i < 6; ++i) {
        CHECK(candidates[i - 1].Correction <= candidates[i].Correction);
    }
}

TEST_CASE("Navigation HRVO between two approaching agents") {
    rr::test::InstallMathDerivedTables();
    NavMeshSteeringAgent steeringA;
    NavMeshSteeringAgent steeringB;
    steeringA.SetVelocity({F(1), F(0)});
    steeringB.SetVelocity({F(-1), F(0)});
    NavMeshAvoidanceAgent agentA;
    NavMeshAvoidanceAgent agentB;
    agentA._avoidanceMask = -1;
    agentB._avoidanceMask = -1;

    Navigation::Entry a;
    a.Position = {F(0), F(0), F(0)};
    a.CurrentWaypoint = {F(10), F(0), F(0)};
    a.SteeringAgent = &steeringA;
    a.AvoidanceAgent = &agentA;
    a.DesiredSpeed = F(1);
    Navigation::Entry b = a;
    b.Position = {F(4), F(0), F(0)};
    b.CurrentWaypoint = {F(-10), F(0), F(0)};
    b.SteeringAgent = &steeringB;
    b.AvoidanceAgent = &agentB;

    Navigation::VelocityObstacle voA;
    Navigation::VelocityObstacle voB;
    Navigation::ComputeHRVO(a, b, F(1).RawValue(), voA, voB, {F(1), F(0)}, {F(-1), F(0)},
                            F(1).RawValue(), Frac(11, 10).RawValue());
    CHECK(voA.IsValid && voB.IsValid);
    // The cone opens around +X for a (towards b) and around -X for b.
    CHECK(voA.Left.X > FP::Zero() && voA.Right.X > FP::Zero());
    CHECK(voA.Left.Y > FP::Zero() && voA.Right.Y < FP::Zero());
    CHECK(voB.Left.X < FP::Zero() && voB.Right.X < FP::Zero());

    // Too far apart vertically: nothing.
    Navigation::VelocityObstacle untouched;
    b.Position.Y = F(5);
    Navigation::ComputeHRVO(a, b, F(1).RawValue(), untouched, untouched, {F(1), F(0)},
                            {F(-1), F(0)}, F(1).RawValue(), Frac(11, 10).RawValue());
    CHECK(!untouched.IsValid);

    // Avoidance picks a velocity outside a's obstacle.
    Navigation::Config config;
    Navigation navigation(config, 1);
    NavMeshAgentConfig agentConfig;
    a.VoStart = &voA;
    a.DesiredVelocity = {F(1), F(0)};
    FPVector2 newVelocity;
    CHECK(Navigation::CalculateAgentAvoidance(navigation.ThreadLocalAt(0), a, agentConfig,
                                              Frac(1, 60), a.DesiredVelocity, a.DesiredSpeed,
                                              newVelocity));
    CHECK(newVelocity != a.DesiredVelocity);
    CHECK(newVelocity.Y != FP::Zero());
    FP speed;
    const FPVector2 direction = Navigation::RunAvoidanceOnAgent(navigation.ThreadLocalAt(0), a,
                                                                agentConfig, Frac(1, 60), speed);
    CHECK(speed > FP::Zero() && speed <= F(1));
    CHECK(Abs(direction.SqrMagnitude() - FP::One()) <= FP::FromRaw(64));
    // No obstacles: no avoidance.
    a.VoStart = nullptr;
    CHECK(!Navigation::CalculateAgentAvoidance(navigation.ThreadLocalAt(0), a, agentConfig,
                                               Frac(1, 60), a.DesiredVelocity, a.DesiredSpeed,
                                               newVelocity));
}

#include "quantum_core/path_utils.h"

TEST_CASE("PathUtils asset path helpers") {
    CHECK(PathUtils::MakeSane("\\Assets\\\\Maps/level.asset/") == "Assets/Maps/level.asset");
    CHECK(PathUtils::GetPathWithoutExtension("a/b.c/d.asset") == "a/b.c/d");
    CHECK(PathUtils::GetPathWithoutExtension("noext") == "noext");
    std::string relative;
    CHECK(PathUtils::MakeRelativeToFolder("Assets/Resources/DB/x.asset", "resources", relative));
    CHECK(relative == "DB/x.asset");
    CHECK(PathUtils::MakeRelativeToFolder("Assets/Resources", "Resources", relative) &&
          relative.empty());
    CHECK(!PathUtils::MakeRelativeToFolder("Assets/Other/x", "Resources", relative));
}
