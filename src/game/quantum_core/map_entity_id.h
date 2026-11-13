// map_entity_id.h - Quantum.MapEntityId and the Quantum.MapEntityLink component.
//
// Entity prototypes baked into a map refer to each other by their index in the map's
// entity list (MapEntityId, stored one-based so the zero value is "none"). When the map's
// entities are created, each gets a MapEntityLink component holding its index.
#pragma once

#include <cstdint>
#include <string>

namespace Quantum {

class FrameSerializer;

struct MapEntityId {
    static constexpr int SIZE = 4;

    std::int32_t SceneIndexPlusOne = 0;

    // 0x7cf7a48
    static constexpr MapEntityId Invalid() { return MapEntityId{}; }
    // 0x7cf7a50
    static constexpr MapEntityId Create(int sceneIndex) { return MapEntityId{sceneIndex + 1}; }

    // 0x7cf7a2c
    constexpr int SceneIndex() const { return SceneIndexPlusOne - 1; }
    // 0x7cf7a38
    constexpr bool IsValid() const { return SceneIndexPlusOne > 0; }

    // 0x7cf7b88 (Equals(MapEntityId)); 0x7cf7ba0 is the boxed Equals(object).
    constexpr bool Equals(MapEntityId other) const {
        return SceneIndexPlusOne == other.SceneIndexPlusOne;
    }
    friend constexpr bool operator==(MapEntityId a, MapEntityId b) { return a.Equals(b); }

    // 0x7cf7b98
    constexpr std::int32_t GetHashCode() const { return SceneIndexPlusOne; }

    // 0x7cf7b0c - the scene index as a decimal number.
    std::string ToString() const { return std::to_string(SceneIndex()); }

    // 0x7cf7a58 - the raw one-based value as a 32-bit integer.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

static_assert(sizeof(MapEntityId) == MapEntityId::SIZE);

// Component: the index of a map entity in its map's entity list.
struct MapEntityLink {
    static constexpr int SIZE = 4;
    // Registered component name (see component_type_id.h).
    static constexpr const char* ComponentTypeName = "MapEntityLink";

    std::int32_t Index = 0;

    // 0x7d2b5c4 - written exactly like a MapEntityId.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

static_assert(sizeof(MapEntityLink) == MapEntityLink::SIZE);

} // namespace Quantum
