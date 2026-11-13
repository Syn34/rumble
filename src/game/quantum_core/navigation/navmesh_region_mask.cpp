#include "quantum_core/navigation/navmesh_region_mask.h"

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/deterministic_frame_serializer.h"
#include "quantum_core/byte_stream.h"

namespace Quantum {

// 0x7d30e2c
std::int32_t NavMeshRegionMask::GetHashCode() const {
    const auto low = static_cast<std::uint32_t>(_regions);
    const auto high = static_cast<std::uint32_t>(_regions >> 32);
    // The constant is what the code generator folded the hash seed into.
    return static_cast<std::int32_t>((low ^ high) + 0xDF5F1798u);
}

// 0x7d30e4c
std::string NavMeshRegionMask::ToString() const {
    return std::to_string(_regions);
}

// 0x7d30c80
void NavMeshRegionMask::Serialize(
    void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
    auto* mask = static_cast<NavMeshRegionMask*>(ptr);
    serializer->Stream().Serialize(&mask->_regions);
}

// 0x7d30e54
void NavMeshRegionMask::Serialize(ByteStream& stream, bool write) {
    _regions = ByteStreamExtension::Serialize(stream, write, _regions);
}

} // namespace Quantum
