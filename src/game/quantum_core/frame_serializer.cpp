#include "frame_serializer.h"

#include "quantum_core/frame_base.h"
#include "quantum_core/frame_checksumer_bit_stream.h"

#include <stdexcept>
#include <string>

namespace Quantum {

namespace {

std::string KeyText(FrameSerializer::PtrKey key) { return key.ToString(); }

} // namespace

std::string FrameSerializer::PtrKey::ToString() const {
    return "[PtrKey " + std::to_string(Key) + "]";
}

// ---------------------------------------------------------------------------------------
// PtrKeyMapping

void FrameSerializer::PtrKeyMapping::VerifyAllKeysResolved() const {
    if (_keysUnresolved.empty()) {
        return;
    }
    std::string keys;
    for (const auto& [key, locations] : _keysUnresolved) {
        if (!keys.empty()) {
            keys += ", ";
        }
        keys += std::to_string(key.Key);
    }
    throw std::runtime_error("FrameSerializer: " + std::to_string(_keysUnresolved.size()) +
                             " unresolved pointer key(s): " + keys);
}

void FrameSerializer::PtrKeyMapping::AddMapping(PtrKey key, Ptr ptr) {
    if (!_keyToPtr.emplace(key, ptr).second) {
        throw std::invalid_argument("PtrKeyMapping: " + KeyText(key) + " is already mapped");
    }
    if (!_ptrToKey.emplace(ptr, key).second) {
        throw std::invalid_argument("PtrKeyMapping: " + ptr.ToString() + " is already mapped");
    }
}

bool FrameSerializer::PtrKeyMapping::TryGetPtr(PtrKey key, Ptr& ptr) const {
    auto it = _keyToPtr.find(key);
    if (it == _keyToPtr.end()) {
        ptr = Ptr{};
        return false;
    }
    ptr = it->second;
    return true;
}

bool FrameSerializer::PtrKeyMapping::TryGetKey(Ptr ptr, PtrKey& key) const {
    auto it = _ptrToKey.find(ptr);
    if (it == _ptrToKey.end()) {
        key = PtrKey{};
        return false;
    }
    key = it->second;
    return true;
}

void FrameSerializer::PtrKeyMapping::Reset() {
    _keyCounter = 0;
    _keyToPtr.clear();
    _ptrToKey.clear();
    _keysResolved.clear();
    _keysUnresolved.clear();
    AddMapping(PtrKey(NULL_PTR_KEY), Ptr{NULL_PTR_OFFSET});
    ResolveKey(PtrKey(NULL_PTR_KEY));
}

void FrameSerializer::PtrKeyMapping::VerifyKeyResolved(PtrKey key) const {
    if (!IsKeyResolved(key)) {
        throw std::logic_error("PtrKeyMapping: " + KeyText(key) + " is not resolved");
    }
}

Ptr FrameSerializer::PtrKeyMapping::ResolveKey(PtrKey key) {
    if (IsKeyResolved(key)) {
        throw std::logic_error("PtrKeyMapping: " + KeyText(key) + " was resolved before");
    }
    Ptr ptr;
    if (!TryGetPtr(key, ptr)) {
        throw std::logic_error("PtrKeyMapping: " + KeyText(key) + " has no mapping");
    }
    PtrKey back;
    if (!TryGetKey(ptr, back)) {
        throw std::logic_error("PtrKeyMapping: " + KeyText(key) + " has no reverse mapping");
    }
    if (back != key) {
        throw std::logic_error("PtrKeyMapping: " + KeyText(key) + " and " + KeyText(back) +
                               " map to the same block");
    }
    _keysResolved.insert(key);
    if (auto it = _keysUnresolved.find(key); it != _keysUnresolved.end()) {
        for (const PtrLocation& location : it->second) {
            *location.Target = ptr;
        }
        _keysUnresolved.erase(it);
    }
    return ptr;
}

void FrameSerializer::PtrKeyMapping::AddUnresolved(PtrKey key, Ptr* location) {
    _keysUnresolved[key].push_back(PtrLocation(location));
}

// ---------------------------------------------------------------------------------------
// FrameSerializer

FrameSerializer::FrameSerializer() = default;

FrameSerializer::FrameSerializer(DeterministicFrameSerializeMode mode, Core::FrameBase* frame,
                                 std::span<std::uint8_t> data)
    : Mode(mode), _frame(frame),
      _ownedStream(std::make_unique<Photon::Deterministic::BitStream>(data)),
      _stream(_ownedStream.get()) {}

FrameSerializer::FrameSerializer(DeterministicFrameSerializeMode mode, Core::FrameBase* frame,
                                 int size)
    : Mode(mode), _frame(frame),
      _ownedStream(std::make_unique<Photon::Deterministic::BitStream>(size)),
      _stream(_ownedStream.get()) {}

FrameSerializer::FrameSerializer(DeterministicFrameSerializeMode mode, Core::FrameBase* frame,
                                 Photon::Deterministic::IBitStream* stream)
    : Mode(mode), _checksum(dynamic_cast<FrameChecksumerBitStream*>(stream) != nullptr),
      _frame(frame), _stream(stream) {}

void FrameSerializer::Reset() {
    _stream->Reset();
    _mapping.Reset();
}

std::byte* FrameSerializer::HeapMemory() const {
    return _frame->get_TrackedHeap()->GetHeap()->Memory();
}

namespace {

void CheckResolvable(const FrameSerializer::PtrKeyMapping& mapping,
                     FrameSerializer::PtrKey key) {
    if (key.IsNull()) {
        throw std::logic_error("FrameSerializer: " + key.ToString() + " is the null key");
    }
    if (!key.IsValid()) {
        throw std::logic_error("FrameSerializer: " + key.ToString() + " is not valid");
    }
    if (mapping.IsKeyResolved(key)) {
        throw std::logic_error("FrameSerializer: " + key.ToString() +
                               " has already been resolved");
    }
}

} // namespace

void* FrameSerializer::PointerResolve(PtrKey key, int size) {
    CheckResolvable(_mapping, key);
    if (!Writing()) {
        const Ptr block = _frame->get_TrackedHeap()->AllocateAndClearPtr(size);
        _mapping.AddMapping(key, block);
    }
    const Ptr resolved = _mapping.ResolveKey(key);
    return HeapMemory() + resolved.Offset;
}

void* FrameSerializer::PointerResolve(PtrKey key, Ptr ptr) {
    CheckResolvable(_mapping, key);
    if (Writing()) {
        return HeapMemory() + ptr.Offset;
    }
    _mapping.AddMapping(key, ptr);
    const Ptr resolved = _mapping.ResolveKey(key);
    return HeapMemory() + resolved.Offset;
}

bool FrameSerializer::PointerSerialize(Ptr* p, PtrKey& key) {
    key = PtrKey{};
    if (Writing()) {
        if (!_mapping.TryGetKey(*p, key)) {
            key = _mapping.NextKey();
            _mapping.AddMapping(key, *p);
        }
        _stream->WriteInt(key.Key);
    } else {
        key = PtrKey(_stream->ReadInt());
        Ptr mapped;
        if (_mapping.TryGetPtr(key, mapped)) {
            _mapping.VerifyKeyResolved(key);
            *p = mapped;
        } else {
            // Patched by ResolveKey once the block for this key is known.
            p->Offset = -1;
            _mapping.AddUnresolved(key, p);
        }
    }
    return !_mapping.IsKeyResolved(key);
}

FrameSerializer::PtrKey FrameSerializer::PointerSerialize(Ptr* p) {
    PtrKey key;
    PointerSerialize(p, key);
    return key;
}

bool FrameSerializer::PointerSerializeVoid(void* ptr) {
    const int offset = static_cast<int>(static_cast<std::byte*>(ptr) - HeapMemory());
    Ptr local{offset};
    PtrKey key;
    const bool unresolved = PointerSerialize(&local, key);
    if (unresolved) {
        PointerResolve(key, Ptr{offset});
    }
    return unresolved;
}

} // namespace Quantum
