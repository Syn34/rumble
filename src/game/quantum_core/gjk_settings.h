// gjk_settings.h - Quantum.GJKSettings and Quantum.CCDSettings: the tuning values of the
// physics engine's GJK queries (shape casts) and continuous collision detection, derived
// from the engine configuration when the simulation starts.
#pragma once

#include "photon_deterministic/fp.h"

#include <cstdint>

namespace Quantum {

using Photon::Deterministic::FP;

// The fields of the physics engine configuration (the original's nested Config class of
// the physics settings, not reimplemented here) that the two settings structs read.
struct PhysicsQueryConfig {
    std::int32_t SimplexMinBitShift = 0;
    std::int32_t SimplexMaxBitShift = 0;
    std::int32_t ShapeCastMaxIterations = 0;
    FP ShapeCastHardTolerance;
    FP ShapeCastSoftTolerance;
    bool AllowCCD = false;
    FP CCDLinearVelocityThreshold;
    FP CCDAngularVelocityThreshold;
    FP CCDDistanceTolerance;
    std::int32_t MaxTimeOfImpactIterations = 0;
    std::int32_t MaxRootFindingIterations = 0;
};

struct GJKSettings {
    static constexpr int SIZE = 24;

    // Extra precision bits for simplex points: GJK starts at the minimum and doubles the
    // simplex (one more bit) when it becomes degenerate, up to the maximum.
    std::int16_t _simplexMinBitShift = 0;
    std::int16_t _simplexMaxBitShift = 0;
    std::int32_t _shapeCastMaxIterations = 0;
    FP _shapeCastHardTolerance;
    FP _shapeCastSoftTolerance;

    GJKSettings() = default;
    // 0x7d2e53c - clamps: at least 4 iterations, hard tolerance at least raw 6, soft
    // tolerance at least the hard one, shifts non-negative with max >= min.
    explicit GJKSettings(const PhysicsQueryConfig& config);

    std::int32_t SimplexMinBitShift() const { return _simplexMinBitShift; }
    std::int32_t SimplexMaxBitShift() const { return _simplexMaxBitShift; }
    FP ShapeCastHardTolerance() const { return _shapeCastHardTolerance; }
    FP ShapeCastSoftTolerance() const { return _shapeCastSoftTolerance; }

    // 0x7d2e6f8
    void SetSimplexMinBitShift(std::int32_t value);
    // 0x7d2e768 - never below the minimum.
    void SetSimplexMaxBitShift(std::int32_t value);
    // 0x7d2e7dc
    std::int32_t ShapeCastMaxIterations() const { return _shapeCastMaxIterations; }
    // 0x7d2e7e4 - at least 4.
    void SetShapeCastMaxIterations(std::int32_t value);
    // 0x7d2e854 - at least raw 6.
    void SetShapeCastHardTolerance(FP value);
    // 0x7d2e8c4 - at least the hard tolerance.
    void SetShapeCastSoftTolerance(FP value);

    // TODO(0x7d2e938, Quantum.GJKSettings.Serialize): the five fields through the frame
    // serializer's stream (FrameSerializer is not reimplemented yet).
};
static_assert(sizeof(GJKSettings) == GJKSettings::SIZE);

struct CCDSettings {
    static constexpr int SIZE = 40;

    std::int32_t _allowCCD = 0;
    std::int32_t MaxTimeOfImpactIterations = 0;
    std::int32_t MaxRootFindingIterations = 0;
    std::int32_t _padding = 0;
    FP CCDDistanceTolerance;
    FP _linearVelocityThresholdSqr;
    FP _angularVelocityThresholdSqr;

    CCDSettings() = default;
    // 0x7d2e10c - thresholds are stored squared (negative ones count as zero).
    explicit CCDSettings(const PhysicsQueryConfig& config);

    // 0x7d2e20c
    bool AllowCCD() const { return _allowCCD == 1; }
    // 0x7d2e21c
    void SetLinearVelocityThreshold(FP value);
    // 0x7d2e294
    FP AngularVelocityThreshold() const;
    // 0x7d2e2a0
    void SetAngularVelocityThreshold(FP value);

    // TODO(0x7d2e318, Quantum.CCDSettings.Serialize): needs FrameSerializer.
};
static_assert(sizeof(CCDSettings) == CCDSettings::SIZE);

} // namespace Quantum
