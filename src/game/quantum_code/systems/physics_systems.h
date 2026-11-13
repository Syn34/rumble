// physics_systems.h - systems around physics bodies and the kinematic character
// controller extension (KccExtension).
#pragma once

#include "quantum_code/systems/system_main_thread.h"
#include "quantum_core/physics_components.h"

namespace Quantum {

// Bodies that may sleep are created asleep.
class PhysicsSleepSystem final : public SystemSignalsOnly, public ISignalOnComponentAdded<PhysicsBody3D> {
  public:
    // 0x7bdf298
    PhysicsSleepSystem() = default;
    // 0x7bdf27c
    void OnInit(Frame* f) override { (void)f; }
    // 0x7bdf280
    void OnAdded(Frame* f, EntityRef entity, PhysicsBody3D* component) override;
};

struct KccSurfaceResetSystemFilter {
    EntityRef entity;
    KccExtension* kccExtension = nullptr;
    static constexpr auto FilterEntity = &KccSurfaceResetSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&KccSurfaceResetSystemFilter::kccExtension);
};

// Clears each frame which surface set the character's surface type, so surfaces can
// claim it again (see KccExtension).
class KccSurfaceResetSystem final : public SystemMainThreadFilter<KccSurfaceResetSystemFilter> {
  public:
    // 0x7bdd12c
    KccSurfaceResetSystem() = default;
    // 0x7bdd114
    void Update(Frame* f, Filter& filter) override;
};

} // namespace Quantum
