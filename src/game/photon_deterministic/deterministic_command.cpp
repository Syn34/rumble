#include "deterministic_command.h"

#include <exception>
#include <stdexcept>

namespace Photon::Deterministic {

void DeterministicCommand::Serialize(BitStream& stream, DeterministicCommandSerializer&) {
    Serialize(stream);
}

std::type_index DeterministicCommand::CommandType() const {
    return typeid(*this);
}

std::shared_ptr<DeterministicCommand> DeterministicCommand::GetCommandInstance() {
    if (Pool != nullptr) {
        return Pool->Acquire();
    }
    return CreateInstance();
}

void DeterministicCommand::Dispose() {
    if (Pool != nullptr) {
        Pool->Release(shared_from_this());
    }
}

void DeterministicCommandSerializer::RegisterFactory(
    std::shared_ptr<IDeterministicCommandFactory> factory) {
    if (!factory) {
        throw std::invalid_argument("DeterministicCommandSerializer: null factory");
    }
    // The original also skips factories whose CommandType is not a DeterministicCommand;
    // here a factory can only produce DeterministicCommand instances, so that always holds.
    const std::type_index type = factory->CommandType();
    if (typeToId_.contains(type)) {
        return;
    }
    const auto id = static_cast<std::uint16_t>(idToFactory_.size() + 1);
    idToFactory_.emplace(id, std::move(factory));
    typeToId_.emplace(type, id);
}

void DeterministicCommandSerializer::RegisterFactories(
    std::span<const std::shared_ptr<IDeterministicCommandFactory>> factories) {
    for (const auto& factory : factories) {
        RegisterFactory(factory);
    }
}

bool DeterministicCommandSerializer::PackNext(BitStream& s, DeterministicCommand& cmd) {
    const int start = s.Position();
    s.WriteUShort(typeToId_.at(cmd.CommandType()), 16);
    cmd.Serialize(s, *this);
    if (s.Overflowing()) {
        s.SetPosition(start);
        return false;
    }
    return true;
}

bool DeterministicCommandSerializer::ReadNext(BitStream& s,
                                              std::shared_ptr<DeterministicCommand>& cmd) {
    try {
        if (!s.CanRead(16)) {
            cmd.reset();
            return false;
        }
        const std::uint8_t low = s.InternalReadByte(8);
        const std::uint8_t high = s.InternalReadByte(8);
        const auto id = static_cast<std::uint16_t>(low | (high << 8));
        cmd = idToFactory_.at(id)->GetCommandInstance();
        cmd->Serialize(s, *this);
        return true;
    } catch (const std::exception&) {
        cmd.reset();
        return false;
    }
}

} // namespace Photon::Deterministic
