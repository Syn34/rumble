// input_types.cpp - QBoolean, PlayerRef, Button and ComponentPrototypeRef (quantum.core.dll).
#include "quantum_core/button.h"
#include "quantum_core/component_prototype_ref.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/hash_code_utils.h"
#include "quantum_core/player_ref.h"
#include "quantum_core/q_boolean.h"

#include "photon_deterministic/bit_stream.h"

namespace Quantum {

void QBoolean::Serialize(void* ptr, FrameSerializer* serializer) {
    serializer->Stream().Serialize(static_cast<QBoolean*>(ptr)->Value);
}

std::string PlayerRef::ToString() const {
    if (!IsValid()) {
        return "PlayerRef(none)";
    }
    return "PlayerRef(" + std::to_string(ToIndex()) + ")";
}

void PlayerRef::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* player = static_cast<PlayerRef*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    if (serializer->Writing()) {
        stream.WriteInt(player->_index);
    } else {
        player->_index = stream.ReadInt();
    }
}

Button Button::Update(int currentFrame, bool isDown) {
    _frameCurrent = currentFrame;
    if (isDown) {
        // Pressed now, or the very first press of a button that has never changed.
        if (_frameUp > _frameDown || (_frameUp | _frameDown) == 0) {
            _frameDown = currentFrame;
        }
    } else if (_frameDown > _frameUp) {
        _frameUp = currentFrame;
    }
    return *this;
}

std::int32_t Button::GetHashCode() const {
    std::uint32_t hash = 17u;
    hash = hash * 31u + static_cast<std::uint32_t>(_frameUp);
    hash = hash * 31u + static_cast<std::uint32_t>(_frameDown);
    hash = hash * 31u + static_cast<std::uint32_t>(_frameCurrent);
    return static_cast<std::int32_t>(hash);
}

std::string Button::ToString() const {
    return std::string("Button(") + (IsDown() ? "down" : "up") +
           ", up=" + std::to_string(_frameUp) + ", down=" + std::to_string(_frameDown) +
           ", current=" + std::to_string(_frameCurrent) + ")";
}

void Button::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* button = static_cast<Button*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    if (serializer->InputMode()) {
        if (serializer->Writing()) {
            stream.WriteBoolean(button->IsDown());
        } else {
            *button = FromBool(stream.ReadBoolean());
        }
        return;
    }
    if (serializer->Writing()) {
        stream.WriteInt(button->_frameUp);
        stream.WriteInt(button->_frameDown);
        stream.WriteInt(button->_frameCurrent);
    } else {
        button->_frameUp = stream.ReadInt();
        button->_frameDown = stream.ReadInt();
        button->_frameCurrent = stream.ReadInt();
    }
}

std::int32_t ComponentPrototypeRef::GetHashCode() const {
    return HashCodeUtils::CombineHashCodes(Asset.Id.GetHashCode(), InnerAssetId,
                                           ComponentTypeIndex);
}

std::string ComponentPrototypeRef::ToString() const {
    return "ComponentPrototypeRef(" + Asset.Id.ToString(false) + ", " +
           std::to_string(InnerAssetId) + ", " + std::to_string(ComponentTypeIndex) + ")";
}

void ComponentPrototypeRef::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* ref = static_cast<ComponentPrototypeRef*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    stream.Serialize(ref->Asset.Id.Value);
    stream.Serialize(&ref->InnerAssetId);
    stream.Serialize(&ref->ComponentTypeIndex);
}

} // namespace Quantum
