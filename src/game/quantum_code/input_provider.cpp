// input_provider.cpp - Quantum.InputProvider and InputProviderExtensions.
#include "quantum_code/input_provider.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace Quantum {

using Photon::Deterministic::DeterministicFrameInputTemp;
using Photon::Deterministic::DeterministicInputFlags;
using Photon::Deterministic::DeterministicTickInput;
using Photon::Deterministic::DeterministicTickInputSet;

namespace InputProviderExtensions {

void Clear(DeterministicTickInput& input) {
    input.Tick = 0;
    input.DataArray.reset();
    input.Rpc.reset();
    // The original also resets the two state fields at the start of the object; mapped to
    // the sent and pooled flags (uncertain which state the second one is).
    input.Sent = false;
    input.Pooled = false;
}

void Set(DeterministicTickInput& input, const DeterministicFrameInputTemp& value) {
    input.Tick = value.Frame;
    input.PlayerIndex = value.Player;
    input.DataArray = value.CloneData();
    input.DataLength = value.DataLength;
    input.Flags = value.Flags;
    input.Rpc = value.Rpc;
}

bool IsComplete(const DeterministicTickInputSet& set) {
    for (const auto& input : set.Inputs) {
        if (!input) {
            throw std::logic_error("InputProvider: missing input object");
        }
        if (input->Tick == 0) {
            return false;
        }
    }
    return true;
}

} // namespace InputProviderExtensions

InputProvider::InputProvider(const Photon::Deterministic::DeterministicSessionConfig& config,
                             int capacity, int growSize)
    : InputProvider(config.PlayerCount, config.RollbackWindow, capacity, growSize) {}

InputProvider::InputProvider(std::vector<DeterministicTickInputSet> inputList) {
    ImportFromList(std::move(inputList));
}

InputProvider::InputProvider(int playerCount, int startFrame, int capacity, int growSize)
    : _playerCount(playerCount), _growSize(growSize), _startFrame(startFrame) {
    if (capacity > 0) {
        Allocate(capacity);
    }
}

void InputProvider::Clear(int startFrame) {
    _startFrame = startFrame;
    for (std::size_t i = 0; i < _inputs.size(); ++i) {
        _inputs[i].Tick = startFrame + static_cast<int>(i);
        for (int player = 0; player < _playerCount; ++player) {
            InputProviderExtensions::Clear(*_inputs[i].Inputs.at(static_cast<std::size_t>(player)));
        }
    }
}

void InputProvider::ImportFromList(std::vector<DeterministicTickInputSet> inputList) {
    _startFrame = inputList.empty() ? 0 : inputList.front().Tick;
    _inputs = std::move(inputList);
    for (auto& set : _inputs) {
        for (auto& input : set.Inputs) {
            if (!input) {
                throw std::logic_error("InputProvider: missing input object");
            }
            input->Sent = true;
        }
    }
}

void InputProvider::OnInputConfirmed(QuantumGame* game, const DeterministicFrameInputTemp& input) {
    (void)game;
    const int index = input.Frame - _startFrame;
    if (index < 0) {
        return;
    }
    if (input.Frame >= get_MaxFrame()) {
        const int needed = std::max(index, static_cast<int>(_inputs.size()));
        Allocate(_growSize > 0 ? needed + _growSize : needed * 2);
    }
    DeterministicTickInputSet& set = _inputs.at(static_cast<std::size_t>(input.Frame - _startFrame));
    InputProviderExtensions::Set(*set.Inputs.at(static_cast<std::size_t>(input.Player)), input);
}

bool InputProvider::CanSimulate(int frame) {
    const int index = frame - _startFrame;
    if (index < 0 || index >= static_cast<int>(_inputs.size())) {
        return false;
    }
    return InputProviderExtensions::IsComplete(_inputs[static_cast<std::size_t>(index)]);
}

Photon::Deterministic::QTuple<Photon::Deterministic::ByteArray, bool> InputProvider::GetRpc(int frame,
                                                                                            int player) {
    if (frame >= get_MaxFrame()) {
        return {};
    }
    const auto& input = *_inputs.at(static_cast<std::size_t>(frame - _startFrame))
                             .Inputs.at(static_cast<std::size_t>(player));
    const bool command = (static_cast<std::uint8_t>(input.Flags) & 0x08) != 0;
    return {input.Rpc, command};
}

DeterministicFrameInputTemp InputProvider::GetInput(int frame, int player) {
    if (frame >= get_MaxFrame()) {
        return DeterministicFrameInputTemp{};
    }
    const auto& input = *_inputs.at(static_cast<std::size_t>(frame - _startFrame))
                             .Inputs.at(static_cast<std::size_t>(player));
    return DeterministicFrameInputTemp::Verified(frame, player, nullptr, input.DataArray,
                                                 input.DataLength, input.Flags);
}

void InputProvider::Allocate(int size) {
    const std::size_t oldSize = _inputs.size();
    _inputs.resize(static_cast<std::size_t>(size));
    for (std::size_t i = oldSize; i < _inputs.size(); ++i) {
        DeterministicTickInputSet& set = _inputs[i];
        set.Tick = _startFrame + static_cast<int>(i);
        set.Inputs.assign(static_cast<std::size_t>(_playerCount), nullptr);
        for (auto& input : set.Inputs) {
            input = std::make_shared<DeterministicTickInput>();
        }
    }
}

} // namespace Quantum
