// physics_scene_settings.h - Quantum.PhysicsSceneSettings: the per-frame physics settings the
// scenes cache at the start of every step (solver, layers, tolerances, gravity, GJK/CCD).
// The layout is the original's (296 bytes); the scenes copy it byte for byte.
#pragma once

#include "quantum_core/gjk_settings.h"
#include "quantum_core/transform.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstddef>
#include <cstdint>

namespace Quantum {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;

// Quantum.ConstraintsSolverType
enum class ConstraintsSolverType : std::uint8_t {
    HybridJacobiGaussSeidel = 1,
    LegacyGaussSeidel = 2,
};

struct PhysicsSceneSettings {
    static constexpr int LAYER_START = 8;
    static constexpr int LAYER_COUNT = 32;
    static constexpr int FIRST_FP = 136;
    static constexpr int SIZE = 296;

    // PhysicsSceneSettingsFlags
    enum Flags : std::uint8_t {
        AllowSleeping = 1,
        UseVerticalTransform = 2,
        UseAngularVelocity = 4,
        UseJoints = 8,
    };

    std::uint8_t _flags = 0;
    ConstraintsSolverType SolverType = ConstraintsSolverType::HybridJacobiGaussSeidel;
    std::int16_t _padding = 0;
    std::uint16_t SolverIterations = 0;
    std::uint16_t PositionSolverIterations = 0;
    // Layers[i] bit j set: layer i collides with layer j.
    std::int32_t Layers[LAYER_COUNT] = {};
    FP PenetrationAllowance;
    FP PenetrationCorrection;
    FP MinLinearIntegration;
    FP MinAngularIntegration;
    FP SleepTimeSec;
    FP LinearSleepTolerance;
    FP AngularSleepToleranceInRad;
    FPVector3 Gravity;
    Transform2DVertical DefaultVerticalTransform;
    GJKSettings GJKSettingsValue; // GJKSettings in the original
    CCDSettings CCDSettingsValue; // CCDSettings in the original

    // 0x7d3c26c
    bool AllowSleepingEnabled() const { return (_flags & AllowSleeping) != 0; }
    // 0x7d3c278
    void SetAllowSleeping(bool value) { SetFlag(AllowSleeping, value); }
    // 0x7d3c28c
    void SetUseVerticalTransform(bool value) { SetFlag(UseVerticalTransform, value); }
    // 0x7d3c2ac
    void SetUseAngularVelocity(bool value) { SetFlag(UseAngularVelocity, value); }
    bool UseAngularVelocityEnabled() const { return (_flags & UseAngularVelocity) != 0; }
    // 0x7d3c2cc
    bool UseJointsEnabled() const { return (_flags & UseJoints) != 0; }
    // 0x7d3c2d8
    void SetUseJoints(bool value) { SetFlag(UseJoints, value); }

    bool LayersCollide(std::int32_t a, std::int32_t b) const { return ((Layers[a] >> b) & 1) != 0; }

    // TODO(0x7d3c2f8, Quantum.PhysicsSceneSettings.Init): copies the values of a PhysicsConfig
    // asset (not reimplemented yet) and builds the GJK/CCD settings and layer matrix.
    // TODO(0x7d3c4c4, Quantum.PhysicsSceneSettings.InitLayers): layer matrix from the config.
    // TODO(0x7d3c514, Quantum.PhysicsSceneSettings.Serialize)

    void SetFlag(Flags flag, bool value) {
        _flags = static_cast<std::uint8_t>(value ? (_flags | flag) : (_flags & ~flag));
    }

    // Test/setup helper (not in the original): every layer collides with every layer.
    void SetAllLayersColliding() {
        for (std::int32_t& row : Layers) {
            row = -1;
        }
    }
};
static_assert(sizeof(PhysicsSceneSettings) == PhysicsSceneSettings::SIZE);
static_assert(offsetof(PhysicsSceneSettings, PenetrationAllowance) == PhysicsSceneSettings::FIRST_FP);
static_assert(offsetof(PhysicsSceneSettings, GJKSettingsValue) == 0xe8);

} // namespace Quantum
