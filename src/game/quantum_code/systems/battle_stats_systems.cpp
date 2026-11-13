// battle_stats_systems.cpp - BattleStatsActorSystem.
#include "quantum_code/systems/battle_stats_systems.h"

#include "quantum_code/systems/combat_libs.h"

namespace Quantum {

bool BattleStatsActorSystem::TryGetWeaponUsageStats(Frame* f, BattleStatsActor* pBattleStatsActor, EntityRef entity,
                                                    EntityRef weapon, WeaponUsageStats*& stats) {
    stats = nullptr;
    GENERATED_WEAPON_ID weaponId{};
    if (!WeaponLoadoutLib::TryGetWeaponIdInLoadout(f, entity, weapon, weaponId)) {
        return false;
    }
    Collections::QEnumDictionary<GENERATED_WEAPON_ID, WeaponUsageStats> usage;
    if (!f->TryResolveEnumDictionary(pBattleStatsActor->get_weaponUsageStats(), usage)) {
        return false;
    }
    if (!usage.ContainsKey(weaponId)) {
        usage.Add(weaponId, WeaponUsageStats{});
    }
    return usage.TryGetValuePointer(weaponId, stats);
}

} // namespace Quantum
