// physics_engine3d.cpp - Quantum.Physics3D.PhysicsEngine3D.
#include "physics_engine3d.h"

#include "quantum_core/frame_base.h"

namespace Quantum::Physics3D {

PhysicsEngine3D::PhysicsEngine3D(Core::FrameContext* context)
    : _scene3D(std::make_unique<Scene3D>()), _context(context) {}

void PhysicsEngine3D::Update(Core::FrameBase* frame, ICollisionCallbacks3D* callbacks, FP dt) {
    _scene3D->Update(frame, dt, callbacks);
}

void PhysicsEngine3D::Linecast(Core::FrameBase* frame, HitCollection3D* collection,
                               const FPVector3& start, const FPVector3& end, bool firstHitOnly,
                               std::int32_t layerMask, std::int32_t options) {
    _scene3D->Linecast(frame, collection, start, end, firstHitOnly, layerMask, options);
}

void PhysicsEngine3D::OverlapShape(Core::FrameBase* frame, HitCollection3D* collection,
                                   const FPVector3& pos, const FPQuaternion& rot,
                                   const Shape3D& shape, std::int32_t layerMask,
                                   std::int32_t options) {
    _scene3D->OverlapShape(frame, collection, pos, rot, shape, layerMask, options);
}

} // namespace Quantum::Physics3D
