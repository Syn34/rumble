// deterministic_command.h - Photon.Deterministic.DeterministicCommand and its plumbing.
//
// Commands are one-off player actions (as opposed to the per-tick input struct) that
// travel in the rpc slot of a player's input. A command type is registered with the
// session's DeterministicCommandSerializer, which assigns it a 16-bit id; on the wire a
// command is that id followed by whatever the command's Serialize writes.
//
// Commands are shared_ptr-owned (frames keep references to the command decoded from each
// player's rpc). A pooled command returns itself to its pool on Dispose().
#pragma once

#include "bit_stream.h"
#include "deterministic_types.h"

#include <cstdint>
#include <memory>
#include <span>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Photon::Deterministic {

class DeterministicCommand;
class DeterministicCommandSerializer;

// Photon.Deterministic.IDeterministicCommandFactory
class IDeterministicCommandFactory {
  public:
    virtual ~IDeterministicCommandFactory() = default;
    // Interface slot 0: get_CommandType - the concrete command type this factory makes.
    virtual std::type_index CommandType() const = 0;
    // Interface slot 1: GetCommandInstance - a fresh (or recycled) command to read into.
    virtual std::shared_ptr<DeterministicCommand> GetCommandInstance() = 0;
};

// Photon.Deterministic.IDeterministicCommandPool
class IDeterministicCommandPool {
  public:
    virtual ~IDeterministicCommandPool() = default;
    // Interface slot 0
    virtual std::shared_ptr<DeterministicCommand> Acquire() = 0;
    // Interface slot 1 - false if the command was already pooled or is of another type.
    virtual bool Release(const std::shared_ptr<DeterministicCommand>& cmd) = 0;
};

class DeterministicCommand : public IDeterministicCommandFactory,
                             public std::enable_shared_from_this<DeterministicCommand> {
  public:
    // The rpc array this command was decoded from (so an unchanged rpc is not decoded twice).
    ByteArray Source;
    // Owning pool, if the command came from one.
    IDeterministicCommandPool* Pool = nullptr;
    // True while the command sits in its pool.
    bool Pooled = false;

    // 0x7a78730
    DeterministicCommand() = default;
    ~DeterministicCommand() override = default;

    // 0x7a78738 - vtable slot 7. By default just Serialize(stream).
    virtual void Serialize(BitStream& stream, DeterministicCommandSerializer& cmdSerializer);
    // Abstract (vtable slot 8): reads or writes the command's fields.
    virtual void Serialize(BitStream& stream) = 0;

    // 0x7a78744 - the dynamic type of the command (GetType()).
    std::type_index CommandType() const override;
    // 0x7a7874c - from the pool if there is one, otherwise a new instance of the same type.
    std::shared_ptr<DeterministicCommand> GetCommandInstance() override;
    // 0x7a78850 - returns the command to its pool, if it has one.
    virtual void Dispose();

  protected:
    // Stands in for Activator.CreateInstance(GetType()): a default-constructed command of
    // the same concrete type. Needed only for commands that are not pooled.
    virtual std::shared_ptr<DeterministicCommand> CreateInstance() const = 0;
};

// Photon.Deterministic.DeterministicCommandPool<T> (shared generic code; bodies studied at
// 0x71c9514..0x71c96a4).
template <typename T>
class DeterministicCommandPool final : public IDeterministicCommandPool,
                                       public IDeterministicCommandFactory {
  public:
    DeterministicCommandPool() = default;

    std::type_index CommandType() const override { return typeid(T); }

    std::shared_ptr<DeterministicCommand> GetCommandInstance() override { return Acquire(); }

    std::shared_ptr<DeterministicCommand> Acquire() override {
        std::shared_ptr<T> cmd;
        if (!pool_.empty()) {
            cmd = std::move(pool_.back());
            pool_.pop_back();
        } else {
            cmd = std::make_shared<T>();
        }
        cmd->Pool = this;
        cmd->Pooled = false;
        return cmd;
    }

    bool Release(const std::shared_ptr<DeterministicCommand>& cmd) override {
        if (!cmd || cmd->Pooled) {
            return false;
        }
        auto typed = std::dynamic_pointer_cast<T>(cmd);
        if (!typed) {
            return false;
        }
        typed->Pooled = true;
        pool_.push_back(std::move(typed));
        return true;
    }

  private:
    std::vector<std::shared_ptr<T>> pool_;
};

class DeterministicCommandSerializer {
  public:
    // Scratch streams of 8 KiB used to decode rpc commands and to pack outgoing ones.
    BitStream CommandSerializerStreamRead{0x2000};
    BitStream CommandSerializerStreamWrite{0x2000};

    // 0x7a78900
    DeterministicCommandSerializer() = default;

    // 0x7a78a98 - ids are handed out in registration order starting at 1; registering a
    // type twice keeps the first id.
    void RegisterFactory(std::shared_ptr<IDeterministicCommandFactory> factory);
    // 0x7a78d18
    void
    RegisterFactories(std::span<const std::shared_ptr<IDeterministicCommandFactory>> factories);
    // 0x7a78d84 - writes id + payload; on overflow rewinds the stream and returns false.
    // Throws std::out_of_range for an unregistered command type.
    bool PackNext(BitStream& s, DeterministicCommand& cmd);
    // 0x7a78f00 - reads one command; false (and cmd reset) if the stream is exhausted or
    // decoding throws.
    bool ReadNext(BitStream& s, std::shared_ptr<DeterministicCommand>& cmd);

  private:
    std::unordered_map<std::type_index, std::uint16_t> typeToId_;
    std::unordered_map<std::uint16_t, std::shared_ptr<IDeterministicCommandFactory>> idToFactory_;
};

} // namespace Photon::Deterministic
