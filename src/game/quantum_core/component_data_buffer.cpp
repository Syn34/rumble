#include "component_data_buffer.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>

namespace Quantum {

namespace {

const std::string& TypeName(int typeIndex) {
    return ComponentTypeId<>::GetComponentName(typeIndex);
}

} // namespace

EntityRef ComponentDataBuffer::get_SingletonRef() const {
    if (!get_IsSingleton()) {
        throw std::logic_error("ComponentDataBuffer: " + TypeName(_componentTypeID) +
                               " is not a singleton component");
    }
    if (_singletonSparse == 0) {
        return EntityRef::None();
    }
    return GetEntityRefByIndex(_singletonSparse);
}

ComponentDataBuffer ComponentDataBuffer::Create(Allocator* allocator,
                                                const Core::EntitiesConfig& config,
                                                int typeIndex, int dataStride,
                                                ComponentFlags flags) {
    if (static_cast<int>(flags) >= 0x10000 || static_cast<int>(flags) < 0) {
        throw std::overflow_error("ComponentDataBuffer: component flags do not fit 16 bits");
    }
    const int blockCapacity = config.GetComponentBlockCapacity(TypeName(typeIndex));
    const int dataOffset =
        Photon::Deterministic::Native::RoundUpToAlignment(blockCapacity * EntityRef::SIZE, 8);

    ComponentDataBuffer buffer;
    buffer._flags = static_cast<std::uint16_t>(flags);
    buffer._singletonSparse = 0;
    buffer._count = 1;
    buffer._pendingRemoval = 0;
    buffer._stride = dataStride;
    buffer._version = 0;
    buffer._componentTypeID = typeIndex;
    buffer._blocksList = static_cast<Block*>(
        allocator->AllocAndClear(DEFAULT_BLOCK_CAPACITY * static_cast<int>(sizeof(Block))));
    buffer._blocksListCapacity = DEFAULT_BLOCK_CAPACITY;
    buffer._blocksListCount = 0;
    buffer._blockCapacity = blockCapacity;
    buffer._blockByteSize = blockCapacity * dataStride + dataOffset;
    buffer._blockDataOffset = dataOffset;
    buffer._sparse = nullptr;
    buffer._sparseCapacity = 0;
    return buffer;
}

void ComponentDataBuffer::Free(Allocator* allocator, ComponentDataBuffer* buffer) {
    for (int i = 0; i < buffer->_blocksListCapacity; ++i) {
        if (buffer->_blocksList[i].PackedHandles != nullptr) {
            allocator->Free(buffer->_blocksList[i].PackedHandles);
        }
    }
    allocator->Free(buffer->_blocksList);
}

void ComponentDataBuffer::Copy(Allocator* allocator, ComponentDataBuffer* to,
                               const ComponentDataBuffer* from) {
    if (to->_blocksListCapacity < from->_blocksListCapacity) {
        to->ExpandBlockListCapacity(allocator, from->_blocksListCapacity);
    }
    for (int i = 0; i < from->_blocksListCapacity; ++i) {
        const Block& source = from->_blocksList[i];
        if (source.PackedData == nullptr) {
            break;
        }
        Block& target = to->_blocksList[i];
        if (target.PackedData == nullptr) {
            target = to->AllocateBlock(allocator);
        }
        std::memcpy(target.PackedHandles, source.PackedHandles,
                    static_cast<std::size_t>(to->_blockByteSize));
    }
    to->_flags = from->_flags;
    to->_singletonSparse = from->_singletonSparse;
    to->_count = from->_count;
    to->_pendingRemoval = from->_pendingRemoval;
    to->_version = from->_version;
    to->_blocksListCount = from->_blocksListCount;
}

void ComponentDataBuffer::ChangeEntityCapacity(const std::uint16_t* oldBuffer, int oldCapacity,
                                               std::uint16_t* newBuffer, int newCapacity) {
    std::memmove(newBuffer, oldBuffer, static_cast<std::size_t>(oldCapacity) * 2);
    _sparse = newBuffer;
    _sparseCapacity = newCapacity;
}

void* ComponentDataBuffer::GetDataPointer(EntityRef entityRef) const {
    const int slot = _sparse[entityRef.Index];
    if (slot == 0) {
        throw std::logic_error("ComponentDataBuffer: " + entityRef.ToString() +
                               " has no component of type " + TypeName(_componentTypeID));
    }
    return GetDataPointerByIndex(slot);
}

void* ComponentDataBuffer::GetBufferEntry(Allocator* allocator, EntityRef entityRef) {
    int slot = _sparse[entityRef.Index];
    if (slot != 0) {
        if (get_IsSingleton() && _singletonSparse != slot) {
            if (_singletonSparse != 0) {
                throw std::logic_error("ComponentDataBuffer: a second instance of singleton "
                                       "component " +
                                       TypeName(_componentTypeID) + " was requested");
            }
            _singletonSparse = static_cast<std::uint16_t>(slot);
        }
        return GetDataPointerByIndex(slot);
    }

    if (_count >= _blocksListCount * _blockCapacity) {
        UseNextBlock(allocator);
    }
    if (get_IsSingleton()) {
        if (_singletonSparse != 0) {
            throw std::logic_error("ComponentDataBuffer: a second instance of singleton "
                                   "component " +
                                   TypeName(_componentTypeID) + " was requested");
        }
        _singletonSparse = static_cast<std::uint16_t>(_count);
    }
    slot = static_cast<std::uint16_t>(_count);
    ++_count;
    _blocksList[slot / _blockCapacity].PackedHandles[slot % _blockCapacity] = entityRef;
    _sparse[entityRef.Index] = static_cast<std::uint16_t>(slot);
    return GetDataPointerByIndex(slot);
}

void ComponentDataBuffer::Remove(EntityRef entityRef) {
    const int slot = _sparse[entityRef.Index];
    if (slot == 0) {
        throw std::logic_error("ComponentDataBuffer: cannot remove, " + entityRef.ToString() +
                               " has no component of type " + TypeName(_componentTypeID));
    }
    --_count;
    --_pendingRemoval;
    const int last = _count;
    EntityRef& vacated = _blocksList[slot / _blockCapacity].PackedHandles[slot % _blockCapacity];
    if (last > slot) {
        // Move the last instance into the freed slot. Its old slot keeps the stale data;
        // only its handle is cleared.
        EntityRef& moved = _blocksList[last / _blockCapacity].PackedHandles[last % _blockCapacity];
        const EntityRef movedEntity = moved;
        vacated = movedEntity;
        std::memcpy(GetDataPointerByIndex(slot), GetDataPointerByIndex(last),
                    static_cast<std::size_t>(_stride));
        _sparse[movedEntity.Index] = static_cast<std::uint16_t>(slot);
        moved = EntityRef::None();
        if (get_IsSingleton() && _singletonSparse == _count) {
            _singletonSparse = static_cast<std::uint16_t>(slot);
        }
    } else {
        vacated = EntityRef::None();
        std::memset(GetDataPointerByIndex(slot), 0, static_cast<std::size_t>(_stride));
    }
    ++_version;
    _sparse[entityRef.Index] = 0;
}

void ComponentDataBuffer::ExpandBlockListCapacity(Allocator* allocator, int newCapacity) {
    if (_blocksListCapacity >= newCapacity) {
        return;
    }
    const int blockSize = static_cast<int>(sizeof(Block));
    _blocksList = static_cast<Block*>(allocator->Expand(
        _blocksList, _blocksListCapacity * blockSize, newCapacity * blockSize));
    _blocksListCapacity = newCapacity;
}

void ComponentDataBuffer::UseNextBlock(Allocator* allocator) {
    if (_blocksListCount == _blocksListCapacity) {
        ExpandBlockListCapacity(allocator, _blocksListCapacity * 2);
    }
    Block& block = _blocksList[_blocksListCount++];
    if (block.PackedHandles != nullptr) {
        std::memset(static_cast<void*>(block.PackedHandles), 0, static_cast<std::size_t>(_blockByteSize));
    } else {
        block = AllocateBlock(allocator);
    }
}

ComponentDataBuffer::Block ComponentDataBuffer::AllocateBlock(Allocator* allocator) const {
    auto* memory = static_cast<std::uint8_t*>(allocator->AllocAndClear(_blockByteSize, 8));
    return Block{reinterpret_cast<EntityRef*>(memory), memory + _blockDataOffset};
}

void ComponentDataBuffer::RemovalPending(EntityRef /*entityRef*/) {
    ++_pendingRemoval;
    if (get_IsSingleton()) {
        _singletonSparse = 0;
    }
}

// ---------------------------------------------------------------------------------------
// ComponentBlockIterator

void ComponentBlockIterator<void>::Reset(int offset, int count) {
    const int start = std::min(offset + 1, _buffer->_count);
    _currentBlock = start / _buffer->_blockCapacity;
    _currentBlockOffset = start % _buffer->_blockCapacity;
    _remaining = std::clamp(count, 0, _buffer->_count - start);
}

void ComponentBlockIterator<void>::ValidateIterator() const {
    if (_version != _buffer->_version) {
        throw std::logic_error("ComponentBlockIterator: components of type " +
                               ComponentTypeId<>::GetComponentName(_buffer->_componentTypeID) +
                               " were removed while iterating over them");
    }
}

bool ComponentBlockIterator<void>::NextBlock(std::uint8_t*& components, EntityRef*& entities,
                                       int& count) {
    ValidateIterator();
    while (_currentBlock < _buffer->_blocksListCount && _remaining > 0) {
        const int available = _buffer->_blockCapacity - _currentBlockOffset;
        count = available;
        if (available >= 1) {
            const int taken = std::min(_remaining, available);
            count = taken;
            const ComponentDataBuffer::Block& block = _buffer->_blocksList[_currentBlock];
            entities = block.PackedHandles + _currentBlockOffset;
            components = block.PackedData +
                         static_cast<std::ptrdiff_t>(_buffer->_stride) * _currentBlockOffset;
            _remaining -= taken;
            _currentBlockOffset += taken;
            return true;
        }
        ++_currentBlock;
        _currentBlockOffset = 0;
    }
    components = nullptr;
    entities = nullptr;
    count = 0;
    return false;
}

} // namespace Quantum
