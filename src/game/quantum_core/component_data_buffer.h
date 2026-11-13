// component_data_buffer.h - Quantum.ComponentDataBuffer and Quantum.ComponentBlockIterator.
//
// One ComponentDataBuffer per component type holds the data of every instance of that
// type in the frame, densely packed:
//
//  * Instances live in fixed-size blocks of `_blockCapacity` slots. A block is a single
//    allocation: the entity handles of its slots first, then (at `_blockDataOffset`) the
//    component data, `_stride` bytes per slot.
//  * Slots are numbered across blocks (slot n is in block n / capacity at n % capacity).
//    Slot 0 is never used, so `_count` (one past the last used slot) starts at 1.
//  * `_sparse` maps an entity index to its slot (0 = the entity has no such component).
//    The array belongs to the frame, which hands each buffer its slice.
//  * Removing an instance moves the last instance into its slot, keeping the slots dense;
//    removals requested during a frame are only counted (`_pendingRemoval`) and applied
//    when the frame commits them.
//  * `_version` changes whenever slots move, which invalidates running iterators.
//
// Singleton components additionally remember the slot of their only instance.
#pragma once

#include "photon_deterministic/native.h"
#include "quantum_core/component_type_id.h"
#include "quantum_core/entities_config.h"
#include "quantum_core/entity_ref.h"

#include <cstdint>
#include <typeinfo>

namespace Quantum {

struct ComponentDataBuffer {
    using Allocator = Photon::Deterministic::Native::Allocator;

    // Initial length of the block list.
    static constexpr int DEFAULT_BLOCK_CAPACITY = 4;

    // One block: the slot handles, and where the component data starts.
    struct Block {
        EntityRef* PackedHandles = nullptr;
        std::uint8_t* PackedData = nullptr;
    };

    std::uint16_t _flags = 0; // ComponentFlags
    std::uint16_t _singletonSparse = 0;
    int _count = 0;
    int _pendingRemoval = 0;
    int _stride = 0;
    int _version = 0;
    int _componentTypeID = 0;
    Block* _blocksList = nullptr;
    int _blocksListCapacity = 0;
    int _blocksListCount = 0;
    int _blockCapacity = 0;
    int _blockByteSize = 0;
    int _blockDataOffset = 0;
    std::uint16_t* _sparse = nullptr;
    int _sparseCapacity = 0;

    // 0x7ceab70 - Instances, including those pending removal.
    int get_Count() const { return _count - 1; }
    // 0x7cf0fac - Instances not pending removal.
    int get_UsedCount() const { return _count - _pendingRemoval - 1; }
    // 0x7cf0fbc
    int get_Version() const { return _version; }
    // 0x7cf0fc4
    int get_BlockListCount() const { return _blocksListCount; }
    // 0x7cf0fcc - The entity holding the singleton instance (None if there is none).
    // Throws std::logic_error for a non-singleton type.
    EntityRef get_SingletonRef() const;
    // 0x7cf1084
    bool get_IsSingleton() const { return (_flags & 1) != 0; }

    // 0x7cf1090 - An empty buffer for the registered type `typeIndex` (the original takes
    // the System.Type and looks the index up): takes the block capacity for the type from
    // `config`, computes the block layout and allocates the initial block list (no blocks
    // yet). Throws std::overflow_error if `flags` does not fit 16 bits.
    static ComponentDataBuffer Create(Allocator* allocator, const Core::EntitiesConfig& config,
                                      int typeIndex, int dataStride, ComponentFlags flags);
    // 0x7cf11a4 - Releases every block and the block list.
    static void Free(Allocator* allocator, ComponentDataBuffer* buffer);
    // 0x7cf1234 - Makes `to` hold the same instances as `from`: blocks are copied byte for
    // byte (allocating missing ones), then the counters. The sparse array is the frame's
    // business and is not touched.
    static void Copy(Allocator* allocator, ComponentDataBuffer* to,
                     const ComponentDataBuffer* from);

    // 0x7cf13f0 - Moves the sparse slice to `newBuffer` (copying `oldCapacity` entries;
    // the regions may overlap) and adopts the new capacity.
    void ChangeEntityCapacity(const std::uint16_t* oldBuffer, int oldCapacity,
                              std::uint16_t* newBuffer, int newCapacity);

    // 0x7cf1484, 0x7cf14a0
    std::uint8_t* GetBlockData(int block) const { return _blocksList[block].PackedData; }
    EntityRef* GetBlockEntityRefs(int block) const { return _blocksList[block].PackedHandles; }
    // 0x7cf14bc
    EntityRef GetEntityRefByIndex(int index) const {
        return _blocksList[index / _blockCapacity].PackedHandles[index % _blockCapacity];
    }
    // 0x7cf14e8
    void* GetDataPointerByIndex(int index) const {
        return _blocksList[index / _blockCapacity].PackedData +
               static_cast<std::ptrdiff_t>(_stride) * (index % _blockCapacity);
    }
    // 0x7cf151c - Throws std::logic_error if the entity has no instance.
    void* GetDataPointer(EntityRef entityRef) const;
    // 0x7cf15f8 - No check: the entity must have an instance.
    void* GetDataPointerFastUnsafe(EntityRef entityRef) const {
        return GetDataPointerByIndex(_sparse[entityRef.Index]);
    }
    // 0x4c7f94c (instance for Transform3D)
    template <class T> T GetData(EntityRef entityRef) const {
        return *static_cast<const T*>(GetDataPointer(entityRef));
    }
    // 0x4c81514 (instance for Transform3D) - Stores `value` in the entity's slot (making
    // one if needed) and returns it.
    template <class T> T* Set(Allocator* allocator, EntityRef entityRef, const T& value) {
        T* data = static_cast<T*>(GetBufferEntry(allocator, entityRef));
        *data = value;
        return data;
    }
    // 0x7cf163c - The entity's slot, appended at the end (growing the block list) if it
    // has none yet. A second singleton instance throws std::logic_error. New slots are not
    // cleared here; the data of freed slots is cleared when they are vacated.
    void* GetBufferEntry(Allocator* allocator, EntityRef entityRef);
    // 0x7cf1848 - Frees the entity's slot by moving the last instance into it. Throws if
    // the entity has no instance.
    void Remove(EntityRef entityRef);
    // 0x7cf1364
    void ExpandBlockListCapacity(Allocator* allocator, int newCapacity);
    // 0x7cf178c - Starts using the next block, reusing (and clearing) a block left from an
    // earlier, larger state or allocating a new one.
    void UseNextBlock(Allocator* allocator);
    // 0x7cf13b8 - One zeroed block.
    Block AllocateBlock(Allocator* allocator) const;
    // 0x7cf1a30 - Counts one more pending removal; a singleton forgets its instance.
    void RemovalPending(EntityRef entityRef);
};

static_assert(sizeof(ComponentDataBuffer) == 72);

// Walks a buffer's slots block by block. The C# non-generic ComponentBlockIterator and the
// generic ComponentBlockIterator<T> (component_iterators.h) share one name, so here the
// non-generic one is ComponentBlockIterator<> (= ComponentBlockIterator<void>).
template <class T = void> struct ComponentBlockIterator;

template <> struct ComponentBlockIterator<void> {
    int _version = 0;
    int _remaining = 0;
    int _currentBlock = 0;
    int _currentBlockOffset = 0;
    ComponentDataBuffer* _buffer = nullptr;

    ComponentBlockIterator() = default;
    // 0x7ceab4c - All slots, starting after the reserved slot 0.
    explicit ComponentBlockIterator(ComponentDataBuffer* buffer)
        : _version(buffer->_version), _remaining(buffer->get_Count()), _currentBlock(0),
          _currentBlockOffset(1), _buffer(buffer) {}

    // 0x7ceab7c - Restricts the walk to `count` instances starting with instance
    // `offset` (both clamped to what exists).
    void Reset(int offset, int count);
    // 0x7ceabd4 - Throws std::logic_error if instances moved since the iterator was made.
    void ValidateIterator() const;
    // 0x7ceacd0 - The next run of consecutive slots: their data, their handles and how
    // many. False (with null outputs) when nothing is left.
    bool NextBlock(std::uint8_t*& components, EntityRef*& entities, int& count);
};

} // namespace Quantum
