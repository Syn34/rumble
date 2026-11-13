#include "component_set.h"

#include "quantum_core/frame_serializer.h"

namespace Quantum {

namespace {

// HashCodeUtils.UInt64HashCode (0x7d3d374): the two halves XORed.
std::uint32_t FoldHash(std::uint64_t value) {
    return static_cast<std::uint32_t>(value) ^ static_cast<std::uint32_t>(value >> 32);
}

} // namespace

int ComponentSet::GetHashCode() const {
    std::uint32_t hash = FoldHash(_set[0]);
    for (int i = 1; i < BLOCK_COUNT; ++i) {
        hash = (hash * 33u) ^ FoldHash(_set[i]);
    }
    return static_cast<int>(hash);
}

std::string ComponentSet::ToString() const {
    std::string names;
    for (int i = 1; i < ComponentTypeId<>::Count() && i < MAX_COMPONENTS; ++i) {
        if (!IsSet(i)) {
            continue;
        }
        if (!names.empty()) {
            names += ", ";
        }
        names += ComponentTypeId<>::GetComponentName(i);
    }
    return "{" + names + "}";
}

void ComponentSet::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* set = static_cast<ComponentSet*>(ptr);
    for (std::uint64_t& block : set->_set) {
        serializer->Stream().Serialize(&block);
    }
}

} // namespace Quantum
