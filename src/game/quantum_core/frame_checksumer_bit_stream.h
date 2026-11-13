// frame_checksumer_bit_stream.h - Quantum.FrameChecksumerBitStream: an IBitStream that
// checksums instead of storing.
//
// Frame checksums are computed by running the normal serialisation code over this stream:
// every value "written" is fed, as its raw little-endian bytes, into a running CRC64
// (photon_deterministic/crc64.h). Booleans count as one byte (0 or 1); values written with
// an explicit bit count still contribute all their bytes. Reading is not possible, and
// asking for the data gives the 8 bytes of the checksum itself.
#pragma once

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/crc64.h"

#include <cstdint>
#include <span>
#include <vector>

namespace Quantum {

class FrameChecksumerBitStream final : public Photon::Deterministic::IBitStream {
  public:
    // 0x7cefbb0 - Checksum 0, neither reading nor writing.
    FrameChecksumerBitStream() = default;

    // 0x7cefb6c, 0x7cefb74, 0x7cefb7c, 0x7cefb84 - Plain flags (callers set Writing).
    bool Writing() const override { return _writing; }
    void SetWriting(bool value) override { _writing = value; }
    bool Reading() const override { return _reading; }
    void SetReading(bool value) override { _reading = value; }
    // 0x7cefb8c, 0x7cefb94
    std::uint64_t get_Checksum() const { return _crc; }
    void set_Checksum(std::uint64_t value) { _crc = value; }
    // 0x7cefb9c - Always the size of the checksum.
    int BytesRequired() const override { return 8; }
    // 0x7cefba4 - The checksum's bytes (BitConverter.GetBytes, little-endian). The span
    // refers to storage inside this object that is refreshed on every call.
    std::span<std::uint8_t> Data() const override;

    // 0x4f006c8 (instance for byte), 0x4f00740 (byte*), 0x4f00d80 (byte*, length) - Feeds
    // the bytes of a value, of the value at a pointer, or of an array into the checksum.
    template <class T> void CRC(T value) { _crc = Photon::Deterministic::CRC64::Calculate(_crc, &value, sizeof(T)); }
    template <class T> void CRC(const T* value) {
        _crc = Photon::Deterministic::CRC64::Calculate(_crc, value, sizeof(T));
    }
    template <class T> void CRC(const T* value, int length) {
        _crc = Photon::Deterministic::CRC64::Calculate(_crc, value,
                                                        static_cast<int>(sizeof(T)) * length);
    }

    // 0x7cefbcc - Checksums the condition as one byte and returns it.
    bool Condition(bool condition) override;
    // 0x7cefc2c, 0x7cefc84, 0x7cefcdc - By reference, the value is checksummed.
    void Serialize(std::int64_t& value) override { CRC(value); }
    void Serialize(std::uint32_t& value) override { CRC(value); }
    void Serialize(std::int32_t& value) override { CRC(value); }
    // 0x7cefd34 .. 0x7ceff9c - By pointer.
    void Serialize(std::uint8_t* v) override { CRC<std::uint8_t>(v); }
    void Serialize(std::int8_t* v) override { CRC<std::int8_t>(v); }
    void Serialize(std::int16_t* v) override { CRC<std::int16_t>(v); }
    void Serialize(std::uint16_t* v) override { CRC<std::uint16_t>(v); }
    void Serialize(std::int32_t* v) override { CRC<std::int32_t>(v); }
    void Serialize(std::uint32_t* v) override { CRC<std::uint32_t>(v); }
    void Serialize(std::int64_t* v) override { CRC<std::int64_t>(v); }
    void Serialize(std::uint64_t* v) override { CRC<std::uint64_t>(v); }
    // 0x7cf004c, 0x7cefff4 - The bit count is ignored; all four bytes count.
    void Serialize(std::int32_t* v, int /*bits*/) override { CRC<std::int32_t>(v); }
    void Serialize(std::uint32_t* v, int /*bits*/) override { CRC<std::uint32_t>(v); }
    // 0x7cf00a4, 0x7cf0104, 0x7cf0164, 0x7cf01c4
    void SerializeBuffer(std::uint8_t* buffer, int length) override { CRC(buffer, length); }
    void SerializeBuffer(std::int16_t* buffer, int length) override { CRC(buffer, length); }
    void SerializeBuffer(std::int32_t* buffer, int length) override { CRC(buffer, length); }
    void SerializeBuffer(std::uint64_t* buffer, int length) override { CRC(buffer, length); }
    // 0x7cf0224, 0x7cf0284 - One byte (0/1); returns the value.
    bool WriteBool(bool b) override;
    bool WriteBoolean(bool b) override;
    // 0x7cf02e4, 0x7cf033c
    void WriteInt(int v) override { CRC(v); }
    void WriteUInt(std::uint32_t v) override { CRC(v); }
    // 0x7cf0394, 0x7cf03cc, 0x7cf0404, 0x7cf043c - Reading is not supported (throws
    // std::logic_error, InvalidOperationException in the original).
    std::uint32_t ReadUInt() override;
    bool ReadBool() override;
    bool ReadBoolean() override;
    int ReadInt() override;
    // 0x7cf0474, 0x7cf04ac - Not supported either (throw).
    void CopyFromArray(std::span<const std::uint8_t> data) override;
    std::vector<std::uint8_t> ToArray() const override;
    // 0x7cf04e4 - Checksum back to 0.
    void Reset() override { _crc = 0; }

  private:
    std::uint64_t _crc = 0;
    bool _writing = false;
    bool _reading = false;
    mutable std::uint8_t _dataBytes[8] = {};
};

} // namespace Quantum
