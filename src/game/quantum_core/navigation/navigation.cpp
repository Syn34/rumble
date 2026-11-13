#include "quantum_core/navigation/navigation.h"

#include "photon_deterministic/fp_collision.h"
#include "photon_deterministic/fp_math.h"
#include "quantum_core/navigation/navigation_constants.h"
#include "quantum_core/navigation/navmesh.h"
#include "quantum_core/transform.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;
namespace FPCollision = Photon::Deterministic::FPCollision;

namespace {

constexpr FP kEpsilon = FP::FromRaw(Navigation::Epsilon);

// Velocity a VO is anchored on: a steering agent's velocity, else an obstacle's.
FPVector2 AgentVelocity(const Navigation::Entry& entry) {
    if (entry.SteeringAgent != nullptr) {
        return entry.SteeringAgent->_velocity;
    }
    if (entry.AvoidanceObstacle != nullptr) {
        return entry.AvoidanceObstacle->_velocity;
    }
    return FPVector2::Zero();
}

std::uint8_t AgentPriority(const Navigation::Entry& entry) {
    return entry.AvoidanceAgent != nullptr ? entry.AvoidanceAgent->_priority : std::uint8_t{50};
}

bool LayerVisible(std::int32_t layer, std::int32_t mask) {
    return ((std::uint32_t{1} << (layer & 31)) & static_cast<std::uint32_t>(mask)) != 0;
}

void AddCandidate(std::vector<Navigation::VelocityCandidate>& candidates, int& candidateIndex,
                  std::int32_t index1, std::int32_t index2, FP correction, FPVector2 velocity) {
    candidates.at(static_cast<std::size_t>(candidateIndex)) = {index1, index2, correction,
                                                               velocity};
    ++candidateIndex;
}

} // namespace

// ---------------------------------------------------------------------------------------
// Entry

// 0x7d26714
std::int32_t Navigation::Entry::AvoidanceMask() const {
    return AvoidanceAgent != nullptr ? AvoidanceAgent->_avoidanceMask : -1;
}

// 0x7d2672c
std::int32_t Navigation::Entry::AvoidanceLayer() const {
    if (AvoidanceAgent != nullptr) {
        return AvoidanceAgent->_avoidanceLayer;
    }
    if (AvoidanceObstacle != nullptr) {
        return AvoidanceObstacle->_avoidanceLayer;
    }
    return 0;
}

// 0x7d2674c
bool Navigation::Entry::IsActive() const {
    if (PathfinderAgent != nullptr) {
        return PathfinderAgent->_active == 1;
    }
    if (AvoidanceObstacle != nullptr) {
        return AvoidanceObstacle->_disabled == 0;
    }
    return false;
}

// ---------------------------------------------------------------------------------------
// ThreadLocal

// 0x7d2a6ac
Navigation::ThreadLocal::ThreadLocal(int rawPathSize,
                                     Quantum::RawPathAllocationStrategy rawPathAllocationStrategy)
    : PathFinder(std::make_unique<Quantum::PathFinder>(NavigationConstants::PathFinderSize,
                                                       rawPathSize, rawPathAllocationStrategy)) {}

// 0x7d2a780
void Navigation::ThreadLocal::AllocateMailboxBuffer(const NavMesh& navmesh) {
    const int capacity =
        std::max(navmesh.BorderCount(), static_cast<int>(navmesh.Triangles.size()));
    if (Mailbox == nullptr) {
        Mailbox = std::make_unique<MailboxSet8>(capacity);
    } else if (Mailbox->Capacity() < capacity) {
        Mailbox->SetCapacity(capacity);
    }
}

// 0x7d2a898
void Navigation::ThreadLocal::Dispose() {
    if (PathFinder != nullptr) {
        PathFinder->Dispose();
        PathFinder.reset();
    }
    if (Mailbox != nullptr) {
        Mailbox->Dispose();
        Mailbox.reset();
    }
}

// ---------------------------------------------------------------------------------------
// Navigation

Navigation::Navigation(const Config& config, int threadCount) : config_(config) {
    for (int i = 0; i < threadCount; ++i) {
        auto threadLocal =
            std::make_unique<ThreadLocal>(config.RawPathSize, config.RawPathAllocationStrategy);
        // Uncertain: the candidate buffer is sized by the (not yet ported) update loop in
        // the original; MaxAvoidanceCandidates is the bound every use clamps to.
        threadLocal->VelocityCandidates.resize(
            static_cast<std::size_t>(std::max(config.MaxAvoidanceCandidates, 1)));
        threadLocals_.push_back(std::move(threadLocal));
    }
}

Navigation::ThreadLocal& Navigation::ThreadLocalAt(int threadIndex) {
    return *threadLocals_.at(static_cast<std::size_t>(threadIndex));
}

// 0x7cf993c
bool Navigation::LineOfSight(const FPVector3& positionStart, const FPVector3& positionDestination,
                             NavMeshRegionMask regionMask, const NavMesh& navmesh,
                             int triangleStart, int triangleDestination) {
    return navmesh.LineOfSight(positionStart, positionDestination, regionMask, triangleStart,
                               triangleDestination, false);
}

// 0x7cf99cc
bool Navigation::Raycast2D(int threadIndex, const FPVector3& origin, const FPVector3& direction,
                           FP distance, NavMeshRegionMask regionMask, const NavMesh& navmesh,
                           FPVector2& hit) {
    ThreadLocal& threadLocal = ThreadLocalAt(threadIndex);
    threadLocal.AllocateMailboxBuffer(navmesh);
    return navmesh.Raycast2D(FPVector2(origin.X, origin.Z), FPVector2(direction.X, direction.Z),
                             distance, regionMask, threadLocal.Mailbox.get(), hit);
}

// 0x7cfb3f8
FPVector3 Navigation::ToVector3(FPVector2 position,
                                const Transform2DVertical& transform2DVertical) {
    return {position.X, transform2DVertical.Position, position.Y};
}

// 0x7cfb414
void Navigation::ApplyToTransform(const FPVector3& position, Quantum::Transform2D& transform,
                                  Transform2DVertical& transform2DVertical) {
    transform.Position.X = position.X;
    transform2DVertical.Position = position.Y;
    transform.Position.Y = position.Z;
}

// 0x7cfbde0
void Navigation::UpdateDesiredVelocity(Entry& entry) {
    const FP toWaypointX = entry.CurrentWaypoint.X - entry.Position.X;
    const FP toWaypointZ = entry.CurrentWaypoint.Z - entry.Position.Z;
    const FP distance = FPMath::Sqrt(toWaypointZ * toWaypointZ + toWaypointX * toWaypointX);
    const NavMeshSteeringAgent& steering = *entry.SteeringAgent;
    if (distance.RawValue() != 0) {
        entry.DesiredDirection = {toWaypointX / distance, toWaypointZ / distance};
        const FP maxSpeed = steering.MaxSpeed();
        entry.DesiredVelocity = {maxSpeed * entry.DesiredDirection.X,
                                 maxSpeed * entry.DesiredDirection.Y};
        entry.DesiredSpeed = maxSpeed;
    } else {
        entry.DesiredVelocity = steering._velocity;
        entry.DesiredDirection = FPVector2::Normalize(steering._velocity);
        entry.DesiredSpeed = steering.CurrentSpeed();
    }
}

// 0x7cfbeb8
void Navigation::ComputeHRVO(const Entry& a, const Entry& b, std::int64_t radiusSumRaw,
                             VelocityObstacle& hrvoA, VelocityObstacle& hrvoB,
                             FPVector2 desiredVelocity, FPVector2 otherDesiredVelocity,
                             std::int64_t maxHeightDelta, std::int64_t voTruncationFactorRaw) {
    const FP radiusSum = FP::FromRaw(radiusSumRaw);
    const FP truncationFactor = FP::FromRaw(voTruncationFactorRaw);
    const bool aStatic = a.DesiredSpeed < kEpsilon;
    const bool bStatic = b.DesiredSpeed < kEpsilon;
    if (aStatic && bStatic) {
        return;
    }
    if (FPMath::Abs(b.Position.Y - a.Position.Y) > FP::FromRaw(maxHeightDelta)) {
        return;
    }

    const FP radiusSumSqr = radiusSum * radiusSum;
    const FP halfRadiusSum = FP::FromRaw(-(radiusSumRaw >> 1));
    const auto distanceSqrXZ = [](const FPVector3& p, const FPVector3& q) {
        const FP dx = p.X - q.X;
        const FP dz = p.Z - q.Z;
        return dz * dz + dx * dx;
    };

    // A standing agent only reacts to a mover that is heading towards it (or whose
    // waypoint lies within reach of it).
    if (aStatic) {
        if (distanceSqrXZ(b.Position, b.CurrentWaypoint) < distanceSqrXZ(b.Position, a.Position)) {
            if (distanceSqrXZ(a.Position, b.CurrentWaypoint) > radiusSumSqr) {
                return;
            }
        } else {
            const FP approach = (a.Position.Z - b.Position.Z) * otherDesiredVelocity.Y +
                                (a.Position.X - b.Position.X) * otherDesiredVelocity.X;
            if (approach < halfRadiusSum) {
                return;
            }
        }
    }
    if (bStatic) {
        if (distanceSqrXZ(a.Position, a.CurrentWaypoint) < distanceSqrXZ(a.Position, b.Position)) {
            if (distanceSqrXZ(b.Position, a.CurrentWaypoint) > radiusSumSqr) {
                return;
            }
        } else {
            const FP approach = (b.Position.Z - a.Position.Z) * desiredVelocity.Y +
                                (b.Position.X - a.Position.X) * desiredVelocity.X;
            if (approach < halfRadiusSum) {
                return;
            }
        }
    }

    FP aX = a.Position.X;
    const FP relZ = b.Position.Z - a.Position.Z;
    FP relX = b.Position.X - aX;
    FP distanceSqr = relZ * relZ + relX * relX;
    FP distance = FPMath::Sqrt(distanceSqr);
    if (distance.RawValue() == 0) {
        // Coincident agents: nudge a sideways by 0.01 so the maths has a direction.
        aX = aX + FP::_0_01();
        relX = relX - FP::_0_01();
        distanceSqr = FP::FromRaw(6);
        distance = FP::_0_01();
    }

    // The agent with the higher priority takes the smaller share of the avoidance.
    const std::uint8_t aPriority = AgentPriority(a);
    const std::uint8_t bPriority = AgentPriority(b);
    FP weightA = FP::_0_50();
    FP weightB = FP::_0_50();
    if (aPriority > bPriority) {
        weightA = FP::_0_25();
        weightB = FP::_0_75();
    } else if (aPriority < bPriority) {
        weightA = FP::_0_75();
        weightB = FP::_0_25();
    }
    const FPVector2 aVelocity = AgentVelocity(a);
    const FPVector2 bVelocity = AgentVelocity(b);
    const bool aSeesB = LayerVisible(b.AvoidanceLayer(), a.AvoidanceMask());
    const bool bSeesA = LayerVisible(a.AvoidanceLayer(), b.AvoidanceMask());

    if (distanceSqr < radiusSumSqr) {
        // Overlapping: the VO is a half-plane perpendicular to the separation.
        const FP separationX = aX - b.Position.X;
        const FP overlapDistance = FPMath::Sqrt(relZ * relZ + separationX * separationX);
        if (overlapDistance.RawValue() == 0) {
            return;
        }
        const FP perpX = separationX / overlapDistance;
        const FP perpZ = relZ / overlapDistance;
        if (aSeesB && !aStatic) {
            hrvoA.IsValid = true;
            hrvoA.Right = {perpZ, perpX};
            hrvoA.Left = {-perpZ, -perpX};
        }
        if (!bStatic && bSeesA) {
            hrvoB.IsValid = true;
            hrvoB.Left = {perpZ, perpX};
            hrvoB.Right = {-perpZ, -perpX};
        }
        const FP directionX = relX / distance;
        const FP directionZ = relZ / distance;
        const FP sumX = bVelocity.X + aVelocity.X;
        const FP sumY = bVelocity.Y + aVelocity.Y;
        const FP penetration = radiusSum - distance;
        if (hrvoA.IsValid) {
            hrvoA.Apex = {sumX * weightA - directionX * penetration,
                          sumY * weightA - directionZ * penetration};
        }
        if (hrvoB.IsValid) {
            hrvoB.Apex = {sumX * weightB - (-penetration) * directionX,
                          sumY * weightB - (-penetration) * directionZ};
        }
        return;
    }

    // Apart: a cone from the angle towards the other agent, opened by asin(r / d).
    const FP angle = FPMath::Atan2(relZ, relX);
    const FP ratio = radiusSum / distance;
    const FP opening = FPMath::Asin(ratio); // MinValue when |ratio| > 1, as the table lookup
    FP rightSin;
    FP rightCos;
    FP leftSin;
    FP leftCos;
    FPMath::SinCos(angle - opening, rightSin, rightCos);
    FPMath::SinCos(opening + angle, leftSin, leftCos);
    if (aSeesB && !aStatic) {
        hrvoA.IsValid = true;
        hrvoA.Right = {rightCos, rightSin};
        hrvoA.Left = {leftCos, leftSin};
    }
    if (!bStatic && bSeesA) {
        hrvoB.IsValid = true;
        hrvoB.Right = {-rightCos, -rightSin};
        hrvoB.Left = {-leftCos, -leftSin};
    }

    // Against a standing agent only nearby velocities are blocked (truncated cone).
    const FP truncation = FP::FromRaw(std::max<std::int64_t>(
        0, (distance / truncationFactor - radiusSum / truncationFactor).RawValue()));
    if (aStatic) {
        hrvoB.TruncationLength = truncation;
    }
    if (bStatic) {
        hrvoA.TruncationLength = truncation;
    }
    if (bStatic || aStatic) {
        return;
    }

    if (!(aSeesB && bSeesA)) {
        // One-sided: a plain velocity obstacle anchored on the other's velocity.
        if (hrvoA.IsValid) {
            hrvoA.Apex = bVelocity;
        } else {
            hrvoB.Apex = aVelocity;
        }
        return;
    }

    // Hybrid reciprocal VO: the apex moves along one leg depending on which side the
    // agents' desired velocities pass each other.
    const FP desiredDeltaX = desiredVelocity.X - otherDesiredVelocity.X;
    const FP desiredDeltaY = desiredVelocity.Y - otherDesiredVelocity.Y;
    const FP velocityDeltaX = aVelocity.X - bVelocity.X;
    const FP velocityDeltaY = aVelocity.Y - bVelocity.Y;
    if (relX * desiredDeltaY > relZ * desiredDeltaX) {
        const FP s = (hrvoA.Left.Y * velocityDeltaX - hrvoA.Left.X * velocityDeltaY) * weightA;
        hrvoA.Apex = {bVelocity.X + s * hrvoA.Right.X, bVelocity.Y + s * hrvoA.Right.Y};
        const FP t =
            ((-velocityDeltaX) * hrvoB.Left.Y - (-velocityDeltaY) * hrvoB.Left.X) * weightB;
        hrvoB.Apex = {aVelocity.X + t * hrvoB.Right.X, aVelocity.Y + t * hrvoB.Right.Y};
    } else {
        const FP s = (hrvoA.Right.Y * velocityDeltaX - hrvoA.Right.X * velocityDeltaY) * weightA;
        hrvoA.Apex = {bVelocity.X + s * hrvoA.Left.X, bVelocity.Y + s * hrvoA.Left.Y};
        const FP t =
            ((-velocityDeltaX) * hrvoB.Right.Y - (-velocityDeltaY) * hrvoB.Right.X) * weightB;
        hrvoB.Apex = {aVelocity.X + t * hrvoB.Left.X, aVelocity.Y + t * hrvoB.Left.Y};
    }
}

// 0x7cfc71c
void Navigation::Quicksort(std::vector<VelocityCandidate>& candidates, int lo, int hi) {
    while (lo < hi) {
        const FP pivot = candidates.at(static_cast<std::size_t>(hi)).Correction;
        int store = lo;
        for (int i = lo; i < hi; ++i) {
            if (candidates.at(static_cast<std::size_t>(i)).Correction < pivot) {
                std::swap(candidates[static_cast<std::size_t>(store)],
                          candidates[static_cast<std::size_t>(i)]);
                ++store;
            }
        }
        std::swap(candidates.at(static_cast<std::size_t>(store)),
                  candidates[static_cast<std::size_t>(hi)]);
        Quicksort(candidates, lo, store - 1);
        lo = store + 1;
    }
}

// 0x7d2a910
void Navigation::GetCandidates0(FPVector2 preferredVelocity, FP preferredSpeed,
                                FP /*preferredDistanceSqr*/, int voIndex,
                                const VelocityObstacle& vo, int& candidateIndex,
                                std::vector<VelocityCandidate>& candidates, int maxCandidates) {
    if (candidateIndex >= maxCandidates) {
        return;
    }
    const FPVector2 apex = vo.Apex;
    const FP apexSide = apex.Y * preferredVelocity.X;
    const FP apexOther = apex.X * preferredVelocity.Y;
    FPVector2 leg;
    if (apexSide < apexOther) {
        leg = vo.Left;
        if (leg.Y * preferredVelocity.X <= leg.X * preferredVelocity.Y) {
            return;
        }
    } else {
        if (apexSide == apexOther) {
            return;
        }
        leg = vo.Right;
        if (leg.Y * preferredVelocity.X >= leg.X * preferredVelocity.Y) {
            return;
        }
    }
    // Slow down along the preferred direction until the leg is reached.
    const FP reach = preferredSpeed * 5;
    const FPVector2 legEnd{apex.X + leg.X * reach, apex.Y + leg.Y * reach};
    FPVector2 point;
    if (!FPCollision::LineIntersectsLine(FPVector2::Zero(), preferredVelocity, apex, legEnd,
                                         point)) {
        return;
    }
    if (candidateIndex >= maxCandidates) {
        return;
    }
    const FP correction = (preferredVelocity - point).SqrMagnitude() *
                          FP::FromRaw(VelocityObstacle::CorrectionBiasCandidates0);
    AddCandidate(candidates, candidateIndex, voIndex, voIndex, correction, point);
}

// 0x7d2ab94
void Navigation::GetCandidates1(FPVector2 preferredVelocity, FP preferredDistanceSqr, int voIndex,
                                const VelocityObstacle& vo, int& candidateIndex,
                                std::vector<VelocityCandidate>& candidates, int maxCandidates) {
    if (candidateIndex >= maxCandidates) {
        return;
    }
    const FPVector2 apex = vo.Apex;
    const FPVector2 offset = preferredVelocity - apex;

    const FPVector2 right = vo.Right;
    const FP alongRight = right.Y * offset.Y + right.X * offset.X;
    if (offset.Y * right.X > right.Y * offset.X && alongRight >= FP::FromRaw(1)) {
        const FPVector2 projected{apex.X + alongRight * right.X, apex.Y + alongRight * right.Y};
        if (projected.SqrMagnitude() < preferredDistanceSqr) {
            AddCandidate(candidates, candidateIndex, voIndex, voIndex,
                         (preferredVelocity - projected).SqrMagnitude(), projected);
            if (candidateIndex >= maxCandidates) {
                return;
            }
        }
    }

    const FPVector2 left = vo.Left;
    const FP alongLeft = left.Y * offset.Y + left.X * offset.X;
    if (alongLeft < FP::FromRaw(1) || left.X * offset.Y >= left.Y * offset.X) {
        return;
    }
    const FPVector2 projected{apex.X + alongLeft * left.X, apex.Y + alongLeft * left.Y};
    if (projected.SqrMagnitude() < preferredDistanceSqr) {
        AddCandidate(candidates, candidateIndex, voIndex, voIndex,
                     (preferredVelocity - projected).SqrMagnitude(), projected);
    }
}

// 0x7d2ada8
void Navigation::GetCandidates2(FPVector2 preferredVelocity, FP preferredDistanceSqr, int voIndex,
                                const VelocityObstacle& vo, int& candidateIndex,
                                std::vector<VelocityCandidate>& candidates, int maxCandidates) {
    if (candidateIndex >= maxCandidates) {
        return;
    }
    const FPVector2 apex = vo.Apex;
    // Intersections of the leg ray apex + t * leg with the circle |v|^2 = distanceSqr.
    // Returns false when the candidate buffer filled up.
    const auto addLegCrossings = [&](FPVector2 leg) {
        const FP cross = leg.Y * apex.X - leg.X * apex.Y;
        const FP discriminant = preferredDistanceSqr - cross * cross;
        if (discriminant < FP::FromRaw(1)) {
            return true;
        }
        const FP root = FPMath::Sqrt(discriminant);
        const FP along = leg.Y * apex.Y + leg.X * apex.X;
        const FP near = root - along;
        const FP far = (-along) - root;
        if (near >= FP::Zero()) {
            const FPVector2 velocity{apex.X + near * leg.X, apex.Y + near * leg.Y};
            AddCandidate(candidates, candidateIndex, -1, voIndex,
                         (preferredVelocity - velocity).SqrMagnitude(), velocity);
            if (candidateIndex >= maxCandidates) {
                return false;
            }
        }
        if (far >= FP::Zero() && candidateIndex < maxCandidates) {
            const FPVector2 velocity{apex.X + far * leg.X, apex.Y + far * leg.Y};
            AddCandidate(candidates, candidateIndex, -1, voIndex,
                         (preferredVelocity - velocity).SqrMagnitude(), velocity);
            if (candidateIndex >= maxCandidates) {
                return false;
            }
        }
        return true;
    };
    if (addLegCrossings(vo.Right)) {
        addLegCrossings(vo.Left);
    }
}

// 0x7d2b1b4
void Navigation::GetCandidates3(FPVector2 /*preferredVelocity*/, FP /*preferredDistanceSqr*/,
                                int /*vo1Index*/, const VelocityObstacle& vo1, int /*vo2Index*/,
                                const VelocityObstacle& vo2, int& candidateIndex,
                                std::vector<VelocityCandidate>& /*candidates*/, int maxCandidates) {
    // The original evaluates the leg/leg determinants and discards them; nothing is added.
    if (candidateIndex >= maxCandidates) {
        return;
    }
    (void)FPVector2::Determinant(vo1.Right, vo2.Right);
    (void)FPVector2::Determinant(vo1.Left, vo2.Right);
    (void)FPVector2::Determinant(vo1.Right, vo2.Left);
    (void)FPVector2::Determinant(vo1.Left, vo2.Left);
}

// 0x7cfcad4
bool Navigation::CalculateAgentAvoidance(ThreadLocal& threadLocal, const Entry& entry,
                                         const NavMeshAgentConfig& config, FP /*deltaTime*/,
                                         FPVector2 preferredVelocity, FP preferredSpeed,
                                         FPVector2& newVelocity) {
    const bool valid = entry.VoStart != nullptr && preferredSpeed > FP::Zero();
    if (!valid) {
        return false;
    }
    std::vector<VelocityCandidate>& candidates = threadLocal.VelocityCandidates;
    candidates.at(0) = {-1, -1, FP::Zero(), preferredVelocity};
    int count = 1;
    const FP preferredDistanceSqr = preferredSpeed * preferredSpeed;
    const auto maxCandidates = [&]() {
        return std::min(config.MaxAvoidanceCandidates, static_cast<int>(candidates.size()));
    };

    int voIndex = 0;
    for (const VelocityObstacle* vo = entry.VoStart; vo != nullptr; vo = vo->Next, ++voIndex) {
        GetCandidates1(preferredVelocity, preferredDistanceSqr, voIndex, *vo, count, candidates,
                       maxCandidates());
    }
    if (config.AvoidanceQuality >= Quantum::AvoidanceQuality::Good) {
        if (config.AvoidanceCanReduceSpeed) {
            voIndex = 0;
            for (const VelocityObstacle* vo = entry.VoStart; vo != nullptr;
                 vo = vo->Next, ++voIndex) {
                GetCandidates0(preferredVelocity, preferredSpeed, preferredDistanceSqr, voIndex,
                               *vo, count, candidates, maxCandidates());
            }
        }
        voIndex = 0;
        for (const VelocityObstacle* vo = entry.VoStart; vo != nullptr; vo = vo->Next, ++voIndex) {
            GetCandidates2(preferredVelocity, preferredDistanceSqr, voIndex, *vo, count, candidates,
                           maxCandidates());
        }
        if (config.AvoidanceQuality >= Quantum::AvoidanceQuality::Best) {
            voIndex = 0;
            for (const VelocityObstacle* vo1 = entry.VoStart; vo1 != nullptr;
                 vo1 = vo1->Next, ++voIndex) {
                int otherIndex = 0;
                for (const VelocityObstacle* vo2 = vo1->Next; vo2 != nullptr;
                     vo2 = vo2->Next, ++otherIndex) {
                    GetCandidates3(preferredVelocity, preferredDistanceSqr, voIndex, *vo1,
                                   otherIndex, *vo2, count, candidates, maxCandidates());
                }
            }
        }
    }

    if (count - 1 > 0) {
        Quicksort(candidates, 1, count - 1);
    }

    // First candidate (cheapest correction) outside every VO wins; otherwise the one
    // whose first violated VO comes latest in the list.
    newVelocity = preferredVelocity;
    int latestViolation = -1;
    for (int i = 0; i < count; ++i) {
        const VelocityCandidate& candidate = candidates.at(static_cast<std::size_t>(i));
        int violated = -1;
        int index = 0;
        for (const VelocityObstacle* vo = entry.VoStart; vo != nullptr; vo = vo->Next, ++index) {
            if (candidate.Index1 == index || candidate.Index2 == index) {
                continue;
            }
            if (vo->TruncationLength > FP::Zero() &&
                candidate.Velocity.SqrMagnitude() <= vo->TruncationLength * vo->TruncationLength) {
                continue;
            }
            const FPVector2 offset = candidate.Velocity - vo->Apex;
            if (offset.Y * vo->Right.X <= vo->Right.Y * offset.X) {
                continue;
            }
            if (vo->Left.X * offset.Y < vo->Left.Y * offset.X) {
                violated = index;
                break;
            }
        }
        if (violated < 0) {
            newVelocity = candidate.Velocity;
            return valid;
        }
        if (violated > latestViolation) {
            newVelocity = candidate.Velocity;
            latestViolation = violated;
        }
    }
    return valid;
}

// 0x7cfc884
FPVector2 Navigation::RunAvoidanceOnAgent(ThreadLocal& threadLocal, const Entry& entry,
                                          const NavMeshAgentConfig& config, FP deltaTime,
                                          FP& desiredSpeed) {
    desiredSpeed = FP::Zero();
    if (entry.DesiredSpeed.RawValue() == 0) {
        return entry.DesiredDirection;
    }
    desiredSpeed = entry.DesiredSpeed;
    if (config.AvoidanceType != Quantum::AvoidanceType::Internal) {
        return entry.DesiredDirection;
    }
    const FPVector3 waypoint = entry.CurrentWaypoint;
    FPVector2 newVelocity = FPVector2::Zero();
    if (!CalculateAgentAvoidance(threadLocal, entry, config, deltaTime, entry.DesiredVelocity,
                                 entry.DesiredSpeed, newVelocity)) {
        return entry.DesiredDirection;
    }

    // Close to the waypoint, blend back towards the unavoided velocity so agents can
    // still reach it.
    if (config.ReduceAvoidanceAtWaypoints && config.ReduceAvoidanceFactor > FP::Zero()) {
        const FP toWaypointX =
            (entry.Position.X - waypoint.X) + entry.DesiredVelocity.X * deltaTime;
        const FP toWaypointZ =
            (entry.Position.Z - waypoint.Z) + entry.DesiredVelocity.Y * deltaTime;
        const FP distanceSqr = toWaypointZ * toWaypointZ + toWaypointX * toWaypointX;
        const FP reduceRadius =
            config.ReduceAvoidanceFactor * FPMath::Max(FP::Zero(), config.AvoidanceRadius);
        if (distanceSqr < reduceRadius * reduceRadius) {
            FP t = distanceSqr / reduceRadius;
            t = t * t;
            FP blend = t < FP::One() ? t : FP::One();
            if (t < deltaTime) {
                blend = deltaTime;
            }
            newVelocity = {
                entry.DesiredVelocity.X + (newVelocity.X - entry.DesiredVelocity.X) * blend,
                entry.DesiredVelocity.Y + (newVelocity.Y - entry.DesiredVelocity.Y) * blend};
        }
    }

    const FP speed = FPMath::Sqrt(newVelocity.SqrMagnitude());
    if (speed < FP::FromRaw(1)) {
        return entry.DesiredDirection;
    }
    const FPVector2 direction{newVelocity.X / speed, newVelocity.Y / speed};
    desiredSpeed = FPMath::Min(FPMath::Max(FP::Zero(), speed), desiredSpeed);
    return direction;
}

} // namespace Quantum
