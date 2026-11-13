// asset_decls.h - TEMPORARY declarations of the quantum.code asset classes the systems
// read.
//
// The asset classes themselves are being reimplemented separately (quantum_code/assets/).
// Until they land, the systems declare here only the classes and fields they use, with
// the original's names, types, field order and base classes. When the real classes
// exist, this header is replaced by includes of theirs (a declaration here then has to
// go: the names clash on purpose, so a mismatch cannot go unnoticed).
#pragma once

#include "quantum_code/asset_refs.h"
#include "quantum_code/enums.h"
#include "quantum_core/asset_object.h"

#include "photon_deterministic/fp.h"

#include <vector>

namespace Quantum {

using Photon::Deterministic::FP;

class EnhancementSpec : public AssetObject {};

class WeaponEnhancementSpec : public EnhancementSpec {};

class WeaponProjectileSwitcherEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype AlternateAmmunitionAsset;
    std::uint32_t AmmunitionChangeModulo = 0;
    bool DestroyPreviousAmmoInstanceOnSwitch = false;
    bool SwitchAmmoAfterUseTime = false;
    FP SwitchAmmoAfterUseTimeDelay;
};

} // namespace Quantum
