// kcc_extension.cpp - KccExtension's timed modifier lists (move speed, max speed, gravity,
// max fall speed, acceleration, braking), forced input and orientation helpers.
#include "photon_deterministic/fp_math.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/frame.h"

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

namespace {

// The modifier clock: seconds since the session started.
FP ModifierTime(Frame* f) {
    return f->get_DeltaTime() * f->Number;
}

// A modifier with a non-negative duration has run out once more than that has passed.
bool HasExpired(const ModInfo& modifier, FP now) {
    return modifier.duration.RawValue() >= 0 && now - modifier.date > modifier.duration;
}

} // namespace

FP ModInfo::GetValueAtTime(FP time) const {
    if (falloffDuration <= FP{}) {
        return value;
    }
    const FP remaining = date - time + duration;
    if (remaining > falloffDuration) {
        return value;
    }
    return FPMath::Lerp(FP{}, value, remaining / falloffDuration);
}

void KccExtension::UpdateForceInput(Frame* f) {
    if (forceInputFallOff > FP{}) {
        forceInputBlend = FPMath::Lerp(FP{}, forceInputMaxBlend, FPMath::Clamp01(forceInputCounter / forceInputFallOff));
    } else {
        forceInputBlend = forceInputCounter.RawValue() >= 1 ? forceInputMaxBlend : FP{};
    }
    if (forceInputCounter.RawValue() >= 1) {
        forceInputCounter = forceInputCounter - f->get_DeltaTime();
    }
}

void KccExtension::ResetOrientationSharpness(Frame* f) {
    KccExtensionConfig* config = nullptr;
    if (f->TryFindAssetInternal(Config.Id, true, config)) {
        orientationSharpness = config->OrientationSharpness;
    }
}

void KccExtension::AddModifier(Frame* f, Collections::QListPtr<ModInfo> list, MOD_ORIGIN origin, FP value, FP duration,
                               FP falloffDuration, bool originOverride) {
    if (duration.RawValue() == 0) {
        return;
    }
    Collections::QList<ModInfo> modifiers = f->ResolveList(list);
    ModInfo modifier{};
    modifier.origin = origin;
    modifier.date = ModifierTime(f);
    modifier.duration = duration;
    modifier.falloffDuration = falloffDuration;
    modifier.value = value;
    if (originOverride) {
        for (std::int32_t i = 0; i < modifiers.get_Count(); ++i) {
            if (modifiers.get_Item(i).origin == origin) {
                modifiers.set_Item(i, modifier);
                return;
            }
        }
    }
    modifiers.Add(modifier);
}

void KccExtension::AddModifier_MoveSpeed(Frame* f, EntityRef entity, MOD_ORIGIN origin, FP value, FP duration,
                                         FP falloffDuration, bool originOverride, bool notifyView) {
    if (notifyView) {
        f->Events.KccExtensions_OnMoveSpeedModifierAdded(entity, value, duration);
    }
    AddModifier_MoveSpeed(f, origin, value, duration, falloffDuration, originOverride);
}

bool KccExtension::HasModifier(Frame* f, Collections::QListPtr<ModInfo> list, MOD_ORIGIN origin) const {
    Collections::QList<ModInfo> modifiers = f->ResolveList(list);
    for (std::int32_t i = 0; i < modifiers.get_Count(); ++i) {
        if (modifiers.get_Item(i).origin == origin) {
            return true;
        }
    }
    return false;
}

void KccExtension::ClearModifiersOfTypeInList(Frame* f, Collections::QListPtr<ModInfo> list, MOD_ORIGIN origin) {
    Collections::QList<ModInfo> modifiers = f->ResolveList(list);
    for (std::int32_t i = modifiers.get_Count() - 1; i >= 0; --i) {
        if (modifiers.get_Item(i).origin == origin) {
            modifiers.RemoveAt(i);
        }
    }
}

FP KccExtension::ComputeModifiers(Frame* f, Collections::QListPtr<ModInfo> list) {
    return FPMath::Clamp(ComputeAllModifiers_Additive(f, list) + FP::One(), FP{}, FP::MaxValue());
}

FP KccExtension::ComputeAllModifiers_Additive(Frame* f, Collections::QListPtr<ModInfo> list) {
    const FP now = ModifierTime(f);
    Collections::QList<ModInfo> modifiers = f->ResolveList(list);
    FP total;
    for (std::int32_t i = modifiers.get_Count() - 1; i >= 0; --i) {
        const ModInfo modifier = modifiers.get_Item(i);
        if (HasExpired(modifier, now)) {
            modifiers.RemoveAt(i);
        } else {
            total = modifier.GetValueAtTime(now) + total;
        }
    }
    return total;
}

FP KccExtension::ComputeModifiersOfType_Additive(Frame* f, Collections::QListPtr<ModInfo> list, MOD_ORIGIN origin) const {
    const FP now = ModifierTime(f);
    Collections::QList<ModInfo> modifiers = f->ResolveList(list);
    FP total;
    for (std::int32_t i = modifiers.get_Count() - 1; i >= 0; --i) {
        const ModInfo modifier = modifiers.get_Item(i);
        if (modifier.origin == origin) {
            total = modifier.GetValueAtTime(now) + total;
        }
    }
    return total;
}

void KccExtension::RemoveExpiredModifiers(Frame* f) {
    RemoveExpiredModifiers(f, get_speedModifiers());
    RemoveExpiredModifiers(f, get_maxSpeedModifiers());
    RemoveExpiredModifiers(f, get_gravityModifiers());
    RemoveExpiredModifiers(f, get_maxFallSpeedModifiers());
    RemoveExpiredModifiers(f, get_accelerationModifiers());
    RemoveExpiredModifiers(f, get_brakingModifiers());
}

void KccExtension::RemoveExpiredModifiers(Frame* f, Collections::QListPtr<ModInfo> list) {
    const FP now = ModifierTime(f);
    Collections::QList<ModInfo> modifiers = f->ResolveList(list);
    for (std::int32_t i = modifiers.get_Count() - 1; i >= 0; --i) {
        if (HasExpired(modifiers.get_Item(i), now)) {
            modifiers.RemoveAt(i);
        }
    }
}

} // namespace Quantum
