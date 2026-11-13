#include "frame_checksumer_bit_stream.h"

#include <stdexcept>

namespace Quantum {

namespace {

[[noreturn]] void ThrowNotSupported(const char* what) {
    throw std::logic_error(std::string("FrameChecksumerBitStream: ") + what +
                           " is not supported");
}

} // namespace

std::span<std::uint8_t> FrameChecksumerBitStream::Data() const {
    for (int i = 0; i < 8; ++i) {
        _dataBytes[i] = static_cast<std::uint8_t>(_crc >> (8 * i));
    }
    return std::span<std::uint8_t>(_dataBytes, 8);
}

bool FrameChecksumerBitStream::Condition(bool condition) {
    CRC(static_cast<std::uint8_t>(condition ? 1 : 0));
    return condition;
}

bool FrameChecksumerBitStream::WriteBool(bool b) {
    CRC(static_cast<std::uint8_t>(b ? 1 : 0));
    return b;
}

bool FrameChecksumerBitStream::WriteBoolean(bool b) {
    CRC(static_cast<std::uint8_t>(b ? 1 : 0));
    return b;
}

std::uint32_t FrameChecksumerBitStream::ReadUInt() { ThrowNotSupported("ReadUInt"); }
bool FrameChecksumerBitStream::ReadBool() { ThrowNotSupported("ReadBool"); }
bool FrameChecksumerBitStream::ReadBoolean() { ThrowNotSupported("ReadBoolean"); }
int FrameChecksumerBitStream::ReadInt() { ThrowNotSupported("ReadInt"); }

void FrameChecksumerBitStream::CopyFromArray(std::span<const std::uint8_t> /*data*/) {
    ThrowNotSupported("CopyFromArray");
}

std::vector<std::uint8_t> FrameChecksumerBitStream::ToArray() const {
    ThrowNotSupported("ToArray");
}

} // namespace Quantum
