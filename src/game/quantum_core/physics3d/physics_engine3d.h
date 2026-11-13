// physics_engine3d.h - Quantum.Physics3D.PhysicsEngine3D: the frame context's 3D physics
// engine. It owns the scene, runs a step per frame and answers immediate queries.
#pragma once

#include "quantum_core/physics3d/scene3d.h"

#include <memory>

namespace Quantum::Core {
class FrameContext;
}

namespace Quantum::Physics3D {

class PhysicsEngine3D {
  public:
    // 0x7d48e84 - allocates the scene (Scene3D.Allocate).
    explicit PhysicsEngine3D(Core::FrameContext* context);
    // 0x7d48f24
    void Dispose() { _scene3D.reset(); }

    // 0x7d48924 - the scene's first map setup.
    void Init(Core::FrameBase* frame, AssetGuid mapGuid) { _scene3D->InitScene(frame, mapGuid); }
    // 0x7d48930 - one step with the given callbacks (null: no callbacks).
    void Update(Core::FrameBase* frame, ICollisionCallbacks3D* callbacks, FP dt);
    // 0x7d48f68 - the original resets the scene here and records whether the frame is
    // predicted (for the triangle mesh); Update resets it again, which is harmless.
    void OnFrameSimulationBegin(Core::FrameBase* frame) { _scene3D->ResetScene(frame); }
    // 0x7d48fb8 - only marks the (unported) scene mesh as predicted.
    void OnFrameSimulationEnd() {}

    // 0x7d48b80 / 0x7d48c4c - a temp hit collection with the segment's hits.
    void Linecast(Core::FrameBase* frame, HitCollection3D* collection, const FPVector3& start,
                  const FPVector3& end, bool firstHitOnly, std::int32_t layerMask,
                  std::int32_t options);
    // 0x7d48d90 / 0x7d48e5c
    void OverlapShape(Core::FrameBase* frame, HitCollection3D* collection, const FPVector3& pos,
                      const FPQuaternion& rot, const Shape3D& shape, std::int32_t layerMask,
                      std::int32_t options);

    // 0x7d48c6c / 0x7d48d54 - `shape` from `start` moved by `translation`.
    void ShapeCast(Core::FrameBase* frame, HitCollection3D* collection, const FPVector3& start,
                   const FPQuaternion& rot, const Shape3D& shape, const FPVector3& translation,
                   bool firstHitOnly, std::int32_t layerMask, std::int32_t options);

    Scene3D* Scene() { return _scene3D.get(); }
    Core::FrameContext* Context() const { return _context; }

    // 0x7d487c8 - no flags removes the entity's PhysicsCallbacks3D; otherwise adds it when
    // missing and, if the flags change, stores them and forgets the tracked collisions.
    static void SetCallbacks(Core::FrameBase* frame, EntityRef entity, CallbackFlags flags);

    // TODO(0x7d48988, Quantum.Physics3D.PhysicsEngine3D.AllocatePersistentHitCollection3D),
    // TODO(0x7d48a04, Quantum.Physics3D.PhysicsEngine3D.GetQueryHits),
    // TODO(0x7d48afc, Quantum.Physics3D.PhysicsEngine3D.TryGetQueryHits),
    // TODO(0x7d48b50, Quantum.Physics3D.PhysicsEngine3D.AddLinecastQuery),
    // TODO(0x7d48d84, Quantum.Physics3D.PhysicsEngine3D.AddOverlapShapeQuery): broad-phase
    // queries resolved during the step.
    // TODO(0x7d48e7c, Quantum.Physics3D.PhysicsEngine3D.get_SceneMesh): needs TriangleMesh.
    // TODO(0x7d48fd4, Quantum.Physics3D.PhysicsEngine3D.RegisterInternalTypes)

  private:
    std::unique_ptr<Scene3D> _scene3D;
    Core::FrameContext* _context = nullptr;
};

} // namespace Quantum::Physics3D
