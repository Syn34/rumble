// byte_stream.h - Quantum.ByteStream and Quantum.ByteStreamExtension (quantum.core.dll).
//
// A cursor over a byte buffer used for baked asset data (navmeshes, static colliders...).
// Values are stored little-endian, byte by byte; there is no padding or alignment.
// The extension methods implement the "one function for both directions" pattern:
// when `write` is true the value is appended and returned unchanged, otherwise a value
// is read and returned.
//
// The original stream wraps a .NET byte[]; reading or writing past its end throws
// IndexOutOfRangeException. Here that becomes std::out_of_range.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace Quantum {

class ByteStream {
public:
    // 0x7d3e5a0 - wraps `buffer` (shared, not copied); position starts at 0.
    explicit ByteStream(std::vector<std::uint8_t>* buffer);

    // 0x7d3e5e8
    void WriteByte(std::uint8_t value);
    // 0x7d3e624
    std::uint8_t ReadByte();
    // 0x7d3e660
    void WriteInt(std::int32_t value);
    // 0x7d3e724
    std::int32_t ReadInt();
    // 0x7d3e7e0
    void WriteULong(std::uint64_t value);
    // 0x7d3e950
    std::uint64_t ReadULong();
    // 0x7d3eab8
    void WriteLong(std::int64_t value);
    // 0x7d3ec28
    std::int64_t ReadLong();

    // 0x7d3ed90 - does nothing in the original (the buffer belongs to the caller).
    void Dispose() {}

    int Position() const { return position_; }
    int Length() const { return length_; }

private:
    std::uint8_t& At(int index);

    int position_ = 0; // _position
    int length_ = 0;   // _length: buffer size when the stream was created
    std::vector<std::uint8_t>* buffer_;
};

// Static helpers of Quantum.ByteStreamExtension.
namespace ByteStreamExtension {

// 0x7d3eef8
std::uint8_t Serialize(ByteStream& stream, bool write, std::uint8_t value);
// 0x7d3efec
std::int32_t Serialize(ByteStream& stream, bool write, std::int32_t value);
// 0x7d3f020
std::uint64_t Serialize(ByteStream& stream, bool write, std::uint64_t value);
// 0x7d3f054 - also used for FP (its raw value).
std::int64_t Serialize(ByteStream& stream, bool write, std::int64_t value);
// FP fields are serialised through the long overload above.
Photon::Deterministic::FP Serialize(ByteStream& stream, bool write,
                                    Photon::Deterministic::FP value);
// 0x7d3f088 - X, Y, Z as raw longs.
Photon::Deterministic::FPVector3 Serialize(ByteStream& stream, bool write,
                                           const Photon::Deterministic::FPVector3& value);

// Generic SerializeArrayLength<T> (shared generic body, no address of its own in the
// method list): writes the length (0 for a null array) or reads a length and returns a
// fresh array of that many default elements. Note a written null array reads back as
// an empty one.
template <class T>
std::vector<T> SerializeArrayLength(ByteStream& stream, bool write, std::vector<T> array) {
    if (write) {
        stream.WriteInt(static_cast<std::int32_t>(array.size()));
        return array;
    }
    const std::int32_t length = stream.ReadInt();
    if (length < 0) {
        // new T[negative] throws OverflowException in the original.
        throw std::length_error("ByteStream: negative array length");
    }
    return std::vector<T>(static_cast<std::size_t>(length));
}

// 0x7d3ee38 - length, then each byte.
std::vector<std::uint8_t> SerializeByteArray(ByteStream& stream, bool write,
                                             std::vector<std::uint8_t> array);
// 0x7d3ef2c - length, then each int.
std::vector<std::int32_t> SerializeIntArray(ByteStream& stream, bool write,
                                            std::vector<std::int32_t> array);
// 0x7d3ed94 - the string as a UTF-8 byte array (std::string already holds UTF-8).
std::string SerializeString(ByteStream& stream, bool write, const std::string& value);

} // namespace ByteStreamExtension

} // namespace Quantum
