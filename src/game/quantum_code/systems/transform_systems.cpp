// transform_systems.cpp
#include "quantum_code/systems/transform_systems.h"

namespace Quantum {

void HierarchyLib::LookAt(Frame* f, Hierarchy* hierarchy, FPVector3 direction) {
    Transform3D* parent = nullptr;
    if (f->TryGetPointer<Transform3D>(hierarchy->Parent, parent)) {
        hierarchy->LocalRotation = FPQuaternion::LookRotation(parent->InverseTransformDirection(direction), false);
    }
}

void HierarchyLib::UpdateHierarchyImmediately(Frame* f, EntityRef child) {
    Hierarchy* hierarchy = nullptr;
    Transform3D* transform = nullptr;
    if (f->TryGetPointer<Hierarchy>(child, hierarchy) && f->TryGetPointer<Transform3D>(child, transform)) {
        UpdateHierarchyImmediately(f, hierarchy, transform);
    }
}

void HierarchyLib::UpdateHierarchyImmediately(Frame* f, Hierarchy* hierarchy, Transform3D* transform) {
    Transform3D* parent = nullptr;
    if (!f->TryGetPointer<Transform3D>(hierarchy->Parent, parent)) {
        return;
    }
    transform->Position = parent->TransformPoint(hierarchy->Offset);
    transform->Rotation = FPQuaternion::Product(parent->Rotation, hierarchy->LocalRotation);
}

void HierarchySystem::OnAdded(Frame* f, EntityRef entity, Hierarchy* component) {
    (void)f;
    (void)entity;
    const FPQuaternion& rotation = component->LocalRotation;
    if (rotation.X == FP{} && rotation.Y == FP{} && rotation.Z == FP{} && rotation.W == FP{}) {
        component->LocalRotation = FPQuaternion::Identity();
    }
}

namespace {
// Quantum.AutoDestroyOnMapChangedSystem.ToDestroyFilter
struct ToDestroyFilter {
    EntityRef entity;
    AutoDestroyOnMapChanged* pComp = nullptr;
    static constexpr auto FilterEntity = &ToDestroyFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&ToDestroyFilter::pComp);
};
} // namespace

void AutoDestroyOnMapChangedSystem::OnMapChanged(Frame* f, AssetRefMap previousMap) {
    (void)previousMap;
    auto filter = FilterStruct<ToDestroyFilter>(f);
    ToDestroyFilter current;
    while (filter.Next(current)) {
        f->Destroy(current.entity);
    }
}

} // namespace Quantum
