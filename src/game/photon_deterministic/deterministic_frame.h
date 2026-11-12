// deterministic_frame.h - Photon.Deterministic.DeterministicFrame and per-frame input storage.
//
// A DeterministicFrame is one snapshot of the simulation state. This base class only knows
// the frame number, whether it was simulated from fully confirmed input, and the input it
// was simulated with; the actual game state lives in the derived class supplied by the
// game (Quantum's Frame), which implements copying, checksumming and serialisation.
//
// Input is stored per player: a fixed-size block of unmanaged memory holding the game's
// decoded input struct, plus an optional rpc byte array, the rpc decoded as a command, and
// flags.
#pragma once

#include "deterministic_types.h"
#include "native.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace Photon::Deterministic {

class DeterministicCommand;
class DeterministicCommandSerializer;
class IDeterministicGame;

// Photon.Deterministic.DeterministicFrameSerializeMode
enum class DeterministicFrameSerializeMode : int {
    Serialize = 0,
    Blit = 1,
};

// Photon.Deterministic.DeterministicFrameInputTemp: one player's raw input for one frame,
// still in serialised form, on its way into the simulator.
struct DeterministicFrameInputTemp {
    int Frame = 0;
    int Player = 0;
    ByteArray Rpc;
    ByteArray Data;
    int DataLength = 0;
    // Confirmed input (as opposed to a local prediction).
    bool IsVerified = false;
    DeterministicInputFlags Flags = DeterministicInputFlags::None;

    DeterministicFrameInputTemp() = default;
    // 0x7a79d00
    DeterministicFrameInputTemp(int frame, int player, ByteArray rpc, ByteArray data,
                                int dataLength, DeterministicInputFlags flags, bool verified);

    // 0x7a79c08 - a copy of the first DataLength bytes, or null when there is no data.
    ByteArray CloneData() const;
    // 0x7a79c90
    static DeterministicFrameInputTemp Verified(int frame, int player, ByteArray rpc,
                                                ByteArray data, int dataLength,
                                                DeterministicInputFlags flags);
    // 0x7a79d5c - unconfirmed input; carries no rpc.
    static DeterministicFrameInputTemp Predicted(int frame, int player, ByteArray data,
                                                 int dataLength, DeterministicInputFlags flags);
};

// Photon.Deterministic.DeterministicFrameInput: the decoded input of all players for one
// frame. The input block is unmanaged memory and must be released with Free().
class DeterministicFrameInput {
  public:
    std::vector<ByteArray> Rpcs;
    std::vector<std::shared_ptr<DeterministicCommand>> Cmds;
    std::vector<DeterministicInputFlags> Flags;
    // playerCount * inputSize bytes, one input struct per player.
    std::uint8_t* Input = nullptr;

    // 0x7a792d8 - allocates and zeroes the input block from `allocator`.
    DeterministicFrameInput(int playerCount, int inputSize, Native::Allocator* allocator);

    DeterministicFrameInput(const DeterministicFrameInput&) = delete;
    DeterministicFrameInput& operator=(const DeterministicFrameInput&) = delete;

    // 0x7a79150
    int PlayerCount() const { return playerCount_; }
    int InputSize() const { return inputSize_; }

    // 0x7a79158 - zeroes the input block and drops all rpcs, flags and commands (disposing
    // the commands, which returns pooled ones to their pool).
    void Clear();
    // 0x7a792c4 - the player's input struct (no range check, like the original).
    std::uint8_t* GetPlayerInput(int player) const;
    // 0x7a79408 - decodes every rpc flagged as a command into Cmds, reusing the previous
    // command when it was decoded from the very same rpc array.
    void DeserializeCommands(DeterministicCommandSerializer& serializer);
    // 0x7a7981c - stores one player's raw input: rpc and flags as they are, the data through
    // the game's DeserializeInputInto.
    void Assign(const DeterministicFrameInputTemp& input, IDeterministicGame& sessionGame);
    // 0x7a7993c - copies everything (commands and rpcs by reference).
    void CopyFrom(const DeterministicFrameInput& from);
    // 0x7a79ac4 - repeats `player`'s input struct and flags (minus the command flag) from
    // another frame; the rpc and command are not repeated.
    void CopyForPrediction(const DeterministicFrameInput& from, int player);
    // 0x7a79b98 - releases the input block; the object is unusable afterwards.
    void Free(Native::Allocator* allocator);

  private:
    int playerCount_ = 0;
    int inputSize_ = 0;
};

class DeterministicFrame {
  public:
    // Flag for DumpFrame: leave out the heap contents.
    static constexpr int DumpFlag_NoHeap = 1;

    std::unique_ptr<DeterministicFrameInput> Input;
    bool Verified = false;
    int Number = 0;

    // 0x7a887f4
    DeterministicFrame() = default;
    virtual ~DeterministicFrame() = default;

    DeterministicFrame(const DeterministicFrame&) = delete;
    DeterministicFrame& operator=(const DeterministicFrame&) = delete;

    // 0x7a8869c
    bool IsVerified() const { return Verified; }
    // 0x7a886a4
    bool IsPredicted() const { return !Verified; }

    // 0x7a7a758 - creates the input storage on first use, otherwise clears it.
    void Init(int playerCount, int inputSize, Native::Allocator* allocator);

    // 0x7a886b4 - the raw flags (None for an out-of-range player).
    DeterministicInputFlags GetPlayerInputFlagsInternal(int player) const;
    // 0x7a88704 - the player's rpc, or null.
    ByteArray GetRawRpc(int player) const;
    // 0x7a88754 - the player's decoded input struct, or null for an out-of-range player.
    std::uint8_t* GetRawInput(int player) const;
    // 0x7a88790 - the flags without the Repeatable bit, which is internal to prediction.
    DeterministicInputFlags GetPlayerInputFlags(int player) const;
    // 0x7a887a4 - the command decoded from the player's rpc, or null.
    std::shared_ptr<DeterministicCommand> GetPlayerCommand(int player) const;

    // Abstract members implemented by the game's frame type.
    virtual std::string DumpFrame(int dumpFlags) = 0;
    virtual std::uint64_t CalculateChecksum() = 0;
    // Releases the game state's unmanaged memory (not the input storage).
    virtual void Free() = 0;
    virtual std::vector<std::uint8_t> Serialize(DeterministicFrameSerializeMode mode) = 0;
    virtual void Deserialize(std::span<const std::uint8_t> data) = 0;
    // Copies the game state of another frame of the same type.
    virtual void Copy(const DeterministicFrame& frame) = 0;

    // 0x7a7ab80 - Copy() plus the frame number. Verified and Input are left alone.
    void CopyFrom(const DeterministicFrame& frame);
};

} // namespace Photon::Deterministic
