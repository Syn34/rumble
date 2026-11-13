// physics_engine3d.cpp - Quantum.Physics3D.PhysicsEngine3D.
#include "physics_engine3d.h"

#include "quantum_core/frame_base.h"

#include <stdexcept>
#include <string>

namespace Quantum::Physics3D {

PhysicsEngine3D::PhysicsEngine3D(Core::FrameContext* context)
    : _scene3D(std::make_unique<Scene3D>()), _sceneMesh(std::make_unique<TriangleMesh>()), _context(context) {
    _scene3D->SetSceneMesh(_sceneMesh.get());
}

void PhysicsEngine3D::OnFrameSimulationBegin(Core::FrameBase* frame) {
    _sceneMesh->SetPredicted(frame->IsPredicted());
    _scene3D->ResetScene(frame);
}

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

void PhysicsEngine3D::ShapeCast(Core::FrameBase* frame, HitCollection3D* collection, const FPVector3& start,
                                const FPQuaternion& rot, const Shape3D& shape, const FPVector3& translation,
                                bool firstHitOnly, std::int32_t layerMask, std::int32_t options) {
    _scene3D->ShapeCast(frame, collection, start, rot, shape, translation, firstHitOnly, layerMask, options);
}

HitCollection3D PhysicsEngine3D::GetQueryHits(std::int32_t index) const {
    if (index < 0 || index >= _scene3D->HitCollectionsCount()) {
        throw std::out_of_range("query hits index " + std::to_string(index) + " out of range (" +
                                std::to_string(_scene3D->HitCollectionsCount()) + ")");
    }
    return *_scene3D->GetHitCollection(index);
}

bool PhysicsEngine3D::TryGetQueryHits(std::int32_t index, HitCollection3D& hits) const {
    if (index < 0 || index >= _scene3D->HitCollectionsCount()) {
        hits = HitCollection3D{};
        return false;
    }
    hits = *_scene3D->GetHitCollection(index);
    return true;
}

// 0x7d487c8
void PhysicsEngine3D::SetCallbacks(Core::FrameBase* frame, EntityRef entity, CallbackFlags flags) {
    if (frame->Has<PhysicsCallbacks3D>(entity)) {
        if (flags == CallbackFlags::None) {
            frame->Remove<PhysicsCallbacks3D>(entity);
            return;
        }
    } else {
        if (flags == CallbackFlags::None) {
            return;
        }
        frame->Add<PhysicsCallbacks3D>(entity);
    }
    PhysicsCallbacks3D* callbacks = frame->GetPointer<PhysicsCallbacks3D>(entity);
    if (callbacks != nullptr && callbacks->Flags != flags) {
        callbacks->Flags = flags;
        callbacks->_listCount = 0;
    }
}

} // namespace Quantum::Physics3D
