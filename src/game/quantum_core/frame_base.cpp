#include "frame_base.h"

#include "photon_deterministic/log.h"
#include "quantum_core/component_reference.h"
#include "quantum_core/frame_serializer.h"

#include <algorithm>
#include <bit>
#include <cstring>
#include <stdexcept>

namespace Quantum::Core {

using Collections::QHashCollection;
using Quantum::Allocator::DisabledHeapAllocTracker;
using Quantum::Allocator::FrameHeap;
using Quantum::Allocator::Heap;

namespace {

constexpr int EntityInfoSize = static_cast<int>(sizeof(EntityInfo));
constexpr int ComponentSetSize = static_cast<int>(sizeof(ComponentSet));
constexpr int BufferSize = static_cast<int>(sizeof(ComponentDataBuffer));

// Runs `body` for every index set in `mask`, lowest first.
template <class F> void ForEachSetIndex(const ComponentSet& mask, F&& body) {
    for (int block = 0; block < ComponentSet::BLOCK_COUNT; ++block) {
        std::uint64_t bits = mask._set[block];
        while (bits != 0) {
            const int bit = std::countr_zero(bits);
            body(block * ComponentSet::BITS_PER_BLOCK + bit);
            bits &= bits - 1;
        }
    }
}

// Clears a deferred set even when processing it throws (the original's finally block).
struct ClearOnExit {
    FrameHeap& heap;
    QHashCollection* collection;
    ~ClearOnExit() { QHashCollection::Clear(heap, collection); }
};

} // namespace

FrameBase::FrameBase(FrameContext* context) : Unsafe(this), _context(context) {
    _heap = context->ReserveHeap();
    InitTrackedHeap(std::make_shared<DisabledHeapAllocTracker>());
}

FrameBase::~FrameBase() = default;

void FrameBase::InitTrackedHeap(std::shared_ptr<Quantum::Allocator::IHeapAllocationTracker> tracker) {
    _trackedHeap = std::make_unique<FrameHeap>(_heap, std::move(tracker));
}

void FrameBase::Initialize(FrameBase* f, const EntitiesConfig& config) {
    f->_freeHead = 0;
    f->_usedCount = 1;
    f->_freeCount = 0;
    const int capacity = std::max(32, config.DefaultEntityCapacity);
    f->_capacity = capacity;
    f->_capacityActual = capacity;

    NativeAllocator* allocator = f->NativeAlloc();
    f->_componentMasks =
        static_cast<ComponentSet*>(allocator->AllocAndClear(capacity * ComponentSetSize));

    FrameMetaData* meta = f->get_FrameMetaData();
    meta->_entityDestroyList.Allocate<EntityRef>(*f->_trackedHeap, capacity >> 2, 4);
    meta->_componentRemovalList.Allocate<ComponentReference>(*f->_trackedHeap, capacity >> 2, 4);

    f->_culled =
        static_cast<std::uint64_t*>(allocator->AllocAndClear(RoundToCulledByteSize(capacity)));
    f->_info = static_cast<EntityInfo*>(allocator->AllocAndClear(capacity * EntityInfoSize));
    // Slot 0 never holds an entity: its version can never match a live handle.
    f->_info[0].Ref.Version = EntityInfo::VERSION_MASK;

    const int typeCount = ComponentTypeId<>::Count();
    f->_componentBuffersLength = typeCount;
    f->_componentBuffers =
        static_cast<ComponentDataBuffer*>(allocator->AllocAndClear(typeCount * BufferSize));
    for (int i = COMPONENT_START_INDEX; i < typeCount; ++i) {
        f->_componentBuffers[i] =
            ComponentDataBuffer::Create(allocator, config, i, ComponentTypeId<>::Size[i],
                                        ComponentTypeId<>::Flags[i]);
    }

    f->_sparse = static_cast<std::uint16_t*>(allocator->AllocAndClear(typeCount * capacity * 2));
    for (int i = COMPONENT_START_INDEX; i < typeCount; ++i) {
        f->_componentBuffers[i]._sparseCapacity = capacity;
        f->_componentBuffers[i]._sparse = f->_sparse + static_cast<std::ptrdiff_t>(i) * capacity;
    }

    // The game's frame installs the signals; make room for one per type (the original's
    // arrays are created by the game with that length).
    if (static_cast<int>(f->_ComponentSignalsOnAdded.size()) < typeCount) {
        f->_ComponentSignalsOnAdded.resize(static_cast<std::size_t>(typeCount));
    }
    if (static_cast<int>(f->_ComponentSignalsOnRemoved.size()) < typeCount) {
        f->_ComponentSignalsOnRemoved.resize(static_cast<std::size_t>(typeCount));
    }
}

void FrameBase::Free() {
    _context->ReleaseHeap(_heap);
    _trackedHeap.reset();
    FreeEntityStore();
    Unsafe.Dispose();
}

void FrameBase::FreeEntityStore() {
    NativeAllocator* allocator = NativeAlloc();
    allocator->Free(_info);
    allocator->Free(_culled);
    allocator->Free(_sparse);
    allocator->Free(_componentMasks);
    for (int i = COMPONENT_START_INDEX; i < _componentBuffersLength; ++i) {
        ComponentDataBuffer::Free(allocator, &_componentBuffers[i]);
    }
    allocator->Free(_componentBuffers);
    _info = nullptr;
    _culled = nullptr;
    _sparse = nullptr;
    _componentMasks = nullptr;
    _componentBuffers = nullptr;
}

void FrameBase::Copy(FrameBase* to, const FrameBase* from) {
    // (The original brackets the two halves with HostProfiler samples.)
    if (to->_capacity != from->_capacity) {
        to->ChangeCapacity(from->_capacity);
    }
    const auto capacity = static_cast<std::size_t>(from->_capacity);
    std::memcpy(to->_info, from->_info, capacity * sizeof(EntityInfo));
    std::memcpy(to->_componentMasks, from->_componentMasks, capacity * sizeof(ComponentSet));
    std::memcpy(to->_sparse, from->_sparse,
                static_cast<std::size_t>(from->_componentBuffersLength) * capacity * 2);

    for (int i = COMPONENT_START_INDEX; i < to->_componentBuffersLength; ++i) {
        ComponentDataBuffer::Copy(to->NativeAlloc(), &to->_componentBuffers[i],
                                  &from->_componentBuffers[i]);
    }
    to->_usedCount = from->_usedCount;
    to->_freeCount = from->_freeCount;
    to->_freeHead = from->_freeHead;
}

void FrameBase::Serialize(FrameBase* r, FrameSerializer* serializer) {
    if (serializer->Reading()) {
        // A fresh heap; its contents come from the stream through pointer resolution.
        r->_context->ReleaseHeap(r->_heap);
        r->_heap = r->_context->ReserveHeap();
        r->_trackedHeap =
            std::make_unique<FrameHeap>(r->_heap, r->_trackedHeap->SharedTracker());
    }

    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    // When the context has physics engines, the 2D state and the 3D static mesh + state
    // are serialised here (see the TODO in frame_base.h). Without them the presence flags
    // are false.
    if (stream.Condition(false)) {
        throw std::logic_error("FrameBase.Serialize: 2D physics state is not supported yet");
    }
    if (stream.Condition(false)) {
        throw std::logic_error("FrameBase.Serialize: 3D physics state is not supported yet");
    }

    stream.Serialize(r->_usedCount);
    stream.Serialize(r->_freeCount);
    stream.Serialize(r->_freeHead);
    stream.Serialize(r->_capacity);
    if (serializer->Reading()) {
        r->_capacityActual = r->_capacity;
    }

    NativeAllocator* allocator = r->NativeAlloc();
    const int capacity = r->_capacity;

    if (serializer->Reading()) {
        if (r->_info != nullptr) {
            allocator->Free(r->_info);
        }
        r->_info = static_cast<EntityInfo*>(allocator->AllocAndClear(capacity * EntityInfoSize));
    }
    for (int i = 0; i < capacity; ++i) {
        EntityInfo::Serialize(&r->_info[i], serializer);
    }

    if (serializer->Reading()) {
        if (r->_componentMasks != nullptr) {
            allocator->Free(r->_componentMasks);
        }
        r->_componentMasks =
            static_cast<ComponentSet*>(allocator->AllocAndClear(capacity * ComponentSetSize));
    }
    for (int i = 0; i < capacity; ++i) {
        ComponentSet::Serialize(&r->_componentMasks[i], serializer);
    }

    if (serializer->Reading()) {
        if (r->_sparse != nullptr) {
            allocator->Free(r->_sparse);
        }
        r->_sparse = static_cast<std::uint16_t*>(
            allocator->AllocAndClear(r->_componentBuffersLength * capacity * 2));
    }
    const int sparseCount = r->_componentBuffersLength * capacity;
    for (int i = 0; i < sparseCount; ++i) {
        stream.Serialize(&r->_sparse[i]);
    }

    if (serializer->Reading()) {
        if (r->_culled != nullptr) {
            allocator->Free(r->_culled);
        }
        // Not cleared: every word is read below.
        r->_culled = static_cast<std::uint64_t*>(allocator->Alloc(RoundToCulledByteSize(capacity)));
    }
    const int culledWords = RoundToCulledWordCount(capacity);
    for (int i = 0; i < culledWords; ++i) {
        stream.Serialize(reinterpret_cast<std::int64_t*>(&r->_culled[i]));
    }

    for (int i = COMPONENT_START_INDEX; i < r->_componentBuffersLength; ++i) {
        ComponentDataBuffer& buffer = r->_componentBuffers[i];
        stream.Serialize(&buffer._count);
        stream.Serialize(&buffer._pendingRemoval);
        stream.Serialize(&buffer._version);
        stream.Serialize(&buffer._blockCapacity);
        stream.Serialize(&buffer._flags);
        stream.Serialize(&buffer._singletonSparse);

        if (serializer->Reading()) {
            buffer._componentTypeID = i;
            buffer._stride = ComponentTypeId<>::GetComponentSize(i);
            buffer._sparseCapacity = capacity;
            buffer._sparse = r->_sparse + static_cast<std::ptrdiff_t>(i) * capacity;
            buffer._blockDataOffset = Photon::Deterministic::Native::RoundUpToAlignment(
                buffer._blockCapacity * EntityRef::SIZE, 8);
            buffer._blockByteSize = buffer._blockCapacity * buffer._stride + buffer._blockDataOffset;
            if (buffer._blocksList != nullptr) {
                for (int b = 0; b < buffer._blocksListCapacity; ++b) {
                    if (buffer._blocksList[b].PackedHandles != nullptr) {
                        allocator->Free(buffer._blocksList[b].PackedHandles);
                    }
                    buffer._blocksList[b] = ComponentDataBuffer::Block{};
                }
                allocator->Free(buffer._blocksList);
                buffer._blocksList = nullptr;
            }
        }

        stream.Serialize(&buffer._blocksListCount);
        stream.Serialize(&buffer._blocksListCapacity);

        if (serializer->Reading()) {
            buffer._blocksList = static_cast<ComponentDataBuffer::Block*>(allocator->AllocAndClear(
                buffer._blocksListCapacity * static_cast<int>(sizeof(ComponentDataBuffer::Block))));
            for (int b = 0; b < buffer._blocksListCapacity; ++b) {
                if (stream.ReadBool()) {
                    buffer._blocksList[b] = buffer.AllocateBlock(allocator);
                }
            }
        } else {
            for (int b = 0; b < buffer._blocksListCapacity; ++b) {
                stream.WriteBool(buffer._blocksList[b].PackedHandles != nullptr);
            }
        }

        const ComponentSerializeDelegate serialize = ComponentTypeId<>::Callbacks[i].Serialize;
        for (int slot = 1; slot < buffer._count; ++slot) {
            const ComponentDataBuffer::Block& block = buffer._blocksList[slot / buffer._blockCapacity];
            const int offset = slot % buffer._blockCapacity;
            EntityRef::Serialize(&block.PackedHandles[offset], serializer);
            serialize(block.PackedData + static_cast<std::ptrdiff_t>(buffer._stride) * offset,
                      serializer);
        }
    }
}

void FrameBase::OnFrameSimulateBegin() {
    get_FrameMetaData()->FreeAllTrackedInternal(*_trackedHeap);
    if (get_CommitCommandsMode() == CommitCommandsModes::OnFrameSimulationBegin) {
        CommitRemoves();
        CommitDestroys();
    }
    ClearCulledState();
}

void FrameBase::OnFrameSimulateEnd() {
    if (get_CommitCommandsMode() == CommitCommandsModes::OnFrameSimulationEnd) {
        CommitRemoves();
        CommitDestroys();
    }
}

FrameBase::Stats FrameBase::GetMemoryStats() const {
    int memory = 0;
    for (int i = COMPONENT_START_INDEX; i < _componentBuffersLength; ++i) {
        const ComponentDataBuffer& buffer = _componentBuffers[i];
        memory += buffer._blocksListCount * buffer._blockCapacity * buffer._stride;
    }
    // Per entity: 16 bytes of entity info and 2 of sparse entry, plus the culling bits.
    memory += _capacity * 18 + RoundToCulledByteSize(_capacity);

    Stats stats;
    stats.HeapStats = Heap::GetStats(_heap);
    stats.EntityCount = _usedCount - _freeCount;
    stats.EntityTotalMemory = memory;
    return stats;
}

void FrameBase::ChangeCapacity(int newCapacity) {
    const int oldCapacity = _capacity;
    NativeAllocator* allocator = NativeAlloc();
    const int typeCount = _componentBuffersLength;

    if (oldCapacity < newCapacity) {
        if (_capacityActual >= newCapacity) {
            // Grow back into memory kept from an earlier shrink: only the sparse slices
            // move apart again, via a temporary copy.
            auto* old = static_cast<std::uint16_t*>(
                _context->TempAllocate(typeCount * oldCapacity * 2, 8));
            std::memcpy(old, _sparse, static_cast<std::size_t>(typeCount) * oldCapacity * 2);
            _capacity = newCapacity;
            for (int i = COMPONENT_START_INDEX; i < typeCount; ++i) {
                _componentBuffers[i].ChangeEntityCapacity(
                    old + static_cast<std::ptrdiff_t>(i) * oldCapacity, oldCapacity,
                    _sparse + static_cast<std::ptrdiff_t>(i) * newCapacity, newCapacity);
            }
            return;
        }

        auto* culled =
            static_cast<std::uint64_t*>(allocator->AllocAndClear(RoundToCulledByteSize(newCapacity)));
        std::memcpy(culled, _culled, static_cast<std::size_t>(RoundToCulledByteSize(oldCapacity)));
        _info = static_cast<EntityInfo*>(
            allocator->Expand(_info, oldCapacity * EntityInfoSize, newCapacity * EntityInfoSize));
        _componentMasks = static_cast<ComponentSet*>(allocator->Expand(
            _componentMasks, oldCapacity * ComponentSetSize, newCapacity * ComponentSetSize));
        _capacity = newCapacity;
        _capacityActual = newCapacity;

        auto* sparse =
            static_cast<std::uint16_t*>(allocator->AllocAndClear(newCapacity * typeCount * 2));
        for (int i = COMPONENT_START_INDEX; i < typeCount; ++i) {
            _componentBuffers[i].ChangeEntityCapacity(
                _sparse + static_cast<std::ptrdiff_t>(i) * oldCapacity, oldCapacity,
                sparse + static_cast<std::ptrdiff_t>(i) * newCapacity, newCapacity);
        }
        allocator->Free(_culled);
        _culled = culled;
        allocator->Free(_sparse);
        _sparse = sparse;
        return;
    }

    if (oldCapacity > newCapacity) {
        // Shrink in place: compact the sparse slices (each copy may spill into the next
        // slice, which that slice's own copy then overwrites) and clear the dropped tail.
        _capacity = newCapacity;
        for (int i = COMPONENT_START_INDEX; i < typeCount; ++i) {
            _componentBuffers[i].ChangeEntityCapacity(
                _sparse + static_cast<std::ptrdiff_t>(i) * oldCapacity, oldCapacity,
                _sparse + static_cast<std::ptrdiff_t>(i) * newCapacity, newCapacity);
        }
        const int dropped = oldCapacity - newCapacity;
        std::memset(static_cast<void*>(_info + newCapacity), 0, static_cast<std::size_t>(dropped) * sizeof(EntityInfo));
        const int keptWords = RoundToCulledWordCount(newCapacity);
        const int droppedWords = RoundToCulledWordCount(oldCapacity) - keptWords;
        std::memset(_culled + keptWords, 0, static_cast<std::size_t>(droppedWords) * 8);
        std::memset(_sparse + static_cast<std::ptrdiff_t>(newCapacity) * typeCount, 0,
                    static_cast<std::size_t>(dropped) * typeCount * 2);
        std::memset(static_cast<void*>(_componentMasks + newCapacity), 0,
                    static_cast<std::size_t>(dropped) * sizeof(ComponentSet));
    }
}

EntityRef FrameBase::Create() {
    int index;
    if (_freeHead != 0) {
        index = _freeHead;
        --_freeCount;
        _freeHead = _info[index].Ref.Index;
    } else {
        if (_usedCount == _capacity) {
            ChangeCapacity(_usedCount * 2);
        }
        index = _usedCount++;
    }
    EntityInfo& info = _info[index];
    info.Ref.Index = index;
    info.Ref.Version = (info.Ref.Version + 1) | EntityInfo::ACTIVE_BIT;
    return info.Ref;
}

bool FrameBase::Destroy(EntityRef entityRef) {
    if (!Exists(entityRef)) {
        return false;
    }
    EntityInfo& info = _info[entityRef.Index];
    info.Flags = info.Flags | EntityFlags::DestroyPending;

    const ComponentSet components = _componentMasks[entityRef.Index];
    ForEachSetIndex(components, [&](int index) {
        _componentBuffers[index].RemovalPending(entityRef);
        ComponentRemoveSignals(entityRef, index);
    });
    if (!components.Equals(_componentMasks[entityRef.Index])) {
        Photon::Deterministic::Log::Error(
            "Components were added to or removed from {0} by an OnRemoved callback or signal "
            "while it was being destroyed",
            entityRef.ToString());
    }

    FrameMetaData* meta = get_FrameMetaData();
    QHashCollection* set = meta->_entityDestroyList.GetCurrentQHashCollection<EntityRef>(*_trackedHeap);
    QHashCollection::Add(*_trackedHeap, set, info.Ref, Collections::GetKeyHash(info.Ref));
    info.Ref.Version = (info.Ref.Version + 1) & EntityInfo::VERSION_MASK;
    return true;
}

void FrameBase::CheckExistsAndThrowIfNot(EntityRef entityRef) const {
    if (!Exists(entityRef)) {
        throw std::logic_error("FrameBase: " + entityRef.ToString() + " does not exist");
    }
}

ComponentSet FrameBase::GetComponentSet(EntityRef entityRef) const {
    CheckExistsAndThrowIfNot(entityRef);
    return _componentMasks[entityRef.Index];
}

bool FrameBase::Has(EntityRef entityRef, const ComponentSet& set) const {
    CheckExistsAndThrowIfNot(entityRef);
    return _componentMasks[entityRef.Index].IsSupersetOf(set);
}

void FrameBase::ClearCulledState() {
    std::memset(_culled, 0, static_cast<std::size_t>(RoundToCulledByteSize(_capacity)));
}

bool FrameBase::IsCulled(EntityRef entityRef) const {
    if (!Exists(entityRef)) {
        return true;
    }
    return ((_culled[entityRef.Index / 64] >> (entityRef.Index & 63)) & 1) != 0;
}

void FrameBase::SetCullable(EntityRef entityRef, bool cullable) {
    if (!Exists(entityRef)) {
        return;
    }
    EntityInfo& info = _info[entityRef.Index];
    if (cullable) {
        info.Flags = info.Flags & ~EntityFlags::NotCullable;
        return;
    }
    info.Flags = info.Flags | EntityFlags::NotCullable;
    _culled[entityRef.Index / 64] &= ~(std::uint64_t{1} << (entityRef.Index & 63));
}

void FrameBase::SetCulled(EntityRef entityRef, bool culled) {
    const std::uint64_t bit = std::uint64_t{1} << (entityRef.Index & 63);
    if (culled) {
        _culled[entityRef.Index / 64] |= bit;
    } else {
        _culled[entityRef.Index / 64] &= ~bit;
    }
}

bool FrameBase::RemoveComponentIndexUnchecked(EntityRef entityRef, int componentIndex) {
    if (!Has(entityRef, componentIndex)) {
        return false;
    }
    _componentBuffers[componentIndex].RemovalPending(entityRef);
    ComponentRemoveSignals(entityRef, componentIndex);
    _componentMasks[entityRef.Index].Remove(componentIndex);

    const ComponentReference reference(entityRef, componentIndex);
    FrameMetaData* meta = get_FrameMetaData();
    QHashCollection* set =
        meta->_componentRemovalList.GetCurrentQHashCollection<ComponentReference>(*_trackedHeap);
    QHashCollection::Add(*_trackedHeap, set, reference, Collections::GetKeyHash(reference));
    return true;
}

void FrameBase::ComponentRemoveSignals(EntityRef entity, int componentIndex) {
    ComponentDataBuffer& buffer = _componentBuffers[componentIndex];
    if (const ComponentReactiveCallbackInvoker& signal =
            _ComponentSignalsOnRemoved.at(static_cast<std::size_t>(componentIndex))) {
        signal(entity, buffer.GetDataPointer(entity));
    }
    if (ComponentChangedDelegate onRemoved =
            ComponentTypeId<>::Callbacks.at(static_cast<std::size_t>(componentIndex)).OnRemoved) {
        onRemoved(this, entity, buffer.GetDataPointer(entity));
    }
}

void FrameBase::CommitAllCommands() {
    CommitRemoves();
    CommitDestroys();
}

void FrameBase::CommitRemoves() {
    FrameMetaData::QHashCollectionList& list = get_FrameMetaData()->_componentRemovalList;
    QHashCollection* sets = nullptr;
    int setCount = 0;
    list.GetQHashCollections(*_trackedHeap, sets, setCount);

    for (int s = 0; s < setCount; ++s) {
        QHashCollection* set = &sets[s];
        if (set->UsedCount == 0) {
            break;
        }
        ClearOnExit clear{*_trackedHeap, set};
        for (int i = 0; i < set->UsedCount; ++i) {
            auto* entries = static_cast<std::uint8_t*>(_trackedHeap->Void(set->Entries.Ptr));
            const ComponentReference reference = *reinterpret_cast<const ComponentReference*>(
                entries + static_cast<std::ptrdiff_t>(set->Entries.Stride) * i + set->KeyOffset);
            const EntityRef entity = reference.Entity;
            if (static_cast<std::uint32_t>(entity.Index) >= static_cast<std::uint32_t>(_capacity)) {
                continue;
            }
            // Still the same entity: alive with the same version, or destroyed since (the
            // destruction bumped the version once and cleared the live bit).
            const int storedVersion = _info[entity.Index].Ref.Version;
            const int expectedVersion =
                storedVersion < 0 ? entity.Version : ((entity.Version + 1) & EntityInfo::VERSION_MASK);
            if (storedVersion != expectedVersion) {
                continue;
            }
            // A component added back after its removal keeps its data.
            if (!_componentMasks[entity.Index].IsSet(reference.Component)) {
                _componentBuffers[reference.Component].Remove(entity);
            }
        }
    }
    list.Reset(*_trackedHeap);
}

void FrameBase::CommitDestroys() {
    FrameMetaData::QHashCollectionList& list = get_FrameMetaData()->_entityDestroyList;
    QHashCollection* sets = nullptr;
    int setCount = 0;
    list.GetQHashCollections(*_trackedHeap, sets, setCount);

    for (int s = 0; s < setCount; ++s) {
        QHashCollection* set = &sets[s];
        if (set->UsedCount == 0) {
            break;
        }
        ClearOnExit clear{*_trackedHeap, set};
        for (int i = 0; i < set->UsedCount; ++i) {
            auto* entries = static_cast<std::uint8_t*>(_trackedHeap->Void(set->Entries.Ptr));
            const EntityRef entity = *reinterpret_cast<const EntityRef*>(
                entries + static_cast<std::ptrdiff_t>(set->Entries.Stride) * i + set->KeyOffset);
            const ComponentSet components = _componentMasks[entity.Index];
            ForEachSetIndex(components,
                            [&](int index) { _componentBuffers[index].Remove(entity); });
            _componentMasks[entity.Index] = ComponentSet{};

            EntityInfo& info = _info[entity.Index];
            info.Flags = EntityFlags::None;
            info.Ref.Index = _freeHead;
            ++_freeCount;
            _freeHead = entity.Index;
        }
    }
    list.Reset(*_trackedHeap);
}

} // namespace Quantum::Core
