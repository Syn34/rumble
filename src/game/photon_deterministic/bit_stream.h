// bit_stream.h - Photon.Deterministic.BitStream, the bit-level serializer used for
// player inputs, commands and frame snapshots, and the IBitStream interface it
// implements.
//
// Bits are packed LSB-first: the first bit written lands in bit 0 of the first byte.
// Multi-byte values are written low byte first, each byte through the same bit packer,
// so a value written with N bits occupies exactly N bits of the stream. The position
// is a bit index into the whole buffer (including the start offset, see below).
//
// The stream works on a byte buffer that it may or may not own. A window of the buffer
// can be used: `offset` bytes are skipped and `size` bytes are available after that.
// Reads never check that window, only the buffer bounds, exactly like the original;
// Overflowing / CanRead let callers check it themselves.
//
// Byte arrays that may be null in the original are std::optional here.
#pragma once

#include "byte_utils.h"
#include "fp.h"

#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace Photon::Deterministic {

// Interface slots are listed in the original's order.
class IBitStream {
  public:
    virtual ~IBitStream() = default;

    virtual int BytesRequired() const = 0;
    virtual std::span<std::uint8_t> Data() const = 0;
    virtual bool Writing() const = 0;
    virtual void SetWriting(bool value) = 0;
    virtual bool Reading() const = 0;
    virtual void SetReading(bool value) = 0;
    virtual void Serialize(std::int64_t& value) = 0;
    virtual void Serialize(std::uint32_t& value) = 0;
    virtual void Serialize(std::int32_t& value) = 0;
    virtual void Serialize(std::uint8_t* v) = 0;
    virtual void Serialize(std::int8_t* v) = 0;
    virtual void Serialize(std::int16_t* v) = 0;
    virtual void Serialize(std::uint16_t* v) = 0;
    virtual void Serialize(std::int32_t* v) = 0;
    virtual void Serialize(std::uint32_t* v) = 0;
    virtual void Serialize(std::int64_t* v) = 0;
    virtual void Serialize(std::uint64_t* v) = 0;
    virtual void Serialize(std::int32_t* v, int bits) = 0;
    virtual void Serialize(std::uint32_t* v, int bits) = 0;
    virtual void SerializeBuffer(std::uint8_t* buffer, int length) = 0;
    virtual void SerializeBuffer(std::int16_t* buffer, int length) = 0;
    virtual void SerializeBuffer(std::int32_t* buffer, int length) = 0;
    virtual void SerializeBuffer(std::uint64_t* buffer, int length) = 0;
    virtual bool Condition(bool condition) = 0;
    virtual bool ReadBool() = 0;
    virtual bool ReadBoolean() = 0;
    virtual bool WriteBool(bool b) = 0;
    virtual bool WriteBoolean(bool b) = 0;
    virtual int ReadInt() = 0;
    virtual void WriteInt(int v) = 0;
    virtual void WriteUInt(std::uint32_t v) = 0;
    virtual std::uint32_t ReadUInt() = 0;
    virtual void CopyFromArray(std::span<const std::uint8_t> data) = 0;
    virtual std::vector<std::uint8_t> ToArray() const = 0;
    virtual void Reset() = 0;
};

class BitStream final : public IBitStream {
  public:
    // Streams with their own (zeroed) buffer.
    BitStream();
    explicit BitStream(int size);
    // Streams over a caller-owned buffer, which must outlive the stream (or the next
    // SetBuffer call). The original shared the byte[] the same way.
    explicit BitStream(std::span<std::uint8_t> arr);
    BitStream(std::span<std::uint8_t> arr, int size, int offset);

    BitStream(const BitStream&) = delete;
    BitStream& operator=(const BitStream&) = delete;

    // --- state ------------------------------------------------------------------

    // Bit position relative to the start offset. Setting clamps to [0, window end].
    int Position() const;
    void SetPosition(int value);
    // Bytes touched so far, counted from the start offset (rounded up).
    int BytesRequired() const override;
    int Capacity() const;
    // True once the position has moved past the end of the window.
    bool Overflowing() const;
    bool Writing() const override;
    void SetWriting(bool value) override;
    bool Reading() const override;
    void SetReading(bool value) override;
    std::span<std::uint8_t> Data() const override;

    void SetBuffer(std::span<std::uint8_t> arr);
    void SetBuffer(std::span<std::uint8_t> arr, int size);
    void SetBuffer(std::span<std::uint8_t> arr, int size, int offset);

    // Pads to the next byte boundary (writing zero bits when in write mode) and
    // returns the absolute byte index reached.
    int RoundToByte();
    bool CanRead(int bits) const;
    // Copies `array` to the start of the window and makes it the readable content.
    void CopyFromArray(std::span<const std::uint8_t> array) override;
    // Zeroes the window and rewinds.
    void Reset() override;
    // Zeroes the window and rewinds, with `byteSize` bytes available afterwards.
    void Reset(int byteSize);
    // The bytes written so far (from the start offset, BytesRequired long).
    std::vector<std::uint8_t> ToArray() const override;

    // --- primitive reads/writes -------------------------------------------------

    bool WriteBool(bool value) override;
    bool WriteBoolean(bool value) override;
    bool ReadBool() override;
    bool ReadBoolean() override;

    void WriteByte(std::uint8_t value, int bits);
    std::uint8_t ReadByte(int bits);
    void WriteByte(std::uint8_t value);
    std::uint8_t ReadByte();
    std::int8_t ReadSByte();
    void WriteSByte(std::int8_t value);

    void WriteUShort(std::uint16_t value, int bits);
    std::uint16_t ReadUShort(int bits);
    void WriteUShort(std::uint16_t value);
    std::uint16_t ReadUShort();
    void WriteShort(std::int16_t value, int bits);
    std::int16_t ReadShort(int bits);
    void WriteShort(std::int16_t value);
    std::int16_t ReadShort();

    void WriteUInt(std::uint32_t value, int bits);
    std::uint32_t ReadUInt(int bits);
    void WriteUInt(std::uint32_t value) override;
    std::uint32_t ReadUInt() override;
    void WriteInt(int value, int bits);
    int ReadInt(int bits);
    void WriteInt(int value) override;
    int ReadInt() override;

    void WriteULong(std::uint64_t value, int bits);
    std::uint64_t ReadULong(int bits);
    void WriteULong(std::uint64_t value);
    std::uint64_t ReadULong();
    void WriteLong(std::int64_t value, int bits);
    std::int64_t ReadLong(int bits);
    void WriteLong(std::int64_t value);
    std::int64_t ReadLong();

    // Raw IEEE-754 bit pattern, 64 bits.
    void WriteDouble(double value);
    double ReadDouble();

    void WriteByteArray(std::span<const std::uint8_t> from);
    void WriteByteArray(std::span<const std::uint8_t> from, int count);
    void WriteByteArray(std::span<const std::uint8_t> from, int offset, int count);
    void ReadByteArray(std::span<std::uint8_t> to);
    void ReadByteArray(std::span<std::uint8_t> to, int count);
    void ReadByteArray(std::span<std::uint8_t> to, int offset, int count);

    // Presence bit, then (if present) a 16-bit length and the bytes.
    void WriteByteArrayLengthPrefixed(std::optional<std::span<const std::uint8_t>> array);
    void WriteByteArrayLengthPrefixed(std::optional<std::span<const std::uint8_t>> array,
                                      int maxLength);
    std::optional<std::vector<std::uint8_t>> ReadByteArrayLengthPrefixed();

    // "Is null" bit, then (if not null) a 16-bit byte count and the encoded bytes.
    void WriteString(std::optional<std::string_view> value, Encoding encoding);
    std::optional<std::string> ReadString(Encoding encoding);

    void InternalWriteByte(std::uint8_t value, int bits);
    std::uint8_t InternalReadByte(int bits);

    // FP as its raw 64-bit value.
    void WriteFP(FP fp);
    FP ReadFP();

    // Writes the low `bits` bits of `value` at bit index `ptr` of `data`, leaving the
    // neighbouring bits untouched. `bits` is expected to be 1..8.
    static void WriteByteAt(std::span<std::uint8_t> data, int ptr, int bits, std::uint8_t value);

    // --- symmetric serialisation (write or read depending on mode) ---------------

    // Writes `condition` in write mode; reads and returns it in read mode.
    bool Condition(bool condition) override;

    void Serialize(std::optional<std::string>& value);
    void Serialize(bool& value);
    void Serialize(double& value);
    void Serialize(std::int64_t& value) override;
    void Serialize(std::uint64_t& value);
    void Serialize(FP& value);
    void Serialize(std::uint8_t& value);
    void Serialize(std::uint32_t& value) override;
    void Serialize(std::uint32_t& value, int bits);
    void Serialize(std::uint64_t& value, int bits);
    void Serialize(std::int32_t& value) override;
    void Serialize(std::int32_t& value, int bits);
    void Serialize(std::optional<std::vector<std::int32_t>>& value);
    void Serialize(std::optional<std::vector<std::uint8_t>>& value);
    void Serialize(std::optional<std::vector<std::uint8_t>>& array, int& length);
    void Serialize(std::optional<std::vector<std::uint8_t>>& array, int& length, int fixedSize);

    // Only the element count travels (as a 32-bit int). Reading replaces `array` with
    // a new, default-initialised array of that length (never null).
    // 0x4c5eb7c (fully shared generic instantiation; 0x4c5ec24 for T = TickMissing)
    template <typename T> void SerializeArrayLength(std::optional<std::vector<T>>& array) {
        if (write_) {
            WriteInt(array ? static_cast<int>(array->size()) : 0);
            return;
        }
        const int length = ReadInt();
        if (length < 0) {
            throw std::length_error("BitStream: negative array length"); // OverflowException
        }
        array.emplace(static_cast<std::size_t>(length));
    }

    void Serialize(std::uint8_t* v) override;
    void Serialize(std::int8_t* v) override;
    void Serialize(std::int16_t* v) override;
    void Serialize(std::uint16_t* v) override;
    void Serialize(std::int32_t* v) override;
    void Serialize(std::uint32_t* v) override;
    void Serialize(std::int64_t* v) override;
    void Serialize(std::uint64_t* v) override;
    void Serialize(std::uint32_t* v, int bits) override;
    void Serialize(std::int32_t* v, int bits) override;

    void SerializeBuffer(std::uint8_t* buffer, int length) override;
    void SerializeBuffer(std::int16_t* buffer, int length) override;
    void SerializeBuffer(std::int32_t* buffer, int length) override;
    void SerializeBuffer(std::uint64_t* buffer, int length) override;

  private:
    std::uint8_t& ByteAt(int index) const;

    // _ptr: absolute bit position in the buffer (the offset is included).
    int ptr_ = 0;
    // _maxPtr: absolute bit position of the end of the window.
    int maxPtr_ = 0;
    int offsetBytes_ = 0;
    int capacityBytes_ = 0;
    std::span<std::uint8_t> data_;
    bool write_ = false;
    // Backing storage for streams created with their own buffer.
    std::vector<std::uint8_t> owned_;
};

} // namespace Photon::Deterministic
