// enablers.cpp - the projectile switcher enhancement's enabler and disabler.
#include "quantum_code/systems/enablers.h"

#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"

namespace Quantum {

void WeaponProjectileSwitcherEnhancementSystemEnabler::OnAdded(Frame* f, EntityRef entity,
                                                                 WeaponProjectileSwitcherEnhancement* component) {
    (void)entity;
    WeaponProjectileSwitcherEnhancementSpec* spec = nullptr;
    if (!f->TryFindAssetInternal(component->Spec.Id, true, spec)) {
        return;
    }
    f->SystemEnable<WeaponProjectileSwitcherEnhancementSystem>();
    if (spec->SwitchAmmoAfterUseTime) {
        f->SystemEnable<WeaponProjectileSwitcherEnhancementUpdateSystem>();
    }
}

void WeaponProjectileSwitcherEnhancementSystemDisabler::OnRemoved(Frame* f, EntityRef entity,
                                                                    WeaponProjectileSwitcherEnhancement* component) {
    (void)entity;
    (void)component;
    if (f->ComponentCount<WeaponProjectileSwitcherEnhancement>(false) != 0) {
        return;
    }
    f->SystemDisable<WeaponProjectileSwitcherEnhancementUpdateSystem>();
    f->SystemDisable<WeaponProjectileSwitcherEnhancementSystem>();
}

} // namespace Quantum
