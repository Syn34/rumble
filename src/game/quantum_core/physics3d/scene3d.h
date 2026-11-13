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

#include "quantum_core/physics3d/collision_info3d.h"
#include "quantum_core/physics3d/collision_manifold3d.h"
#include "quantum_core/physics3d/gjk3d.h"
#include "quantum_core/physics3d/hit3d.h"
#include "quantum_core/component_set.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/physics_scene_settings.h"
#include "quantum_core/asset_guid.h"

#include <cstdint>
#include <deque>
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
    Entry3D Entry;                 // the entry of an immediate query
    CollisionManifold3D Manifold;  // scratch manifold of query narrow checks
    PhysicsCollider3D Collider3D;  // the query entry's collider
    ::Quantum::Transform3D Transform3D;
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
    // 0x7d7b294 - the caller links the entry to the island (Entry3D.Island).
    void AddEntry(Entry3D* entry) { Entries.push_back(entry); }
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

// Quantum.Physics3D.Query3D.QueryType
enum class QueryType : std::int32_t {
    ShapeOverlap = 0,
    ShapeCast = 1,
    Linecast = 2,
};

// Quantum.Physics3D.Query3D with its QueryParameters3D union flattened: the transform's
// position is the overlap position or the cast/linecast start; End is the linecast end or
// the shape cast translation.
struct Query3D {
    QueryType Type = QueryType::ShapeOverlap;
    bool FirstHitOnly = false;
    ::Quantum::Transform3D Transform;
    FPVector3 End;
    Shape3D Shape;
    std::int32_t Options = 0;
    std::int32_t LayerMask = 0;
};

// Quantum.Physics3D.StaticComponents3D: the transform and collider of a static entry.
struct StaticComponents3D {
    ::Quantum::Transform3D Transform3D;
    PhysicsCollider3D Collider3D;
};

// Quantum.Physics3D.EntrySolverImpulses: the impulses one constraint gives one body in a
// hybrid solver iteration, chained per body (Entry3D.SolverImpulsesAdr).
struct EntrySolverImpulses {
    EntrySolverImpulses* NextAdr = nullptr;
    FPVector3 Angular; // r x impulse
    FPVector3 Linear;
};

// Quantum.Physics3D.ConstraintFlags
enum ConstraintFlags : std::int32_t {
    IgnoreVelocitySolver = 1,
    IgnorePositionSolver = 2,
    WasSolved = 4,
    HasDynamicBodyA = 8,
    HasDynamicBodyB = 16,
};

// Quantum.Physics3D.ConstraintData with its CollisionConstraintData: one contact point of a
// manifold for the solvers (one 0x228-byte slot each in the original).
struct ContactConstraint {
    std::int32_t Flags = 0; // ConstraintFlags
    EntrySolverImpulses ImpulsesA;
    EntrySolverImpulses ImpulsesB;
    CollisionManifold3D* Manifold = nullptr; // ManifoldAdr
    FP NormalMass;                           // perceived mass along the normal
    FPVector3 Ra;                            // contact offset from A's centre of mass
    FPVector3 Rb;
};

// Scene3D.AccumulatedImpulseFlags
enum AccumulatedImpulseFlags : std::int32_t {
    ImpulseLinear = 1,
    ImpulseAngular = 2,
    ImpulseAdd = 4,        // plain sum
    ImpulseAccumulate = 8, // sum without re-adding what already points the same way
    ImpulseAll = 3,
};

// Scene3D.AccumulatedImpulse: velocity changes collected for one body before they are applied.
struct AccumulatedImpulse {
    FPVector3 Velocity;
    FPVector3 AngularVelocity;

    // 0x7d6e2e0 - per selected part (linear/angular): Add sums; otherwise an impulse that
    // points along the accumulated one only adds the share beyond its projection.
    void Accumulate(const AccumulatedImpulse& impulse, std::int32_t flags);
    // 0x7d6e494 - impulse * massInverse, and with rotation (r x impulse) * inverse inertia
    // (the local tensor's X entry, as the original's isotropic approximation).
    void Integrate(const PhysicsBody3D* body, const FPVector3& impulse, const FPVector3& r,
                   bool useRotation);
    // 0x7d6e564 - adds the changes to the entry's body (waking it; rotation freeze respected).
    void Apply(Entry3D* entry) const;
};

// Scene3D.AccumulatedImpulseCollection
struct AccumulatedImpulseCollection {
    AccumulatedImpulse A;
    AccumulatedImpulse B;
    // 0x7d6e66c
    void Accumulate(const AccumulatedImpulseCollection& impulses, std::int32_t flags) {
        A.Accumulate(impulses.A, flags);
        B.Accumulate(impulses.B, flags);
    }
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
    // The query box on the sweep axes while ResolveQuery walks the buckets.
    struct QueryBox {
        std::int32_t SortMin = 0;
        std::int32_t SortMax = 0;
        std::int32_t SecondaryMin = 0;
        std::int32_t SecondaryMax = 0;
        std::int32_t StabPoint = 0;
        std::int32_t Bucket = 0;
        bool FirstBucket = false;
    };
    void ResolveQuery(Core::FrameBase* frame, HitCollection3D* hits, const Query3D& query, Entry3D* qEntry);
    void ScanQueryCandidates(Core::FrameBase* frame, HitCollection3D* hits, const Query3D& query,
                             Entry3D* qEntry, const StabbingList& stabbing, const QueryBox& q, bool statics,
                             bool detailed);
    void ResolveQueryNarrow(Core::FrameBase* frame, HitCollection3D* hits, const Query3D& query,
                            Entry3D* qEntry, Entry3D* entry, bool detailed);
    void InitLinecastEntry(const Query3D& query, Entry3D* qEntry) const;
    void InitShapeOverlapEntry(const Query3D& query, Entry3D* qEntry);

    // Step stages (the original's Task_* functions).
    void CreateEntries(Core::FrameBase* frame);
    void UpdateMap(Core::FrameBase* frame, AssetGuid mapGuid);
    void SetDefaultWorld();
    void ResetBucketsAndTls();
    void TaskIntegrateForcesFirstHalf();
    void TaskBroadPhaseEntryPreStep();
    void TaskBroadPhase(Core::FrameBase* frame);
    void EntryCheckEntryList(const ActiveEntryList& list, std::int32_t bucket, Entry3D* entry,
                             bool isTriggerManifold, bool isSleeping, bool isKinematic,
                             bool isStatic);
    void TaskNarrowPhase();
    void TaskCallbacks(Core::FrameBase* frame, ICollisionCallbacks3D* callbacks);
    bool CheckIgnoreTriggerManifold(Core::FrameBase* frame, ManifoldData3D* m, Entry3D* entryA,
                                    Entry3D* entryB);
    bool CheckIgnoreCollisionManifold(Core::FrameBase* frame, ManifoldData3D* m, Entry3D* entryA,
                                      Entry3D* entryB);
    void HandleDynamicTrigger(Core::FrameBase* f, ICollisionCallbacks3D* callbacks,
                              ManifoldData3D* manifold, Entry3D* entryA, Entry3D* entryB);
    void HandleStaticTrigger(Core::FrameBase* f, ICollisionCallbacks3D* callbacks, const Map* map,
                             ManifoldData3D* manifold, Entry3D* entryA, Entry3D* entryB);
    void HandleDynamicCollision(Core::FrameBase* f, ICollisionCallbacks3D* callbacks,
                                CollisionManifold3D* manifold, Entry3D* entryA, Entry3D* entryB);
    void HandleStaticCollision(Core::FrameBase* f, ICollisionCallbacks3D* callbacks, const Map* map,
                               CollisionManifold3D* manifold, Entry3D* entryA, Entry3D* entryB);
    void HandleExits(Core::FrameBase* f, ICollisionCallbacks3D* callbacks, const Map* map);
    bool RaiseExit(Core::FrameBase* f, ICollisionCallbacks3D* callbacks, const Map* map,
                   const Entry3D& entry, const PhysicsCallbacksEntry& item);
    void TaskPostCallbacksUpdateEntries(Core::FrameBase* frame);
    void TaskPostCallbacksUpdateManifolds();
    Island3D* GetNextIsland();
    void TaskBuildIslands();
    void TaskCollisionSolverCacheManifolds();
    void TaskCollisionSolverInitLegacyGS(Core::FrameBase* frame);
    void TaskCollisionSolverInit(Core::FrameBase* frame);
    void TaskCollisionSolverCompute();
    void TaskCollisionSolverAggregateContacts();
    void TaskVelocitySolverAggregate();
    void TaskVelocitySolverApply();
    void TaskVelocitySolverLegacyGS();
    void AccumulateImpulses(const ContactConstraint* cst, CollisionManifold3D* m, Entry3D* entryA,
                            Entry3D* entryB, bool useRotation,
                            AccumulatedImpulseCollection& accumulated, bool& solved,
                            std::int32_t flags);
    AccumulatedImpulseCollection ComputeImpulse(const ContactConstraint* cst,
                                                const CollisionManifold3D* m, Entry3D* entryA,
                                                Entry3D* entryB, bool useRotation, bool& solved);
    void TaskPenetrationCorrection();
    void TaskIntegrateVelocity();
    void TaskSleepDetectionEntries();
    void TaskSleepDetectionIslands();

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
    std::deque<Island3D> _islands; // a deque keeps Entry3D.Island pointers valid
    std::vector<Entry3D*> _solverEntries; // the dynamic entries that joined an island
    std::vector<Entry3D> _entries;
    std::int32_t _queriesCount = 1;
    std::vector<StaticComponents3D> _staticComponents;
    std::int32_t _staticComponentsCount = 0;
    FP _deltaTime;
    PhysicsSceneSettings CachedSettings;
    ComponentSet _minComponentSet; // Transform3D + PhysicsCollider3D
    std::vector<ContactConstraint> _collisionSolverData;
    std::vector<CollisionManifold3D*> _collisionSolverManifolds;
};

} // namespace Quantum::Physics3D
