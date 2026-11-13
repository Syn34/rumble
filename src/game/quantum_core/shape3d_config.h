// shape3d_config.h - Quantum.Shape3DConfig (quantum.core.dll): the editor description of a
// 3D collision shape (type, size, local offset and rotation), turned into a Shape3D when
// an entity is created from it.
#pragma once

#include "quantum_core/shape3d.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>
#include <vector>

namespace Quantum {

namespace Core {
class FrameBase;
}

class Shape3DConfig {
  public:
    using FP = Photon::Deterministic::FP;
    using FPVector3 = Photon::Deterministic::FPVector3;

    // One part of a compound shape.
    struct CompoundShapeData3D {
        Shape3DType ShapeType = Shape3DType::None;
        FP SphereRadius;
        FPVector3 PositionOffset;
        FPVector3 BoxExtents;
        FPVector3 RotationOffset; // Euler angles in degrees
        std::uint16_t UserTag = 0;
        bool IsSetFromSourceCollider = false;

        // TODO(0x7d3be00, Quantum.Shape3DConfig.CompoundShapeData3D.CreateShape): builds a
        // sphere or box Shape3D with the local transform from PositionOffset and
        // FPQuaternion.Euler(RotationOffset), and sets UserTag.
    };

    Shape3DType ShapeType = Shape3DType::None;
    FP SphereRadius;
    FPVector3 PositionOffset;
    FPVector3 BoxExtents;
    FPVector3 RotationOffset; // Euler angles in degrees
    std::uint16_t UserTag = 0;
    bool IsPersistent = true;
    std::vector<CompoundShapeData3D> CompoundShapes;
    bool IsSetFromSourceCollider = false;

    // 0x7d3c0e4 - compound shapes are persistent by default.
    Shape3DConfig() = default;

    // 0x7d3ba74 - a sphere (offset by PositionOffset) or a box (offset and rotated by
    // FPQuaternion.Euler(RotationOffset), identity for a zero rotation); any other type
    // gives an empty shape. UserTag is set on the result. Compound configs throw
    // std::logic_error: they need a persistent compound on the frame heap
    // (TODO(0x7d35960, Quantum.Shape3D.CreatePersistentCompound) and
    // CompoundShape3D.AddShape).
    Shape3D CreateShape(Core::FrameBase* frame) const;
};

} // namespace Quantum
