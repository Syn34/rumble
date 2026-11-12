#include "bit_stream.h"

#include <algorithm>
#include <bit>
#include <stdexcept>

namespace Photon::Deterministic {

namespace {

// C# masks 32-bit shift counts to 5 bits; these helpers keep that behaviour (and
// avoid undefined behaviour) for the bit-count arithmetic below.
constexpr std::uint32_t ShiftLeft(std::uint32_t value, int count) {
    return value << (static_cast<unsigned>(count) & 31u);
}
constexpr std::uint32_t ShiftRight(std::uint32_t value, int count) {
    return value >> (static_cast<unsigned>(count) & 31u);
}

// Mask with the low `bits` bits of a byte set (bits in 0..8).
constexpr std::uint32_t LowBits(int bits) {
    return ShiftRight(0xFFu, 8 - bits);
}

constexpr std::uint8_t ToByte(std::uint32_t value) {
    return static_cast<std::uint8_t>(value & 0xFFu);
}

std::uint8_t& CheckedByte(std::span<std::uint8_t> data, int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= data.size()) {
        throw std::out_of_range("BitStream: access outside the buffer");
    }
    return data[static_cast<std::size_t>(index)];
}

constexpr int MaxLengthPrefixedBytes = 0xFFFF;

} // namespace

// 0x7a84ec0
BitStream::BitStream() : BitStream(0) {}

// 0x7a78a38
BitStream::BitStream(int size) {
    if (size < 0) {
        throw std::invalid_argument("BitStream: negative size"); // OverflowException originally
    }
    owned_.assign(static_cast<std::size_t>(size), 0);
    SetBuffer(owned_);
}

// 0x7a835b8
BitStream::BitStream(std::span<std::uint8_t> arr) {
    SetBuffer(arr);
}

// 0x7a84f14
BitStream::BitStream(std::span<std::uint8_t> arr, int size, int offset) {
    SetBuffer(arr, size, offset);
}

std::uint8_t& BitStream::ByteAt(int index) const {
    return CheckedByte(data_, index);
}

// 0x7a78e5c
int BitStream::Position() const {
    return ptr_ - offsetBytes_ * 8;
}

// 0x7a78e84
void BitStream::SetPosition(int value) {
    const int windowEnd = maxPtr_ - offsetBytes_ * 8;
    const int clamped = value < 0 ? 0 : std::min(value, windowEnd);
    ptr_ = clamped + offsetBytes_ * 8;
}

// 0x7a84e30
int BitStream::BytesRequired() const {
    return (Position() + 7) >> 3;
}

// 0x7a84ea0
int BitStream::Capacity() const {
    return capacityBytes_;
}

// 0x7a78e74
bool BitStream::Overflowing() const {
    return ptr_ > maxPtr_;
}

// 0x7a84ea8
bool BitStream::Writing() const {
    return write_;
}

// 0x7a84eb0
void BitStream::SetWriting(bool value) {
    write_ = value;
}

// 0x7a7d004
bool BitStream::Reading() const {
    return !write_;
}

// 0x7a79810
void BitStream::SetReading(bool value) {
    write_ = !value;
}

// 0x7a84eb8
std::span<std::uint8_t> BitStream::Data() const {
    return data_;
}

// 0x7a797d4
void BitStream::SetBuffer(std::span<std::uint8_t> arr) {
    SetBuffer(arr, static_cast<int>(arr.size()), 0);
}

// 0x7a84fac
void BitStream::SetBuffer(std::span<std::uint8_t> arr, int size) {
    SetBuffer(arr, size, 0);
}

// 0x7a84f6c
void BitStream::SetBuffer(std::span<std::uint8_t> arr, int size, int offset) {
    data_ = arr;
    offsetBytes_ = offset;
    capacityBytes_ = size;
    ptr_ = offset * 8;
    maxPtr_ = (offset + size) * 8;
}

// 0x7a84574
int BitStream::RoundToByte() {
    const int usedInByte = ptr_ % 8;
    if (usedInByte >= 1) {
        const int padding = 8 - usedInByte;
        if (write_) {
            WriteByteAt(data_, ptr_, padding, 0);
        }
        ptr_ += padding;
    }
    return ptr_ / 8;
}

// 0x7a79104
bool BitStream::CanRead(int bits) const {
    return ptr_ + bits <= maxPtr_;
}

// 0x7a84fe0
void BitStream::CopyFromArray(std::span<const std::uint8_t> array) {
    const auto start = static_cast<std::size_t>(offsetBytes_);
    if (offsetBytes_ < 0 || start + array.size() > data_.size()) {
        throw std::out_of_range("BitStream::CopyFromArray: array does not fit the buffer");
    }
    std::copy(array.begin(), array.end(), data_.begin() + static_cast<std::ptrdiff_t>(start));
    ptr_ = offsetBytes_ * 8;
    maxPtr_ = (offsetBytes_ + static_cast<int>(array.size())) * 8;
}

// 0x7a85038
void BitStream::Reset() {
    Reset(capacityBytes_);
}

// 0x7a811cc
void BitStream::Reset(int byteSize) {
    // The whole window is cleared regardless of byteSize.
    if (offsetBytes_ < 0 || capacityBytes_ < 0 ||
        static_cast<std::size_t>(offsetBytes_) + static_cast<std::size_t>(capacityBytes_) >
            data_.size()) {
        throw std::out_of_range("BitStream::Reset: window outside the buffer");
    }
    std::fill_n(data_.begin() + offsetBytes_, capacityBytes_, std::uint8_t{0});
    ptr_ = offsetBytes_ * 8;
    maxPtr_ = (offsetBytes_ + byteSize) * 8;
}

// 0x7a81210
std::vector<std::uint8_t> BitStream::ToArray() const {
    const int length = BytesRequired();
    if (length < 0 || offsetBytes_ < 0 ||
        static_cast<std::size_t>(offsetBytes_) + static_cast<std::size_t>(length) > data_.size()) {
        throw std::out_of_range("BitStream::ToArray: written range outside the buffer");
    }
    const auto first = data_.begin() + offsetBytes_;
    return {first, first + length};
}

// --- bools -------------------------------------------------------------------------

// 0x7a7d1a8
bool BitStream::WriteBool(bool value) {
    InternalWriteByte(value ? 1 : 0, 1);
    return value;
}

// 0x7a7cc50 (identical to WriteBool)
bool BitStream::WriteBoolean(bool value) {
    InternalWriteByte(value ? 1 : 0, 1);
    return value;
}

// 0x7a7d1f0
bool BitStream::ReadBool() {
    return InternalReadByte(1) == 1;
}

// 0x7a7cc98 (identical to ReadBool)
bool BitStream::ReadBoolean() {
    return InternalReadByte(1) == 1;
}

// --- bytes -------------------------------------------------------------------------

// 0x7a7d018
void BitStream::WriteByte(std::uint8_t value, int bits) {
    InternalWriteByte(value, bits);
}

// 0x7a7d014
std::uint8_t BitStream::ReadByte(int bits) {
    return InternalReadByte(bits);
}

// 0x7a7d280
void BitStream::WriteByte(std::uint8_t value) {
    InternalWriteByte(value, 8);
}

// 0x7a7d278
std::uint8_t BitStream::ReadByte() {
    return InternalReadByte(8);
}

// 0x7a8519c
std::int8_t BitStream::ReadSByte() {
    return static_cast<std::int8_t>(InternalReadByte(8));
}

// 0x7a851a4
void BitStream::WriteSByte(std::int8_t value) {
    InternalWriteByte(static_cast<std::uint8_t>(value), 8);
}

// --- 16-bit ------------------------------------------------------------------------

// 0x7a851d8
void BitStream::WriteUShort(std::uint16_t value, int bits) {
    if (bits > 8) {
        InternalWriteByte(ToByte(value), 8);
        InternalWriteByte(ToByte(static_cast<std::uint32_t>(value) >> 8), bits - 8);
    } else {
        InternalWriteByte(ToByte(value), bits);
    }
}

// 0x7a8525c
std::uint16_t BitStream::ReadUShort(int bits) {
    if (bits > 8) {
        const std::uint32_t low = InternalReadByte(8);
        const std::uint32_t high = InternalReadByte(bits - 8);
        return static_cast<std::uint16_t>(low | (high << 8));
    }
    return InternalReadByte(bits);
}

// 0x7a78e6c
void BitStream::WriteUShort(std::uint16_t value) {
    WriteUShort(value, 16);
}

// 0x7a79118
std::uint16_t BitStream::ReadUShort() {
    return ReadUShort(16);
}

// 0x7a852b0
void BitStream::WriteShort(std::int16_t value, int bits) {
    WriteUShort(static_cast<std::uint16_t>(value), bits);
}

// 0x7a852b4
std::int16_t BitStream::ReadShort(int bits) {
    return static_cast<std::int16_t>(ReadUShort(bits));
}

// 0x7a852b8
void BitStream::WriteShort(std::int16_t value) {
    WriteUShort(static_cast<std::uint16_t>(value), 16);
}

// 0x7a852c0
std::int16_t BitStream::ReadShort() {
    return static_cast<std::int16_t>(ReadUShort(16));
}

// --- 32-bit ------------------------------------------------------------------------

// 0x7a852f8
void BitStream::WriteUInt(std::uint32_t value, int bits) {
    // Whole low bytes first, then whatever bits remain of the top byte. Bit counts
    // outside 1..32 write nothing.
    const int byteCount = (bits + 7) / 8;
    if (byteCount < 1 || byteCount > 4) {
        return;
    }
    for (int i = 0; i < byteCount - 1; ++i) {
        InternalWriteByte(ToByte(value >> (8 * i)), 8);
    }
    const int topShift = 8 * (byteCount - 1);
    InternalWriteByte(ToByte(value >> topShift), bits - topShift);
}

// 0x7a85464
std::uint32_t BitStream::ReadUInt(int bits) {
    const int byteCount = (bits + 7) / 8;
    if (byteCount < 1 || byteCount > 4) {
        return 0;
    }
    std::uint32_t value = 0;
    for (int i = 0; i < byteCount - 1; ++i) {
        value |= static_cast<std::uint32_t>(InternalReadByte(8)) << (8 * i);
    }
    const int topShift = 8 * (byteCount - 1);
    value |= static_cast<std::uint32_t>(InternalReadByte(bits - topShift)) << topShift;
    return value;
}

// 0x7a855b4
void BitStream::WriteUInt(std::uint32_t value) {
    WriteUInt(value, 32);
}

// 0x7a855bc
std::uint32_t BitStream::ReadUInt() {
    return ReadUInt(32);
}

// 0x7a855c4
void BitStream::WriteInt(int value, int bits) {
    WriteUInt(static_cast<std::uint32_t>(value), bits);
}

// 0x7a855c8 (no sign extension of narrow values)
int BitStream::ReadInt(int bits) {
    return static_cast<int>(ReadUInt(bits));
}

// 0x7a83658
void BitStream::WriteInt(int value) {
    WriteUInt(static_cast<std::uint32_t>(value), 32);
}

// 0x7a83650
int BitStream::ReadInt() {
    return static_cast<int>(ReadUInt(32));
}

// --- 64-bit ------------------------------------------------------------------------

// 0x7a855cc
void BitStream::WriteULong(std::uint64_t value, int bits) {
    if (bits > 32) {
        WriteUInt(static_cast<std::uint32_t>(value), 32);
        WriteUInt(static_cast<std::uint32_t>(value >> 32), bits - 32);
    } else {
        WriteUInt(static_cast<std::uint32_t>(value), bits);
    }
}

// 0x7a845ec
std::uint64_t BitStream::ReadULong(int bits) {
    if (bits > 32) {
        const std::uint64_t low = ReadUInt(32);
        const std::uint64_t high = ReadUInt(bits - 32);
        return low | (high << 32);
    }
    return ReadUInt(bits);
}

// 0x7a85618
void BitStream::WriteULong(std::uint64_t value) {
    WriteUInt(static_cast<std::uint32_t>(value), 32);
    WriteUInt(static_cast<std::uint32_t>(value >> 32), 32);
}

// 0x7a85648
std::uint64_t BitStream::ReadULong() {
    const std::uint64_t low = ReadUInt(32);
    const std::uint64_t high = ReadUInt(32);
    return low | (high << 32);
}

// 0x7a8567c
void BitStream::WriteLong(std::int64_t value, int bits) {
    WriteULong(static_cast<std::uint64_t>(value), bits);
}

// 0x7a85680 (no sign extension of narrow values)
std::int64_t BitStream::ReadLong(int bits) {
    return static_cast<std::int64_t>(ReadULong(bits));
}

// 0x7a85684
void BitStream::WriteLong(std::int64_t value) {
    WriteULong(static_cast<std::uint64_t>(value));
}

// 0x7a856b4
std::int64_t BitStream::ReadLong() {
    return static_cast<std::int64_t>(ReadULong());
}

// 0x7a856e8
void BitStream::WriteDouble(double value) {
    const auto bits = std::bit_cast<std::uint64_t>(value);
    for (int i = 0; i < 8; ++i) {
        InternalWriteByte(static_cast<std::uint8_t>((bits >> (8 * i)) & 0xFFu), 8);
    }
}

// 0x7a85820
double BitStream::ReadDouble() {
    std::uint64_t bits = 0;
    for (int i = 0; i < 8; ++i) {
        bits |= static_cast<std::uint64_t>(InternalReadByte(8)) << (8 * i);
    }
    return std::bit_cast<double>(bits);
}

// --- byte arrays -------------------------------------------------------------------

// 0x7a858c4
void BitStream::WriteByteArray(std::span<const std::uint8_t> from) {
    WriteByteArray(from, 0, static_cast<int>(from.size()));
}

// 0x7a85a48
void BitStream::WriteByteArray(std::span<const std::uint8_t> from, int count) {
    WriteByteArray(from, 0, count);
}

// 0x7a858dc
void BitStream::WriteByteArray(std::span<const std::uint8_t> from, int offset, int count) {
    const int bitOffset = ptr_ % 8;
    int index = ptr_ >> 3;
    if (bitOffset == 0) {
        // Byte aligned: plain copy.
        if (count < 0 || offset < 0 ||
            static_cast<std::size_t>(offset) + static_cast<std::size_t>(count) > from.size() ||
            index < 0 ||
            static_cast<std::size_t>(index) + static_cast<std::size_t>(count) > data_.size()) {
            throw std::out_of_range("BitStream::WriteByteArray: range outside an array");
        }
        std::copy_n(from.begin() + offset, count, data_.begin() + index);
    } else {
        // Each source byte straddles two buffer bytes: its low part fills the top of
        // the current byte, its high part the bottom of the next.
        const std::uint32_t keepBelow = LowBits(bitOffset);
        const std::uint32_t keepAbove = ShiftLeft(0xFFu, bitOffset);
        for (int i = 0; i < count; ++i) {
            const int source = offset + i;
            if (source < 0 || static_cast<std::size_t>(source) >= from.size()) {
                throw std::out_of_range("BitStream::WriteByteArray: source index out of range");
            }
            const std::uint32_t b = from[static_cast<std::size_t>(source)];
            std::uint8_t& current = ByteAt(index);
            current = ToByte((current & keepBelow) | ShiftLeft(b, bitOffset));
            std::uint8_t& next = ByteAt(index + 1);
            next = ToByte((next & keepAbove) | ShiftRight(b, 8 - bitOffset));
            ++index;
        }
    }
    ptr_ += count * 8;
}

// 0x7a85a54
void BitStream::ReadByteArray(std::span<std::uint8_t> to) {
    ReadByteArray(to, 0, static_cast<int>(to.size()));
}

// 0x7a85b70
void BitStream::ReadByteArray(std::span<std::uint8_t> to, int count) {
    ReadByteArray(to, 0, count);
}

// 0x7a85a6c
void BitStream::ReadByteArray(std::span<std::uint8_t> to, int offset, int count) {
    const int bitOffset = ptr_ % 8;
    int index = ptr_ >> 3;
    if (bitOffset == 0) {
        if (count < 0 || offset < 0 ||
            static_cast<std::size_t>(offset) + static_cast<std::size_t>(count) > to.size() ||
            index < 0 ||
            static_cast<std::size_t>(index) + static_cast<std::size_t>(count) > data_.size()) {
            throw std::out_of_range("BitStream::ReadByteArray: range outside an array");
        }
        std::copy_n(data_.begin() + index, count, to.begin() + offset);
    } else {
        const std::uint32_t highMask = LowBits(bitOffset);
        for (int i = 0; i < count; ++i) {
            const std::uint32_t current = ByteAt(index);
            const std::uint32_t next = ByteAt(index + 1);
            const int target = offset + i;
            if (target < 0 || static_cast<std::size_t>(target) >= to.size()) {
                throw std::out_of_range("BitStream::ReadByteArray: target index out of range");
            }
            to[static_cast<std::size_t>(target)] =
                ToByte(ShiftRight(current, bitOffset) | ShiftLeft(next & highMask, 8 - bitOffset));
            ++index;
        }
    }
    ptr_ += count * 8;
}

// 0x7a85b7c
void BitStream::WriteByteArrayLengthPrefixed(std::optional<std::span<const std::uint8_t>> array) {
    WriteByteArrayLengthPrefixed(array, array ? static_cast<int>(array->size()) : 0);
}

// 0x7a85b9c
void BitStream::WriteByteArrayLengthPrefixed(std::optional<std::span<const std::uint8_t>> array,
                                             int maxLength) {
    WriteBool(array.has_value());
    if (!array) {
        return;
    }
    const int arrayLength = static_cast<int>(array->size());
    const int length = std::min(arrayLength, maxLength);
    if (length < arrayLength) {
        // TODO(0x7a85d50, Photon.Deterministic.Log.Warn): the original logs
        // "Only sendig {0}/{1} bytes from byte array" (sic) with length and
        // arrayLength; no logging facility exists yet.
    }
    // Only 16 bits of the length are written, but `length` bytes follow regardless
    // (the original does not guard against more than MaxLengthPrefixedBytes).
    WriteUShort(static_cast<std::uint16_t>(length & MaxLengthPrefixedBytes), 16);
    WriteByteArray(*array, 0, length);
}

// 0x7a85e8c
std::optional<std::vector<std::uint8_t>> BitStream::ReadByteArrayLengthPrefixed() {
    if (!ReadBool()) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> array(ReadUShort(16));
    ReadByteArray(array, 0, static_cast<int>(array.size()));
    return array;
}

// --- strings -----------------------------------------------------------------------

// 0x7a85f3c
void BitStream::WriteString(std::optional<std::string_view> value, Encoding encoding) {
    WriteBool(!value.has_value()); // note: the flag means "is null"
    if (!value) {
        return;
    }
    const auto bytes = EncodingGetBytes(encoding, *value);
    WriteUShort(static_cast<std::uint16_t>(bytes.size() & MaxLengthPrefixedBytes), 16);
    WriteByteArray(bytes);
}

// 0x7a85fb8
std::optional<std::string> BitStream::ReadString(Encoding encoding) {
    if (ReadBool()) {
        return std::nullopt;
    }
    const std::uint16_t length = ReadUShort(16);
    if (length == 0) {
        return std::string();
    }
    std::vector<std::uint8_t> bytes(length);
    ReadByteArray(bytes);
    return EncodingGetString(encoding, bytes);
}

// --- bit packing core --------------------------------------------------------------

// 0x7a85078
void BitStream::InternalWriteByte(std::uint8_t value, int bits) {
    WriteByteAt(data_, ptr_, bits, value);
    ptr_ += bits;
}

// 0x7a850b4
std::uint8_t BitStream::InternalReadByte(int bits) {
    if (bits < 1) {
        return 0;
    }
    const int index = ptr_ >> 3;
    const int bitOffset = ptr_ % 8;

    std::uint32_t value = 0;
    if (bits == 8 && bitOffset == 0) {
        value = ByteAt(index);
    } else {
        const std::uint32_t first = ShiftRight(ByteAt(index), bitOffset);
        const int bitsInFirst = 8 - bitOffset;
        const int bitsInNext = bits - bitsInFirst;
        if (bitsInNext > 0) {
            const std::uint32_t next = ByteAt(index + 1) & LowBits(bitsInNext);
            value = ShiftLeft(next, bitsInFirst) | first;
        } else {
            value = first & LowBits(bits);
        }
    }
    ptr_ += bits;
    return ToByte(value);
}

// 0x7a86188
void BitStream::WriteFP(FP fp) {
    WriteULong(static_cast<std::uint64_t>(fp.RawValue()));
}

// 0x7a861b8
FP BitStream::ReadFP() {
    return FP::FromRaw(static_cast<FP::Raw>(ReadULong()));
}

// 0x7a860ac
void BitStream::WriteByteAt(std::span<std::uint8_t> data, int ptr, int bits, std::uint8_t value) {
    if (bits < 1) {
        return;
    }
    const int bitOffset = ptr & 7;
    const int index = ptr >> 3;
    const std::uint32_t payload = value & LowBits(bits);
    const int freeInFirst = 8 - bitOffset;

    if (freeInFirst >= bits) {
        // Fits in one byte: keep the bits below and above the written range.
        std::uint8_t& target = CheckedByte(data, index);
        const std::uint32_t keep = LowBits(bitOffset) | ShiftLeft(0xFFu, bitOffset + bits);
        target = ToByte((target & keep) | ShiftLeft(payload, bitOffset));
    } else {
        // Straddles two bytes: low part on top of this byte, the rest at the bottom
        // of the next.
        std::uint8_t& first = CheckedByte(data, index);
        first = ToByte((first & LowBits(bitOffset)) | ShiftLeft(payload, bitOffset));
        std::uint8_t& second = CheckedByte(data, index + 1);
        second = ToByte((second & ShiftLeft(0xFFu, bits - freeInFirst)) |
                        ShiftRight(payload, freeInFirst));
    }
}

// --- symmetric serialisation -------------------------------------------------------

// 0x7a861ec
bool BitStream::Condition(bool condition) {
    if (write_) {
        WriteBool(condition);
        return condition;
    }
    return ReadBool();
}

// 0x7a86228
void BitStream::Serialize(std::optional<std::string>& value) {
    if (write_) {
        WriteString(value ? std::optional<std::string_view>(*value) : std::nullopt, Encoding::UTF8);
    } else {
        value = ReadString(Encoding::UTF8);
    }
}

// 0x7a83660
void BitStream::Serialize(bool& value) {
    if (write_) {
        WriteBool(value);
    } else {
        value = ReadBool();
    }
}

// 0x7a86290
void BitStream::Serialize(double& value) {
    if (write_) {
        WriteDouble(value);
    } else {
        value = ReadDouble();
    }
}

// 0x7a862bc
void BitStream::Serialize(std::int64_t& value) {
    if (write_) {
        WriteLong(value);
    } else {
        value = ReadLong();
    }
}

// 0x7a8632c
void BitStream::Serialize(std::uint64_t& value) {
    if (write_) {
        WriteULong(value);
    } else {
        value = ReadULong();
    }
}

// 0x7a8639c
void BitStream::Serialize(FP& value) {
    if (write_) {
        WriteFP(value);
    } else {
        value = ReadFP();
    }
}

// 0x7a8640c
void BitStream::Serialize(std::uint8_t& value) {
    if (write_) {
        InternalWriteByte(value, 8);
    } else {
        value = InternalReadByte(8);
    }
}

// 0x7a86464
void BitStream::Serialize(std::uint32_t& value) {
    Serialize(value, 32);
}

// 0x7a8646c
void BitStream::Serialize(std::uint32_t& value, int bits) {
    if (write_) {
        WriteUInt(value, bits);
    } else {
        value = ReadUInt(bits);
    }
}

// 0x7a8649c
void BitStream::Serialize(std::uint64_t& value, int bits) {
    if (write_) {
        WriteULong(value, bits);
    } else {
        value = ReadULong(bits);
    }
}

// 0x7a7d1a0
void BitStream::Serialize(std::int32_t& value) {
    Serialize(value, 32);
}

// 0x7a7d170
void BitStream::Serialize(std::int32_t& value, int bits) {
    if (write_) {
        WriteUInt(static_cast<std::uint32_t>(value), bits);
    } else {
        value = static_cast<std::int32_t>(ReadUInt(bits));
    }
}

// 0x7a864cc
void BitStream::Serialize(std::optional<std::vector<std::int32_t>>& value) {
    if (write_) {
        WriteBool(value.has_value());
        if (!value) {
            return;
        }
        WriteUShort(static_cast<std::uint16_t>(value->size() & MaxLengthPrefixedBytes), 16);
        for (const std::int32_t element : *value) {
            WriteUInt(static_cast<std::uint32_t>(element), 32);
        }
    } else {
        if (!ReadBool()) {
            value.reset();
            return;
        }
        value.emplace(ReadUShort(16));
        for (std::int32_t& element : *value) {
            element = static_cast<std::int32_t>(ReadUInt(32));
        }
    }
}

// 0x7a7cf24
void BitStream::Serialize(std::optional<std::vector<std::uint8_t>>& value) {
    if (write_) {
        if (value) {
            WriteByteArrayLengthPrefixed(std::span<const std::uint8_t>(*value),
                                         static_cast<int>(value->size()));
        } else {
            WriteByteArrayLengthPrefixed(std::nullopt, 0);
        }
    } else {
        value = ReadByteArrayLengthPrefixed();
    }
}

// 0x7a7cdf4
void BitStream::Serialize(std::optional<std::vector<std::uint8_t>>& array, int& length) {
    if (write_) {
        WriteBool(array.has_value());
        if (!array) {
            return;
        }
        WriteUShort(static_cast<std::uint16_t>(length), 16);
        WriteByteArray(*array, 0, length);
    } else {
        // When the presence bit is clear the array is left as it was.
        if (!ReadBool()) {
            return;
        }
        length = ReadUShort(16);
        // An existing array that is large enough is reused (its tail is untouched).
        if (!array || static_cast<std::size_t>(length) > array->size()) {
            array.emplace(static_cast<std::size_t>(length), std::uint8_t{0});
        }
        ReadByteArray(*array, 0, length);
    }
}

// 0x7a7ccb8
void BitStream::Serialize(std::optional<std::vector<std::uint8_t>>& array, int& length,
                          int fixedSize) {
    length = fixedSize;
    if (write_) {
        if (!array) {
            WriteBoolean(false);
            return;
        }
        // An empty array is sent as "absent".
        const bool hasContent = !array->empty();
        WriteBoolean(hasContent);
        if (hasContent) {
            WriteByteArray(*array, 0, fixedSize);
        }
    } else {
        if (!ReadBool()) {
            array.reset();
            return;
        }
        if (!array || array->size() < static_cast<std::size_t>(std::max(fixedSize, 0))) {
            array.emplace(static_cast<std::size_t>(std::max(fixedSize, 0)), std::uint8_t{0});
        }
        ReadByteArray(*array, 0, fixedSize);
    }
}

// 0x7a86650
void BitStream::Serialize(std::uint8_t* v) {
    if (write_) {
        InternalWriteByte(*v, 8);
    } else {
        *v = InternalReadByte(8);
    }
}

// 0x7a866a8
void BitStream::Serialize(std::int8_t* v) {
    if (write_) {
        InternalWriteByte(static_cast<std::uint8_t>(*v), 8);
    } else {
        *v = static_cast<std::int8_t>(InternalReadByte(8));
    }
}

// 0x7a86700
void BitStream::Serialize(std::int16_t* v) {
    if (write_) {
        WriteUShort(static_cast<std::uint16_t>(*v), 16);
    } else {
        *v = static_cast<std::int16_t>(ReadUShort(16));
    }
}

// 0x7a86760
void BitStream::Serialize(std::uint16_t* v) {
    if (write_) {
        WriteUShort(*v, 16);
    } else {
        *v = ReadUShort(16);
    }
}

// 0x7a867c0
void BitStream::Serialize(std::int32_t* v) {
    if (write_) {
        WriteUInt(static_cast<std::uint32_t>(*v), 32);
    } else {
        *v = static_cast<std::int32_t>(ReadUInt(32));
    }
}

// 0x7a867f4
void BitStream::Serialize(std::uint32_t* v) {
    if (write_) {
        WriteUInt(*v, 32);
    } else {
        *v = ReadUInt(32);
    }
}

// 0x7a86828
void BitStream::Serialize(std::int64_t* v) {
    if (write_) {
        WriteLong(*v);
    } else {
        *v = ReadLong();
    }
}

// 0x7a86898
void BitStream::Serialize(std::uint64_t* v) {
    if (write_) {
        WriteULong(*v);
    } else {
        *v = ReadULong();
    }
}

// 0x7a86908
void BitStream::Serialize(std::uint32_t* v, int bits) {
    if (write_) {
        WriteUInt(*v, bits);
    } else {
        *v = ReadUInt(bits);
    }
}

// 0x7a86938
void BitStream::Serialize(std::int32_t* v, int bits) {
    if (write_) {
        WriteUInt(static_cast<std::uint32_t>(*v), bits);
    } else {
        *v = static_cast<std::int32_t>(ReadUInt(bits));
    }
}

// 0x7a86968
void BitStream::SerializeBuffer(std::uint8_t* buffer, int length) {
    for (int i = 0; i < length; ++i) {
        Serialize(buffer + i);
    }
}

// 0x7a869e4
void BitStream::SerializeBuffer(std::int16_t* buffer, int length) {
    for (int i = 0; i < length; ++i) {
        Serialize(buffer + i);
    }
}

// 0x7a86a6c
void BitStream::SerializeBuffer(std::int32_t* buffer, int length) {
    for (int i = 0; i < length; ++i) {
        Serialize(buffer + i);
    }
}

// 0x7a86ad8
void BitStream::SerializeBuffer(std::uint64_t* buffer, int length) {
    for (int i = 0; i < length; ++i) {
        Serialize(buffer + i);
    }
}

} // namespace Photon::Deterministic
