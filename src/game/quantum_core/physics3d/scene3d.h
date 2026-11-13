// scene3d.h - the 3D physics scene of quantum.core: Quantum.Physics3D.Scene3D with its
// broad-phase buckets (Bucket3D), per-thread scratch (Tls), islands (Island3D) and the
// sweep-and-prune helpers ActiveEntryList / StabbingList / EndPoint.
//
// The original runs every step as a graph of tasks over a thread pool; each task's items
// are independent, so running the tasks one after another on one thread (as here) gives
// the same result. Entry storage keeps the original's order: the map's static colliders
// first (kept between steps), then the entries of broad-phase queries, then the entities'
// colliders. Every per-bucket manifold array reserves index 0, as in the original.
//
// Not reimplemented yet (TODOs in scene3d.cpp): static meshes (TriangleMesh), broad-phase
// queries resolved during the step, compound shapes, joints, CCD time-of-impact
// integration and the hybrid Jacobi/Gauss-Seidel solver.
#pragma once

#include "quantum_core/physics3d/collision_manifold3d.h"
#include "quantum_core/physics3d/gjk3d.h"
#include "quantum_core/physics3d/hit3d.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/physics_scene_settings.h"
#include "quantum_core/asset_guid.h"

#include <cstdint>
#include <vector>

namespace Quantum {
class Map;
struct PhysicsCallbacksData;
namespace Core {
class FrameBase;
}
} // namespace Quantum

namespace Quantum::Physics3D {

// EndPoint: one end of an entry's broad-phase interval on the sort axis. Index carries the
// entry index and the category flags.
struct EndPoint {
    static constexpr std::int32_t MIN_FLAG = static_cast<std::int32_t>(0x80000000u);
    static constexpr std::int32_t STATIC_FLAG = 0x40000000;
    static constexpr std::int32_t TRIGGER_FLAG = 0x20000000;
    static constexpr std::int32_t KINEMATIC_FLAG = 0x10000000;
    static constexpr std::int32_t QUERY_FLAG = 0x08000000;
    static constexpr std::int32_t ALL_FLAGS = static_cast<std::int32_t>(0xf8000000u);

    std::int32_t Value = 0;
    std::int32_t Index = 0;
};

// An entry in a sweep's active set with its interval on the secondary axis.
struct ActiveEntry {
    std::int32_t Index = 0;
    std::int32_t Min = 0;
    std::int32_t Max = 0;
};

// ActiveEntryList: the entries whose interval is open during the sweep. Removal swaps the
// last item in; IndexLookup maps entry index -> position.
struct ActiveEntryList {
    std::vector<ActiveEntry> Items;
    std::vector<std::int32_t> IndexLookup;
    std::int32_t Capacity = 0;
    std::int32_t Count = 0;
    std::int32_t CountUnsorted = 0;

    // 0x7d2fbe4 - appends; IndexLookup[entry.Index] = position. CountUnsorted follows
    // Count unless `sorted`.
    void Add(const ActiveEntry& entry, bool sorted);
    // 0x7d2fb0c
    void Remove(std::int32_t entryIndex);
    // 0x7d2fc24 - copies the items and lookup of `from`.
    void CopyFrom(const ActiveEntryList& from);
    // 0x7d2fdd8 - appends the items of `from` (lookup entries offset by the current count).
    void AddRange(const ActiveEntryList& from);
    // 0x7d2fcfc - capacity rounded up to a power of two; empties the list.
    void ResetAndEnsureCapacity(std::int32_t capacity);
    // 0x7d2ffb8
    void ResetCount() { Count = CountUnsorted = 0; }
};

// StabbingList: snapshots of the dynamic active set taken at points along the sweep, used
// to start query sweeps part-way through a bucket.
struct StabbingList {
    std::vector<ActiveEntryList> Lists;
    std::vector<std::int32_t> StabbingNumbers;
    std::int32_t Capacity = 0;
    std::int32_t Count = 0;
    std::int32_t EntriesCount = 0;
    std::int32_t EntriesPerList = 0;

    // 0x7d30068
    ActiveEntryList* GetNextEntryList(std::int32_t stabbingNumber);
    // 0x7d3008c - the last list whose stabbing number is <= the point (-1 if none).
    std::int32_t Stab(std::int32_t stabbingPoint) const;
    // 0x7d300f0
    void ResetAndEnsureCapacity(std::int32_t capacity, std::int32_t listCapacity);
};

// Quantum.Physics3D.QueryManifold3D: a candidate entry (or mesh triangle) of a query.
struct QueryManifold3D {
    std::int32_t Entry = 0;
    std::int32_t NextInBucket = 0;
    std::int32_t TriIndex = 0;
};

// Quantum.Physics3D.Bucket3D: one slice of the world along the bucket axis.
struct Bucket3D {
    std::int32_t Min = 0;
    std::int32_t Max = 0;
    std::int32_t Index = 0;
    std::vector<EndPoint> EndPoints;
    std::int32_t EndPointsCount = 0; // pairs of endpoints = entries inserted
    std::vector<EndPoint> EndPointsStatic;
    std::int32_t EndPointsStaticCount = 0;
    StabbingList StaticsStabbingList;
    StabbingList DynamicsStabbingList;
    std::vector<CollisionManifold3D> CollisionManifolds; // index 0 reserved
    std::int32_t CollisionManifoldsProcessCount = 1;
    std::vector<CollisionManifold3D*> SolverManifolds;
    std::vector<ManifoldData3D> TriggerManifolds; // index 0 reserved
    std::int32_t TriggerManifoldsProcessCount = 1;
    std::vector<QueryManifold3D> QueryManifolds; // index 0 reserved

    // 0x7d798d0 - room for both endpoints of every entry and query.
    void EnsureEndPointsCapacity(std::int32_t entryCapacity, std::int32_t queryCapacity);
    // 0x7d79988
    void EnsureEndPointsStaticCapacity(std::int32_t capacity);
    // 0x7d79a10 - appends the pair (the original bumps the count atomically).
    void InsertEndPoints(EndPoint min, EndPoint max);
    // 0x7d79a58
    void InsertEndPointsStatic(EndPoint min, EndPoint max);
    // 0x7d79a84 - a zeroed manifold tagged with this bucket's index.
    CollisionManifold3D* GetNextCollisionManifold();
    // 0x7d79cb4
    QueryManifold3D* GetNextQueryManifold(std::int32_t& index);
    // 0x7d79d64 / 0x7d79db8 - the next unprocessed manifold index.
    std::int32_t GetNextCollisionManifoldToProcess() { return CollisionManifoldsProcessCount++; }
    std::int32_t GetNextTriggerManifoldToProcess() { return TriggerManifoldsProcessCount++; }
    // TODO(0x7d79b34, Quantum.Physics3D.Bucket3D.GetNextMeshCollisionManifold),
    // TODO(0x7d79bf4, Quantum.Physics3D.Bucket3D.GetNextTriangleCollisionManifold),
    // TODO(0x7d79d80, Quantum.Physics3D.Bucket3D.GetNextMeshManifoldToProcess),
    // TODO(0x7d79d9c, Quantum.Physics3D.Bucket3D.GetNextTriangleManifoldToProcess): static
    // mesh manifolds; need TriangleMesh.
};

// Quantum.Physics3D.Tls: per-thread scratch of the scene - the sweep's active lists and
// the narrow phase's GJK scratch (NarrowTls3D).
struct Tls {
    ActiveEntryList QueryAll;
    ActiveEntryList QueryDynamic;
    ActiveEntryList QueryKinematic;
    ActiveEntryList QueryKinematicTrigger;
    ActiveEntryList QueryStatic;
    ActiveEntryList QueryStaticTrigger;
    ActiveEntryList Dynamic;
    ActiveEntryList Kinematic;
    ActiveEntryList KinematicTrigger;
    ActiveEntryList Static;
    ActiveEntryList StaticTrigger;
    NarrowTls3D Narrow;
    std::uint64_t OverlapSequence = 0;
    // TODO(0x7d7d5e8, Quantum.Physics3D.Tls.Free), TODO(0x7d7d624,
    // Quantum.Physics3D.Tls.GetTrianglesMailboxes), TODO(0x7d7d63c,
    // Quantum.Physics3D.Tls.EnsureMailboxesCapacity): triangle mailboxes for static meshes.
};

// Quantum.Physics3D.Island3D: bodies connected by contacts, solved and put to sleep
// together.
struct Island3D {
    bool Ignore = false;
    bool VelocityConstraintsSolved = false;
    bool PositionConstraintsSolved = false;
    std::vector<CollisionManifold3D*> CollisionManifolds;
    std::vector<Entry3D*> Entries;

    // 0x7d7b1b8
    void Reset();
    // 0x7d7b204
    void AddManifold(CollisionManifold3D* manifold) { CollisionManifolds.push_back(manifold); }
    // 0x7d7b294 - also links the entry to the island.
    void AddEntry(Entry3D* entry);
};

// The collision/trigger callbacks the scene raises (Quantum.Core.ICollisionCallbacks3D).
// Info structs reduced to what callers use.
struct CollisionInfo3D {
    EntityRef Entity;
    EntityRef Other; // invalid for static colliders
    bool IsStatic = false;
    std::int32_t StaticColliderIndex = -1;
    FPVector3 ContactNormal;
    const CollisionManifold3D* Manifold = nullptr;
};
struct TriggerInfo3D {
    EntityRef Entity;
    EntityRef Other;
    bool IsStatic = false;
    std::int32_t StaticColliderIndex = -1;
};
struct ExitInfo3D {
    EntityRef Entity;
    EntityRef Other;
    bool IsStatic = false;
    std::int32_t StaticColliderIndex = -1;
};
class ICollisionCallbacks3D {
  public:
    virtual ~ICollisionCallbacks3D() = default;
    virtual void OnCollision3D(Core::FrameBase* f, const CollisionInfo3D& info) = 0;
    virtual void OnCollisionEnter3D(Core::FrameBase* f, const CollisionInfo3D& info) = 0;
    virtual void OnCollisionExit3D(Core::FrameBase* f, const ExitInfo3D& info) = 0;
    virtual void OnTrigger3D(Core::FrameBase* f, const TriggerInfo3D& info) = 0;
    virtual void OnTriggerEnter3D(Core::FrameBase* f, const TriggerInfo3D& info) = 0;
    virtual void OnTriggerExit3D(Core::FrameBase* f, const ExitInfo3D& info) = 0;
};

// Quantum.QueryOptions
enum QueryOptions : std::int32_t {
    HitStatics = 1,
    HitKinematics = 2,
    HitDynamics = 4,
    HitTriggers = 8,
    HitAll = 15,
    ComputeDetailedInfo = 32768,
};

// Quantum.Physics3D.StaticComponents3D: the transform and collider of a static entry.
struct StaticComponents3D {
    ::Quantum::Transform3D Transform3D;
    PhysicsCollider3D Collider3D;
};

class Scene3D {
  public:
    static constexpr int MIN_STATIC_CAPACITY = 64;
    static constexpr int MIN_ENTRY_CAPACITY = 32;
    static constexpr int MIN_QUERY_CAPACITY = 32;
    static constexpr int MIN_ISLAND_CAPACITY = 16;
    static constexpr int MIN_MANIFOLD_CAPACITY = 32;
    static constexpr int MIN_BUCKETS_CAPACITY = 4;
    static constexpr CallbackFlags StaticTriggerCallbacks = static_cast<CallbackFlags>(3584);
    static constexpr CallbackFlags DynamicTriggerCallbacks = static_cast<CallbackFlags>(448);

    // 0x7d591a4 Allocate - here the constructor.
    Scene3D();

    // 0x7d5b2f8 - default world (one bucket) when no map is set, then ResetScene and
    // UpdateMap.
    void InitScene(Core::FrameBase* frame, AssetGuid mapGuid);
    // 0x7d5b354 - caches the frame's settings and clears the previous step's dynamic data.
    void ResetScene(Core::FrameBase* frame);
    // 0x7d5a3ec - runs one step (see the task order in scene3d.cpp).
    void Update(Core::FrameBase* frame, FP deltaTime, ICollisionCallbacks3D* callbacks);

    // 0x7d5a060 / 0x7d5a080
    Entry3D* DynamicEntries() { return _entries.data() + _staticComponentsCount; }
    std::int32_t DynamicEntriesCount() const {
        return static_cast<std::int32_t>(_entries.size()) - _staticComponentsCount;
    }
    // 0x7d5a120 - null for an index out of range.
    Entry3D* GetEntry(std::int32_t index);
    // 0x7d5a1e4
    Entry3D* GetEntryUnchecked(std::int32_t index) { return &_entries[static_cast<std::size_t>(index)]; }
    // 0x7d5a20c
    FP DeltaTime() const { return _deltaTime; }
    std::int32_t EntriesCount() const { return static_cast<std::int32_t>(_entries.size()); }
    std::int32_t StaticEntriesCount() const { return _staticComponentsCount; }
    const PhysicsSceneSettings& Settings() const { return CachedSettings; }
    AssetGuid CurrentMap() const { return _currentMap; } // 0x7d5a09c

    // 0x7d5a214 - bucket of a point on the bucket axis (unclamped).
    std::int32_t GetPointBucket(const FPVector3& point) const;
    // 0x7d5a250 - inserts both endpoints into every bucket the interval spans.
    void InsertEndPoints(const IntAABB& aabb, std::int32_t index, std::int32_t& minBucket,
                         std::int32_t& maxBucket);

    // Immediate queries (see scene3d.cpp for how they differ from the original's).
    void Linecast(Core::FrameBase* frame, HitCollection3D* hits, const FPVector3& start,
                  const FPVector3& end, bool firstHitOnly, std::int32_t layerMask,
                  std::int32_t options);
    void OverlapShape(Core::FrameBase* frame, HitCollection3D* hits, const FPVector3& pos,
                      const FPQuaternion& rot, const Shape3D& shape, std::int32_t layerMask,
                      std::int32_t options);

  private:
    // Step stages (the original's Task_* functions).
    void CreateEntries(Core::FrameBase* frame);
    void UpdateMap(Core::FrameBase* frame, AssetGuid mapGuid);
    void ResetBucketsAndTls();
    void TaskIntegrateForcesFirstHalf();
    void TaskBroadPhaseEntryPreStep();
    void TaskBroadPhase(Core::FrameBase* frame);
    void EntryCheckEntryList(const ActiveEntryList& list, std::int32_t bucket, Entry3D* entry,
                             bool isTriggerManifold, bool isSleeping, bool isKinematic,
                             bool isStatic);
    void TaskNarrowPhase();
    void TaskCallbacks(Core::FrameBase* frame, ICollisionCallbacks3D* callbacks);
    void HandleDynamicTrigger(Core::FrameBase* f, ICollisionCallbacks3D* callbacks,
                              ManifoldData3D* manifold, Entry3D* entryA, Entry3D* entryB);
    void HandleStaticTrigger(Core::FrameBase* f, ICollisionCallbacks3D* callbacks,
                             ManifoldData3D* manifold, Entry3D* entryA, Entry3D* entryB);
    void HandleDynamicCollision(Core::FrameBase* f, ICollisionCallbacks3D* callbacks,
                                CollisionManifold3D* manifold, Entry3D* entryA, Entry3D* entryB);
    void HandleStaticCollision(Core::FrameBase* f, ICollisionCallbacks3D* callbacks,
                               CollisionManifold3D* manifold, Entry3D* entryA, Entry3D* entryB);
    void HandleExits(Core::FrameBase* f, ICollisionCallbacks3D* callbacks);
    void TaskBuildIslands();
    void TaskCollisionSolverInitLegacyGS(Core::FrameBase* frame);
    void TaskVelocitySolverLegacyGS();
    void TaskPenetrationCorrection();
    void TaskIntegrateVelocity();
    void TaskSleepDetection();

    Entry3D* GetNextEntry(std::int32_t& index);
    void UpdateManifoldCachedState(ManifoldData3D& m) const;
    Entry3D* Root(Entry3D* entry);

  public:
    // Original field names; containers replace the native buffers.
    std::vector<Bucket3D> _buckets;
    FP::Raw _bucketsSizeRaw = 0;
    std::int32_t _bucketsSubdivisions = 1;
    bool _bucketOnX = false;
    bool _sortOnX = false;
    bool _callbacksWereCalled = false;
    Tls _tls;
    AssetGuid _currentMap;
    FP::Raw _worldSizeRaw = 0;
    FP::Raw _worldOffsetRaw = 0;
    bool _mapInitialized = false;
    std::vector<Island3D> _islands;
    std::vector<Entry3D> _entries;
    std::int32_t _queriesCount = 1;
    std::vector<StaticComponents3D> _staticComponents;
    std::int32_t _staticComponentsCount = 0;
    FP _deltaTime;
    PhysicsSceneSettings CachedSettings;
    // Collision constraints of the legacy solver, one per contact point (plus the average
    // point for multi-point manifolds).
    struct ContactConstraint {
        CollisionManifold3D* Manifold = nullptr;
        bool DynamicA = false;
        bool DynamicB = false;
        FP PerceivedMass;
        FPVector3 RA; // contact offset from A's centre of mass
        FPVector3 RB;
    };
    std::vector<ContactConstraint> _collisionSolverData;
    std::vector<CollisionManifold3D*> _collisionSolverManifolds;
};

} // namespace Quantum::Physics3D
