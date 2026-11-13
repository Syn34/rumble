// scene3d.cpp - Quantum.Physics3D.Scene3D and its helper structures.
//
// Step order (the original's task graph, run sequentially):
//   CreateEntries -> IntegrateForcesFirstHalf -> BroadPhaseEntryPreStep -> BroadPhase ->
//   NarrowPhase -> Callbacks -> BuildIslands -> collision solver (legacy Gauss-Seidel) ->
//   PenetrationCorrection -> IntegrateVelocity -> SleepDetection.
// Stages that need unported subsystems (meshes, queries resolved in the step, joints, CCD
// integration, the hybrid solver) are marked with TODOs where they would run.
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

// 0x7d7b294
void Island3D::AddEntry(Entry3D* entry) {
    Entries.push_back(entry);
    entry->Island = this;
}

// ---------------------------------------------------------------------------------------
// Scene3D: setup

// 0x7d591a4 (Allocate)
Scene3D::Scene3D() {
    _buckets.resize(1);
    _bucketsSizeRaw = FP::Raw{2} << 16;
    _buckets[0].Index = 0;
}

// 0x7d5b2f8
void Scene3D::InitScene(FrameBase* frame, AssetGuid mapGuid) {
    if (!_mapInitialized) {
        // No map yet: a single bucket covering everything.
        _buckets.assign(1, Bucket3D{});
        _bucketsSizeRaw = FP::Raw{2} << 16;
        _worldSizeRaw = 0;
        _worldOffsetRaw = 0;
    }
    ResetScene(frame);
    UpdateMap(frame, mapGuid);
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

// TODO(0x7d5a490, Quantum.Physics3D.Scene3D.UpdateMap): partly reimplemented from the
// field usage seen elsewhere (bucket sizes, static entries); the original also builds the
// static stabbing lists, the triangle mesh and tracks static collider enable state.
void Scene3D::UpdateMap(FrameBase* frame, AssetGuid mapGuid) {
    if (_mapInitialized && mapGuid == _currentMap) {
        return;
    }
    _currentMap = mapGuid;
    _mapInitialized = true;
    Map* map = mapGuid.IsValid() ? frame->FindAsset<Map>(mapGuid) : frame->get_Map();
    std::int32_t bucketsCount = 1;
    if (map != nullptr) {
        bucketsCount = std::max(1, map->BucketsCount);
        _worldSizeRaw = FP::Raw{map->WorldSize} << 16;
        _worldOffsetRaw = _worldSizeRaw / 2;
        _bucketsSizeRaw = std::max<FP::Raw>(1, _worldSizeRaw / bucketsCount);
        _bucketsSubdivisions = std::max(1, map->BucketsSubdivisions);
        _bucketOnX = map->BucketingAxis == Map::BucketAxis::X;
        _sortOnX = map->SortingAxis == Map::SortAxis::X;
    }
    _buckets.assign(static_cast<std::size_t>(bucketsCount), Bucket3D{});
    for (std::int32_t i = 0; i < bucketsCount; ++i) {
        _buckets[static_cast<std::size_t>(i)].Index = i;
    }
    // Static colliders become the first entries.
    _entries.clear();
    _staticComponents.clear();
    if (map != nullptr) {
        for (const MapStaticCollider3D& source : map->StaticColliders3D) {
            if (source.ShapeType != Shape3DType::Sphere && source.ShapeType != Shape3DType::Box) {
                continue; // TODO: meshes need TriangleMesh
            }
            StaticComponents3D& components = _staticComponents.emplace_back();
            components.Transform3D.Position = source.Position;
            components.Transform3D.Rotation = source.Rotation;
            components.Collider3D.Shape = source.ShapeType == Shape3DType::Sphere
                                              ? Shape3D::CreateSphere(source.SphereRadius)
                                              : Shape3D::CreateBox(source.BoxExtents);
            components.Collider3D.SetLayer(source.StaticData.Layer);
            components.Collider3D.SetIsTrigger(source.StaticData.IsTrigger);
            components.Collider3D._material.Id = source.PhysicsMaterial.Id;
        }
    }
    _staticComponentsCount = static_cast<std::int32_t>(_staticComponents.size());
    std::int32_t staticIndex = 0;
    for (StaticComponents3D& components : _staticComponents) {
        std::int32_t index = 0;
        Entry3D* entry = GetNextEntry(index);
        entry->Collider3D = &components.Collider3D;
        entry->Shape = &components.Collider3D.Shape;
        entry->Transform3D = &components.Transform3D;
        entry->StaticColliderIndex = staticIndex;
        entry->UseSmoothSphereMeshCollision =
            map->StaticColliders3D[static_cast<std::size_t>(staticIndex)].SmoothSphereMeshCollisions;
        Entry3D::InitNarrowData(entry, &components.Transform3D.Position,
                                &components.Transform3D.Rotation);
        Entry3D::InitBroadAabb(entry, &components.Transform3D.Position,
                               &components.Transform3D.Rotation, {});
        ++staticIndex;
    }
    // Static endpoints, sorted once.
    for (Entry3D& entry : _entries) {
        std::int32_t flags = entry.Index | EndPoint::STATIC_FLAG;
        if (entry.IsTriggerCollider()) {
            flags |= EndPoint::TRIGGER_FLAG;
        }
        std::int32_t minBucket = 0, maxBucket = 0;
        InsertEndPoints(entry.AABB, flags, minBucket, maxBucket);
        entry.BucketStart = minBucket;
        entry.BucketEnd = maxBucket;
        entry.BucketIncrement = 1;
    }
    for (Bucket3D& bucket : _buckets) {
        std::stable_sort(bucket.EndPointsStatic.begin(),
                         bucket.EndPointsStatic.begin() + bucket.EndPointsStaticCount * 2,
                         [](const EndPoint& a, const EndPoint& b) {
                             return static_cast<std::uint32_t>(a.Value) < static_cast<std::uint32_t>(b.Value);
                         });
    }
    // Static entry pointers into _staticComponents are stable from here on.
    (void)frame;
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

// 0x7d6c3c8 (ManifoldData3D.UpdateCachedState)
void Scene3D::UpdateManifoldCachedState(ManifoldData3D& m) const {
    const Entry3D& a = _entries[static_cast<std::size_t>(m._entryIdxA)];
    const Entry3D& b = _entries[static_cast<std::size_t>(m._entryIdxB)];
    m.SetIsDynamic(a.Ref.IsValid() && b.Ref.IsValid());
    const Entry3D* rootA = a._compoundRootIndexPlusOne >= 1 ? a._compoundRoot : &a;
    const Entry3D* rootB = b._compoundRootIndexPlusOne >= 1 ? b._compoundRoot : &b;
    m.SetUsesCcd(rootA->_useCCD || rootB->_useCCD);
}

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
    const ComponentSet minComponents = ComponentSet::Create<::Quantum::Transform3D, PhysicsCollider3D>();
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
                const FPMatrix3x3& m = entry->WorldInertiaTensorInverse;
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
        // Radix.SortEndPoints (0x7d3d558): stable LSD radix over Index bytes 0-1 then Value
        // bytes 0-3, i.e. a stable sort by (Value, low 16 bits of Index).
        std::stable_sort(bucket.EndPoints.begin(), bucket.EndPoints.begin() + 2 * n,
                         [](const EndPoint& a, const EndPoint& b) {
                             const std::uint64_t ka = (std::uint64_t{static_cast<std::uint32_t>(a.Value)} << 16) |
                                                      static_cast<std::uint16_t>(a.Index);
                             const std::uint64_t kb = (std::uint64_t{static_cast<std::uint32_t>(b.Value)} << 16) |
                                                      static_cast<std::uint16_t>(b.Index);
                             return ka < kb;
                         });
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
bool Wants(const PhysicsCallbacksData* callbacks, CallbackFlags flag) {
    return callbacks != nullptr && HasFlag(callbacks->Flags, flag);
}
} // namespace

// 0x7d60450 - for each side with callbacks: Enter the first time the pair is seen (when the
// entity tracks enter/exit), every step OnTrigger; the pair is (re)marked in the list.
void Scene3D::HandleDynamicTrigger(FrameBase* f, ICollisionCallbacks3D* callbacks,
                                   ManifoldData3D* /*manifold*/, Entry3D* entryA, Entry3D* entryB) {
    const auto side = [&](Entry3D* self, Entry3D* other) {
        PhysicsCallbacks3D* cb = self->Callbacks;
        if (cb == nullptr || cb->Flags == CallbackFlags::None) {
            return;
        }
        const TriggerInfo3D info{self->Ref, other->Ref, false, -1};
        const bool tracks = Wants(cb, CallbackFlags::OnDynamicTriggerEnter) ||
                            Wants(cb, CallbackFlags::OnDynamicTriggerExit);
        if (tracks) {
            std::int32_t index = 0;
            if (cb->IsCollidingWith(f, other->Ref, index)) {
                cb->SetEntryIsActive(f, index, true);
            } else {
                if (Wants(cb, CallbackFlags::OnDynamicTriggerEnter)) {
                    callbacks->OnTriggerEnter3D(f, info);
                    _callbacksWereCalled = true;
                }
                cb->AddCollision(f, other->Ref);
            }
        }
        if (Wants(cb, CallbackFlags::OnDynamicTrigger)) {
            callbacks->OnTrigger3D(f, info);
            _callbacksWereCalled = true;
        }
    };
    side(entryA, entryB);
    side(entryB, entryA);
}

// TODO(0x7d60934, Quantum.Physics3D.Scene3D.HandleStaticTrigger): written after the
// dynamic version (the static entry has no callbacks; the list entry carries the static
// collider index and the trigger flag); not checked instruction by instruction.
void Scene3D::HandleStaticTrigger(FrameBase* f, ICollisionCallbacks3D* callbacks,
                                  ManifoldData3D* /*manifold*/, Entry3D* entryA, Entry3D* entryB) {
    Entry3D* self = entryA->IsDynamic() ? entryA : entryB;
    Entry3D* other = self == entryA ? entryB : entryA;
    PhysicsCallbacks3D* cb = self->Callbacks;
    if (cb == nullptr || cb->Flags == CallbackFlags::None) {
        return;
    }
    const TriggerInfo3D info{self->Ref, EntityRef{}, true, other->StaticColliderIndex};
    const bool tracks = Wants(cb, CallbackFlags::OnStaticTriggerEnter) ||
                        Wants(cb, CallbackFlags::OnStaticTriggerExit);
    if (tracks) {
        std::int32_t index = 0;
        if (cb->IsCollidingWith(f, EntityRef{}, other->StaticColliderIndex, index)) {
            cb->SetEntryIsActive(f, index, true);
        } else {
            if (Wants(cb, CallbackFlags::OnStaticTriggerEnter)) {
                callbacks->OnTriggerEnter3D(f, info);
                _callbacksWereCalled = true;
            }
            cb->AddCollision(f, EntityRef{}, other->StaticColliderIndex, true);
        }
    }
    if (Wants(cb, CallbackFlags::OnStaticTrigger)) {
        callbacks->OnTrigger3D(f, info);
        _callbacksWereCalled = true;
    }
}

// TODO(0x7d60f3c, Quantum.Physics3D.Scene3D.HandleDynamicCollision): same pattern as the
// dynamic trigger with the collision flags; the contact info fields are reduced.
void Scene3D::HandleDynamicCollision(FrameBase* f, ICollisionCallbacks3D* callbacks,
                                     CollisionManifold3D* manifold, Entry3D* entryA, Entry3D* entryB) {
    const auto side = [&](Entry3D* self, Entry3D* other, bool inverse) {
        PhysicsCallbacks3D* cb = self->Callbacks;
        if (cb == nullptr || cb->Flags == CallbackFlags::None) {
            return;
        }
        const CollisionInfo3D info{self->Ref, other->Ref, false, -1,
                                   inverse ? -manifold->Normal : manifold->Normal, manifold};
        if (Wants(cb, CallbackFlags::OnDynamicCollisionEnter) || Wants(cb, CallbackFlags::OnDynamicCollisionExit)) {
            std::int32_t index = 0;
            if (cb->IsCollidingWith(f, other->Ref, index)) {
                cb->SetEntryIsActive(f, index, true);
            } else {
                if (Wants(cb, CallbackFlags::OnDynamicCollisionEnter)) {
                    callbacks->OnCollisionEnter3D(f, info);
                    _callbacksWereCalled = true;
                }
                cb->AddCollision(f, other->Ref);
            }
        }
        if (Wants(cb, CallbackFlags::OnDynamicCollision)) {
            callbacks->OnCollision3D(f, info);
            _callbacksWereCalled = true;
        }
    };
    side(entryA, entryB, false);
    side(entryB, entryA, true);
}

// TODO(0x7d61488, Quantum.Physics3D.Scene3D.HandleStaticCollision): as HandleStaticTrigger
// with the collision flags; not checked instruction by instruction.
void Scene3D::HandleStaticCollision(FrameBase* f, ICollisionCallbacks3D* callbacks,
                                    CollisionManifold3D* manifold, Entry3D* entryA, Entry3D* entryB) {
    const bool selfIsA = entryA->IsDynamic();
    Entry3D* self = selfIsA ? entryA : entryB;
    Entry3D* other = selfIsA ? entryB : entryA;
    PhysicsCallbacks3D* cb = self->Callbacks;
    if (cb == nullptr || cb->Flags == CallbackFlags::None) {
        return;
    }
    const CollisionInfo3D info{self->Ref, EntityRef{}, true, other->StaticColliderIndex,
                               selfIsA ? manifold->Normal : -manifold->Normal, manifold};
    if (Wants(cb, CallbackFlags::OnStaticCollisionEnter) || Wants(cb, CallbackFlags::OnStaticCollisionExit)) {
        std::int32_t index = 0;
        if (cb->IsCollidingWith(f, EntityRef{}, other->StaticColliderIndex, index)) {
            cb->SetEntryIsActive(f, index, true);
        } else {
            if (Wants(cb, CallbackFlags::OnStaticCollisionEnter)) {
                callbacks->OnCollisionEnter3D(f, info);
                _callbacksWereCalled = true;
            }
            cb->AddCollision(f, EntityRef{}, other->StaticColliderIndex, false);
        }
    }
    if (Wants(cb, CallbackFlags::OnStaticCollision)) {
        callbacks->OnCollision3D(f, info);
        _callbacksWereCalled = true;
    }
}

// Second half of Task_Callbacks: list entries not seen this step raise their exit callback
// and are dropped; the rest are reset for the next step.
void Scene3D::HandleExits(FrameBase* f, ICollisionCallbacks3D* callbacks) {
    for (std::size_t i = static_cast<std::size_t>(_staticComponentsCount); i < _entries.size(); ++i) {
        Entry3D& entry = _entries[i];
        PhysicsCallbacks3D* cb = entry.Callbacks;
        if (cb == nullptr || cb->ListCount() == 0) {
            continue;
        }
        PhysicsCallbacksEntry* items = cb->Entries(f);
        std::int32_t count = cb->ListCount();
        for (std::int32_t k = 0; k < count;) {
            PhysicsCallbacksEntry& item = items[k];
            if ((item._flags & PhysicsCallbacksEntry::IsActive) != 0) {
                item._flags &= ~PhysicsCallbacksEntry::IsActive;
                ++k;
                continue;
            }
            const bool isStatic = (item._flags & PhysicsCallbacksEntry::IsStatic) != 0;
            const bool isTrigger = (item._flags & PhysicsCallbacksEntry::IsTrigger) != 0 ||
                                   (!isStatic && entry.Collider3D->IsTrigger());
            const ExitInfo3D info{entry.Ref, item.Other, isStatic, isStatic ? item.StaticColliderIndex : -1};
            const CallbackFlags flag = isStatic ? (isTrigger ? CallbackFlags::OnStaticTriggerExit
                                                             : CallbackFlags::OnStaticCollisionExit)
                                                : (isTrigger ? CallbackFlags::OnDynamicTriggerExit
                                                             : CallbackFlags::OnDynamicCollisionExit);
            if (HasFlag(cb->Flags, flag)) {
                if (isTrigger) {
                    callbacks->OnTriggerExit3D(f, info);
                } else {
                    callbacks->OnCollisionExit3D(f, info);
                }
                _callbacksWereCalled = true;
            }
            items[k] = items[count - 1];
            --count;
        }
        cb->SetListCount(count);
    }
}

// TODO(0x7d617a0, Quantum.Physics3D.Scene3D.Task_Callbacks): per bucket, collision
// manifolds (static or dynamic by the manifold's IsDynamic flag), then trigger manifolds,
// then the exit pass. Missing: mesh manifolds, the CheckIgnore* component filters (pairs
// whose entities lost their components during callbacks) and CCD callbacks.
void Scene3D::TaskCallbacks(FrameBase* frame, ICollisionCallbacks3D* callbacks) {
    if (callbacks == nullptr) {
        return;
    }
    for (Bucket3D& bucket : _buckets) {
        for (std::size_t i = 1; i < bucket.CollisionManifolds.size(); ++i) {
            CollisionManifold3D* m = &bucket.CollisionManifolds[i];
            if (m->GetIgnore()) continue;
            Entry3D* a = &_entries[static_cast<std::size_t>(m->_m._entryIdxA)];
            Entry3D* b = &_entries[static_cast<std::size_t>(m->_m._entryIdxB)];
            if (m->_m.GetIsDynamic()) {
                HandleDynamicCollision(frame, callbacks, m, a, b);
            } else {
                HandleStaticCollision(frame, callbacks, m, a, b);
            }
        }
        for (std::size_t i = 1; i < bucket.TriggerManifolds.size(); ++i) {
            ManifoldData3D* m = &bucket.TriggerManifolds[i];
            if (m->GetIgnore()) continue;
            Entry3D* a = &_entries[static_cast<std::size_t>(m->_entryIdxA)];
            Entry3D* b = &_entries[static_cast<std::size_t>(m->_entryIdxB)];
            if (m->GetIsDynamic()) {
                HandleDynamicTrigger(frame, callbacks, m, a, b);
            } else {
                HandleStaticTrigger(frame, callbacks, m, a, b);
            }
        }
    }
    HandleExits(frame, callbacks);
}

// ---------------------------------------------------------------------------------------
// Islands, solver, integration, sleeping

// TODO(0x7d6351c, Quantum.Physics3D.Scene3D.Task_BuildIslands): union of dynamic bodies
// through their non-trigger contacts (static/kinematic bodies don't join islands); the
// original's traversal order and the joint islands (Task_BuildJointIslands) are not
// reproduced exactly.
void Scene3D::TaskBuildIslands() {
    _islands.clear();
    const std::size_t first = static_cast<std::size_t>(_staticComponentsCount);
    const std::size_t count = _entries.size();
    // Contacts per dynamic entry.
    std::vector<std::vector<CollisionManifold3D*>> contacts(count);
    for (Bucket3D& bucket : _buckets) {
        for (std::size_t i = 1; i < bucket.CollisionManifolds.size(); ++i) {
            CollisionManifold3D* m = &bucket.CollisionManifolds[i];
            if (m->GetIgnore()) continue;
            Entry3D* a = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxA)]);
            Entry3D* b = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxB)]);
            if (a->HasDynamicBody) contacts[static_cast<std::size_t>(a->Index)].push_back(m);
            if (b->HasDynamicBody) contacts[static_cast<std::size_t>(b->Index)].push_back(m);
        }
    }
    std::vector<char> visited(count, 0);
    std::vector<char> manifoldAdded;
    std::vector<Entry3D*> stack;
    _islands.reserve(count);
    for (std::size_t i = first; i < count; ++i) {
        Entry3D* start = &_entries[i];
        start->Island = nullptr;
        if (!start->HasDynamicBody || visited[i] || start->_compoundRootIndexPlusOne >= 1) continue;
        Island3D& island = _islands.emplace_back();
        stack.assign(1, start);
        visited[i] = 1;
        while (!stack.empty()) {
            Entry3D* e = stack.back();
            stack.pop_back();
            island.AddEntry(e);
            for (CollisionManifold3D* m : contacts[static_cast<std::size_t>(e->Index)]) {
                if (std::find(island.CollisionManifolds.begin(), island.CollisionManifolds.end(), m) ==
                    island.CollisionManifolds.end()) {
                    island.AddManifold(m);
                }
                Entry3D* a = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxA)]);
                Entry3D* b = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxB)]);
                Entry3D* other = a == e ? b : a;
                if (other->HasDynamicBody && !visited[static_cast<std::size_t>(other->Index)]) {
                    visited[static_cast<std::size_t>(other->Index)] = 1;
                    stack.push_back(other);
                }
            }
        }
    }
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

// The perceived-mass term of one body for a contact offset r along the normal (n squared
// terms precomputed): (|r x n|^2) * |Right|^2 * inverseInertia.x, as the original computes
// it (an isotropic approximation of the inertia).
FP::Raw AngularMassTerm(const Entry3D* entry, const FPVector3& r, FP::Raw nxx, FP::Raw nyy,
                        FP::Raw nzz, FP::Raw nzx, FP::Raw nyx, FP::Raw nzy) {
    const FP::Raw rx = Raw(r.X), ry = Raw(r.Y), rz = Raw(r.Z);
    const FP::Raw zz = Mul(rz, rz), yy = Mul(ry, ry), xx = Mul(rx, rx);
    const FP::Raw yz = Mul(ry, rz), xy = Mul(rx, ry), xz = Mul(rx, rz);
    FP::Raw cross = Mul(zz, nyy) + Mul(zz, nxx);
    cross += Mul(yy, nxx);
    cross += Mul(yy, nzz);
    cross -= (yz * nzy) >> 15;
    cross += Mul(xx, nyy);
    cross += Mul(xx, nzz);
    cross -= ((xy * nyx) >> 15) + ((xz * nzx) >> 15);
    const FPVector3& right = entry->Narrow.RotatedRight;
    const FP::Raw rightSq = Mul(Raw(right.Y), Raw(right.Y)) + Mul(Raw(right.X), Raw(right.X)) +
                            Mul(Raw(right.Z), Raw(right.Z));
    const FP::Raw inertia = Mul(rightSq, entry->Body3D->_inertiaTensorInverse.RawIntX);
    return Mul(inertia, cross);
}
} // namespace

// 0x7d572fc - combined material values and one constraint per contact point (plus one at
// the average point for multi-point manifolds): offsets from both centres of mass (at the
// manifold's time of impact) and the perceived mass along the normal. Constraints are made
// for the island manifolds in island order here (the original slices the cached manifold
// list over threads, same result).
void Scene3D::TaskCollisionSolverInitLegacyGS(FrameBase* frame) {
    _collisionSolverData.clear();
    _collisionSolverManifolds.clear();
    for (Island3D& island : _islands) {
        for (CollisionManifold3D* m : island.CollisionManifolds) {
            _collisionSolverManifolds.push_back(m);
        }
    }
    for (CollisionManifold3D* m : _collisionSolverManifolds) {
        Entry3D* entryA = &_entries[static_cast<std::size_t>(m->_m._entryIdxA)];
        Entry3D* entryB = &_entries[static_cast<std::size_t>(m->_m._entryIdxB)];
        Entry3D* rootA = Root(entryA);
        Entry3D* rootB = Root(entryB);
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
        const std::int32_t points = m->_contactPointsCount == 1 ? 1 : m->_contactPointsCount + 1;
        m->_constraintDataIdx = static_cast<std::int32_t>(_collisionSolverData.size());
        const FP::Raw nx = Raw(m->Normal.X), ny = Raw(m->Normal.Y), nz = Raw(m->Normal.Z);
        const FP::Raw nxx = Mul(nx, nx), nyy = Mul(ny, ny), nzz = Mul(nz, nz);
        const FP::Raw nzx = Mul(nz, nx), nyx = Mul(ny, nx), nzy = Mul(nz, ny);
        for (std::int32_t p = 0; p < points; ++p) {
            const FPVector3 contact = p < m->_contactPointsCount ? m->_contactPoints[p]
                                                                 : CollisionManifold3D::GetAverageContactPoint(m);
            ContactConstraint& c = _collisionSolverData.emplace_back();
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
            c.RA = offset(rootA);
            c.RB = offset(rootB);
            if (rootA->HasDynamicBody) {
                c.DynamicA = true;
                k += rootA->Body3D->_massInverseRawInt;
                k += AngularMassTerm(entryA, c.RA, nxx, nyy, nzz, nzx, nyx, nzy);
            }
            if (rootB->HasDynamicBody) {
                c.DynamicB = true;
                k += rootB->Body3D->_massInverseRawInt;
                k += AngularMassTerm(entryB, c.RB, nxx, nyy, nzz, nzx, nyx, nzy);
            }
            c.PerceivedMass = FP::FromRaw(k);
        }
    }
}

namespace {
FPVector3 Cross(const FPVector3& a, const FPVector3& b) { return FPVector3::Cross(a, b); }

void ApplyImpulse(Entry3D* entry, const FPVector3& linear, const FPVector3& r, bool useRotation) {
    PhysicsBody3D* body = entry->Body3D;
    const FP massInv = body->MassInverse();
    body->Velocity = body->Velocity + linear * massInv;
    if (useRotation) {
        const FPVector3 angular = Cross(r, linear);
        const FPMatrix3x3& m = entry->WorldInertiaTensorInverse;
        const FPVector3 dw{m.M00 * angular.X + m.M01 * angular.Y + m.M02 * angular.Z,
                           m.M10 * angular.X + m.M11 * angular.Y + m.M12 * angular.Z,
                           m.M20 * angular.X + m.M21 * angular.Y + m.M22 * angular.Z};
        const auto freeze = static_cast<std::uint8_t>(body->RotationFreeze);
        if (!(freeze & 1)) body->AngularVelocity.X += dw.X;
        if (!(freeze & 2)) body->AngularVelocity.Y += dw.Y;
        if (!(freeze & 4)) body->AngularVelocity.Z += dw.Z;
    }
}
} // namespace

// TODO(0x7d57ebc, Quantum.Physics3D.Scene3D.Task_VelocitySolverLegacyGS): the iteration
// loop is not studied yet. The impulse per contact follows ComputeImpulse (0x7d589b0):
// relative velocity at the contact; separating contacts get nothing; normal impulse
// -(1 + restitution) * vn / k (restitution dropped below the resting threshold); friction
// along the tangent clamped by the static coefficient, else dynamic friction * normal
// impulse. ApplyImpulse/AccumulateImpulses are approximated with plain velocity updates.
void Scene3D::TaskVelocitySolverLegacyGS() {
    const bool useRotation = CachedSettings.UseAngularVelocityEnabled();
    const FPVector3 gravityStep = CachedSettings.Gravity * _deltaTime;
    const FP::Raw restThreshold = Mul(Raw(FPVector3::Dot(gravityStep, gravityStep)), 1 << 16) + 0x41;
    const int iterations = std::max<int>(1, CachedSettings.SolverIterations);
    for (int iteration = 0; iteration < iterations; ++iteration) {
        for (ContactConstraint& c : _collisionSolverData) {
            CollisionManifold3D* m = c.Manifold;
            if (Raw(c.PerceivedMass) == 0) continue;
            Entry3D* a = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxA)]);
            Entry3D* b = Root(&_entries[static_cast<std::size_t>(m->_m._entryIdxB)]);
            FPVector3 vRel = FPVector3::Zero();
            if (c.DynamicA) {
                vRel = vRel - a->Body3D->Velocity;
                if (useRotation) vRel = vRel - Cross(a->Body3D->AngularVelocity, c.RA);
            }
            if (c.DynamicB) {
                vRel = vRel + b->Body3D->Velocity;
                if (useRotation) vRel = vRel + Cross(b->Body3D->AngularVelocity, c.RB);
            }
            const FPVector3 n = m->Normal;
            const FP vn = FPVector3::Dot(vRel, n);
            if (Raw(vn) >= 1) continue; // separating
            const FP::Raw speedSq = Raw(FPVector3::Dot(vRel, vRel));
            const FP factor = speedSq < restThreshold ? -FP::One()
                                                      : FP::FromRaw(-(FP::Raw{1} << 16) - Raw(m->CombinedRestitution));
            const FP jn = (factor * vn) / c.PerceivedMass;
            FPVector3 impulse = n * jn;
            // Friction along the tangent.
            FPVector3 tangent = vRel - n * vn;
            if (Raw(FPVector3::Dot(tangent, tangent)) > 0x41) {
                tangent = tangent.Normalized();
                const FP jt = -FPVector3::Dot(tangent, vRel) / c.PerceivedMass;
                const FP limit = m->CombinedStaticFriction * jn;
                if (Photon::Deterministic::FPMath::Abs(jt) < limit) {
                    impulse = impulse + tangent * jt;
                } else {
                    impulse = impulse - tangent * (jn * m->CombinedDynamicFriction);
                }
            }
            if (c.DynamicA) ApplyImpulse(a, -impulse, c.RA, useRotation);
            if (c.DynamicB) ApplyImpulse(b, impulse, c.RB, useRotation);
        }
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
            const FPMatrix3x3& m = entry->WorldInertiaTensorInverse;
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

// TODO(0x7d68798, Quantum.Physics3D.Scene3D.Task_SleepDetectionEntries) and
// TODO(0x7d68cdc, Quantum.Physics3D.Scene3D.Task_SleepDetectionIslands): written from the
// settings' meaning, not from the original's code: a body below both sleep tolerances
// accumulates sleep time; an island whose bodies all reached SleepTimeSec falls asleep.
void Scene3D::TaskSleepDetection() {
    if (!CachedSettings.AllowSleepingEnabled()) {
        return;
    }
    const FP linearTol = CachedSettings.LinearSleepTolerance;
    const FP angularTol = CachedSettings.AngularSleepToleranceInRad;
    for (Island3D& island : _islands) {
        bool allAsleep = true;
        for (Entry3D* entry : island.Entries) {
            PhysicsBody3D* body = entry->Body3D;
            const bool slow = body->Velocity.SqrMagnitude() <= linearTol * linearTol &&
                              body->AngularVelocity.SqrMagnitude() <= angularTol * angularTol;
            if (!body->AllowSleeping() || !slow) {
                body->_sleepTimeRawInt = 0;
                allAsleep = false;
                continue;
            }
            body->_sleepTimeRawInt = static_cast<std::int32_t>(Raw(body->SleepTime()) + Raw(_deltaTime));
            if (body->SleepTime() < CachedSettings.SleepTimeSec) {
                allAsleep = false;
            }
        }
        if (allAsleep && !island.Entries.empty()) {
            for (Entry3D* entry : island.Entries) {
                entry->Body3D->Sleep();
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
    // TODO: CCD manifolds (Task_SortCcdManifolds, Task_CcdCallbacks, Task_IntegrateFirstTOI)
    // and Task_PostCallbacksUpdateEntries/Manifolds (entities changed by callbacks).
    TaskBuildIslands();
    if (CachedSettings.SolverIterations != 0) {
        // TODO(0x7d65098, Quantum.Physics3D.Scene3D.Task_CollisionSolverInit) and the rest
        // of the hybrid Jacobi/Gauss-Seidel solver: not ported; both solver types use the
        // legacy Gauss-Seidel solver here.
        TaskCollisionSolverInitLegacyGS(frame);
        TaskVelocitySolverLegacyGS();
    }
    TaskPenetrationCorrection();
    TaskIntegrateVelocity();
    TaskSleepDetection();
}

// ---------------------------------------------------------------------------------------
// Immediate queries

// TODO(0x7d56464, Quantum.Physics3D.Scene3D.Linecast): the original walks the broad-phase
// buckets along the segment; here every non-compound entry is tested (same hits, order
// not guaranteed - callers sort). Static mesh triangles are not tested.
void Scene3D::Linecast(FrameBase* frame, HitCollection3D* hits, const FPVector3& start,
                       const FPVector3& end, bool firstHitOnly, std::int32_t layerMask,
                       std::int32_t options) {
    const bool detailed = (options & ComputeDetailedInfo) != 0;
    for (Entry3D& entry : _entries) {
        if (entry.Collider3D == nullptr || ((layerMask >> entry.Collider3D->Layer()) & 1) == 0) continue;
        const bool isStatic = !entry.IsDynamic();
        const bool trigger = entry.IsTriggerCollider();
        if (trigger && !(options & HitTriggers)) continue;
        if (isStatic && !(options & HitStatics)) continue;
        if (!isStatic && entry.HasDynamicBody && !(options & HitDynamics)) continue;
        if (!isStatic && !entry.HasDynamicBody && !(options & HitKinematics)) continue;
        Hit3D hit{};
        if (NarrowChecks::LinecastBody::Check(_currentMap, &entry, start, end, detailed, hit)) {
            hits->Add(hit, frame->get_Context());
        }
    }
    if (firstHitOnly && hits->Count() > 1) {
        hits->SortCastDistance();
        hits->SetCount(1);
    }
}

// TODO(0x7d56d18, Quantum.Physics3D.Scene3D.OverlapShape): brute force over the entries
// with the narrow checks (sphere/box query shapes); the original goes through the broad
// phase and GJK for other pairs.
void Scene3D::OverlapShape(FrameBase* frame, HitCollection3D* hits, const FPVector3& pos,
                           const FPQuaternion& rot, const Shape3D& shape, std::int32_t layerMask,
                           std::int32_t options) {
    Entry3D query{};
    PhysicsCollider3D collider{};
    collider.Shape = shape;
    query.Collider3D = &collider;
    query.Shape = &collider.Shape;
    query.Index = -1;
    NarrowData3D::Init(&query.Narrow, &query, pos, rot);
    const bool detailed = (options & ComputeDetailedInfo) != 0;
    for (Entry3D& entry : _entries) {
        if (entry.Collider3D == nullptr || ((layerMask >> entry.Collider3D->Layer()) & 1) == 0) continue;
        if (entry.IsTriggerCollider() && !(options & HitTriggers)) continue;
        if (!entry.IsDynamic() && !(options & HitStatics)) continue;
        CollisionManifold3D m{};
        CollisionManifoldInfo3D info{&m, &query, &entry, nullptr};
        if (NarrowChecks::CheckShapes(&info, detailed)) {
            Hit3D hit{};
            hit._staticColliderIndex = entry.StaticColliderIndex;
            if (entry.IsDynamic()) {
                hit._entity = HitEntityRef{entry.Ref.Index, entry.Ref.Version};
            } else {
                hit._mapGuid = _currentMap;
            }
            hit.SetIsTrigger(entry.IsTriggerCollider());
            hit.ShapeUserTag = entry.Shape->UserTag;
            hit.OverlapPenetration = m.Penetration;
            hit.Normal = m.Normal;
            if (m._contactPointsCount > 0) hit.Point = m._contactPoints[0];
            hits->Add(hit, frame->get_Context());
        }
    }
}

} // namespace Quantum::Physics3D
