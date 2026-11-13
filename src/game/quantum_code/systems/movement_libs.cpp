// movement_libs.cpp - small helpers the movement code relies on: the input-action rules of
// ultimates and gadgets, unlocked features, and the player's input.
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/input.h"
#include "quantum_code/systems/libs.h"

namespace Quantum {

::Quantum::Input* PlayerActor::GetInput(Frame* f) const {
    if (isBot) {
        return nullptr;
    }
    return f->GetPlayerInput(playerRef.ToIndex());
}

void UnlockedFeaturesManager::UnlockFeature(Frame* f, EntityRef entity, UnlockedFeature feature) {
    QBoolean& unlocked = get_unlockedFeatures()[static_cast<std::size_t>(feature)];
    if (unlocked) {
        return;
    }
    unlocked = true;
    f->Signals.OnFeatureUnlocked(entity, feature);
    f->Events.OnFeatureUnlocked(entity, feature);
}

bool PlayerActorInputActionLib::IsAllowedByUltimate(Frame* frame, EntityRef entity, PlayerActorInputAction inputAction) {
    UltimateAbilityUser* user = nullptr;
    if (!frame->TryGetPointer<UltimateAbilityUser>(entity, user) || !user->get_IsActive()) {
        return true;
    }
    UltimateAbilitySpec* spec = nullptr;
    if (!frame->TryFindAssetInternal<UltimateAbilitySpec>(user->abilitySpec.Id, true, spec)) {
        return true;
    }
    return FilterContains(spec->AllowedInputActionDuringUse, inputAction);
}

bool PlayerActorInputActionLib::IsAllowedByGadget(Frame* frame, EntityRef entity, PlayerActorInputAction inputAction) {
    GadgetUser* user = nullptr;
    if (!frame->TryGetPointer<GadgetUser>(entity, user) || !user->get_IsActive()) {
        return true;
    }
    GadgetSpec* spec = nullptr;
    if (!frame->TryFindAssetInternal<GadgetSpec>(user->gadgetSpec.Id, true, spec)) {
        return true;
    }
    return FilterContains(spec->AllowedInputActionDuringUse, inputAction);
}

bool UnlockedFeaturesLib::IsFeatureUnlocked(Frame* f, EntityRef entity, UnlockedFeature feature,
                                            bool isUnlockedByDefault) {
    UnlockedFeaturesManager* manager = nullptr;
    if (f->TryGetPointer<UnlockedFeaturesManager>(entity, manager)) {
        return manager->IsFeatureUnlocked(feature);
    }
    return isUnlockedByDefault;
}

bool UnlockedFeaturesLib::TryUnlockFeature(Frame* f, EntityRef entity, UnlockedFeature feature) {
    UnlockedFeaturesManager* manager = nullptr;
    if (!f->TryGetPointer<UnlockedFeaturesManager>(entity, manager)) {
        return false;
    }
    manager->UnlockFeature(f, entity, feature);
    return true;
}

} // namespace Quantum
