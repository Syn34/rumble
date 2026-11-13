// path_finder.h - Quantum.Navigation's PathFinder and its helpers (quantum.core.dll):
// PathFinder, ActiveVertex, ActiveHeap, PathVertex, FunnelNode and FindPathResult.
//
// Pathfinding runs A* over the navmesh triangles (crossing portals and off-mesh links),
// producing a "raw path" of one point per visited triangle, then string-pulls it with a
// funnel algorithm in the XZ plane. Every step uses FP maths in the original's order.
//
// The original allocates its pools from a native allocator and takes a frame for
// thread-local lookups and pool growth; here the pools are owned by the object and
// FindPath takes the thread's MailboxSet8 directly.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"
#include "quantum_core/navigation/navmesh_agent_config.h"
#include "quantum_core/navigation/navmesh_region_mask.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Quantum {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;

class MailboxSet8;
class NavMesh;

// One point of a raw or funnelled path.
struct PathVertex {
    std::int32_t Index = 0; // triangle index (-1 for the start/end point)
    FPVector3 Point;
    FPVector3 Normal;
    std::int32_t Link = -1; // off-mesh link this point belongs to, or -1
};

// One side of a funnel portal.
struct FunnelNode {
    FPVector3 Point;
    FPVector3 Normal;
    std::int32_t RawPathIndex = 0;
};

struct FindPathResult {
    enum class Code : std::int32_t {
        Success = 0,
        InvalidStartPosition = 1,
        InvalidEndPosition = 2,
        NoPathFound = 3,
        PathTooLong = 4,
    };

    Code Result = Code::Success;
    FPVector3 CorrectedStartPosition;
    FPVector3 CorrectedEndPosition;
    // IPathFinderData _pathFinderData: always null in this build.

    // 0x7cfcfc8
    static FindPathResult CreateSuccess(const FPVector3& correctedStartPosition,
                                        const FPVector3& correctedEndPosition);
    // 0x7cfd038
    static FindPathResult CreateError(Code errorCode);
};

// An A* node: one per triangle reached. Costs are raw FP values.
struct ActiveVertex {
    std::int32_t Slot = 0;  // position in the open heap
    std::int32_t Index = 0; // triangle index
    ActiveVertex* Parent = nullptr;
    FPVector3 Pivot; // point the path passes through in this triangle
    std::int64_t F = 0;
    std::int64_t G = 0;
    std::int64_t H = 0;
    std::int32_t PortalLeft = -1;
    std::int32_t PortalRight = -1;
    std::int32_t Link = -1;

    // 0x7d2b268
    std::string ToString() const { return std::to_string(Index); }
    // 0x7d2b274
    std::int32_t GetHashCode() const { return Index; }
    // 0x7d2b27c - Equals(object): same triangle index.
    bool Equals(const ActiveVertex& other) const { return Index == other.Index; }

    // 0x7d28c4c - starts a vertex without parent (F/G/H untouched).
    void Create(int index, const FPVector3& pivot);
    // 0x7d2a36c - the portal and quality arguments are not stored: portals become -1.
    void Create(int index, const FPVector3& pivot, int portalLeft, int portalRight,
                ActiveVertex* parent, PathQuality quality, int slot);

    // 0x7d2a494 - Manhattan distance for Fast/Good, Euclidean for Best.
    static std::int64_t Distance(const FPVector3& a, const FPVector3& b, PathQuality quality);

    // 0x7d29de0 - where the path crosses into `index`: the triangle centre (Fast, or no
    // portal), the point on the portal closest to the parent (Good), or where the
    // parent->goal line crosses the portal / the portal end nearer the goal (Best).
    static FPVector3 CreatePivot(int index, int portalLeft, int portalRight, const FPVector3& start,
                                 const FPVector3& goal, const ActiveVertex* parent,
                                 PathQuality quality, const NavMesh& navmesh);

    // 0x7d2a398 - G of a vertex: parent's G plus the distance to it, scaled by
    // `costFactor` unless that is <= 0 or exactly 1.
    static std::int64_t CalculateCost(const FPVector3& start, const FPVector3& pivot,
                                      const ActiveVertex* parent, PathQuality quality,
                                      FP costFactor);

    // 0x7d2b2fc - XZ closest point on segment v0-v1 (Y is 0 unless an end point is
    // returned as is).
    static FPVector3 ClosestPointOnSegment(const FPVector3& pt, const FPVector3& v0,
                                           const FPVector3& v1);

    // 0x7d2b438 - XZ intersection of segments p1-p2 and q1-q2 (Y of `point` is 0).
    static bool LineIntersectsLine(const FPVector3& p1, const FPVector3& p2, const FPVector3& q1,
                                   const FPVector3& q2, FPVector3& point);
};

// Binary min-heap of ActiveVertex by F, 1-based (slot 0 unused).
class ActiveHeap {
  public:
    // 0x7d28240
    explicit ActiveHeap(int capacity);

    // 0x7d2a60c
    int Size() const { return size_ - 1; }

    // 0x7d28ab4
    void Clear();
    // 0x7d2a5a0 - restores the heap after `node`'s F decreased.
    void Update(ActiveVertex* node);
    // 0x7d2a4fc
    void Push(ActiveVertex* node);
    // 0x7d29d24 - removes the lowest-F vertex.
    ActiveVertex* Pop();
    // 0x7d2b50c - doubles the capacity.
    void ExpandHeap();
    // 0x7d284a4
    void Free();

  private:
    void SiftUp(ActiveVertex* node, int slot);

    int size_ = 1;                    // _size: next free slot
    int heapLength_ = 0;              // _heapLength
    std::vector<ActiveVertex*> heap_; // _heap
};

class PathFinder {
  public:
    // 0x7d28034
    PathFinder(int openNodesSize, int rawPathSize,
               RawPathAllocationStrategy rawPathAllocationStrategy);

    // 0x7d267c8
    int PathSize() const { return funneledPathSize_; }
    // 0x7d267d0 - the funnelled path (first PathSize() entries are valid).
    const std::vector<PathVertex>& Path() const { return funnel_; }

    // Raw (per-triangle) path of the last search.
    int RawPathSize() const { return rawPathSize_; }
    const std::vector<PathVertex>& RawPath() const { return rawPath_; }

    // 0x7d282ac - grows the per-triangle tables to `size` triangles.
    void Resize(int size);
    // 0x7d2835c
    void SetNavMesh(const NavMesh* navmesh);
    // 0x7d283b0
    void Dispose();

    // 0x7d284d8 - finds a path; the result's corrected positions are the start/end
    // snapped onto the navmesh. `mailbox` is the calling thread's (Navigation.ThreadLocal)
    // mailbox, sized for the navmesh; the original takes the frame and looks it up.
    FindPathResult FindPath(MailboxSet8* mailbox, FPVector3 start, FPVector3 end,
                            bool automaticTargetCorrection, FP automaticTargetCorrectionRadius,
                            NavMeshRegionMask regionMask, PathQuality quality,
                            const NavMesh* navmesh);

    // 0x7d27ff8 - twice the signed XZ area of a, b, c (raw); positive when c is to the
    // right of a->b (a, b, c turn clockwise with X right and Z up).
    static std::int64_t CalculateTriangleArea(const FPVector3& a, const FPVector3& b,
                                              const FPVector3& c);

  private:
    // 0x7d267d8
    void Funnel(const FPVector3& start, const FPVector3& end, NavMeshRegionMask regionMask,
                bool calculateNormals);
    // 0x7d27eec - drops consecutive duplicate points; returns the new size.
    static int CleanupFunnel(std::vector<PathVertex>& funnel, int size);
    // 0x7d27774
    void FillFunneledPath(FPVector3 apex, int& nextFunnelNode, int startFunnelIndex);

    // 0x7d29160 - runs A* to completion; false only if the raw path did not fit.
    bool PathFindComplete(PathQuality quality, NavMeshRegionMask regionMask);
    // 0x7d28c78
    bool PathAdd(const FPVector3& point, int index);
    // 0x7d28dcc
    void CreateOrUpdateVertex(int triangleIndex, ActiveVertex* current, PathQuality quality);
    // 0x7d2a618
    ActiveVertex* GetActiveVertexFromIndex(int index);
    // 0x7d28b24 - next pooled vertex, cleared, with Index set to its pool index.
    ActiveVertex* GetNextActiveVertex();

    bool IsClosed(int triangle) const;
    void SetClosed(int triangle);
    void PushRawPath(const PathVertex& vertex);

    std::vector<FunnelNode> funnelLeft_;                          // _funnelLeft
    std::vector<FunnelNode> funnelRight_;                         // _funnelRight
    int funnelIndex_ = 0;                                         // _funnelIndex
    std::vector<PathVertex> funnel_;                              // _funnel
    int funneledPathSize_ = 0;                                    // _funneledPathSize
    ActiveHeap open_;                                             // _open
    std::vector<ActiveVertex*> active_;                           // _active: vertex per triangle
    std::vector<std::unique_ptr<ActiveVertex[]>> verticesBlocks_; // _verticesBlocks
    int verticesCount_ = 0;                                       // _verticesCount
    std::vector<std::uint8_t> closedSet_; // _closedSet: one bit per triangle
    int rawPathSize_ = 0;                 // _rawPathSize
    std::vector<PathVertex> rawPath_;     // _rawPath
    RawPathAllocationStrategy rawPathAllocationStrategy_;
    FPVector3 posStart_;                  // pos_start
    FPVector3 posEnd_;                    // pos_end
    int triStart_ = 0;                    // tri_start
    int triEnd_ = 0;                      // tri_end
    ActiveVertex* vertexStart_ = nullptr; // vertex_start
    ActiveVertex* vertexEnd_ = nullptr;   // vertex_end
    bool popEnd_ = false;                 // pop_end
    bool findDone_ = false;               // find_done
    const NavMesh* navmesh_ = nullptr;
};

} // namespace Quantum
