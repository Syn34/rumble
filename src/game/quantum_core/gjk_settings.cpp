// gjk_settings.cpp - Quantum.GJKSettings and Quantum.CCDSettings.
#include "gjk_settings.h"

#include "photon_deterministic/fp_math.h"

#include <algorithm>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

namespace {

constexpr std::int32_t MinShapeCastIterations = 4;
constexpr FP MinHardTolerance = FP::FromRaw(6);

// Math.Max(0, value) on the raw values, squared.
FP SquaredNonNegative(FP value) {
    const FP clamped = FP::FromRaw(std::max<FP::Raw>(0, value.RawValue()));
    return clamped * clamped;
}

} // namespace

// 0x7d2e53c
GJKSettings::GJKSettings(const PhysicsQueryConfig& config) {
    _shapeCastMaxIterations = std::max(MinShapeCastIterations, config.ShapeCastMaxIterations);
    _shapeCastHardTolerance = std::max(MinHardTolerance, config.ShapeCastHardTolerance);
    _shapeCastSoftTolerance = std::max(_shapeCastHardTolerance, config.ShapeCastSoftTolerance);
    _simplexMinBitShift = static_cast<std::int16_t>(std::max(config.SimplexMinBitShift, 0));
    _simplexMaxBitShift =
        std::max(static_cast<std::int16_t>(config.SimplexMaxBitShift), _simplexMinBitShift);
}

// 0x7d2e6f8
void GJKSettings::SetSimplexMinBitShift(std::int32_t value) {
    _simplexMinBitShift = static_cast<std::int16_t>(std::max(value, 0));
}

// 0x7d2e768
void GJKSettings::SetSimplexMaxBitShift(std::int32_t value) {
    _simplexMaxBitShift = std::max(static_cast<std::int16_t>(value), _simplexMinBitShift);
}

// 0x7d2e7e4
void GJKSettings::SetShapeCastMaxIterations(std::int32_t value) {
    _shapeCastMaxIterations = std::max(MinShapeCastIterations, value);
}

// 0x7d2e854
void GJKSettings::SetShapeCastHardTolerance(FP value) {
    _shapeCastHardTolerance = std::max(MinHardTolerance, value);
}

// 0x7d2e8c4
void GJKSettings::SetShapeCastSoftTolerance(FP value) {
    _shapeCastSoftTolerance = std::max(_shapeCastHardTolerance, value);
}

// 0x7d2e10c
CCDSettings::CCDSettings(const PhysicsQueryConfig& config)
    : _allowCCD(config.AllowCCD ? 1 : 0),
      MaxTimeOfImpactIterations(config.MaxTimeOfImpactIterations),
      MaxRootFindingIterations(config.MaxRootFindingIterations),
      _padding(0),
      CCDDistanceTolerance(config.CCDDistanceTolerance),
      _linearVelocityThresholdSqr(SquaredNonNegative(config.CCDLinearVelocityThreshold)),
      _angularVelocityThresholdSqr(SquaredNonNegative(config.CCDAngularVelocityThreshold)) {}

// 0x7d2e21c
void CCDSettings::SetLinearVelocityThreshold(FP value) {
    _linearVelocityThresholdSqr = SquaredNonNegative(value);
}

// 0x7d2e294
FP CCDSettings::AngularVelocityThreshold() const {
    return FPMath::Sqrt(_angularVelocityThresholdSqr);
}

// 0x7d2e2a0
void CCDSettings::SetAngularVelocityThreshold(FP value) {
    _angularVelocityThresholdSqr = SquaredNonNegative(value);
}

} // namespace Quantum
