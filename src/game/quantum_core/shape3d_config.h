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

    // TODO(0x7d3ba74, Quantum.Shape3DConfig.CreateShape): sphere/box shapes with the local
    // transform (PositionOffset, FPQuaternion.Euler(RotationOffset); a zero rotation keeps
    // the identity), or for Compound a persistent compound (Shape3D.CreatePersistentCompound
    // on the frame) filled with each CompoundShapes entry's shape; sets UserTag. Needs the
    // compound shape buffer on FrameBase.
};

} // namespace Quantum
