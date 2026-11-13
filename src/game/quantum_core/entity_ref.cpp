#include "entity_ref.h"

#include "quantum_core/frame_serializer.h"

namespace Quantum {

void EntityRef::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* entity = static_cast<EntityRef*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    if (serializer->InputMode()) {
        if (stream.Condition(entity->Index != 0)) {
            stream.Serialize(&entity->Index);
            stream.Serialize(&entity->Version);
        } else if (serializer->Reading()) {
            *entity = None();
        }
        return;
    }
    stream.Serialize(&entity->Index);
    stream.Serialize(&entity->Version);
}

std::string EntityRef::ToString() const {
    std::string text = "Entity " + std::to_string(Index) + ":" +
                       std::to_string(Version & EntityInfo::VERSION_MASK);
    if (Version >= 0) {
        text += " (inactive)";
    }
    return text;
}

void EntityInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* info = static_cast<EntityInfo*>(ptr);
    EntityRef::Serialize(&info->Ref, serializer);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    if (serializer->Reading()) {
        info->Flags = static_cast<EntityFlags>(stream.ReadInt());
    } else {
        stream.WriteInt(static_cast<int>(info->Flags));
    }
}

} // namespace Quantum
