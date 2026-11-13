// frame_base.h - Quantum.Core.FrameBase: the entity-component store of a simulation
// frame.
//
// A frame owns, in memory from the context's native allocator:
//  * the entity table (EntityInfo per slot; free slots are chained through Ref.Index,
//    newest first),
//  * one component mask (ComponentSet) per entity slot,
//  * one ComponentDataBuffer per registered component type, plus the shared sparse array
//    (entity slot -> component slot) the buffers index into,
//  * a culling bit per entity slot (set by the culling pass on predicted frames),
// and a heap (Quantum.Allocator.Heap, reserved from the context's group) for dynamic
// simulation data, accessed through a FrameHeap.
//
// Removing a component or destroying an entity takes effect for queries at once (the
// mask bit is cleared, the entity version bumped) but the component data stays in place
// until the frame commits: removals and destructions are recorded in FrameMetaData's
// deferred sets and applied by CommitRemoves / CommitDestroys, at the moment chosen by
// the game's CommitCommandsMode. This keeps component slots from moving while systems
// iterate over them.
//
// FrameBase is abstract: the game's Frame (quantum.code) supplies the frame metadata, the
// commit mode, the delta time and the DeterministicFrame members (checksum, dumps,
// serialisation of the whole frame, copying).
#pragma once

#include "photon_deterministic/deterministic_frame.h"
#include "photon_deterministic/fp.h"
#include "quantum_core/component_data_buffer.h"
#include "quantum_core/component_set.h"
#include "quantum_core/component_type_id.h"
#include "quantum_core/entities_config.h"
#include "quantum_core/entity_ref.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/frame_heap.h"
#include "quantum_core/frame_meta_data.h"
#include "quantum_core/heap.h"
#include "quantum_core/q_dictionary.h"
#include "quantum_core/q_hash_collection.h"
#include "quantum_core/q_list.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace Quantum {

class FrameSerializer;

template <class T> struct ComponentIterator;
template <class... Ts> struct ComponentFilter;

// Outcome of FrameBase::Add.
enum class AddResult : int {
    EntityDoesNotExist = 0,
    ComponentAlreadyExists = 1,
    ComponentAdded = 2,
};

// Outcome of FrameBase::Set.
enum class SetResult : int {
    EntityDoesNotExist = 0,
    ComponentUpdated = 1,
    ComponentAdded = 2,
};

namespace Core {

class FrameBase : public Photon::Deterministic::DeterministicFrame {
  public:
    static constexpr int COMPONENT_START_INDEX = 1;
    static constexpr int CULLED_WORD_BYTE_SIZE = 8;
    static constexpr int CULLED_WORD_BIT_SIZE = 64;
    static constexpr int COMPONENT_MASK_WORD_BYTESIZE = 4;
    static constexpr int COMPONENT_MASK_WORD_BITSIZE = 32;
    static constexpr int DefaultInitialCollectionCapacity = 8;

    using EntitiesConfig = Core::EntitiesConfig;
    using NativeAllocator = Photon::Deterministic::Native::Allocator;

    // 0x7d1cf4c's result (Quantum.Core.FrameBase.Stats).
    struct Stats {
        Quantum::Allocator::Heap::Stats HeapStats;
        int EntityCount = 0;
        int EntityTotalMemory = 0;
    };

    // Signals raised when a component of one type is added or removed: (entity, data).
    // 0x7d1e390 (.ctor) / 0x7d1e430 (Invoke) in the original, a delegate type.
    using ComponentReactiveCallbackInvoker = std::function<void(EntityRef, void*)>;

    // Pointer-based access without the safety checks of the managed API, plus the
    // reader/writer lock used by multithreaded systems (Quantum.Core.FrameBase.
    // FrameBaseUnsafe).
    class FrameBaseUnsafe {
      public:
        // 0x7d1e29c
        explicit FrameBaseUnsafe(FrameBase* f) : _f(f) {}

        // 0x52e5640 (instance for AINavigation)
        template <class T> ComponentBlockIterator<T> GetComponentBlockIterator();
        // 0x52e5e84 (instance for AINavigation)
        template <class T> T* GetPointer(EntityRef entityRef) { return _f->GetPointer<T>(entityRef); }
        // 0x52e65cc (instance for AIBlackboardComponent)
        template <class T> bool TryGetPointer(EntityRef entityRef, T*& value) {
            return _f->TryGetPointer<T>(entityRef, value);
        }
        // 0x7d1e36c
        void CommitAllCommands() {
            _f->CommitRemoves();
            _f->CommitDestroys();
        }
        // 0x52e6548 (instance for ActorManager)
        template <class T> T* GetPointerSingleton() { return _f->GetPointerSingleton<T>(); }
        // 0x52e9540 (instance for ActorManager)
        template <class T> bool TryGetPointerSingleton(T*& component) {
            return _f->TryGetPointerSingleton<T>(component);
        }
        // 0x52e5d18 (instance for BlastBallGame)
        template <class T> T* GetOrAddSingletonPointer(EntityRef optionalAddTarget = EntityRef{}) {
            return _f->GetOrAddSingletonPointer<T>(optionalAddTarget);
        }
        // 0x7d1cf20 - Drops the lock.
        void Dispose() { _rwLock.reset(); }

        std::shared_mutex* Lock() const { return _rwLock.get(); }

        // TODO(0x52b7474, Quantum.Core.FrameBase.FrameBaseUnsafe.FilterStruct<T>): filters
        // into a user struct described by ComponentFilterStructMeta, which reflects over
        // the struct's fields; needs a C++ description of filter structs.
        // TODO(0x52ad168, Quantum.Core.FrameBase.FrameBaseUnsafe.ComponentGetter<T>): same
        // dependency on ComponentFilterStructMeta.

      private:
        FrameBase* _f;
        std::unique_ptr<std::shared_mutex> _rwLock = std::make_unique<std::shared_mutex>();
    };

    // 0x7d1cc70 - Reserves a heap from the context's group and gives the frame a tracked
    // heap with a disabled tracker. The entity store itself is created by Initialize.
    explicit FrameBase(FrameContext* context);
    ~FrameBase() override;

    FrameBase(const FrameBase&) = delete;
    FrameBase& operator=(const FrameBase&) = delete;

    using DeterministicFrame::Copy;
    using DeterministicFrame::Serialize;

    // --- members supplied by the game's Frame ------------------------------------------

    virtual FrameMetaData* get_FrameMetaData() = 0;
    virtual CommitCommandsModes get_CommitCommandsMode() const = 0;
    virtual Photon::Deterministic::FP get_DeltaTime() const = 0;
    virtual void set_DeltaTime(Photon::Deterministic::FP value) = 0;
    // Also abstract in the original, waiting for their types: get_PhysicsSceneSettings,
    // get_Map, get_NavMeshRegionMask, get__physicsState2D, get__physicsState3D.

    // --- frame lifetime ------------------------------------------------------------------

    // 0x7d1adec - Creates the entity store: capacity max(32, DefaultEntityCapacity), the
    // component buffers of every registered type, the deferred-command sets in the frame
    // metadata. Entity slot 0 is reserved.
    static void Initialize(FrameBase* f, const EntitiesConfig& config);
    // 0x7d1a80c - Makes `to`'s entity store equal to `from`'s (resizing as needed). The
    // culling bits are not copied.
    static void Copy(FrameBase* to, const FrameBase* from);
    // TODO(0x7d199f0, Quantum.Core.FrameBase.Serialize): the physics states are missing
    // (written as absent). Otherwise complete: writes or reads the entity store; when
    // reading, the frame first trades its heap for a fresh one from the group, and every
    // table is reallocated.
    static void Serialize(FrameBase* r, FrameSerializer* serializer);
    // 0x7d1cdf4 - Gives the heap back and frees the entity store.
    void Free() override;
    // 0x7d1cd7c
    void InitTrackedHeap(std::shared_ptr<Quantum::Allocator::IHeapAllocationTracker> tracker);
    // 0x7d1d104 - Frees the previous frame's tracked allocations, applies deferred
    // commands in OnFrameSimulationBegin mode, clears the culling bits.
    void OnFrameSimulateBegin();
    // 0x7d1d158 - Applies deferred commands in OnFrameSimulationEnd mode.
    void OnFrameSimulateEnd();
    // 0x7d1cf4c
    Stats GetMemoryStats() const;
    // 0x7d1a9b4 - Grows or shrinks the entity tables. Shrinking keeps the allocations
    // (later growth up to the old size reuses them) and clears the dropped part.
    void ChangeCapacity(int newCapacity);

    // TODO(0x7d19560, Quantum.Core.FrameBase.Print): needs FramePrinter.

    // --- entities ------------------------------------------------------------------------

    // 0x7d1c560 - A new live entity (reusing the most recently freed slot first). Grows
    // the tables to twice their size when full.
    EntityRef Create();
    // 0x7d1bf20 - Marks every component of the entity removed (raising the removal
    // callbacks and signals), queues the entity for destruction and invalidates the
    // handle at once. False if the entity does not exist.
    bool Destroy(EntityRef entityRef);
    // 0x7d1b274
    bool Exists(EntityRef entityRef) const {
        return static_cast<std::uint32_t>(entityRef.Index) < static_cast<std::uint32_t>(_capacity) &&
               _info[entityRef.Index].Ref.Version == entityRef.Version;
    }
    // 0x7d1cb60 - Throws std::logic_error for a stale or invalid handle.
    void CheckExistsAndThrowIfNot(EntityRef entityRef) const;
    // 0x7d1bee4 - The entity's component mask; throws for a missing entity.
    ComponentSet GetComponentSet(EntityRef entityRef) const;

    // TODO(0x7d1c42c, Quantum.Core.FrameBase.Create(AssetRefEntityPrototype)): needs
    // entity prototypes and the asset database.
    // TODO(0x7d1c4ac, Quantum.Core.FrameBase.Create(EntityPrototype)): same.
    // TODO(0x7d1c8e0, Quantum.Core.FrameBase.Create(prototypes, Map)): same, plus Map.
    // TODO(0x7d1c5e8, Quantum.Core.FrameBase.MaterializePrototypeInternal): same.
    // TODO(0x7d1c7b8, Quantum.Core.FrameBase.InvokeEntityPrototypeMaterializedListeners):
    // same.

    // --- culling -------------------------------------------------------------------------

    // 0x7d1b11c
    void ClearCulledState();
    // 0x7d1b198 - Missing entities count as culled.
    bool IsCulled(EntityRef entityRef) const;
    // 0x7d1b1f8 - A non-cullable entity is also un-culled right away.
    void SetCullable(EntityRef entityRef, bool cullable);
    // 0x7d1a7d8, 0x7d1a7f4 - Size of a culling bit array for `size` entities.
    static int RoundToCulledByteSize(int size) { return RoundToCulledWordCount(size) * CULLED_WORD_BYTE_SIZE; }
    static int RoundToCulledWordCount(int size) { return (size + 63) / 64; }

    // --- components ----------------------------------------------------------------------

    // 0x7d19984
    bool Has(EntityRef entityRef, int componentIndex) const {
        return Exists(entityRef) && _componentMasks[entityRef.Index].IsSet(componentIndex);
    }
    // 0x7d1d190 - Throws for a missing entity.
    bool Has(EntityRef entityRef, const ComponentSet& set) const;
    // 0x4eac484 (instance for AIMoveForwardBehaviour)
    template <class T> bool Has(EntityRef entityRef) const {
        return Has(entityRef, ComponentTypeId<T>::Id);
    }

    // 0x7d1b2b4 - Marks the component removed (callbacks and signals run now; the data
    // goes at the next commit). False if the entity does not have it.
    bool RemoveComponentIndexUnchecked(EntityRef entityRef, int componentIndex);
    // 0x4eadc3c (instance for AIMoveForwardBehaviour)
    template <class T> bool Remove(EntityRef entityRef) {
        return RemoveComponentIndexUnchecked(entityRef, ComponentTypeId<T>::Id);
    }

    // 0x4e9c090 (instance for AINavigation) - Adds a zero-initialised T.
    template <class T> AddResult Add(EntityRef entity) { return Add<T>(entity, T{}); }
    // 0x4e9cce8 (instance for AINavigation)
    template <class T> AddResult Add(EntityRef entity, const T& value);
    // 0x4e9e3a4 (instance for GatlingEnhancement)
    template <class T> AddResult Add(EntityRef entity, T*& result) {
        return Add<T>(entity, T{}, result);
    }
    // 0x4ea0b50 (instance for GatlingEnhancement) - On success `result` points at the
    // stored component; otherwise it is null.
    template <class T> AddResult Add(EntityRef entity, const T& value, T*& result);
    // 0x4ea1788 (instance for AINavigationBlacklist) - Adds a zero-initialised T if
    // missing; false (null result) only for a missing entity.
    template <class T> bool AddOrGet(EntityRef entity, T*& result);
    // 0x4eb09bc (instance for AIBlackboardComponent) - Stores `value`; for a new
    // component the type's OnAdded callback runs first, then the add signal.
    template <class T> SetResult Set(EntityRef entity, const T& value);
    // 0x4eaacc0 (instance for AINavigation) - Throws if the entity or the component is
    // missing.
    template <class T> T* GetPointer(EntityRef entityRef);
    // 0x4ef4bf0 (instance for AIBlackboardComponent)
    template <class T> bool TryGetPointer(EntityRef entityRef, T*& value);
    // 0x4ea78c4 (instance for the first registered type)
    template <class T> T Get(EntityRef entityRef) { return *GetPointer<T>(entityRef); }
    // 0x4eec9c0
    template <class T> bool TryGet(EntityRef entityRef, T& value) {
        T* pointer = nullptr;
        if (TryGetPointer<T>(entityRef, pointer)) {
            value = *pointer;
            return true;
        }
        value = T{};
        return false;
    }
    // 0x4ea4ab4 (instance for AgentOfDoomGroup)
    template <class T> int ComponentCount(bool includePendingRemoval = false) const {
        const ComponentDataBuffer* buffer = GetDataBuffer<T>();
        int count = buffer->get_Count();
        if (!includePendingRemoval) {
            count -= buffer->_pendingRemoval;
        }
        return count;
    }

    // 0x7d1cc14
    ComponentDataBuffer* GetDataBuffer(int index) const { return &_componentBuffers[index]; }
    // 0x4ea9a24 (instance for AISpawner)
    template <class T> ComponentDataBuffer* GetDataBuffer() const {
        return GetDataBuffer(ComponentTypeId<T>::Id);
    }

    // --- singletons ----------------------------------------------------------------------

    // 0x4eabf08 (instance for ActorManager) - Throws if there is no instance.
    template <class T> T* GetPointerSingleton();
    // 0x4eac154 (instance for ActorManager)
    template <class T> T GetSingleton() { return *GetPointerSingleton<T>(); }
    // 0x4eff3f4 (instance for BlastBallGame)
    template <class T> bool TryGetSingletonEntityRef(EntityRef& entityRef) const {
        entityRef = GetDataBuffer<T>()->get_SingletonRef();
        return entityRef.IsValid();
    }
    // 0x4efe1a4 (instance for ActorManager)
    template <class T> bool TryGetPointerSingleton(T*& component);
    // 0x4efeb48 (instance for ActorManager)
    template <class T> bool TryGetSingleton(T& component) {
        T* pointer = nullptr;
        if (TryGetPointerSingleton<T>(pointer)) {
            component = *pointer;
            return true;
        }
        component = T{};
        return false;
    }
    // 0x4eaa880 (instance for BlastBallGame) - The instance, adding one (to
    // `optionalAddTarget`, or to a new entity if that is None) if there is none.
    template <class T> T* GetOrAddSingletonPointer(EntityRef optionalAddTarget = EntityRef{});
    // 0x4eaa600 (instance for TimeScale)
    template <class T> T GetOrAddSingleton(EntityRef optionalAddTarget = EntityRef{}) {
        return *GetOrAddSingletonPointer<T>(optionalAddTarget);
    }

    // --- iteration -----------------------------------------------------------------------

    // 0x4ea94a4 (instance for BlastBallGoal) - Entity/value pairs, skipping instances
    // pending removal (component_iterators.h).
    template <class T> ComponentIterator<T> GetComponentIterator();
    // 0x4ea8a24 (instance for AINavigation) - Entity/pointer pairs block by block.
    template <class T> ComponentBlockIterator<T> GetComponentBlockIterator();
    // 0x4ea5140, 0x4ea584c (instance for <Transform3D, Killable>), 0x4ea5aa8, 0x4ea5c3c -
    // Entities having all of Ts, at least one of `any` (if not empty) and none of
    // `without`.
    template <class... Ts>
    ComponentFilter<Ts...> Filter(ComponentSet without = ComponentSet{},
                                  ComponentSet any = ComponentSet{});

    // --- deferred commands ---------------------------------------------------------------

    // 0x7d1b56c - CommitRemoves, then CommitDestroys.
    void CommitAllCommands();
    // 0x7d1b584 - Drops the data of components removed since the last commit (unless the
    // component was added back meanwhile).
    void CommitRemoves();
    // 0x7d1b978 - Drops the components and frees the slots of entities destroyed since the
    // last commit.
    void CommitDestroys();

    // --- heap collections ----------------------------------------------------------------

    // 0x7d1d0a4, 0x7d1d0c4, 0x7d1d0e4 - Address of a heap block.
    void* ResolveList(Ptr ptr) const { return _trackedHeap->Void(ptr); }
    void* ResolveDictionary(Ptr ptr) const { return _trackedHeap->Void(ptr); }
    void* ResolveHashCollection(Ptr ptr) const { return _trackedHeap->Void(ptr); }

    // 0x4eaf49c (instance for AISpawned)
    template <class T> Collections::QList<T> ResolveList(Collections::QListPtr<T> ptr) {
        return Collections::QList<T>(_trackedHeap.get(),
                                     _trackedHeap->Void<Collections::QListInternal>(ptr.Ptr));
    }
    // 0x4ea3a9c (instance for AssetRefConsideration)
    template <class T> Collections::QList<T> AllocateList(int capacity = DefaultInitialCollectionCapacity) {
        return Collections::QList<T>::Allocate(_trackedHeap.get(), capacity);
    }
    // 0x4ea3d88 (instance for AISpawned) - Also stores the new list's Ptr in `ptr`.
    template <class T>
    Collections::QList<T> AllocateList(Collections::QListPtr<T>& ptr,
                                       int capacity = DefaultInitialCollectionCapacity) {
        Collections::QList<T> list = AllocateList<T>(capacity);
        ptr = list;
        return list;
    }
    // 0x4ea66c4 (instance for AISpawned)
    template <class T> void FreeList(Collections::QListPtr<T> ptr) { ResolveList(ptr).Free(); }
    // 0x4eff918 (instance for AssetRefCallbackAction)
    template <class T>
    bool TryResolveList(Collections::QListPtr<T> ptr, Collections::QList<T>& list) {
        if (ptr.Ptr == Ptr::Null()) {
            list = Collections::QList<T>{};
            return false;
        }
        list = ResolveList(ptr);
        return true;
    }
    // 0x4eec80c (instance for AssetRefEntityPrototype)
    template <class T> bool TryFreeList(Collections::QListPtr<T> ptr) {
        if (ptr.Ptr == Ptr::Null()) {
            return false;
        }
        FreeList(ptr);
        return true;
    }

    // 0x4eaeeec (instance for <AssetRefConsideration, FP>)
    template <class K, class V>
    Collections::QDictionary<K, V> ResolveDictionary(Collections::QDictionaryPtr<K, V> ptr) {
        return Collections::QDictionary<K, V>(
            _trackedHeap.get(), _trackedHeap->Void<Collections::QDictionaryInternal>(ptr.Ptr));
    }
    // 0x4ea3598 (instance for <AssetRefConsideration, FP>)
    template <class K, class V>
    Collections::QDictionary<K, V> AllocateDictionary(int capacity = DefaultInitialCollectionCapacity) {
        return Collections::QDictionary<K, V>::Allocate(_trackedHeap.get(), capacity);
    }
    // 0x4ea3684 (instance for <AssetRefConsideration, FP>)
    template <class K, class V>
    Collections::QDictionary<K, V> AllocateDictionary(Collections::QDictionaryPtr<K, V>& ptr,
                                                      int capacity = DefaultInitialCollectionCapacity) {
        Collections::QDictionary<K, V> dictionary = AllocateDictionary<K, V>(capacity);
        ptr = dictionary;
        return dictionary;
    }
    // 0x4eff6ec (instance for <Int32Enum, int>)
    template <class K, class V>
    bool TryResolveDictionary(Collections::QDictionaryPtr<K, V> ptr,
                              Collections::QDictionary<K, V>& dictionary) {
        if (ptr.Ptr == Ptr::Null()) {
            dictionary = Collections::QDictionary<K, V>{};
            return false;
        }
        dictionary = ResolveDictionary(ptr);
        return true;
    }
    // 0x4eaf294 (instance for <ByteEnum, CollectData>) - Enum-keyed form (the original
    // selects it with a Tag argument).
    template <class K, class V>
    Collections::QEnumDictionary<K, V> ResolveEnumDictionary(Collections::QDictionaryPtr<K, V> ptr) {
        return Collections::QEnumDictionary<K, V>(
            _trackedHeap.get(), _trackedHeap->Void<Collections::QDictionaryInternal>(ptr.Ptr));
    }
    // 0x4ea38f0 (instance for <ByteEnum, CollectData>)
    template <class K, class V>
    Collections::QEnumDictionary<K, V>
    AllocateEnumDictionary(Collections::QDictionaryPtr<K, V>& ptr,
                           int capacity = DefaultInitialCollectionCapacity) {
        Collections::QEnumDictionary<K, V> dictionary =
            Collections::QEnumDictionary<K, V>::Allocate(_trackedHeap.get(), capacity);
        ptr = dictionary;
        return dictionary;
    }
    // 0x4eff5d4 (instance for <Int32Enum, int>)
    template <class K, class V>
    bool TryResolveEnumDictionary(Collections::QDictionaryPtr<K, V> ptr,
                                  Collections::QEnumDictionary<K, V>& dictionary) {
        if (ptr.Ptr == Ptr::Null()) {
            dictionary = Collections::QEnumDictionary<K, V>{};
            return false;
        }
        dictionary = ResolveEnumDictionary(ptr);
        return true;
    }
    // 0x4ea6148 (instance for <ByteEnum, CollectData>) - Releases the dictionary record
    // and its storage.
    template <class K, class V> void FreeDictionary(Collections::QDictionaryPtr<K, V> ptr) {
        Collections::QDictionaryInternal::Free(
            *_trackedHeap, _trackedHeap->Void<Collections::QDictionaryInternal>(ptr.Ptr));
    }

    // --- assets, prototypes, navigation ---------------------------------------------------
    // TODO(0x4ea5dd0, Quantum.Core.FrameBase.FindAsset<T>) and the other FindAsset /
    // TryFindAsset / FindAssetInternal / TryFindAssetInternal overloads: need AssetDB and
    // DynamicAssetDB.
    // TODO(0x7d1d218, Quantum.Core.FrameBase.FindPrototype(EntityPrototypeRef)): needs
    // entity prototypes.
    // TODO(0x7d1d594, Quantum.Core.FrameBase.FindPrototype(ComponentPrototypeRef)): same.
    // TODO(0x7d1d60c, Quantum.Core.FrameBase.ClearAllNavMeshRegions): needs navigation.
    // TODO(0x7d1d650, Quantum.Core.FrameBase.LogAssetNotFound): logging for FindAsset.
    // TODO(0x7d1d774, Quantum.Core.FrameBase.LogAssetTypeMismatch): logging for FindAsset.
    // TODO(0x7d1cc28, Quantum.Core.FrameBase.get__physicsStateInternal2D) and
    // TODO(0x7d1cc38, Quantum.Core.FrameBase.get__physicsStateInternal3D): need the physics
    // engine state.
    // TODO(0x7d1cc48, Quantum.Core.FrameBase.get_Navigation): needs Navigation.

    // --- access --------------------------------------------------------------------------

    // 0x7d1cc60
    FrameContext* get_Context() const { return _context; }
    // 0x7d1cc68
    Quantum::Allocator::FrameHeap* get_TrackedHeap() const { return _trackedHeap.get(); }
    Quantum::Allocator::Heap* HeapPtr() const { return _heap; }
    NativeAllocator* NativeAlloc() const { return _context->get_Allocator(); }

    int UsedCount() const { return _usedCount; }
    int FreeCount() const { return _freeCount; }
    int Capacity() const { return _capacity; }
    // Live entities (the reserved slot 0 not counted; GetMemoryStats counts it, as the
    // original does).
    int EntityCount() const { return _usedCount - _freeCount - 1; }
    const EntityInfo* EntityInfos() const { return _info; }
    const ComponentSet& ComponentMask(int entityIndex) const { return _componentMasks[entityIndex]; }
    int ComponentBuffersLength() const { return _componentBuffersLength; }
    // Marks an entity culled (the culling pass, Task_CullTransform2D/3D, does this).
    void SetCulled(EntityRef entityRef, bool culled);

    // Component signals by type index, installed by the game's frame (null = none).
    std::vector<ComponentReactiveCallbackInvoker> _ComponentSignalsOnAdded;
    std::vector<ComponentReactiveCallbackInvoker> _ComponentSignalsOnRemoved;

    FrameBaseUnsafe Unsafe;

  protected:
    // 0x7d1b444 - Signal first, then the type's OnRemoved callback.
    void ComponentRemoveSignals(EntityRef entity, int componentIndex);

  private:
    void FreeEntityStore();

    int _usedCount = 0;
    int _freeCount = 0;
    int _freeHead = 0;
    int _capacity = 0;
    int _capacityActual = 0;
    EntityInfo* _info = nullptr;
    std::uint64_t* _culled = nullptr;
    std::uint16_t* _sparse = nullptr;
    ComponentSet* _componentMasks = nullptr;
    ComponentDataBuffer* _componentBuffers = nullptr;
    int _componentBuffersLength = 0;
    FrameContext* _context = nullptr;
    Quantum::Allocator::Heap* _heap = nullptr;
    std::unique_ptr<Quantum::Allocator::FrameHeap> _trackedHeap;
    // Not present yet (their subsystems are missing): the dynamic asset database the
    // constructor creates, the prototype-materialized signal and the Physics2D/Physics3D
    // API objects.
};

// ---------------------------------------------------------------------------------------
// Template members

template <class T> AddResult FrameBase::Add(EntityRef entity, const T& value) {
    if (!Exists(entity)) {
        return AddResult::EntityDoesNotExist;
    }
    if (Has(entity, ComponentTypeId<T>::Id)) {
        return AddResult::ComponentAlreadyExists;
    }
    Set<T>(entity, value);
    return AddResult::ComponentAdded;
}

template <class T> AddResult FrameBase::Add(EntityRef entity, const T& value, T*& result) {
    result = nullptr;
    if (!Exists(entity)) {
        return AddResult::EntityDoesNotExist;
    }
    if (Has(entity, ComponentTypeId<T>::Id)) {
        return AddResult::ComponentAlreadyExists;
    }
    Set<T>(entity, value);
    result = static_cast<T*>(GetDataBuffer<T>()->GetDataPointer(entity));
    return AddResult::ComponentAdded;
}

template <class T> bool FrameBase::AddOrGet(EntityRef entity, T*& result) {
    if (!Exists(entity)) {
        result = nullptr;
        return false;
    }
    if (!Has(entity, ComponentTypeId<T>::Id)) {
        Set<T>(entity, T{});
    }
    result = static_cast<T*>(GetDataBuffer<T>()->GetDataPointer(entity));
    return true;
}

template <class T> SetResult FrameBase::Set(EntityRef entity, const T& value) {
    if (!Exists(entity)) {
        return SetResult::EntityDoesNotExist;
    }
    const int id = ComponentTypeId<T>::Id;
    ComponentDataBuffer* buffer = GetDataBuffer(id);
    buffer->Set<T>(NativeAlloc(), entity, value);
    ComponentSet& mask = _componentMasks[entity.Index];
    if (mask.IsSet(id)) {
        return SetResult::ComponentUpdated;
    }
    mask.Add(id);
    if (ComponentChangedDelegate onAdded = ComponentTypeId<>::Callbacks[static_cast<std::size_t>(id)].OnAdded) {
        onAdded(this, entity, buffer->GetDataPointer(entity));
    }
    if (const ComponentReactiveCallbackInvoker& signal =
            _ComponentSignalsOnAdded.at(static_cast<std::size_t>(id))) {
        signal(entity, buffer->GetDataPointer(entity));
    }
    return SetResult::ComponentAdded;
}

template <class T> T* FrameBase::GetPointer(EntityRef entityRef) {
    CheckExistsAndThrowIfNot(entityRef);
    return static_cast<T*>(GetDataBuffer<T>()->GetDataPointer(entityRef));
}

template <class T> bool FrameBase::TryGetPointer(EntityRef entityRef, T*& value) {
    if (Has(entityRef, ComponentTypeId<T>::Id)) {
        value = static_cast<T*>(GetDataBuffer<T>()->GetDataPointerFastUnsafe(entityRef));
        return true;
    }
    value = nullptr;
    return false;
}

template <class T> T* FrameBase::GetPointerSingleton() {
    const EntityRef singleton = GetDataBuffer<T>()->get_SingletonRef();
    if (!singleton.IsValid()) {
        throw std::logic_error(ComponentTypeNameOf<T>() + " singleton not found");
    }
    return GetPointer<T>(singleton);
}

template <class T> bool FrameBase::TryGetPointerSingleton(T*& component) {
    const EntityRef singleton = GetDataBuffer<T>()->get_SingletonRef();
    if (singleton.IsValid()) {
        component = GetPointer<T>(singleton);
        return true;
    }
    component = nullptr;
    return false;
}

template <class T> T* FrameBase::GetOrAddSingletonPointer(EntityRef optionalAddTarget) {
    EntityRef singleton = GetDataBuffer<T>()->get_SingletonRef();
    if (!singleton.IsValid()) {
        singleton = optionalAddTarget == EntityRef::None() ? Create() : optionalAddTarget;
        Add<T>(singleton);
    }
    return GetPointer<T>(singleton);
}

} // namespace Core
} // namespace Quantum

#include "quantum_core/component_iterators.h"
