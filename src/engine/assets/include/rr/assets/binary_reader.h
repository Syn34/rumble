// binary_reader.h - bounds-checked reader over an in-memory byte buffer.
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>

namespace rr::assets {

class ReadError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

enum class Endian { Little, Big };

class BinaryReader {
public:
    explicit BinaryReader(std::span<const std::uint8_t> data, Endian endian = Endian::Little)
        : data_(data), endian_(endian) {}

    void SetEndian(Endian endian) { endian_ = endian; }
    std::size_t Position() const { return pos_; }
    std::size_t Size() const { return data_.size(); }
    std::size_t Remaining() const { return data_.size() - pos_; }
    void Seek(std::size_t pos) {
        if (pos > data_.size()) throw ReadError("seek past end of data");
        pos_ = pos;
    }
    void Skip(std::size_t count) { Seek(pos_ + count); }
    // Moves forward to the next multiple of `alignment` (relative to the buffer start).
    void Align(std::size_t alignment) { Seek(std::min(data_.size(), (pos_ + alignment - 1) / alignment * alignment)); }

    std::uint8_t U8() { return Read<std::uint8_t>(); }
    bool Bool() { return U8() != 0; }
    std::int16_t I16() { return Read<std::int16_t>(); }
    std::uint16_t U16() { return Read<std::uint16_t>(); }
    std::int32_t I32() { return Read<std::int32_t>(); }
    std::uint32_t U32() { return Read<std::uint32_t>(); }
    std::int64_t I64() { return Read<std::int64_t>(); }
    std::uint64_t U64() { return Read<std::uint64_t>(); }
    float F32() { return Read<float>(); }

    // Null-terminated string.
    std::string CString() {
        const auto* begin = data_.data() + pos_;
        const void* end = std::memchr(begin, 0, Remaining());
        if (!end) throw ReadError("unterminated string");
        std::string s(reinterpret_cast<const char*>(begin), static_cast<const std::uint8_t*>(end) - begin);
        pos_ += s.size() + 1;
        return s;
    }

    std::span<const std::uint8_t> Bytes(std::size_t count) {
        Require(count);
        auto s = data_.subspan(pos_, count);
        pos_ += count;
        return s;
    }

private:
    void Require(std::size_t count) const {
        if (count > Remaining()) throw ReadError("read past end of data");
    }

    template <typename T> T Read() {
        Require(sizeof(T));
        std::uint8_t bytes[sizeof(T)];
        std::memcpy(bytes, data_.data() + pos_, sizeof(T));
        pos_ += sizeof(T);
        if (endian_ == Endian::Big) {
            for (std::size_t i = 0; i < sizeof(T) / 2; ++i) std::swap(bytes[i], bytes[sizeof(T) - 1 - i]);
        }
        T value;
        std::memcpy(&value, bytes, sizeof(T));
        return value;
    }

    std::span<const std::uint8_t> data_;
    std::size_t pos_ = 0;
    Endian endian_;
};

} // namespace rr::assets
