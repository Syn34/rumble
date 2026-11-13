// frame_serializer.h - Quantum.FrameSerializer: writes a frame to, or reads it from, a bit
// stream.
//
// Component data and other frame state serialise themselves through the stream the
// serializer carries. Heap references (Ptr) need extra care because their values are
// offsets into one particular heap: instead of the offset, a small integer key is written
// for each distinct block, and the reading side maps every key to the block it allocated
// (or found) for it. PtrKeyMapping keeps that two-way table and the list of Ptr fields
// that were read before their key's block was known; they are patched as soon as the key
// is resolved.
//
// A serializer built over a FrameChecksumerBitStream does not produce bytes at all; it
// only feeds everything into a running CRC (see frame_checksumer_bit_stream.h).
#pragma once

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/deterministic_frame.h"
#include "photon_deterministic/deterministic_frame_serializer.h"
#include "quantum_core/ptr.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Quantum {

namespace Core {
class FrameBase;
}

class FrameSerializer final : public Photon::Deterministic::IDeterministicFrameSerializer {
  public:
    // Key written in place of the null Ptr.
    static constexpr int NULL_PTR_KEY = -1;
    static constexpr int NULL_PTR_OFFSET = 0;

    // A heap block's stand-in in the stream.
    struct PtrKey {
        int Key = 0;

        constexpr PtrKey() = default;
        // 0x7cf3c14
        constexpr explicit PtrKey(int key) : Key(key) {}

        // 0x7cf2fa8
        constexpr bool IsNull() const { return Key == NULL_PTR_KEY; }
        // 0x7cf2f98 - Zero is never handed out.
        constexpr bool IsValid() const { return Key != 0; }
        // 0x7cf3c1c (Equals(PtrKey)); 0x7cf3c34 is the boxed Equals(object).
        friend constexpr bool operator==(PtrKey a, PtrKey b) { return a.Key == b.Key; }
        // 0x7cf3c2c
        constexpr int GetHashCode() const { return Key; }
        // 0x7cf3cac - Debug text (wording is this project's own).
        std::string ToString() const;
    };

    // A Ptr field waiting for its key to be resolved.
    struct PtrLocation {
        Ptr* Target = nullptr;

        PtrLocation() = default;
        // 0x7cf3c0c
        explicit PtrLocation(Ptr* target) : Target(target) {}
    };

    struct PtrKeyHash {
        std::size_t operator()(PtrKey k) const noexcept {
            return static_cast<std::size_t>(static_cast<std::uint32_t>(k.Key));
        }
    };

    class PtrKeyMapping {
      public:
        // 0x7cf2790 - Empty tables. Unlike Reset, no entry for the null key is made.
        PtrKeyMapping() = default;

        // 0x7cf24e8 - Throws if any Ptr field still waits for its key.
        void VerifyAllKeysResolved() const;
        // 0x7cf389c
        PtrKey NextKey() { return PtrKey(++_keyCounter); }
        // 0x7cf3438 - Records key <-> ptr in both directions. Either side already being
        // mapped throws (Dictionary.Add semantics).
        void AddMapping(PtrKey key, Ptr ptr);
        // 0x7cf38b0
        bool TryGetPtr(PtrKey key, Ptr& ptr) const;
        // 0x7cf3834
        bool TryGetKey(Ptr ptr, PtrKey& key) const;
        // 0x7cf2d0c - Forgets everything, then maps and resolves the null key to the null
        // Ptr.
        void Reset();
        // 0x7cf3918 - Throws if the key is not resolved.
        void VerifyKeyResolved(PtrKey key) const;
        // 0x7cf3010 - Marks the key resolved (once only) and patches every Ptr field
        // waiting for it. Returns the mapped Ptr. Throws for a key without a consistent
        // mapping.
        Ptr ResolveKey(PtrKey key);
        // 0x7cf2fb8
        bool IsKeyResolved(PtrKey key) const { return _keysResolved.contains(key); }
        // 0x7cf39f0 - Remembers a Ptr field to patch once `key` is resolved.
        void AddUnresolved(PtrKey key, Ptr* location);

        int KeyCounter() const { return _keyCounter; }
        void SetKeyCounter(int value) { _keyCounter = value; }

      private:
        int _keyCounter = 0;
        std::unordered_map<Ptr, PtrKey> _ptrToKey;
        std::unordered_map<PtrKey, Ptr, PtrKeyHash> _keyToPtr;
        std::unordered_set<PtrKey, PtrKeyHash> _keysResolved;
        // Insertion-ordered per key, like the original List<PtrLocation>.
        std::unordered_map<PtrKey, std::vector<PtrLocation>, PtrKeyHash> _keysUnresolved;
    };

    using DeterministicFrameSerializeMode = Photon::Deterministic::DeterministicFrameSerializeMode;

    // 0x7cf2720 - No frame and no stream yet.
    FrameSerializer();
    // 0x7cf291c - Reads from / writes into the given bytes (the caller keeps them alive).
    FrameSerializer(DeterministicFrameSerializeMode mode, Core::FrameBase* frame,
                    std::span<std::uint8_t> data);
    // 0x7cf29b8 - A stream with its own buffer of `size` bytes.
    FrameSerializer(DeterministicFrameSerializeMode mode, Core::FrameBase* frame, int size);
    // 0x7cf2a54 - Uses `stream` (not owned). A FrameChecksumerBitStream switches the
    // serializer into checksum mode.
    FrameSerializer(DeterministicFrameSerializeMode mode, Core::FrameBase* frame,
                    Photon::Deterministic::IBitStream* stream);

    FrameSerializer(const FrameSerializer&) = delete;
    FrameSerializer& operator=(const FrameSerializer&) = delete;

    // 0x7cf235c, 0x7cf2364
    Core::FrameBase* get_Frame() const { return _frame; }
    void set_Frame(Core::FrameBase* value) { _frame = value; }
    // 0x7cea580 / 0x7cf236c, 0x7cf1aec / 0x7cf2418 - Forwarded to the stream. The
    // IDeterministicFrameSerializer forms (0x7cf3be8, 0x7cf3bec) are the same.
    bool Writing() const override { return _stream->Writing(); }
    void SetWriting(bool value) { _stream->SetWriting(value); }
    bool Reading() const override { return _stream->Reading(); }
    void SetReading(bool value) { _stream->SetReading(value); }
    // 0x7cf24c4, 0x7cf24cc - Input mode writes a presence flag before optional values.
    bool InputMode() const { return _inputMode; }
    void SetInputMode(bool value) { _inputMode = value; }
    bool Checksum() const { return _checksum; }
    // 0x7cf3bf0 - The stream (a public field in the original).
    Photon::Deterministic::IBitStream& Stream() override { return *_stream; }
    void SetStream(Photon::Deterministic::IBitStream* stream) { _stream = stream; }

    // 0x7cf24d4
    void VerifyNoUnresolvedPointers() const { _mapping.VerifyAllKeysResolved(); }
    // 0x7cf2b0c, 0x7cf2bb8 - Forwarded to the stream.
    void CopyFromArray(std::span<const std::uint8_t> data) { _stream->CopyFromArray(data); }
    std::vector<std::uint8_t> ToArray() const { return _stream->ToArray(); }
    // 0x7cf2c5c - Rewinds the stream and resets the key mapping.
    void Reset();

    // 0x7cf2de8 - The block for `key`: when reading, a fresh zeroed block of `size` bytes
    // is allocated from the frame heap and mapped to the key first. Throws for the null
    // key, key 0 and keys resolved before.
    void* PointerResolve(PtrKey key, int size);
    // 0x7cf34cc - Like the above, for a block that already exists at `ptr`. When writing
    // nothing is recorded and the block's address is returned directly.
    void* PointerResolve(PtrKey key, Ptr ptr);
    // 0x7cf3648 - Writes the key for *p (making one up for a new block) or reads a key and
    // stores the mapped Ptr into *p - or, if the key's block is not known yet, marks *p as
    // waiting (offset -1) until it is. Returns true while the key is still unresolved,
    // i.e. when the caller has to serialise the block's contents now.
    bool PointerSerialize(Ptr* p, PtrKey& key);
    // 0x7cf3b40
    PtrKey PointerSerialize(Ptr* p);
    // 0x7cf3b5c - PointerSerialize for a block given by address; resolves the key to that
    // block right away when needed.
    bool PointerSerializeVoid(void* ptr);

    PtrKeyMapping& Mapping() { return _mapping; }
    DeterministicFrameSerializeMode Mode = DeterministicFrameSerializeMode::Serialize;

  private:
    std::byte* HeapMemory() const;

    bool _inputMode = false;
    bool _checksum = false;
    Core::FrameBase* _frame = nullptr;
    PtrKeyMapping _mapping;
    std::unique_ptr<Photon::Deterministic::BitStream> _ownedStream;
    Photon::Deterministic::IBitStream* _stream = nullptr;
};

} // namespace Quantum

template <> struct std::hash<Quantum::FrameSerializer::PtrKey> {
    std::size_t operator()(Quantum::FrameSerializer::PtrKey k) const noexcept {
        return static_cast<std::size_t>(static_cast<std::uint32_t>(k.Key));
    }
};
