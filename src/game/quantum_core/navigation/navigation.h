// navigation.h - Quantum.Navigation (quantum.core.dll): the navigation system's per-frame
// update, thread-local search state and local avoidance (hybrid reciprocal velocity
// obstacles, "HRVO").
//
// Implemented: Config, ThreadLocal, Entry's accessors, the velocity-obstacle maths
// (ComputeHRVO, candidate generation, sorting, choosing a velocity), desired velocity
// and the small transform helpers. The update loop (Updater, task scheduling, entry
// creation, broadphase, moving entities) is TODO until the frame/ECS and task systems
// exist; the functions it calls take the pieces they use explicitly instead of a frame.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "quantum_core/entity_ref.h"
#include "quantum_core/mailbox_set8.h"
#include "quantum_core/navigation/navmesh_agent_config.h"
#include "quantum_core/navigation/navmesh_components.h"
#include "quantum_core/navigation/navmesh_region_mask.h"
#include "quantum_core/navigation/path_finder.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace Quantum::Core {
class FrameBase;
}

namespace Quantum {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;

class NavMesh;
struct Transform2D;
struct Transform2DVertical;
struct Transform3D;
struct PhysicsBody2D;
struct PhysicsBody3D;
struct CharacterController3D;

class Navigation {
  public:
    static constexpr std::int64_t AngularSpeedInstantRotationThreshold = 13107200; // 200
    static constexpr std::int64_t Epsilon = 65;                                    // FP.EN3
    static constexpr std::int64_t FP_0_25 = 16384;
    static constexpr std::int64_t FP_0_50 = 32768;
    static constexpr std::int64_t FP_0_75 = 49152;

    // Navigation.Config: the simulation config's navigation section.
    struct Config {
        AssetGuid DefaultNavMeshAgent; // AssetRefNavMeshAgentConfig
        bool EnableNavigationCallbacks = false;
        FP HeightThresholdForWaypoints = FP::_1();
        FP HeightThresholdForAvoidance = FP::_1();
        std::int32_t RawPathSize = 256;
        Quantum::RawPathAllocationStrategy RawPathAllocationStrategy =
            Quantum::RawPathAllocationStrategy::Fixed;
        bool EnableAvoidance = true;
        FP AvoidanceRange = FP::_2();
        std::int32_t MaxAvoidanceCandidates = 64;
        FP VelocityObstacleTruncationFactor = FP::FromRaw(72089); // 1.1

        // 0x7d265c4 - the defaults above.
        Config() = default;
    };

    // Navigation.AgentFlags (Entry._flags).
    enum AgentFlags : std::uint8_t {
        PreUpdateSuccessful = 1,
        WaypointReached = 2,
        SearchFailed = 4,
        UpdateThisTick = 8,
        IsAvoidanceInternal = 16,
        ShowDebugAvoidance = 32,
        IsOnLink = 64,
        Teleport = 128,
    };

    // A velocity obstacle: the cone of velocities (apex + left/right edge directions) that
    // would lead into another agent, optionally truncated near the apex. VOs of one agent
    // form a linked list through Next.
    struct VelocityObstacle {
        static constexpr std::int64_t CorrectionBiasCandidates0 = 39321; // 0.6

        bool IsValid = false;
        FPVector2 Apex;
        FPVector2 Left;
        FPVector2 Right;
        FP TruncationLength;
        VelocityObstacle* Next = nullptr;

        // 0x7d2a904
        static FPVector2 ZeroVector() { return FPVector2::Zero(); }
    };

    // A candidate avoidance velocity; Index1/Index2 name the VOs it lies on (-1: none).
    struct VelocityCandidate {
        std::int32_t Index1 = 0;
        std::int32_t Index2 = 0;
        FP Correction; // squared distance from the preferred velocity (sort key)
        FPVector2 Velocity;
    };

    // Per-agent working record of a navigation update. Component pointers are null when
    // the entity lacks that component.
    struct Entry {
        EntityRef Entity;
        FPVector3 Position;
        FP Rotation;
        std::int32_t EntryIndex = 0;
        NavMeshPathfinder* PathfinderAgent = nullptr;
        NavMeshAvoidanceAgent* AvoidanceAgent = nullptr;
        NavMeshAvoidanceObstacle* AvoidanceObstacle = nullptr;
        NavMeshSteeringAgent* SteeringAgent = nullptr;
        Quantum::Transform2D* Transform2D = nullptr;
        Quantum::Transform3D* Transform3D = nullptr;
        Quantum::PhysicsBody2D* PhysicsBody2D = nullptr;
        Quantum::PhysicsBody3D* PhysicsBody3D = nullptr;
        CharacterController3D* KCC3D = nullptr;
        std::uint8_t _flags = 0;
        NavMeshRegionMask RegionMask;
        FPVector3 CurrentWaypoint;
        std::int32_t AvoidanceRadius = 0; // typed int in the original
        VelocityObstacle* VoStart = nullptr;
        FPVector2 DesiredVelocity;
        FPVector2 DesiredDirection;
        FP DesiredSpeed;

        // 0x7d26674
        std::uint8_t Flags() const { return _flags; }
        // 0x7d2667c
        void SetFlags(std::uint8_t value) { _flags = value; }
        // 0x7d26714 - the avoidance agent's mask, or every layer.
        std::int32_t AvoidanceMask() const;
        // 0x7d2672c - the avoidance agent's or obstacle's layer, or 0.
        std::int32_t AvoidanceLayer() const;
        // 0x7d2674c - the pathfinder is active / the obstacle is enabled.
        bool IsActive() const;

        // 0x7d26684 - the pathfinder's config asset (frame.FindAsset; null without a
        // pathfinder).
        NavMeshAgentConfig* TryGetConfig(Core::FrameBase* frame) const;
        // TODO(0x7d26780, Navigation.Entry.ClearEntityPhysics): zeroes the 2D/3D body's
        // velocity and wakes it; needs PhysicsBody2D/3D.
    };

    // Search state owned by one simulation thread.
    class ThreadLocal {
      public:
        // 0x7d2a6ac - a PathFinder sized by Constants.PathFinderSize. (The allocator
        // argument is replaced by owned storage.)
        ThreadLocal(int rawPathSize, Quantum::RawPathAllocationStrategy rawPathAllocationStrategy);

        // 0x7d2a780 - makes the mailbox big enough for the navmesh's triangles and borders.
        void AllocateMailboxBuffer(const NavMesh& navmesh);
        // 0x7d2a898
        void Dispose();

        // TODO(0x7d2a69c, Navigation.ThreadLocal.get_Allocator): the frame allocator; not
        // used by the owned-storage port.
        // TODO(0x7d2a6a4, Navigation.ThreadLocal.set_Allocator)

        std::unique_ptr<Quantum::PathFinder> PathFinder;
        std::vector<VelocityCandidate> VelocityCandidates;
        std::unique_ptr<MailboxSet8> Mailbox;
        FPVector2 LineOfSightHit;
    };

    // Creates the thread-local state for `threadCount` threads (the part of the
    // Updater's constructor that does not need the task system).
    Navigation(const Config& config, int threadCount);

    // TODO(0x7cf8fe0, Quantum.Navigation..ctor): creates the Updater (below) for the frame
    // context.
    // TODO(0x7cf95b8, Quantum.Navigation.Dispose): disposes the Updater.
    // TODO(0x7cf9784, Quantum.Navigation.Update): runs Updater.Update; needs the task system.

    // The calling thread's state (Updater.ThreadLocals[thread index]).
    ThreadLocal& ThreadLocalAt(int threadIndex);

    // TODO(0x7cf9a70, Quantum.Navigation.GetThreadLocal): looks the thread's state up
    // through f.Context.Navigation; needs FrameThreadSafe (ThreadLocalAt is the lookup).

    // 0x7cf993c - NavMesh::LineOfSight without cost restriction.
    static bool LineOfSight(const FPVector3& positionStart, const FPVector3& positionDestination,
                            NavMeshRegionMask regionMask, const NavMesh& navmesh, int triangleStart,
                            int triangleDestination);

    // 0x7cf99cc - NavMesh::Raycast2D in the XZ plane with the thread's mailbox. The
    // original takes a FrameThreadSafe and only uses its thread index.
    bool Raycast2D(int threadIndex, const FPVector3& origin, const FPVector3& direction,
                   FP distance, NavMeshRegionMask regionMask, const NavMesh& navmesh,
                   FPVector2& hit);

    // TODO(0x7cf9abc, Quantum.Navigation.UpdatePathfinderWaypointDetection): advances to the
    // next waypoint when reached (distance or detection axis); needs the pathfinder's
    // frame-heap waypoints.
    // TODO(0x7cf9ce4, Quantum.Navigation.MoveEntityOverLink): moves the agent along an
    // off-mesh link; needs FrameThreadSafe and the waypoint list.
    // TODO(0x7cfa300, Quantum.Navigation.MoveEntity): steering (rotation, acceleration,
    // braking), avoidance result, navmesh clamping and writing transform / body / KCC;
    // needs FrameThreadSafe and the physics components.
    // TODO(0x7cfb0e8, Quantum.Navigation.TryGetVerticalPositionRaw): navmesh or physics
    // raycast height; needs FrameThreadSafe and physics queries.
    // TODO(0x7cfb444, Quantum.Navigation.PreUpdate): repath / line-of-sight / waypoint
    // bookkeeping before moving; needs FrameThreadSafe and the waypoint list.
    // TODO(0x7cfbbd0, Quantum.Navigation.LineOfSightLogic): skips waypoints that are directly
    // visible; needs the waypoint list.
    // TODO(0x7cfb85c, Quantum.Navigation.SearchPath): PathFinder.FindPath plus writing the
    // funnelled path into the agent's waypoints and callbacks; needs the waypoint list.

    // 0x7cfb3f8 - (x, vertical.Position, y).
    static FPVector3 ToVector3(FPVector2 position, const Transform2DVertical& transform2DVertical);
    // 0x7cfb414 - writes a 3D position into a 2D transform plus vertical.
    static void ApplyToTransform(const FPVector3& position, Quantum::Transform2D& transform,
                                 Transform2DVertical& transform2DVertical);

    // 0x7cfbde0 - aims at the current waypoint at the steering agent's max speed, or keeps
    // the current velocity when on it. (The frame argument is unused.)
    static void UpdateDesiredVelocity(Entry& entry);

    // 0x7cfbeb8 - builds the reciprocal velocity obstacles of agents a and b (each only if
    // it can see the other's avoidance layer and is moving). The frame argument is unused.
    static void ComputeHRVO(const Entry& a, const Entry& b, std::int64_t radiusSum,
                            VelocityObstacle& hrvoA, VelocityObstacle& hrvoB,
                            FPVector2 desiredVelocity, FPVector2 otherDesiredVelocity,
                            std::int64_t maxHeightDelta, std::int64_t voTruncationFactor);

    // 0x7cfc71c - in-place quicksort of candidates [lo, hi] by Correction (Lomuto).
    static void Quicksort(std::vector<VelocityCandidate>& candidates, int lo, int hi);

    // 0x7cfc884 - the direction to move in after avoidance, and the speed for it.
    static FPVector2 RunAvoidanceOnAgent(ThreadLocal& threadLocal, const Entry& entry,
                                         const NavMeshAgentConfig& config, FP deltaTime,
                                         FP& desiredSpeed);

    // 0x7cfcad4 - picks the candidate velocity closest to the preferred one that violates
    // no velocity obstacle (or violates them "latest"). False without obstacles or speed.
    static bool CalculateAgentAvoidance(ThreadLocal& threadLocal, const Entry& entry,
                                        const NavMeshAgentConfig& config, FP deltaTime,
                                        FPVector2 preferredVelocity, FP preferredSpeed,
                                        FPVector2& newVelocity);

    // Candidate generators of Navigation.VelocityObstacle.
    // 0x7d2a910 - with speed reduction: where the preferred velocity's ray meets a leg.
    static void GetCandidates0(FPVector2 preferredVelocity, FP preferredSpeed,
                               FP preferredDistanceSqr, int voIndex, const VelocityObstacle& vo,
                               int& candidateIndex, std::vector<VelocityCandidate>& candidates,
                               int maxCandidates);
    // 0x7d2ab94 - projections of the preferred velocity onto both legs.
    static void GetCandidates1(FPVector2 preferredVelocity, FP preferredDistanceSqr, int voIndex,
                               const VelocityObstacle& vo, int& candidateIndex,
                               std::vector<VelocityCandidate>& candidates, int maxCandidates);
    // 0x7d2ada8 - where the legs cross the max-speed circle.
    static void GetCandidates2(FPVector2 preferredVelocity, FP preferredDistanceSqr, int voIndex,
                               const VelocityObstacle& vo, int& candidateIndex,
                               std::vector<VelocityCandidate>& candidates, int maxCandidates);
    // 0x7d2b1b4 - leg/leg intersections of two VOs. In this build only the determinants
    // are evaluated and no candidate is ever added.
    static void GetCandidates3(FPVector2 preferredVelocity, FP preferredDistanceSqr, int vo1Index,
                               const VelocityObstacle& vo1, int vo2Index,
                               const VelocityObstacle& vo2, int& candidateIndex,
                               std::vector<VelocityCandidate>& candidates, int maxCandidates);

    // TODO(0x7cf9060, Navigation.Updater..ctor): allocates entry/pair/radix buffers and
    // registers the task delegates; creates the thread-locals (done by the constructor
    // above). Needs the task system.
    // TODO(0x7cf95e0, Navigation.Updater.Dispose): frees the buffers and thread-locals.
    // TODO(0x7cf9798, Navigation.Updater.Update): schedules the update tasks below.
    // TODO(0x7cfd0d0, Navigation.Updater.Task_MoveAgentCallbacks)
    // TODO(0x7cfd470, Navigation.Updater.Task_CreateEntries)
    // TODO(0x7cfd4d0, Navigation.Updater.CreateEntries): gathers agents from the component
    // buffers into entries.
    // TODO(0x7cfe264, Navigation.Updater.GetNextEntry)
    // TODO(0x7cfe320, Navigation.Updater.GetNextRadixEndPoint)
    // TODO(0x7cfe3fc, Navigation.Updater.GetNextPair)
    // TODO(0x7cfe4b8, Navigation.Updater.Task_IntegrateAvoidanceData)
    // TODO(0x7cfe554, Navigation.Updater.Task_Broadphase)
    // TODO(0x7cfebc0, Navigation.Updater.AvoidanceBroadphase): sweep-and-prune of agent
    // circles into candidate pairs (radix-sorted end points).
    // TODO(0x7cfe5bc, Navigation.Updater.WaypointReachedDetection)
    // TODO(0x7cfee68, Navigation.Updater.Task_UpdateAgents)
    // TODO(0x7cff474, Navigation.Updater.Task_CreateAvoidanceData): ComputeHRVO per pair.
    // TODO(0x7cff600, Navigation.Updater.Task_PreUpdateAgents)

  private:
    Config config_;
    std::vector<std::unique_ptr<ThreadLocal>> threadLocals_; // Updater.ThreadLocals
};

} // namespace Quantum
