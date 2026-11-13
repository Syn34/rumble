// transform_systems.h - entities attached to a parent entity (Hierarchy) and the map
// change clean-up.
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

class HierarchyLib {
  public:
    // 0x7be3254 - Turns the child to face a world direction (expressed in the parent's
    // space).
    static void LookAt(Frame* f, Hierarchy* hierarchy, FPVector3 direction);
    // 0x7be337c - For an entity with both a Hierarchy and a Transform3D.
    static void UpdateHierarchyImmediately(Frame* f, EntityRef child);
    // 0x7be345c - Places the child at its offset in its parent's space with its local
    // rotation on top of the parent's (if the parent has a transform).
    static void UpdateHierarchyImmediately(Frame* f, Hierarchy* hierarchy, Transform3D* transform);
};

struct HierarchySystemFilter {
    EntityRef entity;
    Hierarchy* hierarchy = nullptr;
    Transform3D* transform = nullptr;
    static constexpr auto FilterEntity = &HierarchySystemFilter::entity;
    static constexpr auto FilterComponents =
        std::make_tuple(&HierarchySystemFilter::hierarchy, &HierarchySystemFilter::transform);
};

class HierarchySystem final : public SystemMainThreadFilter<HierarchySystemFilter>,
                              public ISignalOnComponentAdded<Hierarchy> {
  public:
    // 0x7be3658
    HierarchySystem() = default;
    // 0x7be35b4 - An unset (all-zero) local rotation becomes the identity.
    void OnAdded(Frame* f, EntityRef entity, Hierarchy* component) override;
    // 0x7be3648
    void Update(Frame* f, Filter& filter) override {
        HierarchyLib::UpdateHierarchyImmediately(f, filter.hierarchy, filter.transform);
    }
};

// Quantum.ISignalOnMapChanged (quantum.core): raised when the frame's map changes.
// Note: the framework side that raises it (FrameBase map change and Frame's receiver
// list) is not reimplemented yet.
class ISignalOnMapChanged : public virtual ISignal {
  public:
    virtual void OnMapChanged(Frame* f, AssetRefMap previousMap) = 0;
};

// Entities created at run time (FrameUtility.CreateDynamic) go with the map.
class AutoDestroyOnMapChangedSystem final : public SystemSignalsOnly, public ISignalOnMapChanged {
  public:
    // 0x7be1c50
    AutoDestroyOnMapChangedSystem() = default;
    // 0x7be1b40
    void OnMapChanged(Frame* f, AssetRefMap previousMap) override;
};

} // namespace Quantum
