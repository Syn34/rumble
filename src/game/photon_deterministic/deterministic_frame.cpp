#include "deterministic_frame.h"

#include "deterministic_command.h"
#include "deterministic_interfaces.h"
#include "log.h"

#include <algorithm>
#include <stdexcept>

namespace Photon::Deterministic {

// --- DeterministicFrameInputTemp ------------------------------------------------------

DeterministicFrameInputTemp::DeterministicFrameInputTemp(int frame, int player, ByteArray rpc,
                                                         ByteArray data, int dataLength,
                                                         DeterministicInputFlags flags,
                                                         bool verified)
    : Frame(frame), Player(player), Rpc(std::move(rpc)), Data(std::move(data)),
      DataLength(dataLength), IsVerified(verified), Flags(flags) {}

ByteArray DeterministicFrameInputTemp::CloneData() const {
    if (!Data || DataLength == 0) {
        return nullptr;
    }
    if (DataLength < 0 || static_cast<std::size_t>(DataLength) > Data->size()) {
        throw std::out_of_range("DeterministicFrameInputTemp::CloneData: bad DataLength");
    }
    return MakeByteArray({Data->begin(), Data->begin() + DataLength});
}

DeterministicFrameInputTemp DeterministicFrameInputTemp::Verified(int frame, int player,
                                                                  ByteArray rpc, ByteArray data,
                                                                  int dataLength,
                                                                  DeterministicInputFlags flags) {
    return {frame, player, std::move(rpc), std::move(data), dataLength, flags, true};
}

DeterministicFrameInputTemp DeterministicFrameInputTemp::Predicted(int frame, int player,
                                                                   ByteArray data, int dataLength,
                                                                   DeterministicInputFlags flags) {
    return {frame, player, nullptr, std::move(data), dataLength, flags, false};
}

// --- DeterministicFrameInput ----------------------------------------------------------

DeterministicFrameInput::DeterministicFrameInput(int playerCount, int inputSize,
                                                 Native::Allocator* allocator)
    : Rpcs(static_cast<std::size_t>(playerCount)), Cmds(static_cast<std::size_t>(playerCount)),
      Flags(static_cast<std::size_t>(playerCount), DeterministicInputFlags::None),
      playerCount_(playerCount), inputSize_(inputSize) {
    if (allocator == nullptr) {
        throw std::invalid_argument("DeterministicFrameInput: null allocator");
    }
    const int size = inputSize * playerCount;
    Input = static_cast<std::uint8_t*>(allocator->Alloc(size));
    allocator->Clear(Input, size);
}

void DeterministicFrameInput::Clear() {
    Native::RequireUtils().Clear(Input, playerCount_ * inputSize_);
    std::fill(Rpcs.begin(), Rpcs.end(), nullptr);
    std::fill(Flags.begin(), Flags.end(), DeterministicInputFlags::None);
    for (auto& cmd : Cmds) {
        if (cmd) {
            cmd->Dispose();
        }
        cmd.reset();
    }
}

std::uint8_t* DeterministicFrameInput::GetPlayerInput(int player) const {
    return Input + inputSize_ * player;
}

void DeterministicFrameInput::DeserializeCommands(DeterministicCommandSerializer& serializer) {
    for (std::size_t i = 0; i < Rpcs.size(); ++i) {
        if (!HasFlag(Flags.at(i), DeterministicInputFlags::Command)) {
            continue;
        }
        const ByteArray& rpc = Rpcs[i];
        if (!rpc) {
            Log::Error("Input is flagged as a command but carries no rpc data");
            continue;
        }
        if (Cmds.at(i) && Cmds[i]->Source == rpc) {
            continue; // already decoded from this exact array
        }
        BitStream& stream = serializer.CommandSerializerStreamRead;
        stream.SetBuffer(*rpc);
        stream.SetWriting(false);
        std::shared_ptr<DeterministicCommand> cmd;
        if (!serializer.ReadNext(stream, cmd)) {
            Log::Error("Failed to deserialize the command in a player's rpc");
            continue;
        }
        Cmds[i] = std::move(cmd);
        Cmds[i]->Source = Rpcs[i];
    }
}

void DeterministicFrameInput::Assign(const DeterministicFrameInputTemp& input,
                                     IDeterministicGame& sessionGame) {
    const auto player = static_cast<std::size_t>(input.Player);
    Rpcs.at(player) = input.Rpc;
    Flags.at(player) = input.Flags;
    sessionGame.DeserializeInputInto(input.Player, input.Data, Input + inputSize_ * input.Player);
}

void DeterministicFrameInput::CopyFrom(const DeterministicFrameInput& from) {
    for (int i = 0; i < playerCount_; ++i) {
        const auto index = static_cast<std::size_t>(i);
        Cmds.at(index) = from.Cmds.at(index);
        Rpcs.at(index) = from.Rpcs.at(index);
        Flags.at(index) = from.Flags.at(index);
    }
    Native::RequireUtils().Copy(Input, from.Input, inputSize_ * playerCount_);
}

void DeterministicFrameInput::CopyForPrediction(const DeterministicFrameInput& from, int player) {
    const auto index = static_cast<std::size_t>(player);
    Flags.at(index) = from.Flags.at(index) & ~DeterministicInputFlags::Command;
    Native::RequireUtils().Copy(Input + inputSize_ * player, from.Input + from.inputSize_ * player,
                                inputSize_);
}

void DeterministicFrameInput::Free(Native::Allocator* allocator) {
    Rpcs.clear();
    Flags.clear();
    Cmds.clear();
    if (Input != nullptr) {
        if (allocator == nullptr) {
            throw std::invalid_argument("DeterministicFrameInput::Free: null allocator");
        }
        allocator->Free(Input);
        Input = nullptr;
    }
}

// --- DeterministicFrame ---------------------------------------------------------------

void DeterministicFrame::Init(int playerCount, int inputSize, Native::Allocator* allocator) {
    if (Input) {
        Input->Clear();
        return;
    }
    Input = std::make_unique<DeterministicFrameInput>(playerCount, inputSize, allocator);
}

DeterministicInputFlags DeterministicFrame::GetPlayerInputFlagsInternal(int player) const {
    if (player < 0 || player >= Input->PlayerCount()) {
        return DeterministicInputFlags::None;
    }
    return Input->Flags.at(static_cast<std::size_t>(player));
}

ByteArray DeterministicFrame::GetRawRpc(int player) const {
    if (player < 0 || player >= Input->PlayerCount()) {
        return nullptr;
    }
    return Input->Rpcs.at(static_cast<std::size_t>(player));
}

std::uint8_t* DeterministicFrame::GetRawInput(int player) const {
    if (player < 0 || player >= Input->PlayerCount()) {
        return nullptr;
    }
    return Input->GetPlayerInput(player);
}

DeterministicInputFlags DeterministicFrame::GetPlayerInputFlags(int player) const {
    return GetPlayerInputFlagsInternal(player) & ~DeterministicInputFlags::Repeatable;
}

std::shared_ptr<DeterministicCommand> DeterministicFrame::GetPlayerCommand(int player) const {
    if (player < 0 || player >= Input->PlayerCount()) {
        return nullptr;
    }
    return Input->Cmds.at(static_cast<std::size_t>(player));
}

void DeterministicFrame::CopyFrom(const DeterministicFrame& frame) {
    Copy(frame);
    Number = frame.Number;
}

} // namespace Photon::Deterministic
