// physics_systems.cpp
#include "quantum_code/systems/physics_systems.h"

namespace Quantum {

void PhysicsSleepSystem::OnAdded(Frame* f, EntityRef entity, PhysicsBody3D* component) {
    (void)f;
    (void)entity;
    if (component->AllowSleeping()) {
        component->Sleep();
    }
}

void KccSurfaceResetSystem::Update(Frame* f, Filter& filter) {
    (void)f;
    filter.kccExtension->surfaceSetPriority = KccSurfaceSetPriority::None;
}

} // namespace Quantum
