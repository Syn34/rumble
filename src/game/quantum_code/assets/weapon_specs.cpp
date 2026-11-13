// Methods of the weapon, gadget and ultimate assets and their tables (quantum.code.dll).
#include "quantum_code/assets/weapon_specs.h"

#include "quantum_code/frame.h"

namespace Quantum {

namespace {

// The levelled value of the first modifier for `property`: its entry for `level`, the last
// entry past the end (an empty list throws in the original too).
template <class Modifier, class Property>
const FP* LevelledValue(const std::vector<Modifier>& modifiers, Property property, std::int32_t level) {
    for (const Modifier& modifier : modifiers) {
        if (modifier.prop != property) continue;
        if (modifier.values.empty()) throw std::out_of_range("level-up modifier without values");
        const auto last = static_cast<std::int32_t>(modifier.values.size()) - 1;
        return &modifier.values[static_cast<std::size_t>(level < last + 1 ? level : last)];
    }
    return nullptr;
}

// The integer part of a levelled value, -1 without one.
template <class Modifier, class Property>
std::int32_t LevelledInt(const std::vector<Modifier>& modifiers, Property property, std::int32_t level) {
    const FP* value = LevelledValue(modifiers, property, level);
    return value ? static_cast<std::int32_t>(value->RawValue() >> 16) : -1;
}

} // namespace

std::int32_t WeaponSpec::GetWeaponPowerAtLevel(std::int32_t level) const {
    return LevelledInt(levelUpModifiers, WeaponSpecModifier::Property::Power, level);
}

std::int32_t GadgetSpec::GetPowerAtLevel(std::int32_t level) const {
    return LevelledInt(levelUpModifiers, GadgetSpecModifier::Property::Power, level);
}

FP GadgetSpec::GetCooldownAtLevel(std::int32_t level) const {
    const FP* value = LevelledValue(levelUpModifiers, GadgetSpecModifier::Property::Cooldown, level);
    return value ? *value : cooldown;
}

std::int32_t UltimateAbilitySpec::GetPowerAtLevel(std::int32_t level) const {
    return LevelledInt(levelUpModifiers, UltimateSpecModifier::Property::Power, level);
}

std::int32_t UltimateAbilitySpec::GetFullChargeAtLevel(std::int32_t level) const {
    return LevelledInt(levelUpModifiers, UltimateSpecModifier::Property::FullCharge, level);
}

bool SimulationGadgetTable::TryGetGadget(GENERATED_CHARACTER_ID characterID, GadgetDefinition& gadgetDefinition) const {
    for (const GadgetDefinition& definition : definitions) {
        if (definition.characterId == characterID) {
            gadgetDefinition = definition;
            return true;
        }
    }
    gadgetDefinition = GadgetDefinition{};
    return false;
}

bool SimulationUltimateTable::TryGetUltimate(GENERATED_CHARACTER_ID characterID,
                                             UltimateDefinition& ultimateDefinition) const {
    for (const UltimateDefinition& definition : definitions) {
        if (definition.characterId == characterID) {
            ultimateDefinition = definition;
            return true;
        }
    }
    ultimateDefinition = UltimateDefinition{};
    return false;
}

bool SimulationMeleeWeaponsTable::TryGetMeleeWeapon(GENERATED_CHARACTER_ID characterID,
                                                    MeleeWeaponDefinition& meleeWeaponDefinition) const {
    for (const MeleeWeaponDefinition& definition : definitions) {
        if (definition.characterId == characterID) {
            meleeWeaponDefinition = definition;
            return true;
        }
    }
    meleeWeaponDefinition = MeleeWeaponDefinition{};
    return false;
}

bool SimulationWeaponTable::TryGetWeaponId(const Weapon& weapon, GENERATED_WEAPON_ID& weaponId) const {
    weaponId = GENERATED_WEAPON_ID::AMOEBOIDLAUNCHER; // what the original leaves on failure
    if (!weapon.WeaponData.Id.IsValid()) return false;
    for (const Entry& entry : entries) {
        if (entry.weaponSpecRef.Id == weapon.WeaponData.Id) {
            weaponId = entry.uniqueID;
            return true;
        }
    }
    return false;
}

WeaponSpec* SimulationWeaponTable::GetWeaponSpec(Frame* frame, GENERATED_WEAPON_ID weaponId) const {
    for (const Entry& entry : entries) {
        if (entry.uniqueID == weaponId) return frame ? frame->FindAsset<WeaponSpec>(entry.weaponSpecRef.Id) : nullptr;
    }
    return nullptr;
}

} // namespace Quantum
