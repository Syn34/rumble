// frame.cpp - Quantum.Frame.
#include "quantum_code/frame.h"

#include "photon_deterministic/log.h"
#include "quantum_core/dynamic_asset_db.h"
#include "quantum_core/map.h"

#include <cstring>
#include <stdexcept>

namespace Quantum {

Frame::Frame(Core::FrameContext* context, std::vector<SystemBase*> systemsAll,
             std::vector<SystemBase*> systemsRoots,
             Photon::Deterministic::DeterministicSessionConfig* sessionConfig,
             RuntimeConfig* runtimeConfig, SimulationConfig* simulationConfig, FP deltaTime)
    : Core::FrameBase(context), Signals(this), Events(this), _runtimeConfig(runtimeConfig),
      _simulationConfig(simulationConfig), _sessionConfig(sessionConfig),
      _systemsAll(std::move(systemsAll)), _systemsRoots(std::move(systemsRoots)) {
    AllocGen();
    InitStaticGen();
    InitGen();
    for (std::size_t i = 0; i < _systemsAll.size(); ++i) {
        const SystemBase& system = *_systemsAll[i];
        _systemIndexByType.emplace(std::type_index(typeid(system)), static_cast<int>(i));
    }
    _globals->DeltaTime = deltaTime;
    // Not reimplemented yet: the Physics2D/3D API objects and the prototype-materialized
    // signal forwarder the original constructor also creates.
}

Frame::~Frame() = default;

void Frame::SetPlayerInput(int player, const ::Quantum::Input& input) {
    ::Quantum::Input* stored = GetPlayerInput(player);
    const int tick = Number;
    stored->Melee.Update(tick, input.Melee.IsDown());
    stored->Dash.Update(tick, input.Dash.IsDown());
    stored->Jump.Update(tick, input.Jump.IsDown());
    stored->Fire.Update(tick, input.Fire.IsDown());
    stored->Ultimate.Update(tick, input.Ultimate.IsDown());
    stored->Gadget.Update(tick, input.Gadget.IsDown());
    stored->Reload.Update(tick, input.Reload.IsDown());
    stored->EncodedDirection = input.EncodedDirection;
    stored->EncodedLookInputVector = input.EncodedLookInputVector;
    stored->EncodedCameraOrbitDirection = input.EncodedCameraOrbitDirection;
    stored->EncodedCameraOrbitOffset = input.EncodedCameraOrbitOffset;
    stored->EncodedCameraSight = input.EncodedCameraSight;
}

::Quantum::Input* Frame::GetPlayerInput(int player) {
    std::span<::Quantum::Input> inputs = _globals->get_input();
    if (static_cast<unsigned>(player) >= inputs.size()) {
        throw std::out_of_range("Frame::GetPlayerInput: player slot out of range");
    }
    return &inputs[static_cast<std::size_t>(player)];
}

Quantum::Map* Frame::get_Map() const {
    return const_cast<Frame*>(this)->FindAsset<Quantum::Map>(_globals->Map.Id);
}

FP Frame::get_TimeScale() const {
    if (_unscaledDeltaTime.RawValue() < 1) {
        return FP::One();
    }
    return FP::FromRaw((get_DeltaTime().RawValue() << 16) / _unscaledDeltaTime.RawValue());
}

void Frame::Copy(const Photon::Deterministic::DeterministicFrame& frame) {
    const auto& other = dynamic_cast<const Frame&>(frame);
    Quantum::Allocator::Heap::Copy(NativeAlloc(), HeapPtr(), other.HeapPtr());
    Core::FrameBase::Copy(this, &other);
    get_DynamicAssetDB()->CopyFrom(*other.get_DynamicAssetDB());
    CopyFromGen(&other);
}

void Frame::Free() {
    FreeGen();
    Core::FrameBase::Free();
}

std::vector<std::uint8_t> Frame::Serialize(Photon::Deterministic::DeterministicFrameSerializeMode mode) {
    (void)mode;
    throw std::logic_error("Frame::Serialize is not implemented yet");
}

void Frame::Deserialize(std::span<const std::uint8_t> data) {
    (void)data;
    throw std::logic_error("Frame::Deserialize is not implemented yet");
}

std::string Frame::DumpFrame(int dumpFlags) {
    (void)dumpFlags;
    throw std::logic_error("Frame::DumpFrame is not implemented yet");
}

std::uint64_t Frame::CalculateChecksum() {
    throw std::logic_error("Frame::CalculateChecksum is not implemented yet");
}

bool Frame::SystemIsEnabledSelf(const SystemBase* s) const {
    if (s == nullptr) {
        return false;
    }
    return _globals->Systems.IsSet(s->get_RuntimeIndex());
}

bool Frame::SystemIsEnabledInHierarchy(const SystemBase* system) const {
    for (; system != nullptr; system = system->get_ParentSystem()) {
        if (!_globals->Systems.IsSet(system->get_RuntimeIndex())) {
            return false;
        }
    }
    return true;
}

std::pair<SystemBase*, int> Frame::FindSystem(const std::type_info& type) const {
    auto it = _systemIndexByType.find(std::type_index(type));
    if (it == _systemIndexByType.end()) {
        Photon::Deterministic::Log::Error("Frame: no system of type {0}", type.name());
        return {nullptr, -1};
    }
    return {_systemsAll.at(static_cast<std::size_t>(it->second)), it->second};
}

void Frame::SystemEnable(const std::type_info& type) {
    auto [system, index] = FindSystem(type);
    if (system == nullptr || _globals->Systems.IsSet(index)) {
        return;
    }
    _globals->Systems.Set(index);
    if (system->get_ParentSystem() == nullptr || SystemIsEnabledInHierarchy(system->get_ParentSystem())) {
        system->OnEnabled(this);
    }
}

void Frame::SystemDisable(const std::type_info& type) {
    auto [system, index] = FindSystem(type);
    if (system == nullptr || !_globals->Systems.IsSet(index)) {
        return;
    }
    _globals->Systems.Clear(index);
    if (system->get_ParentSystem() == nullptr || SystemIsEnabledInHierarchy(system->get_ParentSystem())) {
        system->OnDisabled(this);
    }
}

void Frame::AddEvent(EventBase* evnt) {
    if (evnt == nullptr) {
        throw std::invalid_argument("Frame::AddEvent: null event");
    }
    evnt->Tick = Number;
    get_Context()->get_Events()->AddLast(evnt);
}

void Frame::InitGen() {
    Core::FrameBase::Initialize(this, _simulationConfig->Entities);
    BuildSignalArrays();
    BuildComponentSignals();
}

void Frame::AllocGen() {
    _globals = static_cast<_globals_*>(NativeAlloc()->AllocAndClear(static_cast<int>(sizeof(_globals_))));
}

void Frame::FreeGen() {
    NativeAlloc()->Free(_globals);
    _globals = nullptr;
}

void Frame::CopyFromGen(const Frame* frame) {
    std::memcpy(static_cast<void*>(_globals), frame->_globals, sizeof(_globals_));
}

} // namespace Quantum
