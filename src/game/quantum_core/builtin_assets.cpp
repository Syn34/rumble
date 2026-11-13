// Original: Quantum.BinaryData, Quantum.PhysicsMaterial, Quantum.CharacterController2DConfig,
// Quantum.CharacterController3DConfig, Quantum.TerrainCollider (quantum.core.dll).
#include "builtin_assets.h"

#include "photon_deterministic/byte_utils.h"
#include "photon_deterministic/fp_math.h"
#include "quantum_core/triangle_ccw.h"

#include <stdexcept>
#include <string>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

std::unique_ptr<BinaryDataStream> BinaryData::CreateByteStream() const {
    return std::make_unique<BinaryDataStream>(
        IsCompressed ? Photon::Deterministic::ByteUtils::GZipDecompressBytes(Data) : Data);
}

void PhysicsMaterial::Loaded(IResourceManager* /*resourceManager*/, Allocator* /*allocator*/) {
    FrictionStatic = FPMath::Clamp01(FrictionStatic);
    FrictionDynamic = FPMath::Clamp01(FrictionDynamic);
}

void CharacterController2DConfig::Loaded(IResourceManager* /*resourceManager*/,
                                         Allocator* /*allocator*/) {
    const std::int64_t x = Gravity.X.RawValue();
    const std::int64_t y = Gravity.Y.RawValue();
    _gravityStrength = FP::FromRaw(FPMath::SqrtRaw(((x * x) >> 16) + ((y * y) >> 16)));
    _gravityNormalized = FPVector2::Normalize(Gravity);
}

void CharacterController3DConfig::Loaded(IResourceManager* /*resourceManager*/,
                                         Allocator* /*allocator*/) {
    const std::int64_t x = Gravity.X.RawValue();
    const std::int64_t y = Gravity.Y.RawValue();
    const std::int64_t z = Gravity.Z.RawValue();
    _gravityStrength =
        FP::FromRaw(FPMath::SqrtRaw(((x * x) >> 16) + ((y * y) >> 16) + ((z * z) >> 16)));
    _gravityNormalized = FPVector3::Normalize(Gravity);
}

const std::vector<TriangleCCW>& TerrainCollider::Triangles() const {
    static const std::vector<TriangleCCW> none;
    return _triangles ? *_triangles : none;
}

bool TerrainCollider::IsHole(int x, int y) const {
    if (HoleMask.empty()) {
        return false;
    }
    const std::int32_t index = Resolution * x + y;
    if (index < 0 || static_cast<std::size_t>(index) >= HoleMask.size() * HoleMaskBitsPerBlock) {
        return false;
    }
    return ((HoleMask[static_cast<std::size_t>(index) / HoleMaskBitsPerBlock] >>
             (index % HoleMaskBitsPerBlock)) &
            1u) != 0;
}

void TerrainCollider::SetHole(int x, int y) {
    const std::int32_t index = Resolution * x + y;
    if (index < 0 || static_cast<std::size_t>(index) >= HoleMask.size() * HoleMaskBitsPerBlock) {
        throw std::out_of_range("terrain hole cell (" + std::to_string(x) + ", " +
                                std::to_string(y) + ") is outside the hole mask (resolution " +
                                std::to_string(Resolution) + ", " +
                                std::to_string(HoleMask.size() * HoleMaskBitsPerBlock) +
                                " bits)");
    }
    HoleMask[static_cast<std::size_t>(index) / HoleMaskBitsPerBlock] |=
        std::uint64_t{1} << (index % HoleMaskBitsPerBlock);
}

} // namespace Quantum
