// core_types.h - small value types of the gameplay simulation that the component types
// build on (quantum.code.dll): fixed-capacity strings, bit sets, a compressed direction
// and the two tagged unions used by the AI.
#pragma once

#include "quantum_code/value_support.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace Quantum {

// --- fixed-capacity strings ------------------------------------------------------------
//
// QStringN keeps a string inside frame memory: a byte count, then up to N - 2 bytes of
// UTF-16 code units (see quantum_core/q_string.h for the shared rules). Strings come in
// and out as UTF-16, as C# strings did.

struct alignas(2) QString34 {
    static constexpr int SIZE = 36;
    static constexpr int ALIGNMENT = 2;
    static constexpr int MaxByteCount = 32;

    std::uint16_t ByteCount;
    std::array<std::uint8_t, 32> Bytes;
    std::uint8_t _alignment_padding_[2];

    // 0x7b8b3c8 - Stores `str`; throws std::invalid_argument if it is longer than 16 code
    // units.
    static QString34 FromString(std::u16string_view str);
    // 0x7b8b424, 0x7b8b4f4 (QString34 -> string)
    std::u16string ToString() const;
    // 0x7b8b47c - Ordinal comparison with stored UTF-16 bytes.
    int CompareOrdinal(const std::uint8_t* bytes, std::uint16_t byteCount) const;
    // 0x7b8b550 (Equals(QString34)); 0x7b8b4f8 is Equals(object), which also accepts a string.
    bool Equals(const QString34& other) const { return CompareOrdinal(other.Bytes.data(), other.ByteCount) == 0; }
    bool Equals(std::u16string_view other) const;
    friend bool operator==(const QString34& a, const QString34& b) { return a.Equals(b); }
    // 0x7b8b5b8
    std::int32_t GetHashCode() const;
    // 0x7b8b62c - The byte count, then that many bytes.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(QString34) == QString34::SIZE);

struct alignas(2) QString50 {
    static constexpr int SIZE = 52;
    static constexpr int ALIGNMENT = 2;
    static constexpr int MaxByteCount = 48;

    std::uint16_t ByteCount;
    std::array<std::uint8_t, 48> Bytes;
    std::uint8_t _alignment_padding_[2];

    // 0x7b8b76c, 0x7b8b880 (string -> QString50)
    static QString50 FromString(std::u16string_view str);
    // 0x7b8b7c8
    std::u16string ToString() const;
    // 0x7b8b820
    int CompareOrdinal(const std::uint8_t* bytes, std::uint16_t byteCount) const;
    // 0x7b8b8f4 (Equals(QString50)); 0x7b8b89c is Equals(object).
    bool Equals(const QString50& other) const { return CompareOrdinal(other.Bytes.data(), other.ByteCount) == 0; }
    bool Equals(std::u16string_view other) const;
    friend bool operator==(const QString50& a, const QString50& b) { return a.Equals(b); }
    // 0x7b8b95c
    std::int32_t GetHashCode() const;
    // 0x7b8b9d0
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(QString50) == QString50::SIZE);

struct alignas(2) QString64 {
    static constexpr int SIZE = 64;
    static constexpr int ALIGNMENT = 2;
    static constexpr int MaxByteCount = 62;

    std::uint16_t ByteCount;
    std::array<std::uint8_t, 62> Bytes;

    // 0x7b8bb10, 0x7b8bc24 (string -> QString64)
    static QString64 FromString(std::u16string_view str);
    // 0x7b8bb6c, 0x7b8bc3c (QString64 -> string)
    std::u16string ToString() const;
    // 0x7b8bbc4
    int CompareOrdinal(const std::uint8_t* bytes, std::uint16_t byteCount) const;
    // 0x7b8bc98 (Equals(QString64)); 0x7b8bc40 is Equals(object).
    bool Equals(const QString64& other) const { return CompareOrdinal(other.Bytes.data(), other.ByteCount) == 0; }
    bool Equals(std::u16string_view other) const;
    friend bool operator==(const QString64& a, const QString64& b) { return a.Equals(b); }
    // 0x7b8bd00
    std::int32_t GetHashCode() const;
    // 0x7b8bd74
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(QString64) == QString64::SIZE);

// --- bit sets --------------------------------------------------------------------------
//
// Bit i lives in word i / 64 at position i % 64. Indices are not range checked, as in the
// original.

struct alignas(8) BitSet16 {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    std::array<std::uint64_t, 1> bits;

    // 0x7b8ad40
    void Set(int bit) { bits[static_cast<std::size_t>(bit / 64)] |= std::uint64_t{1} << (bit & 63); }
    // 0x7b8ad68
    void ClearAll() { bits.fill(0); }
    // 0x7b8adcc
    bool IsSet(int bit) const { return ((bits[static_cast<std::size_t>(bit / 64)] >> (bit & 63)) & 1u) != 0; }
    // 0x7b8adec
    std::int32_t GetHashCode() const { return HashBuilder(43u).Add(bits).Value(); }
    // 0x7b8ae44 - The words as one buffer.
    static void Serialize(void* ptr, FrameSerializer* serializer) {
        SerializeBuffer(static_cast<BitSet16*>(ptr)->bits, serializer);
    }
};
static_assert(sizeof(BitSet16) == BitSet16::SIZE);

struct alignas(8) BitSet32 {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;
    static constexpr int BitsSize = 32;

    std::array<std::uint64_t, 1> bits;

    // 0x7b8afac
    void Set(int bit) { bits[static_cast<std::size_t>(bit / 64)] |= std::uint64_t{1} << (bit & 63); }
    // 0x7b8afd4
    void Clear(int bit) { bits[static_cast<std::size_t>(bit / 64)] &= ~(std::uint64_t{1} << (bit & 63)); }
    // 0x7b8affc
    bool IsSet(int bit) const { return ((bits[static_cast<std::size_t>(bit / 64)] >> (bit & 63)) & 1u) != 0; }
    // 0x7b8b01c
    std::int32_t GetHashCode() const { return HashBuilder(59u).Add(bits).Value(); }
    // 0x7b8b074
    static void Serialize(void* ptr, FrameSerializer* serializer) {
        SerializeBuffer(static_cast<BitSet32*>(ptr)->bits, serializer);
    }
};
static_assert(sizeof(BitSet32) == BitSet32::SIZE);

struct alignas(8) BitSet1024 {
    static constexpr int SIZE = 128;
    static constexpr int ALIGNMENT = 8;
    static constexpr int BitsSize = 1024;

    std::array<std::uint64_t, 16> bits;

    // 0x7b8ab68
    void Set(int bit) { bits[static_cast<std::size_t>(bit / 64)] |= std::uint64_t{1} << (bit & 63); }
    // 0x7b8ab90
    void Clear(int bit) { bits[static_cast<std::size_t>(bit / 64)] &= ~(std::uint64_t{1} << (bit & 63)); }
    // 0x7b8abb8
    bool IsSet(int bit) const { return ((bits[static_cast<std::size_t>(bit / 64)] >> (bit & 63)) & 1u) != 0; }
    // 0x7b8abd8
    std::int32_t GetHashCode() const { return HashBuilder(37u).Add(bits).Value(); }
    // 0x7b8ac30
    static void Serialize(void* ptr, FrameSerializer* serializer) {
        SerializeBuffer(static_cast<BitSet1024*>(ptr)->bits, serializer);
    }
};
static_assert(sizeof(BitSet1024) == BitSet1024::SIZE);

// --- compressed direction --------------------------------------------------------------

// A direction as pitch and yaw in 16 bits each (used by player input): pitch in
// [-90, 90] degrees at 1/90 degree steps, yaw wrapped to (-180, 180) degrees at 1/45
// degree steps.
struct alignas(2) EncodedDirection3 {
    static constexpr int SIZE = 4;
    static constexpr int ALIGNMENT = 2;
    static constexpr int YAW_MULT = 45;
    static constexpr int PITCH_MULT = 90;

    std::int16_t encodedPitch;
    std::int16_t encodedYaw;

    // 0x7b978a8
    FP get_Yaw() const { return FP::FromRaw((static_cast<std::int64_t>(encodedYaw) << 16) / YAW_MULT); }
    // 0x7b978d0 - The angle is wrapped by C# remainder (keeping its sign) into one half turn
    // either way before encoding.
    void set_Yaw(FP value);
    // 0x7b9790c
    FP get_Pitch() const { return FP::FromRaw((static_cast<std::int64_t>(encodedPitch) << 16) / PITCH_MULT); }
    // 0x7b97934 - Clamped to [-90, 90] first.
    void set_Pitch(FP value);
    // 0x7b9796c - Euler(pitch, yaw, 0).
    FPQuaternion get_Rotation() const;
    // 0x7b97a00 - Pitch and yaw of the rotation's Euler angles, encoded like the setters.
    void set_Rotation(const FPQuaternion& value);

    // 0x7b97744
    std::int32_t GetHashCode() const { return HashBuilder(1471u).Add(encodedPitch).Add(encodedYaw).Value(); }
    // 0x7b97784
    static void Serialize(void* ptr, FrameSerializer* serializer) {
        auto* direction = static_cast<EncodedDirection3*>(ptr);
        SerializeField(&direction->encodedPitch, serializer);
        SerializeField(&direction->encodedYaw, serializer);
    }
};
static_assert(sizeof(EncodedDirection3) == EncodedDirection3::SIZE);

// --- tagged unions ---------------------------------------------------------------------
//
// One value of several possible types; _field_used_ records which. Reading a member
// through its accessor switches the union to that member, clearing the storage first if
// another member was active (the accessors return a pointer for in-place writes).

struct alignas(8) BTDataValue {
    static constexpr int SIZE = 16;
    static constexpr int ALIGNMENT = 8;
    static constexpr int FPVALUE = 1;
    static constexpr int INTVALUE = 2;

    std::int32_t _field_used_;
    std::uint8_t _pad0[4];
    union {
        FP _FPValue{};
        std::int32_t _IntValue;
    };

    // 0x7c357d8
    FP* get_FPValue();
    // 0x7c3585c
    std::int32_t* get_IntValue();

    // 0x7c358e0 - Every member is hashed (over the shared storage), then the tag.
    std::int32_t GetHashCode() const;
    // 0x7c35940 - The tag, then the active member.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BTDataValue) == BTDataValue::SIZE);

struct alignas(8) BlackboardValue {
    static constexpr int SIZE = 32;
    static constexpr int ALIGNMENT = 8;
    static constexpr int BOOLEANVALUE = 1;
    static constexpr int BYTEVALUE = 2;
    static constexpr int INTEGERVALUE = 3;
    static constexpr int FPVALUE = 4;
    static constexpr int FPVECTOR2VALUE = 5;
    static constexpr int FPVECTOR3VALUE = 6;
    static constexpr int ENTITYREFVALUE = 7;
    static constexpr int ASSETREFVALUE = 8;

    std::int32_t _field_used_;
    std::uint8_t _pad0[4];
    union {
        AssetRef _AssetRefValue{};
        QBoolean _BooleanValue;
        std::uint8_t _ByteValue;
        EntityRef _EntityRefValue;
        FP _FPValue;
        FPVector2 _FPVector2Value;
        FPVector3 _FPVector3Value;
        std::int32_t _IntegerValue;
    };

    // 0x7b89954
    std::int32_t get_Field() const { return _field_used_; }
    // 0x7b7fed0
    AssetRef* get_AssetRefValue();
    // 0x7b7f52c
    QBoolean* get_BooleanValue();
    // 0x7b87c38
    std::uint8_t* get_ByteValue();
    // 0x7b7fd00
    EntityRef* get_EntityRefValue();
    // 0x7b7f75c
    FP* get_FPValue();
    // 0x7b7f8e8
    FPVector2* get_FPVector2Value();
    // 0x7b7fac8
    FPVector3* get_FPVector3Value();
    // 0x7b7f2f4
    std::int32_t* get_IntegerValue();

    // 0x7b89560 - Every member is hashed (over the shared storage), then the tag.
    std::int32_t GetHashCode() const;
    // 0x7b896f8 - The tag, then the active member.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(BlackboardValue) == BlackboardValue::SIZE);

} // namespace Quantum
