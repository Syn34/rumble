#include "quantum_core/byte_stream.h"

#include <stdexcept>

namespace Quantum {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;

ByteStream::ByteStream(std::vector<std::uint8_t>* buffer)
    : length_(static_cast<int>(buffer->size())), buffer_(buffer) {}

std::uint8_t& ByteStream::At(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= buffer_->size()) {
        throw std::out_of_range("ByteStream: position outside the buffer");
    }
    return (*buffer_)[static_cast<std::size_t>(index)];
}

// 0x7d3e5e8
void ByteStream::WriteByte(std::uint8_t value) {
    At(position_++) = value;
}

// 0x7d3e624
std::uint8_t ByteStream::ReadByte() {
    return At(position_++);
}

// 0x7d3e660
void ByteStream::WriteInt(std::int32_t value) {
    const auto bits = static_cast<std::uint32_t>(value);
    for (int shift = 0; shift < 32; shift += 8) {
        WriteByte(static_cast<std::uint8_t>(bits >> shift));
    }
}

// 0x7d3e724
std::int32_t ByteStream::ReadInt() {
    std::uint32_t bits = 0;
    for (int shift = 0; shift < 32; shift += 8) {
        bits |= static_cast<std::uint32_t>(ReadByte()) << shift;
    }
    return static_cast<std::int32_t>(bits);
}

// 0x7d3e7e0
void ByteStream::WriteULong(std::uint64_t value) {
    for (int shift = 0; shift < 64; shift += 8) {
        WriteByte(static_cast<std::uint8_t>(value >> shift));
    }
}

// 0x7d3e950
std::uint64_t ByteStream::ReadULong() {
    std::uint64_t bits = 0;
    for (int shift = 0; shift < 64; shift += 8) {
        bits |= static_cast<std::uint64_t>(ReadByte()) << shift;
    }
    return bits;
}

// 0x7d3eab8
void ByteStream::WriteLong(std::int64_t value) {
    WriteULong(static_cast<std::uint64_t>(value));
}

// 0x7d3ec28
std::int64_t ByteStream::ReadLong() {
    return static_cast<std::int64_t>(ReadULong());
}

namespace ByteStreamExtension {

// 0x7d3eef8
std::uint8_t Serialize(ByteStream& stream, bool write, std::uint8_t value) {
    if (write) {
        stream.WriteByte(value);
        return value;
    }
    return stream.ReadByte();
}

// 0x7d3efec
std::int32_t Serialize(ByteStream& stream, bool write, std::int32_t value) {
    if (write) {
        stream.WriteInt(value);
        return value;
    }
    return stream.ReadInt();
}

// 0x7d3f020
std::uint64_t Serialize(ByteStream& stream, bool write, std::uint64_t value) {
    if (write) {
        stream.WriteULong(value);
        return value;
    }
    return stream.ReadULong();
}

// 0x7d3f054
std::int64_t Serialize(ByteStream& stream, bool write, std::int64_t value) {
    if (write) {
        stream.WriteLong(value);
        return value;
    }
    return stream.ReadLong();
}

FP Serialize(ByteStream& stream, bool write, FP value) {
    return FP::FromRaw(Serialize(stream, write, value.RawValue()));
}

// 0x7d3f088
FPVector3 Serialize(ByteStream& stream, bool write, const FPVector3& value) {
    FPVector3 result = value;
    if (write) {
        stream.WriteLong(value.X.RawValue());
        stream.WriteLong(value.Y.RawValue());
        stream.WriteLong(value.Z.RawValue());
    } else {
        result.X = FP::FromRaw(stream.ReadLong());
        result.Y = FP::FromRaw(stream.ReadLong());
        result.Z = FP::FromRaw(stream.ReadLong());
    }
    return result;
}

// 0x7d3ee38
std::vector<std::uint8_t> SerializeByteArray(ByteStream& stream, bool write,
                                             std::vector<std::uint8_t> array) {
    array = SerializeArrayLength(stream, write, std::move(array));
    for (auto& value : array) {
        value = Serialize(stream, write, value);
    }
    return array;
}

// 0x7d3ef2c
std::vector<std::int32_t> SerializeIntArray(ByteStream& stream, bool write,
                                            std::vector<std::int32_t> array) {
    array = SerializeArrayLength(stream, write, std::move(array));
    for (auto& value : array) {
        value = Serialize(stream, write, value);
    }
    return array;
}

// 0x7d3ed94
std::string SerializeString(ByteStream& stream, bool write, const std::string& value) {
    if (write) {
        SerializeByteArray(stream, true, std::vector<std::uint8_t>(value.begin(), value.end()));
        return value;
    }
    const std::vector<std::uint8_t> bytes = SerializeByteArray(stream, false, {});
    return std::string(bytes.begin(), bytes.end());
}

} // namespace ByteStreamExtension

} // namespace Quantum
