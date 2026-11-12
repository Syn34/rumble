#include "deterministic_tick_input.h"

#include "bit_stream.h"

#include <algorithm>
#include <optional>

namespace Photon::Deterministic {

namespace {

// BitStream speaks std::optional<std::vector>; the runtime keeps shared byte arrays.
std::optional<std::vector<std::uint8_t>> ToOptional(const ByteArray& array) {
    if (!array) {
        return std::nullopt;
    }
    return *array;
}

ByteArray FromOptional(std::optional<std::vector<std::uint8_t>>&& value) {
    if (!value) {
        return nullptr;
    }
    return MakeByteArray(std::move(*value));
}

} // namespace

std::shared_ptr<DeterministicTickInput> DeterministicTickInput::Pool::Acquire() {
    {
        std::lock_guard<std::mutex> lock(sync_);
        if (!pool_.empty()) {
            std::shared_ptr<DeterministicTickInput> input = std::move(pool_.back());
            pool_.pop_back();
            input->Pooled = false;
            return input;
        }
    }
    return std::make_shared<DeterministicTickInput>();
}

void DeterministicTickInput::Pool::Release(const std::shared_ptr<DeterministicTickInput>& input) {
    if (!input) {
        throw std::invalid_argument("DeterministicTickInput::Pool::Release: null input");
    }
    input->DataLength = 0;
    input->Tick = 0;
    input->PlayerIndex = 0;
    input->Rpc.reset();
    input->Next.reset();
    input->Sent = false;
    if (input->DataArray) {
        std::fill(input->DataArray->begin(), input->DataArray->end(), std::uint8_t{0});
    }
    input->Pooled = true;
    std::lock_guard<std::mutex> lock(sync_);
    pool_.push_back(input);
}

void DeterministicTickInput::CopyToDataArray(std::span<const std::uint8_t> array, int length) {
    if (length < 0 || static_cast<std::size_t>(length) > array.size()) {
        throw std::out_of_range("DeterministicTickInput::CopyToDataArray: bad length");
    }
    if (!DataArray || static_cast<int>(DataArray->size()) < length) {
        DataArray = MakeByteArray(std::vector<std::uint8_t>(static_cast<std::size_t>(length)));
    }
    DataLength = length;
    std::copy_n(array.begin(), length, DataArray->begin());
}

void DeterministicTickInput::SimpleSerialize(Protocol::Serializer*, BitStream& stream,
                                             bool ignoreRpc) {
    stream.Serialize(PlayerIndex, 8);
    stream.Serialize(Tick, 32);

    auto data = ToOptional(DataArray);
    stream.Serialize(data, DataLength);
    if (stream.Reading()) {
        DataArray = FromOptional(std::move(data));
    }

    if (ignoreRpc) {
        if (stream.Writing()) {
            stream.WriteBool(false);
        } else if (stream.InternalReadByte(1) == 1) {
            throw CorruptedException("Unexpected rpc in an input that must not carry one");
        }
    } else {
        auto rpc = ToOptional(Rpc);
        stream.Serialize(rpc);
        if (stream.Reading()) {
            Rpc = FromOptional(std::move(rpc));
        }
    }

    if (stream.Writing()) {
        stream.InternalWriteByte(static_cast<std::uint8_t>(Flags), 8);
    } else {
        Flags = static_cast<DeterministicInputFlags>(stream.InternalReadByte(8));
    }
}

void DeterministicTickInput::SerializeForDecoder(Protocol::Serializer*, BitStream&,
                                                 const DeterministicTickInputEncodeHeader&) {
    // Online-only (see header); intentionally empty.
}

std::string DeterministicTickInput::ToString() const {
    return "TickInput(tick " + std::to_string(Tick) + ", player " + std::to_string(PlayerIndex) +
           ")";
}

void DeterministicTickInputEncodeHeader::Serialize(BitStream&) {
    // Online-only (see header); intentionally empty.
}

} // namespace Photon::Deterministic
