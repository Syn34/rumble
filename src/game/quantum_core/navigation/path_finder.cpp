#include "quantum_core/navigation/path_finder.h"

#include "photon_deterministic/fp_math.h"
#include "quantum_core/mailbox_set8.h"
#include "quantum_core/navigation/navigation_constants.h"
#include "quantum_core/navigation/navmesh.h"
#include "quantum_core/navigation/navmesh_types.h"

#include <algorithm>
#include <stdexcept>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

namespace {

template <class T> T& Element(std::vector<T>& items, std::int64_t index) {
    return items.at(static_cast<std::size_t>(index));
}
template <class T> const T& Element(const std::vector<T>& items, std::int64_t index) {
    return items.at(static_cast<std::size_t>(index));
}

constexpr std::int64_t WrapAdd(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
}

// |x| as the vector unit computes it (wraps for the minimum value).
constexpr std::int64_t WrapAbs(std::int64_t value) {
    return value < 0 ? static_cast<std::int64_t>(0 - static_cast<std::uint64_t>(value)) : value;
}

// Raw clamp bounds [1 - half, half - 1] of the grid along one axis, computed in 32-bit
// integers as the original does.
void GridClampBounds(std::int32_t gridSize, std::int32_t nodeSize, std::int64_t& low,
                     std::int64_t& high) {
    const auto extent = static_cast<std::int32_t>(static_cast<std::uint32_t>(gridSize) *
                                                  static_cast<std::uint32_t>(nodeSize));
    const auto halfRaw = static_cast<std::uint32_t>(extent / 2) << FP::Precision;
    low = static_cast<std::int32_t>(static_cast<std::uint32_t>(FP::RAW_ONE) - halfRaw);
    high = static_cast<std::int32_t>(halfRaw - static_cast<std::uint32_t>(FP::RAW_ONE));
}

} // namespace

// ---------------------------------------------------------------------------------------
// FindPathResult

// 0x7cfcfc8
FindPathResult FindPathResult::CreateSuccess(const FPVector3& correctedStartPosition,
                                             const FPVector3& correctedEndPosition) {
    FindPathResult result;
    result.Result = Code::Success;
    result.CorrectedStartPosition = correctedStartPosition;
    result.CorrectedEndPosition = correctedEndPosition;
    return result;
}

// 0x7cfd038
FindPathResult FindPathResult::CreateError(Code errorCode) {
    FindPathResult result;
    result.Result = errorCode;
    return result;
}

// ---------------------------------------------------------------------------------------
// ActiveVertex

// 0x7d28c4c
void ActiveVertex::Create(int index, const FPVector3& pivot) {
    Slot = 0;
    Index = index;
    PortalLeft = -1;
    PortalRight = -1;
    Link = -1;
    Parent = nullptr;
    Pivot = pivot;
}

// 0x7d2a36c
void ActiveVertex::Create(int index, const FPVector3& pivot, int /*portalLeft*/,
                          int /*portalRight*/, ActiveVertex* parent, PathQuality /*quality*/,
                          int slot) {
    Slot = slot;
    Index = index;
    PortalLeft = -1;
    PortalRight = -1;
    Link = -1;
    Parent = parent;
    Pivot = pivot;
}

// 0x7d2a494
std::int64_t ActiveVertex::Distance(const FPVector3& a, const FPVector3& b, PathQuality quality) {
    const FPVector3 d = a - b;
    if (static_cast<std::uint8_t>(quality) < static_cast<std::uint8_t>(PathQuality::Best)) {
        return WrapAdd(WrapAdd(WrapAbs(d.X.RawValue()), WrapAbs(d.Y.RawValue())),
                       WrapAbs(d.Z.RawValue()));
    }
    return FPMath::SqrtRaw(d.SqrMagnitude().RawValue());
}

// 0x7d29de0
FPVector3 ActiveVertex::CreatePivot(int index, int portalLeft, int portalRight,
                                    const FPVector3& start, const FPVector3& goal,
                                    const ActiveVertex* parent, PathQuality quality,
                                    const NavMesh& navmesh) {
    const NavMeshTriangle& triangle = Element(navmesh.Triangles, index);
    const bool hasPortal = (portalLeft | portalRight) >= 0;

    switch (quality) {
    case PathQuality::Fast:
        return triangle.Center;

    case PathQuality::Good: {
        if (!hasPortal) {
            return triangle.Center;
        }
        const FPVector3& left = Element(navmesh.Vertices, portalLeft).Point;
        const FPVector3& right = Element(navmesh.Vertices, portalRight).Point;
        const FPVector3& from = parent != nullptr ? parent->Pivot : start;
        const FPVector3 onPortal = ClosestPointOnSegment(from, left, right);
        const std::int64_t height = NavMesh::GetHeightAtTriangleRaw(
            onPortal, triangle.Normal, Element(navmesh.Vertices, triangle.Vertex0).Point);
        return {onPortal.X, FP::FromRaw(height), onPortal.Z};
    }

    case PathQuality::Best: {
        if (!hasPortal) {
            return triangle.Center;
        }
        const FPVector3& left = Element(navmesh.Vertices, portalLeft).Point;
        const FPVector3& right = Element(navmesh.Vertices, portalRight).Point;
        if (parent != nullptr) {
            FPVector3 crossing;
            if (LineIntersectsLine(parent->Pivot, goal, left, right, crossing)) {
                const FPVector3 flat{crossing.X, FP::Zero(), crossing.Z};
                const std::int64_t height = NavMesh::GetHeightAtTriangleRaw(
                    flat, triangle.Normal, Element(navmesh.Vertices, triangle.Vertex0).Point);
                return {crossing.X, FP::FromRaw(height), crossing.Z};
            }
        }
        // The straight line misses the portal: use the end nearer the goal.
        const std::int64_t toLeft = Distance(goal, left, PathQuality::Best);
        const std::int64_t toRight = Distance(goal, right, PathQuality::Best);
        return toLeft > toRight ? right : left;
    }
    }
    return FPVector3::Zero();
}

// 0x7d2a398
std::int64_t ActiveVertex::CalculateCost(const FPVector3& start, const FPVector3& pivot,
                                         const ActiveVertex* parent, PathQuality quality,
                                         FP costFactor) {
    if (parent == nullptr) {
        return Distance(start, pivot, quality);
    }
    const std::int64_t parentCost = parent->G;
    const std::int64_t step = Distance(parent->Pivot, pivot, quality);
    if (costFactor <= FP::Zero() || costFactor == FP::One()) {
        return WrapAdd(step, parentCost);
    }
    return WrapAdd(parentCost, (FP::FromRaw(step) * costFactor).RawValue());
}

// 0x7d2b2fc
FPVector3 ActiveVertex::ClosestPointOnSegment(const FPVector3& pt, const FPVector3& v0,
                                              const FPVector3& v1) {
    const FP dz = v0.Z - v1.Z;
    const FP dx = v0.X - v1.X;
    const FP lengthSqr = dz * dz + dx * dx;
    if (lengthSqr.RawValue() == 0) {
        return v0;
    }
    const FP dot = (pt.Z - v0.Z) * (v1.Z - v0.Z) + (pt.X - v0.X) * (v1.X - v0.X);
    const FP t = FPMath::Max(FP::Zero(), FPMath::Min(FP::One(), dot / lengthSqr));
    if (t == FP::One()) {
        return v1;
    }
    return {v0.X + (v1.X - v0.X) * t, FP::Zero(), v0.Z + (v1.Z - v0.Z) * t};
}

// 0x7d2b438
bool ActiveVertex::LineIntersectsLine(const FPVector3& p1, const FPVector3& p2, const FPVector3& q1,
                                      const FPVector3& q2, FPVector3& point) {
    point = FPVector3::Zero();
    const FP rX = p2.X - p1.X;
    const FP rZ = p2.Z - p1.Z;
    const FP sX = q2.X - q1.X;
    const FP sZ = q2.Z - q1.Z;
    const FP denominatorA = sZ * rX;
    const FP denominatorB = sX * rZ;
    if (denominatorA == denominatorB) {
        return false; // parallel or collinear
    }
    const FP denominator = denominatorA - denominatorB;
    const FP qpX = q1.X - p1.X;
    const FP qpZ = q1.Z - p1.Z;
    // Both parameters must lie in [0, 1]; a negative one fails the unsigned test.
    const FP t = (sZ * qpX - qpZ * sX) / denominator;
    if (static_cast<std::uint64_t>(t.RawValue()) > static_cast<std::uint64_t>(FP::RAW_ONE)) {
        return false;
    }
    const FP u = (qpX * rZ - qpZ * rX) / denominator;
    if (static_cast<std::uint64_t>(u.RawValue()) >= static_cast<std::uint64_t>(FP::RAW_ONE + 1)) {
        return false;
    }
    point = {p1.X + t * rX, FP::Zero(), p1.Z + t * rZ};
    return true;
}

// ---------------------------------------------------------------------------------------
// ActiveHeap

// 0x7d28240
ActiveHeap::ActiveHeap(int capacity)
    : heapLength_(capacity), heap_(static_cast<std::size_t>(capacity), nullptr) {}

// 0x7d28ab4
void ActiveHeap::Clear() {
    size_ = 1;
    std::fill(heap_.begin(), heap_.end(), nullptr);
}

void ActiveHeap::SiftUp(ActiveVertex* node, int slot) {
    // Move `node` towards the root while its parent costs strictly more.
    while (slot != 1) {
        const int parentSlot = slot / 2;
        ActiveVertex* parent = heap_[static_cast<std::size_t>(parentSlot)];
        if (parent->F <= node->F) {
            break;
        }
        heap_[static_cast<std::size_t>(slot)] = parent;
        node->Slot = parentSlot;
        heap_[static_cast<std::size_t>(parentSlot)] = node;
        parent->Slot = slot;
        slot = parentSlot;
    }
}

// 0x7d2a5a0
void ActiveHeap::Update(ActiveVertex* node) {
    SiftUp(node, node->Slot);
}

// 0x7d2a4fc
void ActiveHeap::Push(ActiveVertex* node) {
    if (size_ == heapLength_) {
        ExpandHeap();
    }
    heap_[static_cast<std::size_t>(size_)] = node;
    node->Slot = size_;
    SiftUp(node, size_);
    ++size_;
}

// 0x7d29d24
ActiveVertex* ActiveHeap::Pop() {
    --size_;
    ActiveVertex* result = heap_[1];
    heap_[1] = heap_[static_cast<std::size_t>(size_)];

    // Sift down. The bound is the decremented size *inclusive*, as in the original, so
    // the stale copy of the moved node in slot `size_` takes part in the comparisons.
    int slot = 1;
    while (true) {
        const int left = slot * 2;
        const int right = slot * 2 + 1;
        const auto cost = [this](int s) { return heap_[static_cast<std::size_t>(s)]->F; };
        int best = slot;
        if (right <= size_) {
            const int smaller = cost(slot) < cost(left) ? slot : left;
            best = cost(smaller) < cost(right) ? smaller : right;
        } else if (left <= size_) {
            best = cost(slot) < cost(left) ? slot : left;
        }
        if (best == slot) {
            break;
        }
        std::swap(heap_[static_cast<std::size_t>(slot)], heap_[static_cast<std::size_t>(best)]);
        heap_[static_cast<std::size_t>(slot)]->Slot = slot;
        heap_[static_cast<std::size_t>(best)]->Slot = best;
        slot = best;
    }
    return result;
}

// 0x7d2b50c
void ActiveHeap::ExpandHeap() {
    heap_.resize(static_cast<std::size_t>(heapLength_) * 2, nullptr);
    heapLength_ *= 2;
}

// 0x7d284a4
void ActiveHeap::Free() {
    heap_.clear();
    heap_.shrink_to_fit();
}

// ---------------------------------------------------------------------------------------
// PathFinder

// 0x7d28034
PathFinder::PathFinder(int openNodesSize, int rawPathSize,
                       RawPathAllocationStrategy rawPathAllocationStrategy)
    : funnelLeft_(static_cast<std::size_t>(NavigationConstants::FunnelBufferSize)),
      funnelRight_(static_cast<std::size_t>(NavigationConstants::FunnelBufferSize)),
      funnel_(static_cast<std::size_t>(NavigationConstants::MaxFunnelPathSize)),
      open_(FPMath::NextPowerOfTwo(openNodesSize + 1)),
      rawPath_(static_cast<std::size_t>(rawPathSize)),
      rawPathAllocationStrategy_(rawPathAllocationStrategy) {
    verticesBlocks_.push_back(std::make_unique<ActiveVertex[]>(
        static_cast<std::size_t>(NavigationConstants::ActiveVertexDefaultSize)));
    Resize(openNodesSize);
}

// 0x7d282ac
void PathFinder::Resize(int size) {
    if (static_cast<int>(active_.size()) < size) {
        active_.assign(static_cast<std::size_t>(size), nullptr);
    }
    const int closedBytes = (size + 7) >> 3;
    if (static_cast<int>(closedSet_.size()) < closedBytes) {
        closedSet_.assign(static_cast<std::size_t>(closedBytes), 0);
    }
}

// 0x7d2835c
void PathFinder::SetNavMesh(const NavMesh* navmesh) {
    if (navmesh_ == navmesh) {
        return;
    }
    navmesh_ = navmesh;
    if (navmesh == nullptr) {
        throw std::invalid_argument("PathFinder::SetNavMesh: null navmesh");
    }
    Resize(static_cast<int>(navmesh->Triangles.size()));
}

// 0x7d283b0
void PathFinder::Dispose() {
    verticesBlocks_.clear();
    closedSet_.clear();
    active_.clear();
    open_.Free();
    navmesh_ = nullptr;
}

// 0x7d27ff8
std::int64_t PathFinder::CalculateTriangleArea(const FPVector3& a, const FPVector3& b,
                                               const FPVector3& c) {
    return ((c.X - a.X) * (b.Z - a.Z) - (b.X - a.X) * (c.Z - a.Z)).RawValue();
}

bool PathFinder::IsClosed(int triangle) const {
    return ((closedSet_.at(static_cast<std::size_t>(triangle / 8)) >> (triangle % 8)) & 1) != 0;
}

void PathFinder::SetClosed(int triangle) {
    closedSet_.at(static_cast<std::size_t>(triangle / 8)) |=
        static_cast<std::uint8_t>(1 << (triangle % 8));
}

// 0x7d2a618
ActiveVertex* PathFinder::GetActiveVertexFromIndex(int index) {
    const int blockSize = NavigationConstants::ActiveVertexDefaultSize;
    return &verticesBlocks_.at(static_cast<std::size_t>(index / blockSize))[static_cast<std::size_t>(index % blockSize)];
}

// 0x7d28b24
ActiveVertex* PathFinder::GetNextActiveVertex() {
    const int blockSize = NavigationConstants::ActiveVertexDefaultSize;
    if (verticesCount_ == blockSize * static_cast<int>(verticesBlocks_.size())) {
        verticesBlocks_.push_back(std::make_unique<ActiveVertex[]>(static_cast<std::size_t>(blockSize)));
    }
    const int index = verticesCount_++;
    ActiveVertex* vertex = GetActiveVertexFromIndex(index);
    *vertex = ActiveVertex{};
    vertex->Index = index;
    vertex->PortalLeft = 0;
    vertex->PortalRight = 0;
    vertex->Link = 0;
    return vertex;
}

// 0x7d28c78
bool PathFinder::PathAdd(const FPVector3& point, int index) {
    if (rawPathSize_ >= static_cast<int>(rawPath_.size())) {
        if (rawPathAllocationStrategy_ == RawPathAllocationStrategy::Fixed) {
            return false;
        }
        if (rawPathAllocationStrategy_ == RawPathAllocationStrategy::Allocating) {
            rawPath_.resize(rawPath_.size() * 2);
        }
    }
    Element(rawPath_, rawPathSize_++) = PathVertex{index, point, FPVector3::Zero(), -1};
    return true;
}

// 0x7d28dcc
void PathFinder::CreateOrUpdateVertex(int triangleIndex, ActiveVertex* current,
                                      PathQuality quality) {
    const FP cost = Element(navmesh_->Triangles, triangleIndex).Cost;
    ActiveVertex*& slot = Element(active_, triangleIndex);
    if (slot != nullptr) {
        ActiveVertex* existing = slot;
        const FPVector3 pivot =
            ActiveVertex::CreatePivot(triangleIndex, -1, -1, posStart_, posEnd_, current, quality, *navmesh_);
        const std::int64_t g = ActiveVertex::CalculateCost(posStart_, pivot, current, quality, cost);
        if (g < existing->G) {
            existing->Index = triangleIndex;
            existing->Link = -1;
            // The stored pivot is the parent's, not the one costed above (as in the original).
            existing->Pivot = current->Pivot;
            existing->PortalLeft = -1;
            existing->PortalRight = -1;
            existing->Parent = current;
            existing->G = g;
            existing->H = ActiveVertex::Distance(posEnd_, pivot, quality);
            existing->F = WrapAdd(existing->G, existing->H);
            open_.Update(existing);
        }
        return;
    }

    ActiveVertex* vertex = GetNextActiveVertex();
    slot = vertex;
    const FPVector3 pivot =
        ActiveVertex::CreatePivot(triangleIndex, -1, -1, posStart_, posEnd_, current, quality, *navmesh_);
    vertex->Index = triangleIndex;
    vertex->Pivot = current->Pivot;
    vertex->Link = -1;
    vertex->PortalLeft = -1;
    vertex->PortalRight = -1;
    vertex->Parent = current;
    vertex->G = ActiveVertex::CalculateCost(posStart_, pivot, current, quality, cost);
    vertex->H = ActiveVertex::Distance(posEnd_, pivot, quality);
    vertex->F = WrapAdd(vertex->G, vertex->H);
    open_.Push(vertex);
}

// 0x7d284d8
FindPathResult PathFinder::FindPath(MailboxSet8* mailbox, FPVector3 start, FPVector3 end,
                                    bool automaticTargetCorrection,
                                    FP automaticTargetCorrectionRadius,
                                    NavMeshRegionMask regionMask, PathQuality quality,
                                    const NavMesh* navmesh) {
    if (navmesh != nullptr) {
        SetNavMesh(navmesh);
    }
    if (navmesh_ == nullptr) {
        throw std::logic_error("PathFinder::FindPath: no navmesh set");
    }
    const FP endRadius = automaticTargetCorrection ? automaticTargetCorrectionRadius : FP::Zero();
    verticesCount_ = 0;

    // Keep both points one unit inside the grid.
    std::int64_t lowX = 0;
    std::int64_t highX = 0;
    std::int64_t lowZ = 0;
    std::int64_t highZ = 0;
    GridClampBounds(navmesh_->GridSizeX, navmesh_->GridNodeSize, lowX, highX);
    GridClampBounds(navmesh_->GridSizeY, navmesh_->GridNodeSize, lowZ, highZ);
    start.X = FP::FromRaw(FPMath::Clamp(start.X.RawValue(), lowX, highX));
    start.Z = FP::FromRaw(FPMath::Clamp(start.Z.RawValue(), lowZ, highZ));
    end.X = FP::FromRaw(FPMath::Clamp(end.X.RawValue(), lowX, highX));
    end.Z = FP::FromRaw(FPMath::Clamp(end.Z.RawValue(), lowZ, highZ));

    popEnd_ = false;
    findDone_ = false;
    rawPathSize_ = 0;
    funneledPathSize_ = 0;
    open_.Clear();

    // The start is always searched with at least a quarter unit of tolerance.
    const FP startRadius = FP::FromRaw(std::max<std::int64_t>(FP::_0_25().RawValue(), endRadius.RawValue()));
    if (!navmesh_->FindClosestTriangle(start, startRadius, regionMask, triStart_, posStart_, mailbox)) {
        return FindPathResult::CreateError(FindPathResult::Code::InvalidStartPosition);
    }
    if (!navmesh_->FindClosestTriangle(end, endRadius, regionMask, triEnd_, posEnd_, mailbox)) {
        return FindPathResult::CreateError(FindPathResult::Code::InvalidEndPosition);
    }

    vertexEnd_ = GetNextActiveVertex();
    vertexEnd_->Slot = 0;
    vertexEnd_->Index = -2;
    vertexEnd_->Link = -1;
    vertexEnd_->Pivot = posEnd_;
    vertexEnd_->Parent = nullptr;
    vertexEnd_->PortalLeft = -1;
    vertexEnd_->PortalRight = -1;

    vertexStart_ = GetNextActiveVertex();
    vertexStart_->Slot = 0;
    vertexStart_->Index = -1;
    vertexStart_->Link = -1;
    vertexStart_->Pivot = posStart_;
    vertexStart_->Parent = nullptr;
    vertexStart_->PortalLeft = -1;
    vertexStart_->PortalRight = -1;

    if (triStart_ == triEnd_ ||
        navmesh_->LineOfSight(posStart_, posEnd_, regionMask, triStart_, triEnd_, true)) {
        // Straight line: the raw path is just the two points.
        PathAdd(posStart_, vertexStart_->Index);
        PathAdd(posEnd_, vertexEnd_->Index);
    } else {
        CreateOrUpdateVertex(triStart_, vertexStart_, quality);
        findDone_ = false;
    }

    if (!PathFindComplete(quality, regionMask)) {
        return FindPathResult::CreateError(FindPathResult::Code::PathTooLong);
    }
    Funnel(posStart_, posEnd_, regionMask, NavigationConstants::GenerateFunnelPathNormals);
    if (funneledPathSize_ < 2) {
        return FindPathResult::CreateError(FindPathResult::Code::NoPathFound);
    }
    return FindPathResult::CreateSuccess(posStart_, posEnd_);
}

void PathFinder::PushRawPath(const PathVertex& vertex) {
    Element(rawPath_, rawPathSize_) = vertex;
}

// 0x7d29160
bool PathFinder::PathFindComplete(PathQuality quality, NavMeshRegionMask regionMask) {
    bool result = true;
    if (!findDone_) {
        std::fill(closedSet_.begin(), closedSet_.end(), std::uint8_t{0});
        const NavMesh& navmesh = *navmesh_;

        // Grows the raw path by one entry; false when a fixed-size path is full.
        const auto advanceRawPath = [this]() {
            ++rawPathSize_;
            if (rawPathSize_ >= static_cast<int>(rawPath_.size())) {
                if (rawPathAllocationStrategy_ == RawPathAllocationStrategy::Fixed) {
                    return false;
                }
                if (rawPathAllocationStrategy_ == RawPathAllocationStrategy::Allocating) {
                    rawPath_.resize(rawPath_.size() * 2);
                }
            }
            return true;
        };

        while (open_.Size() > 0) {
            ActiveVertex* current = open_.Pop();
            SetClosed(current->Index);

            if (current->Index == triEnd_) {
                // Walk back from the goal, then reverse into start->goal order.
                findDone_ = true;
                PushRawPath(PathVertex{-1, posEnd_, FPVector3::Zero(), -1});
                ++rawPathSize_;
                bool fits = true;
                for (ActiveVertex* vertex = current; vertex != nullptr && fits;
                     vertex = vertex->Parent) {
                    PushRawPath(PathVertex{vertex->Index, vertex->Pivot, FPVector3::Zero(), vertex->Link});
                    if (vertex->Link >= 0 && vertex->Parent != nullptr) {
                        // The link's start point, in the triangle the link leaves from.
                        if (!advanceRawPath()) {
                            fits = false;
                            break;
                        }
                        PushRawPath(PathVertex{vertex->Parent->Index,
                                               Element(navmesh.Links, vertex->Link).Start,
                                               FPVector3::Zero(), vertex->Link});
                    }
                    if (!advanceRawPath()) {
                        fits = false;
                    }
                }
                if (!fits) {
                    rawPathSize_ = 0;
                    result = false;
                    break;
                }
                std::reverse(rawPath_.begin(), rawPath_.begin() + rawPathSize_);
                if (popEnd_) {
                    --rawPathSize_;
                }
                break;
            }
            if (findDone_) {
                continue;
            }

            const NavMeshTriangle& triangle = Element(navmesh.Triangles, current->Index);

            // Portals to neighbouring triangles, last to first.
            for (int i = static_cast<int>(triangle.Neighbors.size()) - 1; i >= 0; --i) {
                const NavMeshPortal& portal = Element(triangle.Neighbors, i);
                const int neighbor = portal.Neighbor;
                if (!Element(navmesh.Triangles, neighbor).Regions.IsSubset(regionMask) ||
                    IsClosed(neighbor)) {
                    continue;
                }
                ActiveVertex*& slot = Element(active_, neighbor);
                if (slot != nullptr) {
                    ActiveVertex* existing = slot;
                    if (existing->PortalLeft == portal.LeftVertex &&
                        existing->PortalRight == portal.RightVertex) {
                        continue;
                    }
                    const FPVector3 pivot =
                        ActiveVertex::CreatePivot(neighbor, portal.LeftVertex, portal.RightVertex,
                                                  posStart_, posEnd_, current, quality, navmesh);
                    const std::int64_t g = ActiveVertex::CalculateCost(posStart_, pivot, current,
                                                                       quality, triangle.Cost);
                    const std::int64_t h = ActiveVertex::Distance(posEnd_, pivot, quality);
                    const std::int64_t f = WrapAdd(h, g);
                    if (f < existing->F) {
                        existing->Index = neighbor;
                        existing->Link = -1;
                        existing->Parent = current;
                        existing->Pivot = pivot;
                        existing->PortalLeft = -1;
                        existing->PortalRight = -1;
                        existing->G = g;
                        existing->H = h;
                        existing->F = f;
                        open_.Update(existing);
                    }
                    continue;
                }
                ActiveVertex* vertex = GetNextActiveVertex();
                slot = vertex;
                const FPVector3 pivot =
                    ActiveVertex::CreatePivot(neighbor, portal.LeftVertex, portal.RightVertex,
                                              posStart_, posEnd_, current, quality, navmesh);
                vertex->Slot = 0;
                vertex->Index = neighbor;
                vertex->Parent = current;
                vertex->Pivot = pivot;
                vertex->Link = -1;
                vertex->PortalLeft = -1;
                vertex->PortalRight = -1;
                vertex->G = ActiveVertex::CalculateCost(posStart_, pivot, current, quality, triangle.Cost);
                vertex->H = ActiveVertex::Distance(posEnd_, pivot, quality);
                vertex->F = WrapAdd(vertex->G, vertex->H);
                open_.Push(vertex);
            }

            // Off-mesh links starting here, last to first. Already reached targets are
            // never re-costed through a link.
            for (int i = static_cast<int>(triangle.Links.size()) - 1; i >= 0; --i) {
                const int linkIndex = Element(triangle.Links, i);
                const NavMeshLink& link = Element(navmesh.Links, linkIndex);
                if (!link.Region.IsSubset(regionMask)) {
                    continue;
                }
                const int target = link.Triangle;
                if (!Element(navmesh.Triangles, target).Regions.IsSubset(regionMask) ||
                    IsClosed(target) || Element(active_, target) != nullptr) {
                    continue;
                }
                ActiveVertex* vertex = GetNextActiveVertex();
                Element(active_, target) = vertex;
                vertex->Slot = 0;
                vertex->Index = target;
                vertex->PortalLeft = -1;
                vertex->PortalRight = -1;
                vertex->Parent = current;
                vertex->Pivot = link.End;
                vertex->Link = linkIndex;
                const FP costFactor = link.CostOverride < FP::Zero() ? triangle.Cost : link.CostOverride;
                vertex->G = ActiveVertex::CalculateCost(posStart_, link.End, current, quality, costFactor);
                vertex->H = ActiveVertex::Distance(posEnd_, vertex->Pivot, quality);
                vertex->F = WrapAdd(vertex->G, vertex->H);
                open_.Push(vertex);
            }
        }
    }

    vertexStart_ = nullptr;
    vertexEnd_ = nullptr;
    std::fill(active_.begin(), active_.end(), nullptr);
    verticesCount_ = 0;
    return result;
}

// 0x7d267d8
void PathFinder::Funnel(const FPVector3& start, const FPVector3& end, NavMeshRegionMask regionMask,
                        bool calculateNormals) {
    if (rawPathSize_ < 2) {
        return;
    }
    if (rawPathSize_ == 2) {
        Element(funnel_, 0) = Element(rawPath_, 0);
        Element(funnel_, 1) = Element(rawPath_, 1);
        funneledPathSize_ = rawPathSize_;
        return;
    }

    const NavMesh& navmesh = *navmesh_;
    const auto ensureCapacity = [this]() {
        if (funnelIndex_ + 1 >= static_cast<int>(funnelLeft_.size())) {
            funnelLeft_.resize(funnelLeft_.size() * 2);
            funnelRight_.resize(funnelRight_.size() * 2);
        }
    };

    // Portal 0 is the start point. (The left node's RawPathIndex is never written; it
    // stays 0.)
    funnelIndex_ = 0;
    Element(funnelLeft_, 0).Point = start;
    Element(funnelLeft_, 0).Normal = FPVector3::Zero();
    Element(funnelRight_, 0) = FunnelNode{start, FPVector3::Zero(), 0};
    funnelIndex_ = 1;

    // One portal per interior raw path point: the edge into the next triangle, or the
    // point itself (both sides) for a link point.
    for (int i = 1; i < rawPathSize_ - 1; ++i) {
        const PathVertex& rawPoint = Element(rawPath_, i);
        const NavMeshTriangle& triangle = Element(navmesh.Triangles, rawPoint.Index);
        if (triangle.Neighbors.empty()) {
            continue;
        }
        if (rawPoint.Link >= 0) {
            ensureCapacity();
            Element(funnelLeft_, funnelIndex_) = FunnelNode{rawPoint.Point, FPVector3::Zero(), i};
            Element(funnelRight_, funnelIndex_) = FunnelNode{rawPoint.Point, FPVector3::Zero(), i};
            ++funnelIndex_;
            continue;
        }
        const int nextTriangle = Element(rawPath_, i + 1).Index;
        const auto portal =
            std::find_if(triangle.Neighbors.begin(), triangle.Neighbors.end(),
                         [nextTriangle](const NavMeshPortal& p) { return p.Neighbor == nextTriangle; });
        if (portal == triangle.Neighbors.end()) {
            continue;
        }
        ensureCapacity();
        FunnelNode& left = Element(funnelLeft_, funnelIndex_);
        FunnelNode& right = Element(funnelRight_, funnelIndex_);
        left.Point = Element(navmesh.Vertices, portal->LeftVertex).Point;
        left.RawPathIndex = i;
        right.Point = Element(navmesh.Vertices, portal->RightVertex).Point;
        right.RawPathIndex = i;
        if (calculateNormals) {
            left.Normal = NavMeshVertex::CalculateNormal(portal->LeftVertex, navmesh, regionMask);
            right.Normal = NavMeshVertex::CalculateNormal(portal->RightVertex, navmesh, regionMask);
        } else {
            left.Normal = FPVector3::Zero();
            right.Normal = FPVector3::Zero();
        }
        ++funnelIndex_;
    }

    // Final portal: the end point.
    Element(funnelLeft_, funnelIndex_) = FunnelNode{end, FPVector3::Zero(), rawPathSize_ - 1};
    Element(funnelRight_, funnelIndex_) = FunnelNode{end, FPVector3::Zero(), rawPathSize_ - 1};
    ++funnelIndex_;

    int nextFunnelNode = 1;
    Element(funnel_, 0) = Element(rawPath_, 0);
    FillFunneledPath(Element(funnelLeft_, 0).Point, nextFunnelNode, 0);
    funneledPathSize_ = CleanupFunnel(funnel_, funneledPathSize_);
}

// 0x7d27eec
int PathFinder::CleanupFunnel(std::vector<PathVertex>& funnel, int size) {
    if (size < 2) {
        return size;
    }
    const int originalSize = size;
    int write = 1;
    for (int i = 1; i < originalSize; ++i) {
        PathVertex& previous = Element(funnel, i - 1);
        const PathVertex& current = Element(funnel, i);
        if (current.Point == previous.Point) {
            // Duplicate: drop it, keeping its link on the (unmoved) previous entry.
            if (current.Link >= 0) {
                previous.Link = current.Link;
                previous.Index = current.Index;
            }
            --size;
        } else {
            if (i != write) {
                Element(funnel, write) = current;
            }
            ++write;
        }
    }
    return size;
}

// 0x7d27774
void PathFinder::FillFunneledPath(FPVector3 apex, int& nextFunnelNode, int startFunnelIndex) {
    // Simple stupid funnel algorithm (string pulling) over the portals, in XZ.
    FPVector3 portalLeft = Element(funnelLeft_, startFunnelIndex).Point;
    FPVector3 portalRight = Element(funnelRight_, startFunnelIndex).Point;
    int leftIndex = 0;
    int rightIndex = 0;
    const int portalCount = funnelIndex_;

    // Emits a funnel corner: the point with the raw path entry's triangle and link.
    const auto emitCorner = [this, &nextFunnelNode](const FPVector3& point, const FunnelNode& node) {
        const PathVertex& raw = Element(rawPath_, node.RawPathIndex);
        Element(funnel_, nextFunnelNode) = PathVertex{raw.Index, point, node.Normal, raw.Link};
        ++nextFunnelNode;
    };

    for (int i = 1; i < portalCount; ++i) {
        if (nextFunnelNode >= NavigationConstants::MaxFunnelPathSize) {
            break;
        }

        // Both sides were set at the previous portal: if that was a link point, emit it
        // and restart the funnel there.
        if (leftIndex >= i - 1 && rightIndex >= i - 1) {
            const FunnelNode* restartAt = nullptr;
            const FunnelNode& rightNode = Element(funnelRight_, rightIndex);
            if (Element(rawPath_, rightNode.RawPathIndex).Link >= 0) {
                restartAt = &rightNode;
            } else {
                const FunnelNode& leftNode = Element(funnelLeft_, leftIndex);
                if (Element(rawPath_, leftNode.RawPathIndex).Link >= 0) {
                    restartAt = &leftNode;
                }
            }
            if (restartAt != nullptr) {
                Element(funnel_, nextFunnelNode++) = Element(rawPath_, restartAt->RawPathIndex);
                apex = restartAt->Point;
                portalLeft = apex;
                portalRight = apex;
                leftIndex = i;
                rightIndex = i;
                continue;
            }
        }

        const FPVector3 right = Element(funnelRight_, i).Point;
        const FPVector3 left = Element(funnelLeft_, i).Point;

        // Right side.
        FPVector3 newRight = portalRight;
        if (CalculateTriangleArea(apex, portalRight, right) <= 0) {
            if (apex == portalRight || CalculateTriangleArea(apex, portalLeft, right) > 0) {
                newRight = right; // tighten
                rightIndex = i;
            } else {
                // Right crossed over left: left becomes a corner and the new apex.
                emitCorner(portalLeft, Element(funnelLeft_, leftIndex));
                apex = portalLeft;
                portalRight = portalLeft;
                rightIndex = leftIndex;
                i = leftIndex;
                continue;
            }
        }

        // Left side.
        if (CalculateTriangleArea(apex, portalLeft, left) >= 0) {
            if (apex == portalLeft || CalculateTriangleArea(apex, newRight, left) < 0) {
                portalLeft = left; // tighten
                leftIndex = i;
            } else {
                // Left crossed over right: right becomes a corner and the new apex.
                emitCorner(newRight, Element(funnelRight_, rightIndex));
                apex = newRight;
                portalLeft = newRight;
                portalRight = newRight;
                leftIndex = rightIndex;
                i = rightIndex;
                continue;
            }
        }
        portalRight = newRight;
    }

    if (nextFunnelNode < NavigationConstants::MaxFunnelPathSize) {
        Element(funnel_, nextFunnelNode) = Element(rawPath_, rawPathSize_ - 1);
        ++nextFunnelNode;
    }
    funneledPathSize_ = nextFunnelNode;
}

} // namespace Quantum
