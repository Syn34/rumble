// scene3d.cpp - Quantum.Physics3D.Scene3D and its helper structures.
//
// Step order (the original's task graph, run sequentially):
//   CreateEntries -> IntegrateForcesFirstHalf -> BroadPhaseEntryPreStep -> BroadPhase ->
//   NarrowPhase -> Callbacks -> SortCcdManifolds -> CcdCallbacks -> PostCallbacksUpdate
//   Entries/Manifolds -> BuildIslands -> IntegrateFirstTOI -> collision solver (legacy
//   Gauss-Seidel or hybrid Jacobi/Gauss-Seidel) -> PenetrationCorrection ->
//   IntegrateVelocity -> SleepDetectionEntries/Islands.
// Stages that need unported subsystems (meshes, queries resolved in the step, joints,
// compounds) are marked with TODOs where they would run.
#include "scene3d.h"

#include "quantum_core/builtin_assets.h"
#include "quantum_core/component_iterators.h"
#include "quantum_core/component_set.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/map.h"
#include "quantum_core/physics3d/narrow_checks.h"
#include "quantum_core/physics_components.h"

#include "photon_deterministic/fp_math.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>

namespace Quantum::Physics3D {

using Core::FrameBase;

namespace {
std::int32_t NextPowerOfTwo(std::int32_t value) {
    std::int32_t v = value - 1;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}
FP::Raw Mul(FP::Raw a, FP::Raw b) { return (a * b) >> 16; }
FP::Raw Raw(FP v) { return v.RawValue(); }
FP FromRaw(FP::Raw v) { return FP::FromRaw(v); }
} // namespace

// ---------------------------------------------------------------------------------------
// ActiveEntryList / StabbingList

// 0x7d2fbe4
void ActiveEntryList::Add(const ActiveEntry& entry, bool sorted) {
    Items[static_cast<std::size_t>(Count)] = entry;
    IndexLookup[static_cast<std::size_t>(entry.Index)] = Count;
    ++Count;
    if (!sorted) {
        CountUnsorted = Count;
    }
}

// 0x7d2fb0c
void ActiveEntryList::Remove(std::int32_t entryIndex) {
    --Count;
    const std::int32_t position = IndexLookup[static_cast<std::size_t>(entryIndex)];
    if (position < Count) {
        Items[static_cast<std::size_t>(position)] = Items[static_cast<std::size_t>(Count)];
        IndexLookup[static_cast<std::size_t>(Items[static_cast<std::size_t>(position)].Index)] = position;
    }
    const std::int32_t unsorted = position < CountUnsorted ? CountUnsorted : position + 1;
    CountUnsorted = std::min(unsorted, Count);
}

// 0x7d2fc24
void ActiveEntryList::CopyFrom(const ActiveEntryList& from) {
    if (from.Count >= 1) {
        ResetAndEnsureCapacity(std::max<std::int32_t>(from.Count, static_cast<std::int32_t>(from.IndexLookup.size())));
        std::copy_n(from.Items.begin(), from.Count, Items.begin());
        std::copy(from.IndexLookup.begin(), from.IndexLookup.end(), IndexLookup.begin());
    }
    Count = from.Count;
    CountUnsorted = from.CountUnsorted;
}

// 0x7d2fdd8
void ActiveEntryList::AddRange(const ActiveEntryList& from) {
    if (from.Count < 1) {
        return;
    }
    const std::int32_t needed = Count + from.Count;
    if (static_cast<std::int32_t>(Items.size()) < needed) {
        Items.resize(static_cast<std::size_t>(needed) * 2);
    }
    if (IndexLookup.size() < from.IndexLookup.size()) {
        IndexLookup.resize(from.IndexLookup.size());
    }
    for (std::int32_t i = 0; i < from.Count; ++i) {
        const ActiveEntry& item = from.Items[static_cast<std::size_t>(i)];
        Items[static_cast<std::size_t>(Count + i)] = item;
        IndexLookup[static_cast<std::size_t>(item.Index)] = Count + i;
    }
    Count += from.Count;
    CountUnsorted += from.CountUnsorted;
}

// 0x7d2fcfc
void ActiveEntryList::ResetAndEnsureCapacity(std::int32_t capacity) {
    if (Capacity < capacity || Items.empty()) {
        Capacity = NextPowerOfTwo(std::max(capacity, 1));
        Items.assign(static_cast<std::size_t>(Capacity), ActiveEntry{});
        IndexLookup.assign(static_cast<std::size_t>(Capacity), 0);
    }
    Count = 0;
    CountUnsorted = 0;
}

// 0x7d30068
ActiveEntryList* StabbingList::GetNextEntryList(std::int32_t stabbingNumber) {
    StabbingNumbers[static_cast<std::size_t>(Count)] = stabbingNumber;
    return &Lists[static_cast<std::size_t>(Count++)];
}

// 0x7d3008c
std::int32_t StabbingList::Stab(std::int32_t stabbingPoint) const {
    std::int32_t hi = Count - 1;
    std::int32_t lo = 0;
    while (lo < hi) {
        const std::int32_t mid = (lo + hi) / 2;
        if (StabbingNumbers[static_cast<std::size_t>(mid)] > stabbingPoint) {
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    return lo - (StabbingNumbers[static_cast<std::size_t>(lo)] > stabbingPoint ? 1 : 0);
}

// 0x7d300f0
void StabbingList::ResetAndEnsureCapacity(std::int32_t capacity, std::int32_t listCapacity) {
    if (Capacity < capacity || Lists.empty()) {
        Capacity = NextPowerOfTwo(std::max(capacity, 1));
        Lists.assign(static_cast<std::size_t>(Capacity), ActiveEntryList{});
        StabbingNumbers.assign(static_cast<std::size_t>(Capacity), 0);
    }
    for (ActiveEntryList& list : Lists) {
        list.ResetAndEnsureCapacity(listCapacity);
    }
    EntriesCount = 0;
    EntriesPerList = 0;
    Count = 0;
}

// ---------------------------------------------------------------------------------------
// Bucket3D

// 0x7d798d0
void Bucket3D::EnsureEndPointsCapacity(std::int32_t entryCapacity, std::int32_t queryCapacity) {
    const std::size_t needed = static_cast<std::size_t>(entryCapacity + queryCapacity);
    if (EndPoints.size() < needed) {
        EndPoints.assign(needed, EndPoint{});
    }
    EndPointsCount = 0;
}

// 0x7d79988
void Bucket3D::EnsureEndPointsStaticCapacity(std::int32_t capacity) {
    if (EndPointsStatic.size() < static_cast<std::size_t>(capacity)) {
        EndPointsStatic.resize(static_cast<std::size_t>(capacity));
    }
}

// 0x7d79a10
void Bucket3D::InsertEndPoints(EndPoint min, EndPoint max) {
    const std::size_t pair = static_cast<std::size_t>(++EndPointsCount) * 2;
    if (EndPoints.size() < pair) {
        EndPoints.resize(pair * 2);
    }
    EndPoints[pair - 2] = min;
    EndPoints[pair - 1] = max;
}

// 0x7d79a58
void Bucket3D::InsertEndPointsStatic(EndPoint min, EndPoint max) {
    const std::size_t pair = static_cast<std::size_t>(EndPointsStaticCount++) * 2;
    if (EndPointsStatic.size() < pair + 2) {
        EndPointsStatic.resize((pair + 2) * 2);
    }
    EndPointsStatic[pair] = min;
    EndPointsStatic[pair + 1] = max;
}

// 0x7d79a84
CollisionManifold3D* Bucket3D::GetNextCollisionManifold() {
    CollisionManifold3D& m = CollisionManifolds.emplace_back();
    m._m.BucketIndex = Index;
    return &m;
}

// 0x7d79cb4
QueryManifold3D* Bucket3D::GetNextQueryManifold(std::int32_t& index) {
    index = static_cast<std::int32_t>(QueryManifolds.size());
    return &QueryManifolds.emplace_back();
}

// ---------------------------------------------------------------------------------------
// Island3D

// 0x7d7b1b8
void Island3D::Reset() {
    Ignore = false;
    VelocityConstraintsSolved = false;
    PositionConstraintsSolved = false;
    CollisionManifolds.clear();
    Entries.clear();
}

// ---------------------------------------------------------------------------------------
// Scene3D: setup

// 0x7d591a4 (Allocate)
Scene3D::Scene3D() {
    _buckets.resize(1);
    _bucketsSizeRaw = FP::Raw{2} << 16;
    _buckets[0].Index = 0;
}

// 0x7d5b2f8 - a scene that never had a world gets the default one (a single bucket of
// size 2 centred on the origin) before the map is applied.
void Scene3D::InitScene(FrameBase* frame, AssetGuid mapGuid) {
    if (_worldSizeRaw == 0) {
        SetDefaultWorld();
    }
    ResetScene(frame);
    UpdateMap(frame, mapGuid);
}

void Scene3D::SetDefaultWorld() {
    _bucketsSizeRaw = FP::Raw{2} << 16;
    _buckets.assign(1, Bucket3D{});
    _worldSizeRaw = FP::Raw{2} << 16;
    _worldOffsetRaw = FP::Raw{1} << 16;
}

// 0x7d5b354
void Scene3D::ResetScene(FrameBase* frame) {
    if (const PhysicsSceneSettings* settings = frame->get_PhysicsSceneSettings()) {
        CachedSettings = *settings;
    }
    _callbacksWereCalled = false;
    // Dynamic entries go; the static ones (first) stay.
    _entries.resize(static_cast<std::size_t>(_staticComponentsCount));
    _queriesCount = 1;
    _collisionSolverData.clear();
    _collisionSolverManifolds.clear();
    _islands.clear();
}

namespace {
// Radix.SortEndPoints (0x7d3d558): a stable LSD radix sort over the low two bytes of Index,
// then the four bytes of Value - a stable sort by (Value, low 16 bits of Index).
void SortEndPoints(EndPoint* first, EndPoint* last) {
    std::stable_sort(first, last, [](const EndPoint& a, const EndPoint& b) {
        const std::uint64_t ka = (std::uint64_t{static_cast<std::uint32_t>(a.Value)} << 16) |
                                 static_cast<std::uint16_t>(a.Index);
        const std::uint64_t kb = (std::uint64_t{static_cast<std::uint32_t>(b.Value)} << 16) |
                                 static_cast<std::uint16_t>(b.Index);
        return ka < kb;
    });
}
} // namespace

// 0x7d5a490 - rebuilds the static world when the map changes (never on predicted frames in
// the original): world size and buckets from the map, one entry per static collider (the
// map's order; meshes get an entry but no endpoints), the static endpoints sorted once per
// bucket and swept to build the buckets' static stabbing lists.
// Two deviations: a map guid that is not valid falls back to the frame's map (the original
// keeps the default world), and the static colliders' mutable state
// (StaticCollidersState, toggled colliders) is not tracked.
void Scene3D::UpdateMap(FrameBase* frame, AssetGuid mapGuid) {
    if (_mapInitialized && mapGuid == _currentMap) {
        return;
    }
    _entries.clear();
    _staticComponents.clear();
    _staticComponentsCount = 0;
    SetDefaultWorld();
    _mapInitialized = true;
    _currentMap = mapGuid;
    Map* map = mapGuid.IsValid() ? frame->FindAsset<Map>(mapGuid) : frame->get_Map();
    if (map == nullptr) {
        return;
    }
    _bucketOnX = map->BucketingAxis == Map::BucketAxis::X;
    _sortOnX = map->SortingAxis == Map::SortAxis::X;
    // The world size is shifted in 32 bits, as the original does.
    const auto worldSize = static_cast<std::int32_t>(static_cast<std::uint32_t>(map->WorldSize) << 16);
    _worldSizeRaw = worldSize;
    _worldOffsetRaw = worldSize >> 1;
    // TODO: a map with no buckets divides by zero in the original; one bucket here.
    const std::int32_t bucketsCount = map->BucketsCount > 0 ? map->BucketsCount : 1;
    _bucketsSizeRaw = _worldSizeRaw / bucketsCount;
    _bucketsSubdivisions = std::max(1, map->BucketsSubdivisions);
    _buckets.assign(static_cast<std::size_t>(bucketsCount), Bucket3D{});
    const auto bucketSize = static_cast<std::int32_t>(_bucketsSizeRaw);
    for (std::int32_t i = 0; i < bucketsCount; ++i) {
        Bucket3D& bucket = _buckets[static_cast<std::size_t>(i)];
        bucket.Index = i;
        bucket.Min = i * bucketSize;
        bucket.Max = bucket.Min + bucketSize;
    }
    // TODO(0x7d39438, Quantum.TriangleMesh.InitializeFromMap): the map's static triangle mesh.

    // Static colliders become the first entries.
    const std::int32_t staticCount = static_cast<std::int32_t>(map->StaticColliders3D.size());
    _staticComponents.reserve(static_cast<std::size_t>(staticCount));
    for (const MapStaticCollider3D& source : map->StaticColliders3D) {
        StaticComponents3D& components = _staticComponents.emplace_back();
        components.Transform3D.Position = source.Position;
        components.Transform3D.Rotation = source.Rotation;
        switch (source.ShapeType) {
        case Shape3DType::Sphere: components.Collider3D.Shape = Shape3D::CreateSphere(source.SphereRadius); break;
        case Shape3DType::Box: components.Collider3D.Shape = Shape3D::CreateBox(source.BoxExtents); break;
        case Shape3DType::Mesh:
            // TODO(0x7d3ac90, Quantum.Shape3D.CreateMesh): mesh shapes need TriangleMesh.
            components.Collider3D.Shape = Shape3D{};
            components.Collider3D.Shape._type = Shape3DType::Mesh;
            break;
        default: break; // the original logs an error for any other shape
        }
        components.Collider3D.SetLayer(source.StaticData.Layer);
        components.Collider3D.SetIsTrigger(source.StaticData.IsTrigger);
        components.Collider3D._material.Id = source.PhysicsMaterial.Id;
    }
    _staticComponentsCount = staticCount;
    for (std::int32_t staticIndex = 0; staticIndex < staticCount; ++staticIndex) {
        StaticComponents3D& components = _staticComponents[static_cast<std::size_t>(staticIndex)];
        std::int32_t index = 0;
        Entry3D* entry = GetNextEntry(index);
        entry->Collider3D = &components.Collider3D;
        entry->Shape = &components.Collider3D.Shape;
        entry->Body3D = nullptr;
        entry->Transform3D = &components.Transform3D;
        entry->StaticColliderIndex = staticIndex;
        if (entry->Shape->Type() == Shape3DType::Mesh) {
            entry->UseSmoothSphereMeshCollision =
                map->StaticColliders3D[static_cast<std::size_t>(staticIndex)].SmoothSphereMeshCollisions;
            continue;
        }
        Entry3D::InitNarrowData(entry, &components.Transform3D.Position, &components.Transform3D.Rotation);
        Entry3D::InitBroadAabb(entry, &components.Transform3D.Position, &components.Transform3D.Rotation, {});
        std::int32_t flags = index | EndPoint::STATIC_FLAG;
        if (entry->Collider3D->_isTrigger == 1) {
            flags |= EndPoint::TRIGGER_FLAG;
        }
        std::int32_t minBucket = 0, maxBucket = 0;
        InsertEndPoints(entry->AABB, flags, minBucket, maxBucket);
        entry->BucketStart = minBucket;
        entry->BucketEnd = maxBucket;
        entry->BucketIncrement = 1;
    }

    // Per bucket: sort the static endpoints and sweep them, taking a snapshot of the open
    // static intervals every EntriesPerList closed ones (for queries that start mid-bucket).
    _tls.Static.ResetAndEnsureCapacity(staticCount);
    _tls.StaticTrigger.ResetAndEnsureCapacity(staticCount);
    for (Bucket3D& bucket : _buckets) {
        bucket.StaticsStabbingList.ResetAndEnsureCapacity(_bucketsSubdivisions, staticCount);
    }
    for (Bucket3D& bucket : _buckets) {
        const std::int32_t pairs = bucket.EndPointsStaticCount;
        if (pairs < 1) {
            continue;
        }
        SortEndPoints(bucket.EndPointsStatic.data(), bucket.EndPointsStatic.data() + 2 * pairs);
        StabbingList& stabbing = bucket.StaticsStabbingList;
        ActiveEntryList* snapshot = stabbing.GetNextEntryList(0);
        stabbing.EntriesPerList = pairs / _bucketsSubdivisions;
        for (std::int32_t i = 0; i < 2 * pairs; ++i) {
            const EndPoint& point = bucket.EndPointsStatic[static_cast<std::size_t>(i)];
            const std::int32_t entryIndex = point.Index & 0x7ffffff;
            const bool trigger = (point.Index & EndPoint::TRIGGER_FLAG) != 0;
            ActiveEntryList& open = trigger ? _tls.StaticTrigger : _tls.Static;
            if ((point.Index & EndPoint::MIN_FLAG) == 0) {
                open.Remove(entryIndex);
                if (stabbing.Count < _bucketsSubdivisions && ++stabbing.EntriesCount >= stabbing.EntriesPerList) {
                    snapshot = stabbing.GetNextEntryList(point.Value);
                    snapshot->CopyFrom(_tls.Static);
                    snapshot->AddRange(_tls.StaticTrigger);
                    stabbing.EntriesCount = 0;
                }
                continue;
            }
            const Entry3D& entry = _entries[static_cast<std::size_t>(entryIndex)];
            const ActiveEntry active{entryIndex, _sortOnX ? entry.AABB.MinZ : entry.AABB.MinX,
                                     _sortOnX ? entry.AABB.MaxZ : entry.AABB.MaxX};
            open.Add(active, false);
            snapshot->Add(active, true);
        }
    }
}

// 0x7d5679c
Entry3D* Scene3D::GetNextEntry(std::int32_t& index) {
    index = static_cast<std::int32_t>(_entries.size());
    Entry3D& entry = _entries.emplace_back();
    entry.Index = index;
    return &entry;
}

// 0x7d5a120
Entry3D* Scene3D::GetEntry(std::int32_t index) {
    if (index < 0 || index >= static_cast<std::int32_t>(_entries.size())) {
        return nullptr;
    }
    return &_entries[static_cast<std::size_t>(index)];
}

Entry3D* Scene3D::Root(Entry3D* entry) {
    return entry->_compoundRootIndexPlusOne >= 1 ? entry->_compoundRoot : entry;
}

void Scene3D::UpdateManifoldCachedState(ManifoldData3D& m) const { m.UpdateCachedState(this); }

// 0x7d7b3d8
void ManifoldData3D::UpdateCachedState(const Scene3D* scene) {
    const Entry3D& a = scene->_entries[static_cast<std::size_t>(_entryIdxA)];
    const Entry3D& b = scene->_entries[static_cast<std::size_t>(_entryIdxB)];
    SetIsDynamic(a.Ref.IsValid() && b.Ref.IsValid());
    const Entry3D* rootA = a._compoundRootIndexPlusOne >= 1 ? a._compoundRoot : &a;
    const Entry3D* rootB = b._compoundRootIndexPlusOne >= 1 ? b._compoundRoot : &b;
    SetUsesCcd(rootA->_useCCD || rootB->_useCCD);
}

// 0x7d7b3c0
void ManifoldData3D::SetEntries(Scene3D* scene, std::int32_t entryA, std::int32_t entryB,
                                std::int32_t tri, bool updateCachedState) {
    _entryIdxA = entryA;
    _entryIdxB = entryB;
    _triIdx = tri;
    if (updateCachedState) {
        UpdateCachedState(scene);
    }
}

// 0x7d7b4a4
Entry3D* ManifoldData3D::GetEntryA(Scene3D* scene) const { return scene->GetEntryUnchecked(_entryIdxA); }
// 0x7d7b4bc
Entry3D* ManifoldData3D::GetEntryB(Scene3D* scene) const { return scene->GetEntryUnchecked(_entryIdxB); }

// 0x7d5a214
std::int32_t Scene3D::GetPointBucket(const FPVector3& point) const {
    const FP::Raw coordinate = _bucketOnX ? Raw(point.X) : Raw(point.Z);
    return static_cast<std::int32_t>((coordinate + _worldOffsetRaw) / _bucketsSizeRaw);
}

// 0x7d5a250
void Scene3D::InsertEndPoints(const IntAABB& aabb, std::int32_t index, std::int32_t& minBucket,
                              std::int32_t& maxBucket) {
    // Sort axis interval, clamped into the endpoint value range.
    const std::int32_t sortMin = _sortOnX ? aabb.MinX : aabb.MinZ;
    const std::int32_t sortMax = _sortOnX ? aabb.MaxX : aabb.MaxZ;
    const std::int32_t max = std::min(std::max(sortMax, -1073741822), 0x3fffffff);
    const std::int32_t min = sortMin < max ? std::max(sortMin, -0x3fffffff) : max - 1;
    // Buckets spanned on the bucket axis.
    const std::int32_t bucketMin = _bucketOnX ? aabb.MinX : aabb.MinZ;
    const std::int32_t bucketMax = _bucketOnX ? aabb.MaxX : aabb.MaxZ;
    const std::int32_t count = static_cast<std::int32_t>(_buckets.size());
    const auto clampBucket = [count](std::int64_t b) {
        return static_cast<std::int32_t>(b < 0 ? 0 : (b >= count ? count - 1 : b));
    };
    minBucket = clampBucket((_worldOffsetRaw + bucketMin) / _bucketsSizeRaw);
    maxBucket = clampBucket((_worldOffsetRaw + bucketMax) / _bucketsSizeRaw);
    const EndPoint minPoint{min + 0x3fffffff, index | EndPoint::MIN_FLAG};
    const EndPoint maxPoint{max + 0x3fffffff, index};
    const bool isStatic = (index & EndPoint::STATIC_FLAG) != 0;
    for (std::int32_t b = minBucket; b <= maxBucket; ++b) {
        Bucket3D& bucket = _buckets[static_cast<std::size_t>(b)];
        if (isStatic) {
            bucket.InsertEndPointsStatic(minPoint, maxPoint);
        } else {
            bucket.InsertEndPoints(minPoint, maxPoint);
        }
    }
}

// 0x7d5b4e4
void Scene3D::ResetBucketsAndTls() {
    const std::int32_t entries = static_cast<std::int32_t>(_entries.size());
    for (ActiveEntryList* list : {&_tls.Dynamic, &_tls.Kinematic, &_tls.KinematicTrigger}) {
        list->ResetAndEnsureCapacity(entries);
    }
    // The static lists keep their storage size.
    _tls.Static.ResetAndEnsureCapacity(entries);
    _tls.StaticTrigger.ResetAndEnsureCapacity(entries);
    for (ActiveEntryList* list : {&_tls.QueryAll, &_tls.QueryDynamic, &_tls.QueryKinematic,
                                  &_tls.QueryKinematicTrigger, &_tls.QueryStatic,
                                  &_tls.QueryStaticTrigger}) {
        list->ResetAndEnsureCapacity(_queriesCount);
    }
    for (Bucket3D& bucket : _buckets) {
        bucket.DynamicsStabbingList.ResetAndEnsureCapacity(_bucketsSubdivisions + 1, entries);
        bucket.EnsureEndPointsCapacity(entries * 2, _queriesCount * 2);
        bucket.QueryManifolds.assign(1, QueryManifold3D{});
        bucket.CollisionManifolds.assign(1, CollisionManifold3D{});
        bucket.CollisionManifoldsProcessCount = 1;
        bucket.SolverManifolds.clear();
        bucket.TriggerManifolds.assign(1, ManifoldData3D{});
        bucket.TriggerManifoldsProcessCount = 1;
    }
}

// 0x7d5ba7c
void Scene3D::CreateEntries(FrameBase* frame) {
    _minComponentSet = ComponentSet::Create<::Quantum::Transform3D, PhysicsCollider3D>();
    const ComponentSet& minComponents = _minComponentSet;
    ComponentBlockIterator<PhysicsCollider3D> blocks = frame->GetComponentBlockIterator<PhysicsCollider3D>();
    EntityRef* entities = nullptr;
    PhysicsCollider3D* colliders = nullptr;
    int count = 0;
    while (blocks.NextBlock(entities, colliders, count)) {
        for (int i = 0; i < count; ++i) {
            const EntityRef entity = entities[i];
            if (frame->IsCulled(entity) || !frame->GetComponentSet(entity).IsSupersetOf(minComponents)) {
                continue;
            }
            PhysicsCollider3D* collider = &colliders[i];
            const Shape3DType type = collider->Shape.Type();
            if (!collider->Enabled() || type == Shape3DType::None) {
                continue;
            }
            if (type == Shape3DType::Compound) {
                // TODO(0x7d5bdfc, Quantum.Physics3D.Scene3D.CreateCompoundEntries): one entry
                // per child shape, rooted at this entry; needs CompoundShape3D.GetShapes.
                continue;
            }
            std::int32_t index = 0;
            Entry3D* entry = GetNextEntry(index);
            entry->Ref = entity;
            entry->Collider3D = collider;
            entry->Shape = &collider->Shape;
            entry->Transform3D = frame->GetPointer<::Quantum::Transform3D>(entity);
            entry->StaticColliderIndex = -1;
            PhysicsCallbacks3D* callbacks = nullptr;
            if (frame->TryGetPointer<PhysicsCallbacks3D>(entity, callbacks)) {
                entry->Callbacks = callbacks;
                const auto flags = static_cast<std::int32_t>(callbacks->Flags);
                entry->HasStaticTriggerCallbacks = (flags & 0xe00) != 0;
                entry->HasDynamicTriggerCallbacks = (flags & 0x1c0) != 0;
            }
            PhysicsBody3D* body = nullptr;
            if (frame->TryGetPointer<PhysicsBody3D>(entity, body) && body->Enabled()) {
                entry->Body3D = body;
                if (!body->GetIsKinematic()) {
                    entry->HasDynamicBody = true;
                    entry->ShouldIntegrateVelocity = body->Status == PhysicsBodyStatus::Active;
                    PhysicsJoints3D* joints = nullptr;
                    if (frame->TryGetPointer<PhysicsJoints3D>(entity, joints) && joints->_count != 0) {
                        entry->Joints = joints;
                    }
                }
            }
        }
    }
}

// 0x7d5c198
void Scene3D::TaskIntegrateForcesFirstHalf() {
    const FP::Raw halfDt = Raw(_deltaTime) >> 1;
    const PhysicsSceneSettings& s = CachedSettings;
    const FP::Raw gx = Raw(s.Gravity.X), gy = Raw(s.Gravity.Y), gz = Raw(s.Gravity.Z);
    const bool useAngular = s.UseAngularVelocityEnabled();
    const bool allowCcd = s.CCDSettingsValue._allowCCD == 1;
    for (std::size_t i = static_cast<std::size_t>(_staticComponentsCount + _queriesCount - 1);
         i < _entries.size(); ++i) {
        Entry3D* entry = &_entries[i];
        entry->Init(_entries.data());
        if (entry->Shape->Type() == Shape3DType::Compound) {
            if (entry->HasDynamicBody) {
                Entry3D::CalculateWorldBodySettings(entry, entry->Transform3D->Rotation);
            }
        } else {
            Entry3D::InitNarrowData(entry, &entry->Transform3D->Position, &entry->Transform3D->Rotation);
        }
        if (!entry->ShouldIntegrateVelocity) {
            continue;
        }
        PhysicsBody3D* body = entry->Body3D;
        const FP::Raw massInv = body->_massInverseRawInt;
        const FP::Raw gravityScale = body->_gravityScaleRawInt;
        // v += (gravity * scale + force * massInv) * dt / 2
        const FP::Raw ax = Mul(gx, gravityScale) + Mul(Raw(body->_force.X), massInv);
        const FP::Raw ay = Mul(gy, gravityScale) + Mul(Raw(body->_force.Y), massInv);
        const FP::Raw az = Mul(gz, gravityScale) + Mul(Raw(body->_force.Z), massInv);
        body->Velocity.X = FromRaw(Raw(body->Velocity.X) + Mul(ax, halfDt));
        body->Velocity.Y = FromRaw(Raw(body->Velocity.Y) + Mul(ay, halfDt));
        body->Velocity.Z = FromRaw(Raw(body->Velocity.Z) + Mul(az, halfDt));
        if (useAngular) {
            const Physics3D::FPVector3RawInt impulse = body->_angularImpulse;
            const bool any = (impulse.RawIntX | impulse.RawIntY | impulse.RawIntZ) != 0 ||
                             body->_torque != FPVector3::Zero();
            if (any) {
                // w += I_world^-1 * (angularImpulse + torque * dt / 2)
                const FP::Raw tx = impulse.RawIntX + Mul(Raw(body->_torque.X), halfDt);
                const FP::Raw ty = impulse.RawIntY + Mul(Raw(body->_torque.Y), halfDt);
                const FP::Raw tz = impulse.RawIntZ + Mul(Raw(body->_torque.Z), halfDt);
                const Photon::Deterministic::FPMatrix3x3& m = entry->WorldInertiaTensorInverse;
                const FP::Raw wx = Mul(Raw(m.M01), ty) + Mul(Raw(m.M00), tx) + Mul(Raw(m.M02), tz);
                const FP::Raw wy = Mul(Raw(m.M11), ty) + Mul(Raw(m.M10), tx) + Mul(Raw(m.M12), tz);
                const FP::Raw wz = Mul(Raw(m.M21), ty) + Mul(Raw(m.M20), tx) + Mul(Raw(m.M22), tz);
                const auto freeze = static_cast<std::uint8_t>(body->RotationFreeze);
                if (!(freeze & 1)) body->AngularVelocity.X = FromRaw(Raw(body->AngularVelocity.X) + wx);
                if (!(freeze & 2)) body->AngularVelocity.Y = FromRaw(Raw(body->AngularVelocity.Y) + wy);
                if (!(freeze & 4)) body->AngularVelocity.Z = FromRaw(Raw(body->AngularVelocity.Z) + wz);
            }
        }
        if (allowCcd && body->UseContinuousCollisionDetection()) {
            const FPVector3& v = body->Velocity;
            const FPVector3& w = body->AngularVelocity;
            const FP::Raw v2 = Mul(Raw(v.Y), Raw(v.Y)) + Mul(Raw(v.X), Raw(v.X)) + Mul(Raw(v.Z), Raw(v.Z));
            const FP::Raw w2 = Mul(Raw(w.Y), Raw(w.Y)) + Mul(Raw(w.X), Raw(w.X)) + Mul(Raw(w.Z), Raw(w.Z));
            if (v2 > Raw(s.CCDSettingsValue._linearVelocityThresholdSqr) ||
                w2 > Raw(s.CCDSettingsValue._angularVelocityThresholdSqr)) {
                entry->_useCCD = true;
                entry->_firstTimeOfImpact = _deltaTime;
            }
        }
    }
}

// 0x7d5c898
void Scene3D::TaskBroadPhaseEntryPreStep() {
    for (std::size_t i = static_cast<std::size_t>(_staticComponentsCount + _queriesCount - 1);
         i < _entries.size(); ++i) {
        Entry3D* entry = &_entries[i];
        if (entry->Shape->Type() == Shape3DType::Compound) {
            continue;
        }
        Entry3D::InitBroadAabb(entry, &entry->Transform3D->Position, &entry->Transform3D->Rotation,
                               _deltaTime);
        std::int32_t flags = entry->Index;
        if (!entry->HasDynamicBody) {
            flags |= EndPoint::KINEMATIC_FLAG;
        }
        if (entry->Collider3D->_isTrigger == 1) {
            flags |= EndPoint::TRIGGER_FLAG;
        }
        std::int32_t minBucket = 0, maxBucket = 0;
        InsertEndPoints(entry->AABB, flags, minBucket, maxBucket);
        entry->BucketStart = minBucket;
        entry->BucketEnd = maxBucket;
        entry->BucketIncrement = 1;
    }
}

// 0x7d5df0c
void Scene3D::EntryCheckEntryList(const ActiveEntryList& list, std::int32_t bucket, Entry3D* entry,
                                  bool isTriggerManifold, bool isSleeping, bool isKinematic,
                                  bool isStatic) {
    const std::int32_t entryMin = _sortOnX ? entry->AABB.MinZ : entry->AABB.MinX;
    const std::int32_t entryMax = _sortOnX ? entry->AABB.MaxZ : entry->AABB.MaxX;
    const std::int32_t layerRow = CachedSettings.Layers[entry->Collider3D->Layer()];
    Bucket3D& b = _buckets[static_cast<std::size_t>(bucket)];
    for (std::int32_t i = 0; i < list.Count; ++i) {
        const ActiveEntry& active = list.Items[static_cast<std::size_t>(i)];
        Entry3D* other = &_entries[static_cast<std::size_t>(active.Index)];
        if (((layerRow >> other->Collider3D->Layer()) & 1) == 0) {
            continue;
        }
        // Pairs spanning several buckets are made in the first one either starts in.
        if (entry->BucketStart != bucket && other->BucketStart != bucket) {
            continue;
        }
        if (isTriggerManifold) {
            if (isStatic) {
                if (!other->HasStaticTriggerCallbacks) continue;
            } else if (other->Ref.Raw() != 0 && !entry->HasDynamicTriggerCallbacks &&
                       !other->HasDynamicTriggerCallbacks) {
                continue;
            }
        } else if (other->Body3D != nullptr) {
            const bool otherSleeping = other->Body3D->Status == PhysicsBodyStatus::Asleep;
            if ((isSleeping || isStatic) && otherSleeping) continue;
            if (otherSleeping && isKinematic &&
                (other->Body3D->Config & PhysicsBody3D::ConfigFlags::IsAwakenedByKinematics) == 0) {
                continue;
            }
        }
        if (entryMin > active.Max || entryMax < active.Min) continue;
        if (entry->AABB.MinY > other->AABB.MaxY || other->AABB.MinY > entry->AABB.MaxY) continue;
        if (entry->Ref.Raw() == other->Ref.Raw()) continue;
        ManifoldData3D* m = nullptr;
        if (isTriggerManifold) {
            m = &b.TriggerManifolds.emplace_back();
            m->SetIsTrigger(true);
        } else {
            m = &b.GetNextCollisionManifold()->_m;
        }
        m->_triIdx = 0;
        m->_entryIdxA = entry->Index;
        m->_entryIdxB = other->Index;
        UpdateManifoldCachedState(*m);
    }
}

// 0x7d5e150 - sweep of each bucket: dynamic and static endpoints merged in order; at a min
// endpoint the entry is checked against the open sets it can collide with, then opened.
// Query endpoints never occur here yet (broad-phase queries are not ported).
void Scene3D::TaskBroadPhase(FrameBase* /*frame*/) {
    Tls& tls = _tls;
    for (Bucket3D& bucket : _buckets) {
        const std::int32_t n = bucket.EndPointsCount;
        if (n == 0) {
            continue;
        }
        for (ActiveEntryList* list : {&tls.QueryAll, &tls.QueryDynamic, &tls.QueryKinematic,
                                      &tls.QueryKinematicTrigger, &tls.QueryStatic,
                                      &tls.QueryStaticTrigger, &tls.Dynamic, &tls.Kinematic,
                                      &tls.KinematicTrigger, &tls.Static, &tls.StaticTrigger}) {
            list->ResetCount();
        }
        StabbingList& stabbing = bucket.DynamicsStabbingList;
        ActiveEntryList* stabList = stabbing.GetNextEntryList(0);
        stabbing.EntriesPerList = n / _bucketsSubdivisions;
        SortEndPoints(bucket.EndPoints.data(), bucket.EndPoints.data() + 2 * n);
        const std::int32_t dynamicTotal = 2 * n;
        const std::int32_t staticTotal = 2 * bucket.EndPointsStaticCount;
        std::int32_t di = 0, si = 0;
        while (di < dynamicTotal) {
            const EndPoint* ep = &bucket.EndPoints[static_cast<std::size_t>(di)];
            if (si < staticTotal &&
                ep->Value > bucket.EndPointsStatic[static_cast<std::size_t>(si)].Value) {
                ep = &bucket.EndPointsStatic[static_cast<std::size_t>(si++)];
            } else {
                ++di;
            }
            const std::int32_t value = ep->Value;
            const std::int32_t flagged = ep->Index;
            const std::int32_t idx = flagged & 0x7ffffff;
            if ((flagged & EndPoint::QUERY_FLAG) != 0) {
                // TODO(0x7d5e150, Quantum.Physics3D.Scene3D.Task_BroadPhase): query endpoints
                // (QueryCheckEntryList / QueryCheckTriangles and the query active lists).
                continue;
            }
            Entry3D* entry = &_entries[static_cast<std::size_t>(idx)];
            // TODO: disabled static colliders (StaticCollidersState) are skipped here.
            if ((flagged & EndPoint::MIN_FLAG) == 0) {
                // Max endpoint: close the interval.
                const std::int32_t category = flagged & 0x70000000;
                ActiveEntryList* list = nullptr;
                bool dynamicSet = false;
                if (category == 0) { list = &tls.Dynamic; dynamicSet = true; }
                else if (category == EndPoint::KINEMATIC_FLAG) { list = &tls.Kinematic; dynamicSet = true; }
                else if (category == (EndPoint::KINEMATIC_FLAG | EndPoint::TRIGGER_FLAG)) { list = &tls.KinematicTrigger; dynamicSet = true; }
                else if (category == EndPoint::STATIC_FLAG) { list = &tls.Static; }
                else if (category == (EndPoint::STATIC_FLAG | EndPoint::TRIGGER_FLAG)) { list = &tls.StaticTrigger; }
                if (list == nullptr) continue;
                list->Remove(idx);
                if (dynamicSet && stabbing.Count < _bucketsSubdivisions) {
                    if (++stabbing.EntriesCount >= stabbing.EntriesPerList) {
                        stabList = stabbing.GetNextEntryList(value);
                        stabList->CopyFrom(tls.Dynamic);
                        stabList->AddRange(tls.Kinematic);
                        stabList->AddRange(tls.KinematicTrigger);
                        stabbing.EntriesCount = 0;
                    }
                }
                continue;
            }
            const auto category = static_cast<std::uint32_t>(flagged) & 0xf0000000u;
            ActiveEntryList* target = nullptr;
            bool isStaticEntry = false;
            const std::int32_t b = bucket.Index;
            if (category == 0x80000000u) { // dynamic
                const bool sleeping = entry->Body3D->Status == PhysicsBodyStatus::Asleep;
                if (tls.Dynamic.Count >= 1) EntryCheckEntryList(tls.Dynamic, b, entry, false, sleeping, false, false);
                if (tls.KinematicTrigger.Count >= 1) EntryCheckEntryList(tls.KinematicTrigger, b, entry, true, sleeping, false, false);
                if (sleeping) {
                    if ((entry->Body3D->Config & PhysicsBody3D::ConfigFlags::IsAwakenedByKinematics) != 0 &&
                        tls.Kinematic.Count >= 1) {
                        EntryCheckEntryList(tls.Kinematic, b, entry, false, true, false, false);
                    }
                } else {
                    if (tls.Static.Count >= 1) EntryCheckEntryList(tls.Static, b, entry, false, false, false, false);
                    if (tls.Kinematic.Count >= 1) EntryCheckEntryList(tls.Kinematic, b, entry, false, false, false, false);
                    if (entry->HasStaticTriggerCallbacks && tls.StaticTrigger.Count >= 1) {
                        EntryCheckEntryList(tls.StaticTrigger, b, entry, true, false, false, false);
                    }
                    // TODO: EntryCheckTriangles (static mesh triangles).
                }
                target = &tls.Dynamic;
            } else if (category == 0x90000000u) { // kinematic
                if (tls.Dynamic.Count >= 1) EntryCheckEntryList(tls.Dynamic, b, entry, false, false, true, false);
                if (tls.KinematicTrigger.Count >= 1) EntryCheckEntryList(tls.KinematicTrigger, b, entry, true, false, true, false);
                if (entry->HasStaticTriggerCallbacks && tls.StaticTrigger.Count >= 1) {
                    EntryCheckEntryList(tls.StaticTrigger, b, entry, true, false, true, false);
                }
                target = &tls.Kinematic;
            } else if (category == 0xb0000000u) { // kinematic trigger
                if (tls.Dynamic.Count >= 1) EntryCheckEntryList(tls.Dynamic, b, entry, true, false, true, false);
                if (tls.Kinematic.Count >= 1) EntryCheckEntryList(tls.Kinematic, b, entry, true, false, true, false);
                if (entry->HasStaticTriggerCallbacks && tls.Static.Count >= 1) {
                    EntryCheckEntryList(tls.Static, b, entry, true, false, true, false);
                }
                target = &tls.KinematicTrigger;
            } else if (category == 0xc0000000u) { // static
                if (tls.Dynamic.Count >= 1) EntryCheckEntryList(tls.Dynamic, b, entry, false, false, false, true);
                if (tls.KinematicTrigger.Count >= 1) EntryCheckEntryList(tls.KinematicTrigger, b, entry, true, false, false, true);
                target = &tls.Static;
                isStaticEntry = true;
            } else if (category == 0xe0000000u) { // static trigger
                if (tls.Dynamic.Count >= 1) EntryCheckEntryList(tls.Dynamic, b, entry, true, false, false, true);
                if (tls.Kinematic.Count >= 1) EntryCheckEntryList(tls.Kinematic, b, entry, true, false, false, true);
                target = &tls.StaticTrigger;
                isStaticEntry = true;
            } else {
                continue; // dynamic triggers get the kinematic flag in practice
            }
            const ActiveEntry active{idx, _sortOnX ? entry->AABB.MinZ : entry->AABB.MinX,
                                     _sortOnX ? entry->AABB.MaxZ : entry->AABB.MaxX};
            target->Add(active, false);
            if (!isStaticEntry) {
                stabList->Add(active, true);
            }
        }
    }
}

// 0x7d5f34c
void Scene3D::TaskNarrowPhase() {
    const CCDSettings& ccd = CachedSettings.CCDSettingsValue;
    for (Bucket3D& bucket : _buckets) {
        const std::int32_t triggers = static_cast<std::int32_t>(bucket.TriggerManifolds.size());
        for (std::int32_t i = bucket.GetNextTriggerManifoldToProcess(); i < triggers;
             i = bucket.GetNextTriggerManifoldToProcess()) {
            ManifoldData3D& m = bucket.TriggerManifolds[static_cast<std::size_t>(i)];
            if (m.GetIgnore()) {
                continue;
            }
            Entry3D* a = &_entries[static_cast<std::size_t>(m._entryIdxA)];
            Entry3D* b = &_entries[static_cast<std::size_t>(m._entryIdxB)];
            // TODO: triggers against static meshes walk the triangle manifolds.
            CollisionManifold3D temp{};
            temp._m._entryIdxA = m._entryIdxA;
            temp._m._entryIdxB = m._entryIdxB;
            UpdateManifoldCachedState(temp._m);
            CollisionManifoldInfo3D info{&temp, a, b, nullptr};
            if (NarrowChecks::Check(_tls.Narrow, ccd, _deltaTime, &info, false)) {
                m.TimeOfImpact = temp._m.TimeOfImpact;
            } else {
                m.SetIgnore(true);
            }
        }
        const std::int32_t collisions = static_cast<std::int32_t>(bucket.CollisionManifolds.size());
        for (std::int32_t i = bucket.GetNextCollisionManifoldToProcess(); i < collisions;
             i = bucket.GetNextCollisionManifoldToProcess()) {
            CollisionManifold3D* m = &bucket.CollisionManifolds[static_cast<std::size_t>(i)];
            CollisionManifoldInfo3D info{m, &_entries[static_cast<std::size_t>(m->_m._entryIdxA)],
                                         &_entries[static_cast<std::size_t>(m->_m._entryIdxB)], nullptr};
            if (!NarrowChecks::Check(_tls.Narrow, ccd, _deltaTime, &info, true)) {
                m->SetIgnore(true);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------
// Callbacks

namespace {
constexpr std::int32_t DynamicCollisionEnterExit = 0x6;
constexpr std::int32_t StaticCollisionEnterExit = 0x30;
constexpr std::int32_t DynamicTriggerEnterExit = 0x180;
constexpr std::int32_t StaticTriggerEnterExit = 0xc00;

bool Wants(const PhysicsCallbacksData* callbacks, std::int32_t mask) {
    return (static_cast<std::int32_t>(callbacks->Flags) & mask) != 0;
}
bool Wants(const PhysicsCallbacksData* callbacks, CallbackFlags flag) {
    return Wants(callbacks, static_cast<std::int32_t>(flag));
}

// Static colliders are recorded in the callbacks list with the map's guid in the entity
// slot (the original stores the AssetRefMap there).
EntityRef MapAsListEntry(const Map* map) {
    const auto raw = static_cast<std::uint64_t>(map->Guid.Value);
    EntityRef ref;
    ref.Index = static_cast<std::int32_t>(static_cast<std::uint32_t>(raw));
    ref.Version = static_cast<std::int32_t>(static_cast<std::uint32_t>(raw >> 32));
    return ref;
}

// The entity behind an entry no longer has what the scene needs: it is gone, lost its
// transform or collider, or the collider was disabled.
bool EntityLost(const FrameBase* f, const Entry3D* entry, const ComponentSet& minComponents) {
    return !f->Exists(entry->Ref) || !f->ComponentMask(entry->Ref.Index).IsSupersetOf(minComponents) ||
           entry->Collider3D->_disabled != 0;
}

// The entry's cached callbacks pointer follows a callbacks component added or removed by a
// callback.
void RefreshCallbacks(FrameBase* f, Entry3D* entry) {
    if (!entry->Ref.IsValid()) {
        return; // static colliders never have callbacks
    }
    const bool has = f->ComponentMask(entry->Ref.Index).IsSet(ComponentTypeId<PhysicsCallbacks3D>::Id);
    if (has != (entry->Callbacks != nullptr)) {
        PhysicsCallbacks3D* callbacks = nullptr;
        f->TryGetPointer<PhysicsCallbacks3D>(entry->Ref, callbacks);
        entry->Callbacks = callbacks;
    }
}

// An enabled, non-kinematic body on the entity.
bool HasDynamicBody(FrameBase* f, EntityRef entity) {
    PhysicsBody3D* body = nullptr;
    return entity.IsValid() && f->TryGetPointer<PhysicsBody3D>(entity, body) &&
           body->Status != PhysicsBodyStatus::None && !body->GetIsKinematic();
}

const Map* MapOf(FrameBase* frame, AssetGuid guid) {
    return guid.IsValid() ? frame->FindAsset<Map>(guid) : frame->get_Map();
}
} // namespace

// 0x7d624e8 - after a callback ran, a trigger pair is dropped when either entity lost its
// collider (the entry is ignored from then on) or when the pair no longer has exactly one
// trigger; otherwise the entries' callbacks pointers are refreshed.
bool Scene3D::CheckIgnoreTriggerManifold(FrameBase* frame, ManifoldData3D* m, Entry3D* entryA,
                                         Entry3D* entryB) {
    for (Entry3D* entry : {entryA, entryB}) {
        if (entry->Ref.IsValid() && EntityLost(frame, entry, _minComponentSet)) {
            entry->Ignore = true;
        }
    }
    if (entryA->Ignore || entryB->Ignore ||
        (entryA->Collider3D->_isTrigger == 1) == (entryB->Collider3D->_isTrigger == 1)) {
        m->SetIgnore(true);
        return true;
    }
    RefreshCallbacks(frame, entryA);
    RefreshCallbacks(frame, entryB);
    return false;
}

// 0x7d6279c - as the trigger check, but a collider that became a trigger also ignores its
// entry, and the pair needs a dynamic (enabled, non-kinematic) body on one side.
bool Scene3D::CheckIgnoreCollisionManifold(FrameBase* frame, ManifoldData3D* m, Entry3D* entryA,
                                           Entry3D* entryB) {
    for (Entry3D* entry : {entryA, entryB}) {
        if (entry->Ref.IsValid() &&
            (EntityLost(frame, entry, _minComponentSet) || entry->Collider3D->_isTrigger == 1)) {
            entry->Ignore = true;
        }
    }
    if (entryA->Ignore || entryB->Ignore ||
        (!HasDynamicBody(frame, entryA->Ref) && !HasDynamicBody(frame, entryB->Ref))) {
        m->SetIgnore(true);
        return true;
    }
    RefreshCallbacks(frame, entryA);
    RefreshCallbacks(frame, entryB);
    return false;
}

// 0x7d60450 - each side with callbacks gets Enter the first time the pair is seen (when it
// tracks enter/exit) and the stay callback every step. Both sides are called before either
// list is updated, and the lists are left alone when a callback made the pair ignored.
void Scene3D::HandleDynamicTrigger(FrameBase* f, ICollisionCallbacks3D* callbacks,
                                   ManifoldData3D* manifold, Entry3D* entryA, Entry3D* entryB) {
    struct Side {
        PhysicsCallbacks3D* callbacks = nullptr;
        bool tracks = false;
        bool known = false;
        std::int32_t index = 0;
    };
    const auto raise = [&](Entry3D* self, Entry3D* other, bool inverse) {
        Side side;
        side.callbacks = self->Callbacks;
        if (side.callbacks == nullptr || side.callbacks->Flags == CallbackFlags::None) {
            return side;
        }
        const TriggerInfo3D info(manifold, entryA, entryB, nullptr, false, inverse);
        side.tracks = Wants(side.callbacks, DynamicTriggerEnterExit);
        if (side.tracks) {
            if (side.callbacks->IsCollidingWith(f, other->Ref, side.index)) {
                side.known = true;
            } else if (Wants(side.callbacks, CallbackFlags::OnDynamicTriggerEnter)) {
                callbacks->OnTriggerEnter3D(f, info);
                _callbacksWereCalled = true;
            }
        }
        if (Wants(side.callbacks, CallbackFlags::OnDynamicTrigger)) {
            callbacks->OnTrigger3D(f, info);
            _callbacksWereCalled = true;
        }
        return side;
    };
    const Side a = raise(entryA, entryB, false);
    const Side b = raise(entryB, entryA, true);
    if (manifold->GetIgnore()) {
        return;
    }
    const auto record = [&](const Side& side, const Entry3D* other) {
        if (!side.tracks || !Wants(side.callbacks, DynamicTriggerEnterExit)) {
            return;
        }
        if (side.known) {
            side.callbacks->SetEntryIsActive(f, side.index, true);
        } else {
            side.callbacks->AddCollision(f, other->Ref);
        }
    };
    record(a, entryB);
    record(b, entryA);
}

// 0x7d60934 - only the entity side can have callbacks; the list entry identifies the static
// collider by the map and its index.
void Scene3D::HandleStaticTrigger(FrameBase* f, ICollisionCallbacks3D* callbacks, const Map* map,
                                  ManifoldData3D* manifold, Entry3D* entryA, Entry3D* entryB) {
    const bool entityIsA = entryA->IsDynamic();
    Entry3D* self = entityIsA ? entryA : entryB;
    const Entry3D* other = entityIsA ? entryB : entryA;
    PhysicsCallbacks3D* cb = self->Callbacks;
    if (cb == nullptr || cb->Flags == CallbackFlags::None) {
        return;
    }
    const TriggerInfo3D info(manifold, entryA, entryB, map, true, !entityIsA);
    const bool tracks = Wants(cb, StaticTriggerEnterExit);
    bool known = false;
    std::int32_t index = 0;
    if (tracks) {
        if (cb->IsCollidingWith(f, MapAsListEntry(map), other->StaticColliderIndex, index)) {
            known = true;
        } else if (Wants(cb, CallbackFlags::OnStaticTriggerEnter)) {
            callbacks->OnTriggerEnter3D(f, info);
            _callbacksWereCalled = true;
        }
    }
    if (Wants(cb, CallbackFlags::OnStaticTrigger)) {
        callbacks->OnTrigger3D(f, info);
        _callbacksWereCalled = true;
    }
    if (!tracks || manifold->GetIgnore() || !Wants(cb, StaticTriggerEnterExit)) {
        return;
    }
    if (known) {
        cb->SetEntryIsActive(f, index, true);
    } else {
        cb->AddCollision(f, MapAsListEntry(map), other->StaticColliderIndex,
                         other->Collider3D->_isTrigger == 1);
    }
}

// 0x7d60f3c - the dynamic trigger's pattern with the collision flags.
void Scene3D::HandleDynamicCollision(FrameBase* f, ICollisionCallbacks3D* callbacks,
                                     CollisionManifold3D* manifold, Entry3D* entryA, Entry3D* entryB) {
    struct Side {
        PhysicsCallbacks3D* callbacks = nullptr;
        bool tracks = false;
        bool known = false;
        std::int32_t index = 0;
    };
    const auto raise = [&](Entry3D* self, Entry3D* other, bool inverse) {
        Side side;
        side.callbacks = self->Callbacks;
        if (side.callbacks == nullptr || side.callbacks->Flags == CallbackFlags::None) {
            return side;
        }
        const CollisionInfo3D info(manifold, entryA, entryB, nullptr, nullptr, false, inverse, false,
                                   nullptr);
        side.tracks = Wants(side.callbacks, DynamicCollisionEnterExit);
        if (side.tracks) {
            if (side.callbacks->IsCollidingWith(f, other->Ref, side.index)) {
                side.known = true;
            } else if (Wants(side.callbacks, CallbackFlags::OnDynamicCollisionEnter)) {
                callbacks->OnCollisionEnter3D(f, info);
                _callbacksWereCalled = true;
            }
        }
        if (Wants(side.callbacks, CallbackFlags::OnDynamicCollision)) {
            callbacks->OnCollision3D(f, info);
            _callbacksWereCalled = true;
        }
        return side;
    };
    const Side a = raise(entryA, entryB, false);
    const Side b = raise(entryB, entryA, true);
    if (manifold->GetIgnore()) {
        return;
    }
    const auto record = [&](const Side& side, const Entry3D* other) {
        if (!side.tracks || !Wants(side.callbacks, DynamicCollisionEnterExit)) {
            return;
        }
        if (side.known) {
            side.callbacks->SetEntryIsActive(f, side.index, true);
        } else {
            side.callbacks->AddCollision(f, other->Ref);
        }
    };
    record(a, entryB);
    record(b, entryA);
}

// 0x7d61488 - the static trigger's pattern with the collision flags.
void Scene3D::HandleStaticCollision(FrameBase* f, ICollisionCallbacks3D* callbacks, const Map* map,
                                    CollisionManifold3D* manifold, Entry3D* entryA, Entry3D* entryB) {
    const bool entityIsA = entryA->IsDynamic();
    Entry3D* self = entityIsA ? entryA : entryB;
    const Entry3D* other = entityIsA ? entryB : entryA;
    PhysicsCallbacks3D* cb = self->Callbacks;
    if (cb == nullptr || cb->Flags == CallbackFlags::None) {
        return;
    }
    const CollisionInfo3D info(manifold, entryA, entryB, nullptr, map, true, !entityIsA, false, nullptr);
    const bool tracks = Wants(cb, StaticCollisionEnterExit);
    bool known = false;
    std::int32_t index = 0;
    if (tracks) {
        if (cb->IsCollidingWith(f, MapAsListEntry(map), other->StaticColliderIndex, index)) {
            known = true;
        } else if (Wants(cb, CallbackFlags::OnStaticCollisionEnter)) {
            callbacks->OnCollisionEnter3D(f, info);
            _callbacksWereCalled = true;
        }
    }
    if (Wants(cb, CallbackFlags::OnStaticCollision)) {
        callbacks->OnCollision3D(f, info);
        _callbacksWereCalled = true;
    }
    if (!tracks || manifold->GetIgnore() || !Wants(cb, StaticCollisionEnterExit)) {
        return;
    }
    if (known) {
        cb->SetEntryIsActive(f, index, true);
    } else {
        cb->AddCollision(f, MapAsListEntry(map), other->StaticColliderIndex, false);
    }
}

// Part of the exit pass of Task_Callbacks (0x7d617a0) for one list item no longer seen.
bool Scene3D::RaiseExit(FrameBase* f, ICollisionCallbacks3D* callbacks, const Map* map,
                        const Entry3D& entry, const PhysicsCallbacksEntry& item) {
    constexpr std::int32_t AnyExit = 0x924;
    constexpr std::int32_t DynamicExits = 0x104;
    constexpr std::int32_t StaticExits = 0x820;
    const PhysicsCallbacks3D* cb = entry.Callbacks;
    const std::int32_t flags = static_cast<std::int32_t>(cb->Flags);
    if ((flags & AnyExit) == 0) {
        return false;
    }
    const bool selfTrigger = entry.Collider3D->_isTrigger == 1;
    if ((item._flags & PhysicsCallbacksEntry::IsStatic) != 0) {
        if ((flags & StaticExits) == 0 || static_cast<std::int64_t>(item.Other.Raw()) != _currentMap.Value) {
            return false;
        }
        const bool itemTrigger = (item._flags & PhysicsCallbacksEntry::IsTrigger) != 0;
        if (itemTrigger && selfTrigger) {
            return false;
        }
        if (itemTrigger != selfTrigger) {
            if (HasFlag(cb->Flags, CallbackFlags::OnStaticTriggerExit)) {
                callbacks->OnTriggerExit3D(f, ExitInfo3D(map, entry.Ref, item.StaticColliderIndex));
                _callbacksWereCalled = true;
            }
        } else if (HasFlag(cb->Flags, CallbackFlags::OnStaticCollisionExit) && HasDynamicBody(f, entry.Ref)) {
            callbacks->OnCollisionExit3D(f, ExitInfo3D(map, entry.Ref, item.StaticColliderIndex));
            _callbacksWereCalled = true;
        }
        return false;
    }
    if ((flags & DynamicExits) == 0 || !f->Exists(item.Other) ||
        !f->ComponentMask(item.Other.Index).IsSupersetOf(_minComponentSet)) {
        return false;
    }
    const PhysicsCollider3D* otherCollider = f->GetPointer<PhysicsCollider3D>(item.Other);
    if (otherCollider->_disabled != 0) {
        return false;
    }
    const bool otherTrigger = otherCollider->_isTrigger == 1;
    if (selfTrigger && otherTrigger) {
        return false;
    }
    if (selfTrigger != otherTrigger) {
        if (HasFlag(cb->Flags, CallbackFlags::OnDynamicTriggerExit)) {
            callbacks->OnTriggerExit3D(f, ExitInfo3D(map, entry.Ref, item.Other));
            _callbacksWereCalled = true;
        }
        return false;
    }
    if (!HasFlag(cb->Flags, CallbackFlags::OnDynamicCollisionExit)) {
        return false;
    }
    PhysicsBody3D* otherBody = nullptr;
    f->TryGetPointer<PhysicsBody3D>(item.Other, otherBody);
    const bool otherDynamic = otherBody != nullptr && otherBody->Status != PhysicsBodyStatus::None &&
                              !otherBody->GetIsKinematic();
    if (!otherDynamic && !HasDynamicBody(f, entry.Ref)) {
        return false;
    }
    if (otherBody != nullptr && otherBody->Status == PhysicsBodyStatus::Asleep) {
        return true;
    }
    callbacks->OnCollisionExit3D(f, ExitInfo3D(map, entry.Ref, item.Other));
    _callbacksWereCalled = true;
    return false;
}

// Last part of Task_Callbacks (0x7d617a0): every awake entity entry with a callbacks list
// walks it. Pairs seen this step are reset for the next one (except against a sleeping
// body, which stays marked as seen); the others raise their exit callback when it is still
// meaningful (the other entity alive with an enabled collider, a trigger exit for exactly
// one trigger, a collision exit when a dynamic body is involved) and are removed by moving
// the last item into their place.
void Scene3D::HandleExits(FrameBase* f, ICollisionCallbacks3D* callbacks, const Map* map) {
    for (std::size_t i = static_cast<std::size_t>(_staticComponentsCount + _queriesCount - 1);
         i < _entries.size(); ++i) {
        Entry3D& entry = _entries[i];
        PhysicsCallbacks3D* cb = entry.Callbacks;
        if (cb == nullptr || cb->_listCount == 0 || entry._compoundRootIndexPlusOne >= 1) {
            continue;
        }
        if (entry.Body3D != nullptr && entry.Body3D->Status == PhysicsBodyStatus::Asleep) {
            continue;
        }
        PhysicsCallbacksEntry* items = cb->Entries(f);
        std::int32_t k = 0;
        while (k < cb->_listCount) {
            if (_callbacksWereCalled &&
                (!f->ComponentMask(entry.Ref.Index).IsSupersetOf(_minComponentSet) ||
                 entry.Collider3D->_disabled != 0)) {
                // The entity itself lost its collider: forget everything, no exits.
                cb->_listCount = 0;
                entry.Ignore = true;
                break;
            }
            PhysicsCallbacksEntry& item = items[k];
            if ((item._flags & PhysicsCallbacksEntry::IsActive) != 0) {
                PhysicsBody3D* otherBody = nullptr;
                const bool otherAsleep = f->TryGetPointer<PhysicsBody3D>(item.Other, otherBody) &&
                                         otherBody->Status == PhysicsBodyStatus::Asleep;
                if (!otherAsleep) {
                    item._flags &= ~PhysicsCallbacksEntry::IsActive;
                }
                ++k;
                continue;
            }
            // Raises the exit callback if it applies; true when the pair must stay listed
            // (a collision with a body that fell asleep).
            const bool keepActive = RaiseExit(f, callbacks, map, entry, item);
            if (keepActive) {
                item._flags |= PhysicsCallbacksEntry::IsActive;
                ++k;
                continue;
            }
            // Removed: the last item takes this slot, which is examined next.
            const std::int32_t last = cb->_listCount - 1;
            if (last >= 0) {
                cb->_listCount = last;
                if (k < last) {
                    items[k] = items[last];
                }
            }
        }
    }
}

// 0x7d617a0 - per bucket: collision manifolds (dynamic or static pair), then the trigger
// manifolds; once any callback ran, each further pair is first checked against what the
// callbacks may have changed. CCD pairs are raised by Task_CcdCallbacks instead.
void Scene3D::TaskCallbacks(FrameBase* frame, ICollisionCallbacks3D* callbacks) {
    if (callbacks == nullptr) {
        return;
    }
    const Map* map = MapOf(frame, _currentMap);
    constexpr std::int32_t Skip = ManifoldData3D::Ignore | ManifoldData3D::UsesCcd;
    for (Bucket3D& bucket : _buckets) {
        for (std::size_t i = 1; i < bucket.CollisionManifolds.size(); ++i) {
            CollisionManifold3D* m = &bucket.CollisionManifolds[i];
            if ((m->_m._flags & Skip) != 0) {
                continue;
            }
            Entry3D* a = &_entries[static_cast<std::size_t>(m->_m._entryIdxA)];
            Entry3D* b = &_entries[static_cast<std::size_t>(m->_m._entryIdxB)];
            if (_callbacksWereCalled && CheckIgnoreCollisionManifold(frame, &m->_m, a, b)) {
                continue;
            }
            if (m->_m.GetIsDynamic()) {
                HandleDynamicCollision(frame, callbacks, m, a, b);
            } else {
                HandleStaticCollision(frame, callbacks, map, m, a, b);
            }
        }
        // TODO(0x7d60c38, Quantum.Physics3D.Scene3D.HandleMeshCollision) and
        // TODO(0x7d62ad0, Quantum.Physics3D.Scene3D.CheckIgnoreMeshCollisionManifold): mesh
        // collision manifolds come here; static triangle meshes are not reimplemented yet.
        for (std::size_t i = 1; i < bucket.TriggerManifolds.size(); ++i) {
            ManifoldData3D* m = &bucket.TriggerManifolds[i];
            if ((m->_flags & Skip) != 0) {
                continue;
            }
            Entry3D* a = &_entries[static_cast<std::size_t>(m->_entryIdxA)];
            Entry3D* b = &_entries[static_cast<std::size_t>(m->_entryIdxB)];
            if (_callbacksWereCalled && CheckIgnoreTriggerManifold(frame, m, a, b)) {
                continue;
            }
            if (m->GetIsDynamic()) {
                HandleDynamicTrigger(frame, callbacks, m, a, b);
            } else {
                HandleStaticTrigger(frame, callbacks, map, m, a, b);
            }
        }
    }
    HandleExits(frame, callbacks, map);
}

// 0x7d5fb98 - the live CCD pairs of every bucket (collision, then trigger manifolds) sorted
// by time of impact.
void Scene3D::TaskSortCcdManifolds() {
    _ccdManifolds.clear();
    constexpr std::int32_t Mask = ManifoldData3D::Ignore | ManifoldData3D::UsesCcd;
    for (Bucket3D& bucket : _buckets) {
        for (std::size_t i = 1; i < bucket.CollisionManifolds.size(); ++i) {
            CollisionManifold3D* m = &bucket.CollisionManifolds[i];
            if ((m->_m._flags & Mask) == ManifoldData3D::UsesCcd) {
                _ccdManifolds.push_back({&m->_m, m});
            }
        }
        // TODO: mesh collision manifolds (static triangle meshes are not reimplemented yet).
        for (std::size_t i = 1; i < bucket.TriggerManifolds.size(); ++i) {
            ManifoldData3D* m = &bucket.TriggerManifolds[i];
            if ((m->_flags & Mask) == ManifoldData3D::UsesCcd) {
                _ccdManifolds.push_back({m, nullptr});
            }
        }
    }
    QuickSortManifoldsByTimeOfImpact(0, static_cast<std::int32_t>(_ccdManifolds.size()) - 1);
}

// 0x7d5fe60 - Lomuto partition on the last item, recursing left and looping right.
void Scene3D::QuickSortManifoldsByTimeOfImpact(std::int32_t lo, std::int32_t hi) {
    while (lo < hi) {
        const FP pivot = _ccdManifolds[static_cast<std::size_t>(hi)].Data->TimeOfImpact;
        std::int32_t store = lo;
        for (std::int32_t j = lo; j < hi; ++j) {
            if (_ccdManifolds[static_cast<std::size_t>(j)].Data->TimeOfImpact < pivot) {
                std::swap(_ccdManifolds[static_cast<std::size_t>(store)], _ccdManifolds[static_cast<std::size_t>(j)]);
                ++store;
            }
        }
        std::swap(_ccdManifolds[static_cast<std::size_t>(store)], _ccdManifolds[static_cast<std::size_t>(hi)]);
        QuickSortManifoldsByTimeOfImpact(lo, store - 1);
        lo = store + 1;
    }
}

// 0x7d5fefc - CCD pairs in time-of-impact order: a pair is skipped when either body already
// hit something earlier; otherwise its callbacks run as in Task_Callbacks and a collision
// that stays in play becomes both bodies' first time of impact.
void Scene3D::TaskCcdCallbacks(FrameBase* frame, ICollisionCallbacks3D* callbacks) {
    const Map* map = MapOf(frame, _currentMap);
    for (const CcdManifold& item : _ccdManifolds) {
        ManifoldData3D* m = item.Data;
        Entry3D* entryA = &_entries[static_cast<std::size_t>(m->_entryIdxA)];
        Entry3D* entryB = &_entries[static_cast<std::size_t>(m->_entryIdxB)];
        Entry3D* rootA = Root(entryA);
        Entry3D* rootB = Root(entryB);
        if ((rootA->_useCCD && rootA->_firstTimeOfImpact < m->TimeOfImpact) ||
            (rootB->_useCCD && rootB->_firstTimeOfImpact < m->TimeOfImpact)) {
            continue;
        }
        if (m->GetIsTrigger()) {
            if (_callbacksWereCalled && CheckIgnoreTriggerManifold(frame, m, entryA, entryB)) {
                continue;
            }
            if (callbacks == nullptr) {
                continue;
            }
            if (m->GetIsDynamic()) {
                HandleDynamicTrigger(frame, callbacks, m, entryA, entryB);
            } else {
                HandleStaticTrigger(frame, callbacks, map, m, entryA, entryB);
            }
            continue;
        }
        // TODO(0x7d60c38, Quantum.Physics3D.Scene3D.HandleMeshCollision): mesh pairs
        // (NextInMesh set) need static triangle meshes.
        if (_callbacksWereCalled && CheckIgnoreCollisionManifold(frame, m, entryA, entryB)) {
            continue;
        }
        if (callbacks != nullptr) {
            if (m->GetIsDynamic()) {
                HandleDynamicCollision(frame, callbacks, item.Collision, entryA, entryB);
            } else {
                HandleStaticCollision(frame, callbacks, map, item.Collision, entryA, entryB);
            }
        }
        if (!m->GetIgnore()) {
            rootA->_firstTimeOfImpact = m->TimeOfImpact;
            rootB->_firstTimeOfImpact = m->TimeOfImpact;
        }
    }
}

// 0x7d62ca8 - runs only when callbacks ran: the dynamic entries re-read their body (it may
// have been added, removed, disabled or made kinematic); entries whose entity lost its
// collider are ignored, triggers lose their body.
void Scene3D::TaskPostCallbacksUpdateEntries(FrameBase* frame) {
    if (!_callbacksWereCalled) {
        return;
    }
    for (std::size_t i = static_cast<std::size_t>(_staticComponentsCount + _queriesCount - 1);
         i < _entries.size(); ++i) {
        Entry3D& entry = _entries[i];
        entry.ShouldIntegrateVelocity = false;
        entry.HasDynamicBody = false;
        entry.Body3D = nullptr;
        if (EntityLost(frame, &entry, _minComponentSet)) {
            entry.Ignore = true;
        }
        if (entry.Ignore || entry.Collider3D->_isTrigger == 1) {
            continue;
        }
        PhysicsBody3D* body = nullptr;
        if (!frame->TryGetPointer<PhysicsBody3D>(entry.Ref, body) || body->Status == PhysicsBodyStatus::None) {
            continue;
        }
        entry.Body3D = body;
        if (body->GetIsKinematic()) {
            continue;
        }
        entry.HasDynamicBody = true;
        if (entry._compoundRootIndexPlusOne >= 1) {
            continue;
        }
        entry.ShouldIntegrateVelocity = body->Status == PhysicsBodyStatus::Active;
        PhysicsJoints3D* joints = nullptr;
        if (frame->TryGetPointer<PhysicsJoints3D>(entry.Ref, joints) && joints->_count != 0) {
            entry.Joints = joints;
        }
    }
}

// 0x7d63188 - per bucket, the collision manifolds the solver works on: every pair still
// active, minus (when callbacks ran) pairs with an ignored entry or without a dynamic body.
void Scene3D::TaskPostCallbacksUpdateManifolds() {
    for (Bucket3D& bucket : _buckets) {
        for (std::size_t i = 1; i < bucket.CollisionManifolds.size(); ++i) {
            CollisionManifold3D* m = &bucket.CollisionManifolds[i];
            if (m->GetIgnore()) {
                continue;
            }
            if (_callbacksWereCalled) {
                const Entry3D& a = _entries[static_cast<std::size_t>(m->_m._entryIdxA)];
                const Entry3D& b = _entries[static_cast<std::size_t>(m->_m._entryIdxB)];
                if (a.Ignore || b.Ignore || (!a.HasDynamicBody && !b.HasDynamicBody)) {
                    m->SetIgnore(true);
                    continue;
                }
            }
            bucket.SolverManifolds.push_back(m);
        }
        // TODO(0x7d63188, Quantum.Physics3D.Scene3D.Task_PostCallbacksUpdateManifolds): mesh
        // manifolds (their live triangle manifolds join the list and set the earliest time
        // of impact); static triangle meshes are not reimplemented yet.
    }
}

// 0x7d64a80 - bodies whose first time of impact is set move up to it before the solver
// (the rest of the step is integrated after it).
void Scene3D::TaskIntegrateFirstTOI() {
    const PhysicsSceneSettings& s = CachedSettings;
    const bool applyMin = Raw(s.MinLinearIntegration) > 0 || Raw(s.MinAngularIntegration) > 0;
    const bool integrateAngular = s.UseAngularVelocityEnabled();
    for (std::size_t i = static_cast<std::size_t>(_staticComponentsCount + _queriesCount - 1);
         i < _entries.size(); ++i) {
        Entry3D* entry = &_entries[i];
        if (entry->ShouldIntegrateVelocity && Raw(entry->_firstTimeOfImpact) >= 1) {
            Entry3D::IntegrateVelocity(entry, entry->_firstTimeOfImpact, s, applyMin, integrateAngular);
        }
    }
}

// 0x7d5b754
Island3D* Scene3D::GetNextIsland() {
    Island3D& island = _islands.emplace_back();
    island.Reset();
    return &island;
}

// 0x7d6351c - in bucket order over the solver manifolds: a pair whose bodies have no island
// starts one, a pair touching one island joins it, a pair bridging two islands merges the
// second into the first (which is then ignored). Only dynamic bodies (compound roots) join
// islands; every body that joins is also listed as a solver entry.
void Scene3D::TaskBuildIslands() {
    _islands.clear();
    _solverEntries.clear();
    const auto join = [this](Island3D* island, Entry3D* root) {
        if (root->HasDynamicBody) {
            island->AddEntry(root);
            root->Island = island;
            _solverEntries.push_back(root);
        }
    };
    for (Bucket3D& bucket : _buckets) {
        for (CollisionManifold3D* m : bucket.SolverManifolds) {
            if (m->_m._triIdx != 0) {
                continue; // triangle manifolds come with their mesh manifold
            }
            Entry3D* rootA = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxA)]);
            Entry3D* rootB = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxB)]);
            Island3D* islandA = rootA->Island;
            Island3D* islandB = rootB->Island;
            if (islandA == nullptr && islandB == nullptr) {
                Island3D* island = GetNextIsland();
                join(island, rootA);
                join(island, rootB);
                island->AddManifold(m);
            } else if (islandA == nullptr) {
                join(islandB, rootA);
                islandB->AddManifold(m);
            } else if (islandB == nullptr) {
                join(islandA, rootB);
                islandA->AddManifold(m);
            } else if (islandA == islandB) {
                islandA->AddManifold(m);
            } else {
                for (Entry3D* entry : islandB->Entries) {
                    entry->Island = islandA;
                    islandA->AddEntry(entry);
                }
                for (CollisionManifold3D* other : islandB->CollisionManifolds) {
                    islandA->AddManifold(other);
                }
                islandA->AddManifold(m);
                islandB->Ignore = true;
                rootB->Island = islandA;
            }
        }
        // TODO(0x7d6351c, Quantum.Physics3D.Scene3D.Task_BuildIslands): mesh manifolds (the
        // entity joins or starts an island); static triangle meshes are not reimplemented yet.
    }
}

// ---------------------------------------------------------------------------------------
// Solver, integration, sleeping

namespace {
// Raw fixed-point helpers for the solver: every product is shifted on its own, as the
// original computes them.
FP::Raw Dot(const FPVector3& a, const FPVector3& b) {
    return Mul(Raw(a.Y), Raw(b.Y)) + Mul(Raw(a.X), Raw(b.X)) + Mul(Raw(a.Z), Raw(b.Z));
}
FP::Raw LengthSquared(const FPVector3& v) { return Dot(v, v); }
// a / b with the sign rules of the original's division; a zero divisor gives zero (what the
// hardware division of the original returns).
FP::Raw Divide(FP::Raw numerator, FP::Raw divisor) { return divisor == 0 ? 0 : numerator / divisor; }
} // namespace

// 0x7d6e2e0
void AccumulatedImpulse::Accumulate(const AccumulatedImpulse& impulse, std::int32_t flags) {
    const auto accumulate = [flags](FPVector3& sum, const FPVector3& add) {
        if ((flags & ImpulseAdd) != 0) {
            sum = FPVector3{FP::FromRaw(Raw(sum.X) + Raw(add.X)), FP::FromRaw(Raw(sum.Y) + Raw(add.Y)),
                            FP::FromRaw(Raw(sum.Z) + Raw(add.Z))};
            return;
        }
        const FP::Raw along = Dot(add, sum);
        if (along <= 0) {
            sum = FPVector3{FP::FromRaw(Raw(sum.X) + Raw(add.X)), FP::FromRaw(Raw(sum.Y) + Raw(add.Y)),
                            FP::FromRaw(Raw(sum.Z) + Raw(add.Z))};
            return;
        }
        // Only the part of the new impulse beyond what the sum already covers.
        const FP::Raw lengthSq = LengthSquared(add);
        const FP::Raw beyond = lengthSq - along;
        if (beyond <= 0) {
            return;
        }
        const FP::Raw share = (beyond << 16) / lengthSq;
        sum.X = FP::FromRaw(Raw(sum.X) + Mul(share, Raw(add.X)));
        sum.Y = FP::FromRaw(Raw(sum.Y) + Mul(Raw(add.Y), share));
        sum.Z = FP::FromRaw(Raw(sum.Z) + Mul(Raw(add.Z), share));
    };
    if ((flags & ImpulseLinear) != 0) {
        accumulate(Velocity, impulse.Velocity);
    }
    if ((flags & ImpulseAngular) != 0) {
        accumulate(AngularVelocity, impulse.AngularVelocity);
    }
}

// 0x7d6e494
void AccumulatedImpulse::Integrate(const PhysicsBody3D* body, const FPVector3& impulse, const FPVector3& r,
                                   bool useRotation) {
    const FP::Raw massInverse = body->_massInverseRawInt;
    Velocity.X = FP::FromRaw(Raw(Velocity.X) + Mul(Raw(impulse.X), massInverse));
    Velocity.Y = FP::FromRaw(Raw(Velocity.Y) + Mul(Raw(impulse.Y), massInverse));
    Velocity.Z = FP::FromRaw(Raw(Velocity.Z) + Mul(Raw(impulse.Z), massInverse));
    if (!useRotation) {
        return;
    }
    const FP::Raw inertiaInverse = body->_inertiaTensorInverse.RawIntX;
    const FP::Raw cx = Mul(Raw(impulse.Z), Raw(r.Y)) - Mul(Raw(impulse.Y), Raw(r.Z));
    const FP::Raw cy = Mul(Raw(impulse.X), Raw(r.Z)) - Mul(Raw(r.X), Raw(impulse.Z));
    const FP::Raw cz = Mul(Raw(r.X), Raw(impulse.Y)) - Mul(Raw(impulse.X), Raw(r.Y));
    AngularVelocity.X = FP::FromRaw(Raw(AngularVelocity.X) + Mul(cx, inertiaInverse));
    AngularVelocity.Y = FP::FromRaw(Raw(AngularVelocity.Y) + Mul(cy, inertiaInverse));
    AngularVelocity.Z = FP::FromRaw(Raw(AngularVelocity.Z) + Mul(cz, inertiaInverse));
}

// 0x7d6e564
void AccumulatedImpulse::Apply(Entry3D* entry) const {
    const bool linear = Raw(Velocity.X) != 0 || Raw(Velocity.Y) != 0 || Raw(Velocity.Z) != 0;
    const bool angular = Raw(AngularVelocity.X) != 0 || Raw(AngularVelocity.Y) != 0 ||
                         Raw(AngularVelocity.Z) != 0;
    if (!linear && !angular) {
        return;
    }
    PhysicsBody3D* body = entry->Body3D;
    if (body->Status == PhysicsBodyStatus::Asleep) {
        body->Status = PhysicsBodyStatus::Active;
        body->_sleepTimeRawInt = 0;
    }
    Entry3D* root = entry->_compoundRoot != nullptr ? entry->_compoundRoot : entry;
    root->ShouldIntegrateVelocity = true;
    body->Velocity.X = FP::FromRaw(Raw(body->Velocity.X) + Raw(Velocity.X));
    body->Velocity.Y = FP::FromRaw(Raw(body->Velocity.Y) + Raw(Velocity.Y));
    body->Velocity.Z = FP::FromRaw(Raw(body->Velocity.Z) + Raw(Velocity.Z));
    const auto freeze = static_cast<std::uint8_t>(body->RotationFreeze);
    if ((freeze & 1) == 0) body->AngularVelocity.X = FP::FromRaw(Raw(body->AngularVelocity.X) + Raw(AngularVelocity.X));
    if ((freeze & 2) == 0) body->AngularVelocity.Y = FP::FromRaw(Raw(body->AngularVelocity.Y) + Raw(AngularVelocity.Y));
    if ((freeze & 4) == 0) body->AngularVelocity.Z = FP::FromRaw(Raw(body->AngularVelocity.Z) + Raw(AngularVelocity.Z));
}

namespace {
PhysicsMaterial* MaterialOf(FrameBase* frame, const Entry3D* entry) {
    PhysicsMaterial* material = nullptr;
    const AssetGuid guid = entry->Collider3D->_material.Id;
    if (guid.IsValid()) {
        frame->TryFindAsset<PhysicsMaterial>(guid, material);
    }
    return material;
}

FP Combine(PhysicsCombineFunction a, PhysicsCombineFunction b, FP va, FP vb) {
    switch (std::min(static_cast<std::int32_t>(a), static_cast<std::int32_t>(b))) {
    case 1: return va > vb ? va : vb;
    case 2: return va < vb ? va : vb;
    case 3: return FP::FromRaw((Raw(va) + Raw(vb)) >> 1);
    case 4: return FP::FromRaw(Mul(Raw(vb), Raw(va)));
    default: return FP::Zero();
    }
}

// The rotational part of a body's perceived mass for a contact offset r along a unit
// direction d (products of d's components precomputed): |r x d|^2 * |Right|^2 * the inverse
// inertia's X entry, the original's isotropic approximation of the inertia.
FP::Raw AngularMassTerm(const Entry3D* entry, const FPVector3& r, FP::Raw dxx, FP::Raw dyy,
                        FP::Raw dzz, FP::Raw dzx, FP::Raw dyx, FP::Raw dzy) {
    const FP::Raw rx = Raw(r.X), ry = Raw(r.Y), rz = Raw(r.Z);
    const FP::Raw zz = Mul(rz, rz), yy = Mul(ry, ry), xx = Mul(rx, rx);
    const FP::Raw yz = Mul(ry, rz), xy = Mul(rx, ry), xz = Mul(rx, rz);
    FP::Raw cross = Mul(zz, dyy) + Mul(zz, dxx);
    cross += Mul(yy, dxx);
    cross += Mul(yy, dzz);
    cross -= (yz * dzy) >> 15;
    cross += Mul(xx, dyy);
    cross += Mul(xx, dzz);
    cross -= ((xy * dyx) >> 15) + ((xz * dzx) >> 15);
    const FP::Raw rightSq = LengthSquared(entry->Narrow.RotatedRight);
    const FP::Raw inertia = Mul(rightSq, entry->Body3D->_inertiaTensorInverse.RawIntX);
    return Mul(inertia, cross);
}
} // namespace

// 0x7d64de0 - the solver works on the buckets' solver manifolds in bucket order; each gets
// a constraint slot per contact point (the legacy solver adds one at the average point of a
// multi-point manifold).
void Scene3D::TaskCollisionSolverCacheManifolds() {
    _collisionSolverManifolds.clear();
    const bool legacy = CachedSettings.SolverType == ConstraintsSolverType::LegacyGaussSeidel;
    std::int32_t constraints = 0;
    for (Bucket3D& bucket : _buckets) {
        for (CollisionManifold3D* m : bucket.SolverManifolds) {
            _collisionSolverManifolds.push_back(m);
            m->_constraintDataIdx = constraints;
            constraints += m->_contactPointsCount;
            if (legacy && m->_contactPointsCount >= 2) {
                ++constraints;
            }
        }
    }
    _collisionSolverData.assign(static_cast<std::size_t>(constraints), ContactConstraint{});
}

namespace {
// The combined material values of a pair.
void CombineMaterials(FrameBase* frame, CollisionManifold3D* m, const Entry3D* entryA, const Entry3D* entryB) {
    PhysicsMaterial defaults;
    const PhysicsMaterial* matA = MaterialOf(frame, entryA);
    const PhysicsMaterial* matB = MaterialOf(frame, entryB);
    if (matA == nullptr) matA = &defaults;
    if (matB == nullptr) matB = &defaults;
    m->CombinedRestitution = Combine(matA->RestitutionCombineFunction, matB->RestitutionCombineFunction,
                                     matA->Restitution, matB->Restitution);
    m->CombinedStaticFriction = Combine(matA->FrictionCombineFunction, matB->FrictionCombineFunction,
                                        matA->FrictionStatic, matB->FrictionStatic);
    m->CombinedDynamicFriction = Combine(matA->FrictionCombineFunction, matB->FrictionCombineFunction,
                                         matA->FrictionDynamic, matB->FrictionDynamic);
}
} // namespace

// 0x7d572fc - combined material values and one constraint per contact point (plus one at
// the average point for multi-point manifolds): offsets from both centres of mass (at the
// manifold's time of impact) and the perceived mass along the normal.
void Scene3D::TaskCollisionSolverInitLegacyGS(FrameBase* frame) {
    for (CollisionManifold3D* m : _collisionSolverManifolds) {
        Entry3D* entryA = &_entries[static_cast<std::size_t>(m->_m._entryIdxA)];
        Entry3D* entryB = &_entries[static_cast<std::size_t>(m->_m._entryIdxB)];
        Entry3D* rootA = Root(entryA);
        Entry3D* rootB = Root(entryB);
        CombineMaterials(frame, m, entryA, entryB);
        const std::int32_t points = m->_contactPointsCount + (m->_contactPointsCount >= 2 ? 1 : 0);
        const FP::Raw nx = Raw(m->Normal.X), ny = Raw(m->Normal.Y), nz = Raw(m->Normal.Z);
        const FP::Raw nxx = Mul(nx, nx), nyy = Mul(ny, ny), nzz = Mul(nz, nz);
        const FP::Raw nzx = Mul(nz, nx), nyx = Mul(ny, nx), nzy = Mul(nz, ny);
        for (std::int32_t p = 0; p < points; ++p) {
            const FPVector3 contact = p < m->_contactPointsCount ? m->_contactPoints[p]
                                                                 : CollisionManifold3D::GetAverageContactPoint(m);
            ContactConstraint& c = _collisionSolverData[static_cast<std::size_t>(m->_constraintDataIdx + p)];
            c.Manifold = m;
            FP::Raw k = 0;
            const auto offset = [&](Entry3D* root) {
                FPVector3 centre = root->Transform3D->Position + root->RotatedCenterOfMass;
                const FP::Raw toi = Raw(m->_m.TimeOfImpact) - Raw(root->_firstTimeOfImpact);
                if (toi != 0 && root->Body3D != nullptr) {
                    centre = centre + root->Body3D->Velocity * FP::FromRaw(toi);
                }
                return contact - centre;
            };
            c.Ra = offset(rootA);
            c.Rb = offset(rootB);
            if (rootA->HasDynamicBody) {
                c.Flags |= HasDynamicBodyA;
                k += rootA->Body3D->_massInverseRawInt;
                k += AngularMassTerm(entryA, c.Ra, nxx, nyy, nzz, nzx, nyx, nzy);
            }
            if (rootB->HasDynamicBody) {
                c.Flags |= HasDynamicBodyB;
                k += rootB->Body3D->_massInverseRawInt;
                k += AngularMassTerm(entryB, c.Rb, nxx, nyy, nzz, nzx, nyx, nzy);
            }
            c.NormalMass = FP::FromRaw(k);
        }
    }
}

// 0x7d589b0 - the impulses for one contact: relative velocity at the contact (every body
// counts, kinematic ones too); a separating contact gets nothing. Otherwise the island is
// marked unsolved, the normal impulse is -(1 + restitution) * vn / normalMass (restitution
// dropped when the relative speed is within what gravity adds in a step) and friction acts
// along the tangent: the full tangential impulse while within the static coefficient,
// else dynamic friction * the normal impulse. The changes are only collected here.
AccumulatedImpulseCollection Scene3D::ComputeImpulse(const ContactConstraint* cst, const CollisionManifold3D* m,
                                                     Entry3D* entryA, Entry3D* entryB, bool useRotation,
                                                     bool& solved) {
    AccumulatedImpulseCollection result{};
    const FPVector3& ra = cst->Ra;
    const FPVector3& rb = cst->Rb;
    const FP::Raw nx = Raw(m->Normal.X), ny = Raw(m->Normal.Y), nz = Raw(m->Normal.Z);
    const PhysicsBody3D* bodyA = entryA->Body3D;
    const PhysicsBody3D* bodyB = entryB->Body3D;
    const bool dynamicA = entryA->HasDynamicBody;
    const bool dynamicB = entryB->HasDynamicBody;

    // Relative velocity at the contact: (vB + wB x rB) - (vA + wA x rA).
    FP::Raw vx = 0, vy = 0, vz = 0;
    if (bodyA != nullptr) {
        const FPVector3& v = bodyA->Velocity;
        if (useRotation) {
            const FPVector3& w = bodyA->AngularVelocity;
            vx = Mul(Raw(ra.Y), Raw(w.Z)) - (Raw(v.X) + Mul(Raw(ra.Z), Raw(w.Y)));
            vy = Mul(Raw(ra.Z), Raw(w.X)) - (Raw(v.Y) + Mul(Raw(ra.X), Raw(w.Z)));
            vz = Mul(Raw(ra.X), Raw(w.Y)) - (Raw(v.Z) + Mul(Raw(ra.Y), Raw(w.X)));
        } else {
            vx = -Raw(v.X);
            vy = -Raw(v.Y);
            vz = -Raw(v.Z);
        }
    }
    if (bodyB != nullptr) {
        const FPVector3& v = bodyB->Velocity;
        vx += Raw(v.X);
        vy += Raw(v.Y);
        vz += Raw(v.Z);
        if (useRotation) {
            const FPVector3& w = bodyB->AngularVelocity;
            vx = vx + Mul(Raw(rb.Z), Raw(w.Y)) - Mul(Raw(rb.Y), Raw(w.Z));
            vy = vy - Mul(Raw(rb.Z), Raw(w.X)) + Mul(Raw(rb.X), Raw(w.Z));
            vz = vz + Mul(Raw(rb.Y), Raw(w.X)) - Mul(Raw(rb.X), Raw(w.Y));
        }
    }
    const FP::Raw vn = Mul(vy, ny) + Mul(vx, nx) + Mul(vz, nz);
    if (vn >= 1) {
        return result; // separating
    }
    solved = false;

    // Tangent direction (unit when long enough) and its component products.
    const FP::Raw tx = vx - Mul(vn, nx);
    const FP::Raw ty = vy - Mul(vn, ny);
    const FP::Raw tz = vz - Mul(vn, nz);
    const FP::Raw tangentSq = Mul(ty, ty) + Mul(tx, tx) + Mul(tz, tz);
    const bool hasTangent = tangentSq > 0x41;
    FP::Raw ux = tx, uy = ty, uz = tz; // unit tangent (left unnormalised when too short)
    FP::Raw txx = 0, tyy = 0, tzz = 0, txy = 0, txz = 0, tyz = 0;
    if (tangentSq >= 0x42) {
        const FP::Raw inverseLength = (FP::Raw{1} << 32) / Photon::Deterministic::FPMath::SqrtRaw(tangentSq);
        ux = Mul(inverseLength, tx);
        uy = Mul(inverseLength, ty);
        uz = Mul(inverseLength, tz);
        txx = Mul(ux, ux);
        tyy = Mul(uy, uy);
        tzz = Mul(uz, uz);
        txy = Mul(uy, ux);
        txz = Mul(uz, ux);
        tyz = Mul(uz, uy);
    }
    FP::Raw tangentMass = 0;
    if (hasTangent && dynamicA) {
        tangentMass = bodyA->_massInverseRawInt + AngularMassTerm(entryA, ra, txx, tyy, tzz, txz, txy, tyz);
    }
    if (hasTangent && dynamicB) {
        tangentMass = tangentMass + bodyB->_massInverseRawInt +
                      AngularMassTerm(entryB, rb, txx, tyy, tzz, txz, txy, tyz);
    }

    // Normal impulse.
    const FPVector3& g = CachedSettings.Gravity;
    const FP::Raw restingSpeedSq = Mul(LengthSquared(g), Raw(_deltaTime)) + 0x41;
    const FP::Raw speedSq = Mul(vy, vy) + Mul(vx, vx) + Mul(vz, vz);
    const FP::Raw factor = speedSq < restingSpeedSq ? -(FP::Raw{1} << 16)
                                                    : -(FP::Raw{1} << 16) - Raw(m->CombinedRestitution);
    const FP::Raw jn = Divide((factor * vn) & ~FP::Raw{0xffff}, Raw(cst->NormalMass));
    const FPVector3 normalImpulse{FP::FromRaw(Mul(jn, nx)), FP::FromRaw(Mul(jn, ny)), FP::FromRaw(Mul(jn, nz))};
    if (dynamicA) {
        result.A.Integrate(bodyA, -normalImpulse, ra, useRotation);
    }
    if (dynamicB) {
        result.B.Integrate(bodyB, normalImpulse, rb, useRotation);
    }

    // Friction.
    if (!hasTangent) {
        return result;
    }
    const FP::Raw along = Mul(uy, vy) + Mul(uz, vz) + Mul(ux, vx);
    const FP::Raw jt = Divide(-(along << 16), tangentMass);
    if (jt == 0) {
        return result;
    }
    FPVector3 friction;
    const FP::Raw staticLimit = Mul(Raw(m->CombinedStaticFriction), jn);
    if ((jt < 0 ? -jt : jt) < staticLimit) {
        friction = FPVector3{FP::FromRaw(Mul(jt, ux)), FP::FromRaw(Mul(jt, uy)), FP::FromRaw(Mul(jt, uz))};
    } else {
        const FP::Raw dynamicFriction = Raw(m->CombinedDynamicFriction);
        friction = FPVector3{FP::FromRaw(Mul(dynamicFriction, (-(jn * ux)) >> 16)),
                             FP::FromRaw(Mul(dynamicFriction, (-(jn * uy)) >> 16)),
                             FP::FromRaw(Mul(dynamicFriction, (-(jn * uz)) >> 16))};
    }
    if (dynamicA) {
        result.A.Integrate(bodyA, -friction, ra, useRotation);
    }
    if (dynamicB) {
        result.B.Integrate(bodyB, friction, rb, useRotation);
    }
    return result;
}

// 0x7d58768 - a single contact point gives the whole impulse; with several, the angular
// parts of the per-point impulses are averaged (only with rotation) and the linear part
// comes from the average contact point. `flags` chooses how the result joins the sum.
void Scene3D::AccumulateImpulses(const ContactConstraint* cst, CollisionManifold3D* m, Entry3D* entryA,
                                 Entry3D* entryB, bool useRotation, AccumulatedImpulseCollection& accumulated,
                                 bool& solved, std::int32_t flags) {
    const std::int32_t points = m->_contactPointsCount;
    if (points == 1) {
        accumulated.Accumulate(ComputeImpulse(cst, m, entryA, entryB, useRotation, solved),
                               flags | ImpulseLinear | ImpulseAngular);
        return;
    }
    if (useRotation) {
        AccumulatedImpulseCollection angular{};
        for (std::int32_t p = 0; p < points; ++p) {
            angular.Accumulate(ComputeImpulse(cst + p, m, entryA, entryB, true, solved),
                               ImpulseAngular | ImpulseAdd);
        }
        const auto average = [points](FPVector3& v) {
            v = FPVector3{FP::FromRaw(Raw(v.X) / points), FP::FromRaw(Raw(v.Y) / points),
                          FP::FromRaw(Raw(v.Z) / points)};
        };
        if (entryA->HasDynamicBody && points != 0) {
            average(angular.A.AngularVelocity);
        }
        if (entryB->HasDynamicBody && points != 0) {
            average(angular.B.AngularVelocity);
        }
        accumulated.Accumulate(angular, flags | ImpulseAngular);
    }
    // The constraint after the points is the one at the average contact point.
    accumulated.Accumulate(ComputeImpulse(cst + points, m, entryA, entryB, useRotation, solved),
                           flags | ImpulseLinear);
}

// 0x7d57ebc - one solver iteration (the task runs once per SolverIterations, each pass over
// the islands). An island is solved again only while its previous pass still found
// approaching contacts: the pass marks it solved and any impulse clears the mark. Per
// manifold the impulses are computed from the current velocities and applied right away
// (Gauss-Seidel).
void Scene3D::TaskVelocitySolverLegacyGS() {
    const bool useRotation = CachedSettings.UseAngularVelocityEnabled();
    for (Island3D& island : _islands) {
        if (island.Ignore || island.VelocityConstraintsSolved) {
            continue;
        }
        island.VelocityConstraintsSolved = true;
        // TODO(0x7d57ebc, Quantum.Physics3D.Scene3D.Task_VelocitySolverLegacyGS): the island's
        // joint constraints are solved first (Distance/Spring/HingeJointConstraint
        // SolveVelocityConstraint); joints are not reimplemented yet.
        for (CollisionManifold3D* m : island.CollisionManifolds) {
            Entry3D* entryA = &_entries[static_cast<std::size_t>(m->_m._entryIdxA)];
            Entry3D* entryB = &_entries[static_cast<std::size_t>(m->_m._entryIdxB)];
            AccumulatedImpulseCollection impulses{};
            // TODO: a mesh manifold (NextInMesh != 0) adds the impulses of its triangle
            // manifolds (the first added, the rest accumulated) and applies them to A only;
            // static triangle meshes are not reimplemented yet.
            AccumulateImpulses(&_collisionSolverData[static_cast<std::size_t>(m->_constraintDataIdx)], m, entryA,
                               entryB, useRotation, impulses, island.VelocityConstraintsSolved, ImpulseAdd);
            if (entryA->HasDynamicBody) {
                impulses.A.Apply(entryA);
            }
            if (entryB->HasDynamicBody) {
                impulses.B.Apply(entryB);
            }
            m->_flags |= CollisionManifold3D::Solved;
        }
    }
}

// ---------------------------------------------------------------------------------------
// Hybrid Jacobi/Gauss-Seidel solver: every iteration computes all contact impulses from the
// same velocities (Compute), averages each multi-point manifold into its first constraint,
// chains the first constraints' impulses per body (islands with nothing left to solve are
// dropped from later iterations) and applies each body's chain at once.

namespace {
// 0x7d67784 - perceived mass of a body for an offset r along a direction d through the full
// world inverse inertia tensor: d . ((I^-1 (r x d)) x r) (the tensor used transposed, as the
// original multiplies it).
FP::Raw ComputePerceivedMassRaw(const Entry3D* entry, const FPVector3& r, const FPVector3& d) {
    const FP::Raw rx = Raw(r.X), ry = Raw(r.Y), rz = Raw(r.Z);
    const FP::Raw dx = Raw(d.X), dy = Raw(d.Y), dz = Raw(d.Z);
    const FP::Raw cx = Mul(dz, ry) - Mul(rz, dy);
    const FP::Raw cy = Mul(rz, dx) - Mul(dz, rx);
    const FP::Raw cz = Mul(rx, dy) - Mul(ry, dx);
    const Photon::Deterministic::FPMatrix3x3& m = entry->WorldInertiaTensorInverse;
    const FP::Raw wx = Mul(Raw(m.M10), cy) + Mul(cx, Raw(m.M00)) + Mul(Raw(m.M20), cz);
    const FP::Raw wy = Mul(Raw(m.M11), cy) + Mul(cx, Raw(m.M01)) + Mul(Raw(m.M21), cz);
    const FP::Raw wz = Mul(Raw(m.M12), cy) + Mul(cx, Raw(m.M02)) + Mul(Raw(m.M22), cz);
    const FP::Raw tx = Mul(wy, rz) - Mul(wz, ry);
    const FP::Raw ty = Mul(wz, rx) - Mul(wx, rz);
    const FP::Raw tz = Mul(wx, ry) - Mul(wy, rx);
    return Mul(tx, dx) + Mul(tz, dz) + Mul(ty, dy);
}

// 0x7d67888 - a linear impulse (and optionally an angular one) on the entry's body; wakes it.
void ApplyImpulse(Entry3D* entry, const FPVector3& linear, const FPVector3* angular) {
    PhysicsBody3D* body = entry->Body3D;
    if (body->Status == PhysicsBodyStatus::Asleep) {
        body->Status = PhysicsBodyStatus::Active;
        body->_sleepTimeRawInt = 0;
    }
    entry->ShouldIntegrateVelocity = true;
    const FP::Raw massInverse = body->_massInverseRawInt;
    body->Velocity.X = FP::FromRaw(Raw(body->Velocity.X) + Mul(Raw(linear.X), massInverse));
    body->Velocity.Y = FP::FromRaw(Raw(body->Velocity.Y) + Mul(Raw(linear.Y), massInverse));
    body->Velocity.Z = FP::FromRaw(Raw(body->Velocity.Z) + Mul(Raw(linear.Z), massInverse));
    if (angular == nullptr) {
        return;
    }
    const Photon::Deterministic::FPMatrix3x3& m = entry->WorldInertiaTensorInverse;
    const FP::Raw ax = Raw(angular->X), ay = Raw(angular->Y), az = Raw(angular->Z);
    const FP::Raw wx = Mul(Raw(m.M10), ay) + Mul(Raw(m.M00), ax) + Mul(Raw(m.M20), az);
    const FP::Raw wy = Mul(Raw(m.M11), ay) + Mul(Raw(m.M01), ax) + Mul(Raw(m.M21), az);
    const FP::Raw wz = Mul(Raw(m.M12), ay) + Mul(Raw(m.M02), ax) + Mul(Raw(m.M22), az);
    const auto freeze = static_cast<std::uint8_t>(body->RotationFreeze);
    if ((freeze & 1) == 0) body->AngularVelocity.X = FP::FromRaw(Raw(body->AngularVelocity.X) + wx);
    if ((freeze & 2) == 0) body->AngularVelocity.Y = FP::FromRaw(Raw(body->AngularVelocity.Y) + wy);
    if ((freeze & 4) == 0) body->AngularVelocity.Z = FP::FromRaw(Raw(body->AngularVelocity.Z) + wz);
}
} // namespace

// 0x7d65098 - materials as the legacy solver; per contact point the offsets from the roots'
// positions, moved to the centres of mass (at the time of impact) for dynamic roots, and
// the perceived mass along the normal through the full inertia tensor.
void Scene3D::TaskCollisionSolverInit(FrameBase* frame) {
    for (CollisionManifold3D* m : _collisionSolverManifolds) {
        Entry3D* rootA = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxA)]);
        Entry3D* rootB = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxB)]);
        CombineMaterials(frame, m, rootA, rootB);
        const auto offsetFromCentre = [m](const Entry3D* root, FPVector3 r) {
            FP::Raw cx = Raw(root->RotatedCenterOfMass.X);
            FP::Raw cy = Raw(root->RotatedCenterOfMass.Y);
            FP::Raw cz = Raw(root->RotatedCenterOfMass.Z);
            const FP::Raw toi = Raw(m->_m.TimeOfImpact) - Raw(root->_firstTimeOfImpact);
            if (toi != 0) {
                const FPVector3& v = root->Body3D->Velocity;
                cx += Mul(Raw(v.X), toi);
                cy += Mul(Raw(v.Y), toi);
                cz += Mul(Raw(v.Z), toi);
            }
            return FPVector3{FP::FromRaw(Raw(r.X) - cx), FP::FromRaw(Raw(r.Y) - cy), FP::FromRaw(Raw(r.Z) - cz)};
        };
        for (std::int32_t p = 0; p < m->_contactPointsCount; ++p) {
            ContactConstraint& c = _collisionSolverData[static_cast<std::size_t>(m->_constraintDataIdx + p)];
            c.Manifold = m;
            const FPVector3& contact = m->_contactPoints[p];
            FPVector3 ra = contact - rootA->Transform3D->Position;
            FPVector3 rb = contact - rootB->Transform3D->Position;
            FP::Raw k = 0;
            if (rootA->HasDynamicBody) {
                c.Flags |= HasDynamicBodyA;
                ra = offsetFromCentre(rootA, ra);
                k = rootA->Body3D->_massInverseRawInt + ComputePerceivedMassRaw(rootA, ra, m->Normal);
            }
            if (rootB->HasDynamicBody) {
                c.Flags |= HasDynamicBodyB;
                rb = offsetFromCentre(rootB, rb);
                k = k + rootB->Body3D->_massInverseRawInt + ComputePerceivedMassRaw(rootB, rb, m->Normal);
            }
            c.NormalMass = FP::FromRaw(k);
            c.Ra = ra;
            c.Rb = rb;
        }
    }
}

// 0x7d65a64 - per contact constraint still in play: relative velocity at the contact (every
// body counts); a separating or resting contact (vn >= 0) is solved. Otherwise the normal
// impulse (restitution dropped below the resting speed, which here only counts gravity's X
// and Y) plus friction along the tangent (the whole tangential speed within the static
// coefficient, else dynamic friction * the normal impulse), stored as impulses on both
// bodies (linear, and with rotation r x impulse for the dynamic sides).
void Scene3D::TaskCollisionSolverCompute() {
    const bool useRotation = CachedSettings.UseAngularVelocityEnabled();
    const FPVector3& g = CachedSettings.Gravity;
    const FP::Raw restingSpeedSq = Mul(Mul(Raw(g.X), Raw(g.X)) + Mul(Raw(g.Y), Raw(g.Y)), Raw(_deltaTime)) + 0x41;
    for (ContactConstraint& c : _collisionSolverData) {
        if ((c.Flags & IgnoreVelocitySolver) != 0) {
            c.Flags |= WasSolved;
            continue;
        }
        c.ImpulsesA = EntrySolverImpulses{};
        c.ImpulsesB = EntrySolverImpulses{};
        const CollisionManifold3D* m = c.Manifold;
        const Entry3D* entryA = &_entries[static_cast<std::size_t>(m->_m._entryIdxA)];
        const Entry3D* entryB = &_entries[static_cast<std::size_t>(m->_m._entryIdxB)];
        const FPVector3& ra = c.Ra;
        const FPVector3& rb = c.Rb;
        const FP::Raw nx = Raw(m->Normal.X), ny = Raw(m->Normal.Y), nz = Raw(m->Normal.Z);
        const PhysicsBody3D* bodyA = entryA->Body3D;
        const PhysicsBody3D* bodyB = entryB->Body3D;
        FP::Raw vx = 0, vy = 0, vz = 0;
        if (bodyA != nullptr) {
            const FPVector3& v = bodyA->Velocity;
            if (useRotation) {
                const FPVector3& w = bodyA->AngularVelocity;
                vx = Mul(Raw(ra.Y), Raw(w.Z)) - (Raw(v.X) + Mul(Raw(ra.Z), Raw(w.Y)));
                vy = Mul(Raw(ra.Z), Raw(w.X)) - (Raw(v.Y) + Mul(Raw(ra.X), Raw(w.Z)));
                vz = Mul(Raw(ra.X), Raw(w.Y)) - (Raw(v.Z) + Mul(Raw(ra.Y), Raw(w.X)));
            } else {
                vx = -Raw(v.X);
                vy = -Raw(v.Y);
                vz = -Raw(v.Z);
            }
        }
        if (bodyB != nullptr) {
            const FPVector3& v = bodyB->Velocity;
            vx += Raw(v.X);
            vy += Raw(v.Y);
            vz += Raw(v.Z);
            if (useRotation) {
                const FPVector3& w = bodyB->AngularVelocity;
                vx = vx + Mul(Raw(rb.Z), Raw(w.Y)) - Mul(Raw(rb.Y), Raw(w.Z));
                vy = vy - Mul(Raw(rb.Z), Raw(w.X)) + Mul(Raw(rb.X), Raw(w.Z));
                vz = vz + Mul(Raw(rb.Y), Raw(w.X)) - Mul(Raw(rb.X), Raw(w.Y));
            }
        }
        const FP::Raw vn = Mul(vy, ny) + Mul(vx, nx) + Mul(vz, nz);
        if (vn >= 0) {
            c.Flags |= WasSolved;
            continue;
        }
        const FP::Raw speedSq = Mul(vy, vy) + Mul(vx, vx) + Mul(vz, vz);
        const FP::Raw factor = speedSq < restingSpeedSq ? -(FP::Raw{1} << 16)
                                                        : -(FP::Raw{1} << 16) - Raw(m->CombinedRestitution);
        const FP::Raw jn = Divide(factor * vn, Raw(c.NormalMass));
        FP::Raw tx = vx - Mul(vn, nx);
        FP::Raw ty = vy - Mul(vn, ny);
        FP::Raw tz = vz - Mul(vn, nz);
        const FP::Raw tangentSq = Mul(ty, ty) + Mul(tx, tx) + Mul(tz, tz);
        c.Flags &= ~WasSolved;
        FP::Raw jx = Mul(jn, nx), jy = Mul(jn, ny), jz = Mul(jn, nz);
        if (tangentSq >= 7) {
            const FP::Raw tangentSpeed = Photon::Deterministic::FPMath::SqrtRaw(tangentSq);
            const FP::Raw inverse = (FP::Raw{1} << 32) / tangentSpeed;
            tx = Mul(tx, inverse);
            ty = Mul(ty, inverse);
            tz = Mul(tz, inverse);
            const FPVector3 tangent{FP::FromRaw(tx), FP::FromRaw(ty), FP::FromRaw(tz)};
            FP::Raw tangentMass = 0;
            if ((c.Flags & HasDynamicBodyA) != 0) {
                tangentMass = ComputePerceivedMassRaw(entryA, ra, tangent) + bodyA->_massInverseRawInt;
            }
            if ((c.Flags & HasDynamicBodyB) != 0) {
                tangentMass = tangentMass + bodyB->_massInverseRawInt + ComputePerceivedMassRaw(entryB, rb, tangent);
            }
            const FP::Raw jt = Divide(-(tangentSpeed << 16), tangentMass);
            FP::Raw scale = jt;
            if ((jt < 0 ? -jt : jt) >= Mul(Raw(m->CombinedStaticFriction), jn)) {
                scale = (-(jn * Raw(m->CombinedDynamicFriction))) >> 16;
            }
            jx += Mul(tx, scale);
            jy += Mul(ty, scale);
            jz += Mul(tz, scale);
        }
        c.ImpulsesB.Linear = FPVector3{FP::FromRaw(jx), FP::FromRaw(jy), FP::FromRaw(jz)};
        c.ImpulsesA.Linear = FPVector3{FP::FromRaw(-jx), FP::FromRaw(-jy), FP::FromRaw(-jz)};
        if (!useRotation) {
            // TODO(0x7d65a64, Quantum.Physics3D.Scene3D.Task_CollisionSolverCompute): without
            // angular velocity the original leaves the task here (after the first constraint
            // of its slice); every constraint is computed here.
            continue;
        }
        if ((c.Flags & HasDynamicBodyB) != 0) {
            c.ImpulsesB.Angular = FPVector3{FP::FromRaw(Mul(Raw(rb.Y), jz) - Mul(Raw(rb.Z), jy)),
                                            FP::FromRaw(Mul(Raw(rb.Z), jx) - Mul(Raw(rb.X), jz)),
                                            FP::FromRaw(Mul(Raw(rb.X), jy) - Mul(Raw(rb.Y), jx))};
        }
        if ((c.Flags & HasDynamicBodyA) != 0) {
            c.ImpulsesA.Angular = FPVector3{FP::FromRaw(Mul(Raw(ra.Y), -jz) - Mul(Raw(ra.Z), -jy)),
                                            FP::FromRaw(Mul(Raw(ra.Z), -jx) - Mul(Raw(ra.X), -jz)),
                                            FP::FromRaw(Mul(Raw(ra.X), -jy) - Mul(Raw(ra.Y), -jx))};
        }
    }
}

// 0x7d66298 - a manifold with several contact points acts through its first constraint: the
// impulses of its unsolved points summed and divided by the point count.
void Scene3D::TaskCollisionSolverAggregateContacts() {
    const bool useRotation = CachedSettings.UseAngularVelocityEnabled();
    for (CollisionManifold3D* m : _collisionSolverManifolds) {
        const std::int32_t points = m->_contactPointsCount;
        if (m->GetIgnore() || points == 1) {
            continue;
        }
        ContactConstraint* first = &_collisionSolverData[static_cast<std::size_t>(m->_constraintDataIdx)];
        const bool angularA = useRotation && (first->Flags & HasDynamicBodyA) != 0;
        const bool angularB = useRotation && (first->Flags & HasDynamicBodyB) != 0;
        FP::Raw ax = 0, ay = 0, az = 0, lx = 0, ly = 0, lz = 0, bx = 0, by = 0, bz = 0;
        bool allSolved = true;
        for (std::int32_t p = 0; p < points; ++p) {
            const ContactConstraint& c = first[p];
            if ((c.Flags & WasSolved) != 0) {
                continue;
            }
            if (angularA) {
                ax += Raw(c.ImpulsesA.Angular.X);
                ay += Raw(c.ImpulsesA.Angular.Y);
                az += Raw(c.ImpulsesA.Angular.Z);
            }
            lx += Raw(c.ImpulsesB.Linear.X);
            ly += Raw(c.ImpulsesB.Linear.Y);
            lz += Raw(c.ImpulsesB.Linear.Z);
            allSolved = false;
            if (angularB) {
                bx += Raw(c.ImpulsesB.Angular.X);
                by += Raw(c.ImpulsesB.Angular.Y);
                bz += Raw(c.ImpulsesB.Angular.Z);
            }
        }
        if (allSolved) {
            continue;
        }
        first->Flags &= ~WasSolved;
        const FP::Raw share = points == 0 ? 0 : (std::int32_t{0x10000} / points); // 1 / points
        first->ImpulsesB.Linear = FPVector3{FP::FromRaw(Mul(lx, share)), FP::FromRaw(Mul(ly, share)),
                                            FP::FromRaw(Mul(lz, share))};
        first->ImpulsesA.Linear = FPVector3{FP::FromRaw(-Mul(lx, share)), FP::FromRaw(-Mul(ly, share)),
                                            FP::FromRaw(-Mul(lz, share))};
        if (angularA) {
            first->ImpulsesA.Angular = FPVector3{FP::FromRaw(Mul(ax, share)), FP::FromRaw(Mul(ay, share)),
                                                 FP::FromRaw(Mul(az, share))};
        }
        if (angularB) {
            first->ImpulsesB.Angular = FPVector3{FP::FromRaw(Mul(bx, share)), FP::FromRaw(Mul(by, share)),
                                                 FP::FromRaw(Mul(bz, share))};
        }
    }
}

// 0x7d66e14 - per island still unsolved: the first constraint of every manifold with an
// impulse is pushed on the front of both roots' chains. An island left with nothing to solve
// is marked solved and its constraints are ignored by later iterations.
void Scene3D::TaskVelocitySolverAggregate() {
    for (Island3D& island : _islands) {
        if (island.Ignore || island.VelocityConstraintsSolved) {
            continue;
        }
        island.VelocityConstraintsSolved = true;
        for (CollisionManifold3D* m : island.CollisionManifolds) {
            ContactConstraint& c = _collisionSolverData[static_cast<std::size_t>(m->_constraintDataIdx)];
            if ((c.Flags & WasSolved) != 0) {
                continue;
            }
            island.VelocityConstraintsSolved = false;
            Entry3D* rootA = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxA)]);
            Entry3D* rootB = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxB)]);
            c.ImpulsesA.NextAdr = rootA->SolverImpulsesAdr;
            rootA->SolverImpulsesAdr = &c.ImpulsesA;
            c.ImpulsesB.NextAdr = rootB->SolverImpulsesAdr;
            rootB->SolverImpulsesAdr = &c.ImpulsesB;
        }
        // TODO(0x7d66e14, Quantum.Physics3D.Scene3D.Task_VelocitySolverAggregate): the island's
        // joint constraints join the chains too; joints are not reimplemented yet.
        if (island.VelocityConstraintsSolved) {
            for (CollisionManifold3D* m : island.CollisionManifolds) {
                for (std::int32_t p = 0; p < m->_contactPointsCount; ++p) {
                    _collisionSolverData[static_cast<std::size_t>(m->_constraintDataIdx + p)].Flags |=
                        IgnoreVelocitySolver;
                }
            }
        }
    }
}

// 0x7d67270 - per solver entry: the chained impulses combined (an impulse along the sum
// only adds the share beyond it) and applied at once; angular only with rotation and a
// body not frozen on every axis.
void Scene3D::TaskVelocitySolverApply() {
    const bool useRotation = CachedSettings.UseAngularVelocityEnabled();
    for (Entry3D* entry : _solverEntries) {
        if (entry->SolverImpulsesAdr == nullptr) {
            continue;
        }
        const bool angular = useRotation && (static_cast<std::uint8_t>(entry->Body3D->RotationFreeze) & 7) != 7;
        AccumulatedImpulse sum{};
        for (const EntrySolverImpulses* impulse = entry->SolverImpulsesAdr; impulse != nullptr;
             impulse = impulse->NextAdr) {
            AccumulatedImpulse next{};
            next.Velocity = impulse->Linear;
            next.AngularVelocity = impulse->Angular;
            sum.Accumulate(next, angular ? (ImpulseLinear | ImpulseAngular | ImpulseAccumulate)
                                         : (ImpulseLinear | ImpulseAccumulate));
        }
        ApplyImpulse(entry, sum.Velocity, angular ? &sum.AngularVelocity : nullptr);
        entry->SolverImpulsesAdr = nullptr;
    }
}

// 0x7d67a6c - per island: every contact deeper than the allowance pushes its dynamic bodies
// apart along the normal (correction split by inverse mass, minus what earlier contacts
// already pushed in that direction); then positions move by correction * massInverse.
void Scene3D::TaskPenetrationCorrection() {
    const FP::Raw allowance = Raw(CachedSettings.PenetrationAllowance);
    const FP::Raw correctionFactor = Raw(CachedSettings.PenetrationCorrection);
    for (Island3D& island : _islands) {
        if (island.Ignore) continue;
        for (CollisionManifold3D* m : island.CollisionManifolds) {
            // TODO: mesh manifolds correct per triangle manifold (NextInMesh chain).
            const FP::Raw penetration = Raw(m->Penetration) - allowance;
            if (penetration <= 0) continue;
            Entry3D* a = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxA)]);
            Entry3D* b = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxB)]);
            const FP::Raw invA = a->HasDynamicBody ? a->Body3D->_massInverseRawInt : 0;
            const FP::Raw invB = b->HasDynamicBody ? b->Body3D->_massInverseRawInt : 0;
            if (invA + invB == 0) continue;
            const FP::Raw correction = Mul((penetration << 16) / (invA + invB), correctionFactor);
            const FP::Raw nx = Raw(m->Normal.X), ny = Raw(m->Normal.Y), nz = Raw(m->Normal.Z);
            if (a->HasDynamicBody) {
                FPVector3& acc = a->AccumulatedCorrection;
                // Already pushed along -n: products negated before the shift, as the original.
                const FP::Raw done = ((-(ny * Raw(acc.Y))) >> 16) + ((-(nx * Raw(acc.X))) >> 16) +
                                     ((-(nz * Raw(acc.Z))) >> 16);
                const FP::Raw amount = done > 0 ? correction - done : correction;
                if (done <= 0 || amount > 0) {
                    acc.X = FromRaw(Raw(acc.X) + Mul(amount, -nx));
                    acc.Y = FromRaw(Raw(acc.Y) + Mul(amount, -ny));
                    acc.Z = FromRaw(Raw(acc.Z) + Mul(amount, -nz));
                }
            }
            if (b->HasDynamicBody) {
                FPVector3& acc = b->AccumulatedCorrection;
                const FP::Raw done = Mul(Raw(acc.Y), ny) + Mul(Raw(acc.X), nx) + Mul(Raw(acc.Z), nz);
                const FP::Raw amount = done > 0 ? correction - done : correction;
                if (done <= 0 || amount > 0) {
                    acc.X = FromRaw(Raw(acc.X) + Mul(amount, nx));
                    acc.Y = FromRaw(Raw(acc.Y) + Mul(amount, ny));
                    acc.Z = FromRaw(Raw(acc.Z) + Mul(amount, nz));
                }
            }
        }
        for (Entry3D* entry : island.Entries) {
            if (entry->Body3D == nullptr || entry->Transform3D == nullptr) continue;
            const FP::Raw massInv = entry->Body3D->_massInverseRawInt;
            FPVector3& pos = entry->Transform3D->Position;
            pos.X = FromRaw(Raw(pos.X) + Mul(Raw(entry->AccumulatedCorrection.X), massInv));
            pos.Y = FromRaw(Raw(pos.Y) + Mul(Raw(entry->AccumulatedCorrection.Y), massInv));
            pos.Z = FromRaw(Raw(pos.Z) + Mul(Raw(entry->AccumulatedCorrection.Z), massInv));
        }
    }
}

namespace {
// 0x7d68704 - clamps each coordinate into the 32-bit raw range; true if any was clamped.
bool ClampPositionUsableSpace(FPVector3& pos) {
    bool clamped = false;
    for (FP* c : {&pos.X, &pos.Y, &pos.Z}) {
        const FP::Raw v = c->RawValue();
        if (v < -(FP::Raw{1} << 31)) {
            *c = FP::FromRaw(-(FP::Raw{1} << 31));
            clamped = true;
        } else if (v >= (FP::Raw{1} << 31)) {
            *c = FP::FromRaw(0x7fffffff);
            clamped = true;
        }
    }
    return clamped;
}
} // namespace

// 0x7d680ec - positions/rotations over the rest of the step, the second half of the force
// integration, then forces cleared; positions kept inside the usable space.
void Scene3D::TaskIntegrateVelocity() {
    const PhysicsSceneSettings& s = CachedSettings;
    const bool applyMin = Raw(s.MinLinearIntegration) > 0 || Raw(s.MinAngularIntegration) > 0;
    const bool integrateAngular = s.UseAngularVelocityEnabled();
    const FP::Raw halfDt = Raw(_deltaTime) >> 1;
    for (std::size_t i = static_cast<std::size_t>(_staticComponentsCount + _queriesCount - 1);
         i < _entries.size(); ++i) {
        Entry3D* entry = &_entries[i];
        PhysicsBody3D* body = entry->Body3D;
        if (!entry->ShouldIntegrateVelocity) {
            if (body != nullptr && entry->_compoundRootIndexPlusOne < 1) {
                body->_angularImpulse = {};
                body->_force = FPVector3::Zero();
                body->_torque = FPVector3::Zero();
            }
            continue;
        }
        const FP rest = FP::FromRaw(Raw(_deltaTime) - Raw(entry->_firstTimeOfImpact));
        if (Raw(rest) > 0) {
            Entry3D::IntegrateVelocity(entry, rest, s, applyMin, integrateAngular);
        }
        const FP::Raw massInv = body->_massInverseRawInt;
        const FP::Raw gravityScale = body->_gravityScaleRawInt;
        const FP::Raw ax = Mul(Raw(s.Gravity.X), gravityScale) + Mul(Raw(body->_force.X), massInv);
        const FP::Raw ay = Mul(Raw(s.Gravity.Y), gravityScale) + Mul(Raw(body->_force.Y), massInv);
        const FP::Raw az = Mul(Raw(s.Gravity.Z), gravityScale) + Mul(Raw(body->_force.Z), massInv);
        body->Velocity.X = FromRaw(Raw(body->Velocity.X) + Mul(ax, halfDt));
        body->Velocity.Y = FromRaw(Raw(body->Velocity.Y) + Mul(ay, halfDt));
        body->Velocity.Z = FromRaw(Raw(body->Velocity.Z) + Mul(az, halfDt));
        if (integrateAngular && body->_torque != FPVector3::Zero()) {
            const FP::Raw tx = Mul(Raw(body->_torque.X), halfDt);
            const FP::Raw ty = Mul(Raw(body->_torque.Y), halfDt);
            const FP::Raw tz = Mul(Raw(body->_torque.Z), halfDt);
            const Photon::Deterministic::FPMatrix3x3& m = entry->WorldInertiaTensorInverse;
            const FP::Raw wx = Mul(Raw(m.M01), ty) + Mul(Raw(m.M00), tx) + Mul(Raw(m.M02), tz);
            const FP::Raw wy = Mul(Raw(m.M11), ty) + Mul(Raw(m.M10), tx) + Mul(Raw(m.M12), tz);
            const FP::Raw wz = Mul(Raw(m.M21), ty) + Mul(Raw(m.M20), tx) + Mul(Raw(m.M22), tz);
            const auto freeze = static_cast<std::uint8_t>(body->RotationFreeze);
            if (!(freeze & 1)) body->AngularVelocity.X = FromRaw(Raw(body->AngularVelocity.X) + wx);
            if (!(freeze & 2)) body->AngularVelocity.Y = FromRaw(Raw(body->AngularVelocity.Y) + wy);
            if (!(freeze & 4)) body->AngularVelocity.Z = FromRaw(Raw(body->AngularVelocity.Z) + wz);
        }
        body->_angularImpulse = {};
        body->_force = FPVector3::Zero();
        body->_torque = FPVector3::Zero();
        if (ClampPositionUsableSpace(entry->Transform3D->Position)) {
            body->Velocity = FPVector3::Zero();
        }
    }
}

// 0x7d68798 - per awake dynamic entry allowed to sleep: while it stays within the linear
// tolerance of its last recorded position and the angular tolerance of its last recorded
// rotation it accumulates sleep time (an entry without an island falls asleep on its own
// once past SleepTimeSec); otherwise the timer restarts and whichever reference was left
// behind is recorded anew.
void Scene3D::TaskSleepDetectionEntries() {
    const PhysicsSceneSettings& s = CachedSettings;
    const FP::Raw linearToleranceSq = Mul(Raw(s.LinearSleepTolerance), Raw(s.LinearSleepTolerance));
    const FP::Raw angularTolerance = Raw(s.AngularSleepToleranceInRad);
    const auto deltaTime = static_cast<std::int32_t>(Raw(_deltaTime));
    for (std::size_t i = static_cast<std::size_t>(_staticComponentsCount + _queriesCount - 1);
         i < _entries.size(); ++i) {
        Entry3D& entry = _entries[i];
        if (!entry.ShouldIntegrateVelocity) {
            continue;
        }
        PhysicsBody3D* body = entry.Body3D;
        if (!body->AllowSleeping()) {
            body->_sleepTimeRawInt = 0;
            continue;
        }
        const FPVector3& position = entry.Transform3D->Position;
        const FPQuaternion& rotation = entry.Transform3D->Rotation;
        // Angle of the rotation since the recorded one: 2 acos(w) of inverse(last) * now.
        const FPQuaternion last = static_cast<FPQuaternion>(body->_lastRotation);
        const FP::Raw lengthSq = Mul(Raw(last.Y), Raw(last.Y)) + Mul(Raw(last.X), Raw(last.X)) +
                                 Mul(Raw(last.Z), Raw(last.Z)) + Mul(Raw(last.W), Raw(last.W));
        const FP::Raw inverseLengthSq = Divide(FP::Raw{1} << 32, lengthSq);
        const FP::Raw ix = (-(Raw(last.X) * inverseLengthSq)) >> 16;
        const FP::Raw iy = (-(Raw(last.Y) * inverseLengthSq)) >> 16;
        const FP::Raw iz = (-(Raw(last.Z) * inverseLengthSq)) >> 16;
        const FP::Raw iw = (inverseLengthSq * Raw(last.W)) >> 16;
        const FP::Raw w = Mul(iw, Raw(rotation.W)) -
                          (Mul(iy, Raw(rotation.Y)) + Mul(ix, Raw(rotation.X)) + Mul(iz, Raw(rotation.Z)));
        const FP::Raw angle = Raw(Photon::Deterministic::FPMath::Acos(FP::FromRaw(w))) << 1;
        const FPVector3 lastPosition = static_cast<FPVector3>(body->_lastPosition);
        const FP::Raw dx = Raw(lastPosition.X) - Raw(position.X);
        const FP::Raw dy = Raw(lastPosition.Y) - Raw(position.Y);
        const FP::Raw dz = Raw(lastPosition.Z) - Raw(position.Z);
        const FP::Raw distanceSq = Mul(dy, dy) + Mul(dx, dx) + Mul(dz, dz);
        if (distanceSq < linearToleranceSq && angle < angularTolerance) {
            body->_sleepTimeRawInt = static_cast<std::int32_t>(static_cast<std::uint32_t>(body->_sleepTimeRawInt) +
                                                               static_cast<std::uint32_t>(deltaTime));
            if (entry.Island == nullptr && Raw(s.SleepTimeSec) < FP::Raw{body->_sleepTimeRawInt}) {
                body->Status = PhysicsBodyStatus::Asleep;
                body->_sleepTimeRawInt = 0;
            }
            continue;
        }
        body->_sleepTimeRawInt = 0;
        if (distanceSq >= linearToleranceSq) {
            body->_lastPosition = FPVector3RawInt::FromFPVector3(position);
        }
        if (angle >= angularTolerance) {
            body->_lastRotation = FPQuaternionRawInt::FromFPQuaternion(rotation);
        }
    }
}

// 0x7d68cdc - an island falls asleep when its least-rested body reached SleepTimeSec: its
// bodies that may sleep stop (velocities cleared) and their timers restart.
void Scene3D::TaskSleepDetectionIslands() {
    const auto sleepTime = static_cast<std::int32_t>(Raw(CachedSettings.SleepTimeSec));
    for (Island3D& island : _islands) {
        if (island.Ignore) {
            continue;
        }
        std::int32_t leastRested = std::numeric_limits<std::int32_t>::max();
        for (const Entry3D* entry : island.Entries) {
            leastRested = std::min(leastRested, entry->Body3D->_sleepTimeRawInt);
        }
        if (leastRested < sleepTime) {
            continue;
        }
        for (Entry3D* entry : island.Entries) {
            PhysicsBody3D* body = entry->Body3D;
            if (body->AllowSleeping()) {
                body->Status = PhysicsBodyStatus::Asleep;
                body->_sleepTimeRawInt = 0;
                body->Velocity = FPVector3::Zero();
                body->AngularVelocity = FPVector3::Zero();
            }
        }
    }
}

// 0x7d5a3ec - one step. Ordering follows TaskManagementData.ScheduleTasks (0x7d6f764).
void Scene3D::Update(FrameBase* frame, FP deltaTime, ICollisionCallbacks3D* callbacks) {
    _deltaTime = deltaTime;
    ResetScene(frame);
    CreateEntries(frame);
    ResetBucketsAndTls();
    TaskIntegrateForcesFirstHalf();
    TaskBroadPhaseEntryPreStep();
    TaskBroadPhase(frame);
    TaskNarrowPhase();
    TaskCallbacks(frame, callbacks);
    TaskSortCcdManifolds();
    TaskCcdCallbacks(frame, callbacks);
    TaskPostCallbacksUpdateEntries(frame);
    TaskPostCallbacksUpdateManifolds();
    TaskBuildIslands();
    TaskIntegrateFirstTOI();
    if (CachedSettings.SolverIterations != 0) {
        // TaskManagementData.ScheduleSolverTasksLGS (0x7d6ff38) / ScheduleSolverTasksHJGS
        // (0x7d700e8): the manifolds are cached, the constraints initialised, then one pass
        // per iteration.
        TaskCollisionSolverCacheManifolds();
        if (CachedSettings.SolverType == ConstraintsSolverType::LegacyGaussSeidel) {
            TaskCollisionSolverInitLegacyGS(frame);
            for (int iteration = 0; iteration < CachedSettings.SolverIterations; ++iteration) {
                TaskVelocitySolverLegacyGS();
            }
        } else {
            TaskCollisionSolverInit(frame);
            for (int iteration = 0; iteration < CachedSettings.SolverIterations; ++iteration) {
                TaskCollisionSolverCompute();
                TaskCollisionSolverAggregateContacts();
                // TODO(0x7d66708, Quantum.Physics3D.Scene3D.Task_CollisionSolverAggregateMesh):
                // mesh manifolds; static triangle meshes are not reimplemented yet.
                TaskVelocitySolverAggregate();
                TaskVelocitySolverApply();
            }
        }
    }
    TaskPenetrationCorrection();
    TaskIntegrateVelocity();
    // TODO(0x7d6902c, Quantum.Physics3D.Scene3D.Task_PositionSolverCompute), TODO(0x7d69378,
    // Quantum.Physics3D.Scene3D.Task_PositionSolverAggregate), TODO(0x7d69530,
    // Quantum.Physics3D.Scene3D.Task_PositionSolverApply): the position solver runs for
    // joints only (UseJoints with PositionSolverIterations); joints are not reimplemented yet.
    if (CachedSettings.AllowSleepingEnabled()) {
        TaskSleepDetectionEntries();
        TaskSleepDetectionIslands();
    }
}

// ---------------------------------------------------------------------------------------
// Immediate queries

// 0x7d5442c - the broad phase of one query: the buckets its box spans are walked in the
// query's direction; in each, the static and then the dynamic stabbing list is entered at
// the snapshot covering the box's start on the sort axis, and every candidate that passes
// the filters (enabled, layer, kind, first bucket of the pair, the other two axes) goes
// through the narrow phase. A first-hit-only query stops after the first bucket in which a
// hit point lies (all hits found so far are kept).
void Scene3D::ResolveQuery(FrameBase* frame, HitCollection3D* hits, const Query3D& query, Entry3D* qEntry) {
    const IntAABB& box = qEntry->AABB;
    const auto bucketCount = static_cast<std::int64_t>(_buckets.size());
    const auto bucketOf = [&](std::int32_t coordinate) {
        return static_cast<std::int32_t>(std::clamp<std::int64_t>((coordinate + _worldOffsetRaw) / _bucketsSizeRaw, 0,
                                                                  bucketCount - 1));
    };
    const std::int32_t low = bucketOf(_bucketOnX ? box.MinX : box.MinZ);
    const std::int32_t high = bucketOf(_bucketOnX ? box.MaxX : box.MaxZ);
    const std::int32_t increment = qEntry->BucketIncrement;
    const bool forward = increment == 1;
    qEntry->BucketStart = forward ? low : high;
    qEntry->BucketEnd = forward ? high : low;
    if (qEntry->BucketStart != qEntry->BucketEnd && forward == (qEntry->BucketStart > qEntry->BucketEnd)) {
        return;
    }
    QueryBox q;
    q.SortMin = _sortOnX ? box.MinX : box.MinZ;
    q.SortMax = _sortOnX ? box.MaxX : box.MaxZ;
    q.SecondaryMin = _sortOnX ? box.MinZ : box.MinX;
    q.SecondaryMax = _sortOnX ? box.MaxZ : box.MaxX;
    q.StabPoint = static_cast<std::int32_t>(static_cast<std::uint32_t>(q.SortMin) + 0x3fffffffu);
    if (q.StabPoint < 0) {
        return;
    }
    // A shape cast's hits lie up to its own extent further along.
    std::int32_t hitBucketOffset = 0;
    if (query.Type == QueryType::ShapeCast && query.FirstHitOnly) {
        const std::int32_t extent = _bucketOnX ? box.MaxX - box.MinX : box.MaxZ - box.MinZ;
        hitBucketOffset = increment * static_cast<std::int32_t>(extent / _bucketsSizeRaw) + increment;
    }
    const std::int32_t options = query.Options;
    const bool detailed = (options & ComputeDetailedInfo) != 0;
    // TODO(0x7d55a20, Quantum.Physics3D.Scene3D.TraverseSceneMeshLinecast), TODO(0x7d54f48,
    // Quantum.Physics3D.Scene3D.TraverseSceneMeshShapeOverlap), TODO(0x7d55208,
    // Quantum.Physics3D.Scene3D.TraverseSceneMeshShapeCast): with HitStatics the static
    // triangle mesh is tested first; meshes are not reimplemented yet.
    const std::int32_t hitsBefore = hits->Count();
    std::int32_t bucketHitsStart = hitsBefore;
    std::int32_t b = qEntry->BucketStart - increment;
    do {
        b += increment;
        q.Bucket = b;
        q.FirstBucket = b == qEntry->BucketStart;
        Bucket3D& bucket = _buckets[static_cast<std::size_t>(b)];
        if ((options & HitStatics) != 0 && bucket.StaticsStabbingList.Count >= 1) {
            ScanQueryCandidates(frame, hits, query, qEntry, bucket.StaticsStabbingList, q, true, detailed);
        }
        if ((options & (HitDynamics | HitKinematics)) != 0 && bucket.DynamicsStabbingList.Count >= 1) {
            ScanQueryCandidates(frame, hits, query, qEntry, bucket.DynamicsStabbingList, q, false, detailed);
        }
        if (query.FirstHitOnly && hits->Count() > hitsBefore) {
            // The new hits remember their bucket (in the hit's sqrDistance slot, as the
            // original); a hit inside this bucket ends the walk.
            for (std::int32_t i = bucketHitsStart; i < hits->Count(); ++i) {
                Hit3D& hit = hits->HitsBuffer()[i];
                const FP::Raw coordinate = Raw(_bucketOnX ? hit.Point.X : hit.Point.Z);
                hit.sqrDistance = static_cast<std::int32_t>(hitBucketOffset +
                                                            static_cast<std::int32_t>((coordinate + _worldOffsetRaw) /
                                                                                      _bucketsSizeRaw));
            }
            for (std::int32_t i = hitsBefore; i < hits->Count(); ++i) {
                if (hits->HitsBuffer()[i].sqrDistance == b) {
                    return;
                }
            }
            bucketHitsStart = hits->Count();
        }
    } while (b != qEntry->BucketEnd);
}

// Part of ResolveQuery (0x7d5442c): the candidates of one stabbing list of a bucket. Lists
// after the first only add the entries that opened after their snapshot (sorted by start),
// so the scan ends at the first of those starting past the query box.
void Scene3D::ScanQueryCandidates(FrameBase* frame, HitCollection3D* hits, const Query3D& query, Entry3D* qEntry,
                                  const StabbingList& stabbing, const QueryBox& q, bool statics, bool detailed) {
    const std::int32_t options = query.Options;
    const std::int32_t first = stabbing.Stab(q.StabPoint);
    for (std::int32_t j = first; j < stabbing.Count; ++j) {
        const ActiveEntryList& list = stabbing.Lists[static_cast<std::size_t>(j)];
        for (std::int32_t k = j == first ? 0 : list.CountUnsorted; k < list.Count; ++k) {
            const ActiveEntry& item = list.Items[static_cast<std::size_t>(k)];
            Entry3D* entry = &_entries[static_cast<std::size_t>(item.Index)];
            // TODO: statics disabled through StaticCollidersState are skipped here.
            const std::int32_t entryMin = _sortOnX ? entry->AABB.MinX : entry->AABB.MinZ;
            const std::int32_t entryMax = _sortOnX ? entry->AABB.MaxX : entry->AABB.MaxZ;
            if (k < list.CountUnsorted) {
                if (q.SortMin > entryMax) continue;
            } else {
                if (entryMin > q.SortMax) return;
                if (entryMax < q.SortMin) continue;
            }
            if (!statics && (!frame->Exists(entry->Ref) ||
                             !frame->ComponentMask(entry->Ref.Index).IsSupersetOf(_minComponentSet) ||
                             entry->Collider3D->_disabled != 0)) {
                continue;
            }
            if (((query.LayerMask >> entry->Collider3D->Layer()) & 1) == 0) continue;
            const bool trigger = entry->Collider3D->_isTrigger == 1;
            if (statics || !entry->HasDynamicBody) {
                if (!statics && (options & HitKinematics) == 0) continue;
                if (trigger && (options & HitTriggers) == 0) continue;
            } else if ((options & HitDynamics) == 0) {
                continue;
            }
            // A pair spanning several buckets is tested in the first one the walk enters.
            if (!q.FirstBucket &&
                (qEntry->BucketIncrement == 1 ? entry->BucketStart : entry->BucketEnd) != q.Bucket) {
                continue;
            }
            if (item.Max < q.SecondaryMin || item.Min > q.SecondaryMax) continue;
            if (qEntry->AABB.MinY > entry->AABB.MaxY || entry->AABB.MinY > qEntry->AABB.MaxY) continue;
            ResolveQueryNarrow(frame, hits, query, qEntry, entry, detailed);
        }
    }
}

// 0x7d55ef4 - the narrow phase of one query against one entry, adding the hit.
void Scene3D::ResolveQueryNarrow(FrameBase* frame, HitCollection3D* hits, const Query3D& query, Entry3D* qEntry,
                                 Entry3D* entry, bool detailed) {
    switch (query.Type) {
    case QueryType::Linecast: {
        // TODO: mesh triangles use TriangleCCW.RayIntersectsTriangle.
        Hit3D hit{};
        if (NarrowChecks::LinecastBody::Check(_currentMap, entry, query.Transform.Position, query.End, detailed, hit)) {
            hits->Add(hit, frame->get_Context());
        }
        return;
    }
    case QueryType::ShapeOverlap: {
        CollisionManifold3D& m = _tls.Manifold;
        m = CollisionManifold3D{};
        m._m._entryIdxA = qEntry->Index;
        m._m._entryIdxB = entry->Index;
        m._m._triIdx = -1;
        CollisionManifoldInfo3D info{&m, qEntry, entry, nullptr};
        if (!NarrowChecks::Check(_tls.Narrow, CachedSettings.CCDSettingsValue, _deltaTime, &info, detailed)) {
            return;
        }
        Hit3D hit{};
        SetHitCollider(hit, entry);
        if (detailed) {
            hit.Point = CollisionManifold3D::GetAverageContactPoint(&m);
            hit.Normal = (m._flags & CollisionManifold3D::NormalInverted) != 0 ? -m.Normal : m.Normal;
            hit.OverlapPenetration = m.Penetration;
        }
        hits->Add(hit, frame->get_Context());
        return;
    }
    case QueryType::ShapeCast: {
        // TODO: mesh triangles go through Gjk3D.ShapeCastManifold with the triangle.
        Hit3D hit{};
        const FPVector3& translation = query.End;
        if (entry->Shape->Type() == Shape3DType::Sphere && query.Shape.Type() == Shape3DType::Sphere) {
            // Sphere against sphere: a ray against the radii summed.
            const FPVector3 start = qEntry->Narrow.Position;
            const FPVector3 end{FP::FromRaw(Raw(start.X) + Raw(translation.X)),
                                FP::FromRaw(Raw(start.Y) + Raw(translation.Y)),
                                FP::FromRaw(Raw(start.Z) + Raw(translation.Z))};
            const FP queryRadius = query.Shape.Sphere.Radius;
            FPVector3 point;
            if (!NarrowChecks::LinecastBody::RaySphereProjection(
                    entry->Narrow.Position, FP::FromRaw(Raw(entry->Shape->Sphere.Radius) + Raw(queryRadius)), start,
                    end, point, hit.CastDistanceNormalized)) {
                return;
            }
            SetHitCollider(hit, entry);
            if (detailed) {
                hit.Normal = FPVector3::Normalize(point - entry->Narrow.Position);
                hit.Point = FPVector3{FP::FromRaw(Raw(point.X) - Mul(Raw(hit.Normal.X), Raw(queryRadius))),
                                      FP::FromRaw(Raw(point.Y) - Mul(Raw(hit.Normal.Y), Raw(queryRadius))),
                                      FP::FromRaw(Raw(point.Z) - Mul(Raw(hit.Normal.Z), Raw(queryRadius)))};
            }
        } else {
            if (!Gjk3D::ShapeCastManifold(_tls.Narrow, entry, qEntry, nullptr, CachedSettings.GJKSettingsValue,
                                          translation, detailed, hit)) {
                return;
            }
            SetHitCollider(hit, entry);
        }
        hits->Add(hit, frame->get_Context());
        return;
    }
    }
}

// The collider identity part of a query hit (static index or entity, trigger, user tag).
void Scene3D::SetHitCollider(Hit3D& hit, const Entry3D* entry) const {
    hit._staticColliderIndex = entry->StaticColliderIndex;
    if (entry->StaticColliderIndex >= 0) {
        hit._mapGuid = _currentMap;
    } else {
        hit._entity = HitEntityRef{entry->Ref.Index, entry->Ref.Version};
    }
    hit.SetIsTrigger(entry->Collider3D->_isTrigger == 1);
    hit.ShapeUserTag = entry->Shape->UserTag;
}

// 0x7d56978 - the query entry carries the shape at its start; its box grows by the
// translation and the walk follows it.
void Scene3D::ShapeCast(FrameBase* frame, HitCollection3D* hits, const FPVector3& start, const FPQuaternion& rot,
                        const Shape3D& shape, const FPVector3& translation, bool firstHitOnly, std::int32_t layerMask,
                        std::int32_t options) {
    Query3D query;
    query.Type = QueryType::ShapeCast;
    query.FirstHitOnly = firstHitOnly;
    query.Transform.Position = start;
    query.Transform.Rotation = rot;
    query.End = translation;
    query.Shape = shape;
    query.Options = options;
    query.LayerMask = layerMask;
    hits->Reset();
    Entry3D* qEntry = &_tls.Entry;
    qEntry->StaticColliderIndex = -1;
    if (shape.Type() == Shape3DType::Compound) {
        // TODO(0x7d56b04, Quantum.Physics3D.Scene3D.CompoundShapeCast): one cast per child
        // shape; compound shapes are not reimplemented yet.
        return;
    }
    InitShapeCastEntry(query, qEntry);
    ResolveQuery(frame, hits, query, qEntry);
}

// 0x7d56c10
void Scene3D::InitShapeCastEntry(const Query3D& query, Entry3D* qEntry) {
    InitShapeOverlapEntry(query, qEntry);
    const FPVector3& t = query.End;
    const auto grow = [](std::int32_t& bound, FP by) {
        bound = static_cast<std::int32_t>(static_cast<std::uint32_t>(bound) +
                                          static_cast<std::uint32_t>(Raw(by)));
    };
    grow(Raw(t.X) < 1 ? qEntry->AABB.MinX : qEntry->AABB.MaxX, t.X);
    grow(Raw(t.Y) > 0 ? qEntry->AABB.MaxY : qEntry->AABB.MinY, t.Y);
    grow(Raw(t.Z) < 1 ? qEntry->AABB.MinZ : qEntry->AABB.MaxZ, t.Z);
    const bool backwards = _bucketOnX ? Raw(t.X) < 1 : Raw(t.Z) < 1;
    qEntry->BucketIncrement = backwards ? -1 : 1;
}

// 0x7d56464 - the query entry spans the segment; it walks the buckets from the start.
void Scene3D::Linecast(FrameBase* frame, HitCollection3D* hits, const FPVector3& start, const FPVector3& end,
                       bool firstHitOnly, std::int32_t layerMask, std::int32_t options) {
    Query3D query;
    query.Type = QueryType::Linecast;
    query.FirstHitOnly = firstHitOnly;
    query.Transform.Position = start;
    query.End = end;
    query.Options = options;
    query.LayerMask = layerMask;
    hits->Reset();
    Entry3D* qEntry = &_tls.Entry;
    qEntry->StaticColliderIndex = -1;
    InitLinecastEntry(query, qEntry);
    ResolveQuery(frame, hits, query, qEntry);
}

// 0x7d56594 - the segment's box (raw values truncated to 32 bits) and the walk direction on
// the bucket axis.
void Scene3D::InitLinecastEntry(const Query3D& query, Entry3D* qEntry) const {
    const FPVector3& start = query.Transform.Position;
    const FPVector3& end = query.End;
    qEntry->StaticColliderIndex = -1;
    const auto lo = [](FP a, FP b) { return static_cast<std::int32_t>(Raw(a < b ? a : b)); };
    const auto hi = [](FP a, FP b) { return static_cast<std::int32_t>(Raw(a > b ? a : b)); };
    qEntry->AABB.MinX = lo(start.X, end.X);
    qEntry->AABB.MaxX = hi(start.X, end.X);
    qEntry->AABB.MinY = lo(start.Y, end.Y);
    qEntry->AABB.MaxY = hi(start.Y, end.Y);
    qEntry->AABB.MinZ = lo(start.Z, end.Z);
    qEntry->AABB.MaxZ = hi(start.Z, end.Z);
    const bool ascending = _bucketOnX ? start.X < end.X : start.Z < end.Z;
    qEntry->BucketIncrement = ascending ? 1 : -1;
}

// 0x7d56d18 - the query entry carries the shape at the pose (narrow data and box).
void Scene3D::OverlapShape(FrameBase* frame, HitCollection3D* hits, const FPVector3& pos, const FPQuaternion& rot,
                           const Shape3D& shape, std::int32_t layerMask, std::int32_t options) {
    Query3D query;
    query.Type = QueryType::ShapeOverlap;
    query.Transform.Position = pos;
    query.Transform.Rotation = rot;
    query.Shape = shape;
    query.Options = options;
    query.LayerMask = layerMask;
    hits->Reset();
    Entry3D* qEntry = &_tls.Entry;
    qEntry->StaticColliderIndex = -1;
    if (shape.Type() == Shape3DType::Compound) {
        // TODO(0x7d56eb4, Quantum.Physics3D.Scene3D.OverlapCompoundShape): one query per
        // child shape; compound shapes are not reimplemented yet.
        return;
    }
    InitShapeOverlapEntry(query, qEntry);
    ResolveQuery(frame, hits, query, qEntry);
}

// 0x7d57288
void Scene3D::InitShapeOverlapEntry(const Query3D& query, Entry3D* qEntry) {
    qEntry->StaticColliderIndex = -1;
    _tls.Collider3D = PhysicsCollider3D{};
    _tls.Collider3D.Shape = query.Shape;
    _tls.Transform3D = query.Transform;
    qEntry->Collider3D = &_tls.Collider3D;
    qEntry->Shape = &_tls.Collider3D.Shape;
    qEntry->Transform3D = &_tls.Transform3D;
    Entry3D::InitNarrowData(qEntry, &_tls.Transform3D.Position, &_tls.Transform3D.Rotation);
    Entry3D::InitBroadAabb(qEntry, &_tls.Transform3D.Position, &_tls.Transform3D.Rotation, {});
    qEntry->BucketIncrement = 1;
}

} // namespace Quantum::Physics3D
