// reward_systems.cpp
#include "quantum_code/systems/reward_systems.h"

#include "photon_deterministic/log.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/libs.h"

#include <algorithm>

namespace Quantum {

void EnergyCellSignalSystem::Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type,
                                                std::uint16_t amount) {
    (void)amount;
    if (type == DroppableType::EnergyCellLow || type == DroppableType::EnergyCellMedium ||
        type == DroppableType::EnergyCellHigh) {
        EnergyCellManager::TryGiveEnergyCell(frame, collectorEntity, type);
    }
}

void BoltSignalSystem::Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type,
                                          std::uint16_t amount) {
    if (type == DroppableType::Bolt) {
        BoltLib::TryChangeBolts(frame, collectorEntity, amount);
    }
}

void BoltSignalSystem::OnAdded(Frame* f, EntityRef entity, BoltGathererActor* component) {
    if (f->FindAsset<BoltGathererActorSpec>(component->Spec.Id) != nullptr) {
        return;
    }
    Photon::Deterministic::Log::Error("BoltSignalSystem: a bolt gatherer without a spec was removed");
    f->Remove<BoltGathererActor>(entity);
}

namespace {
// Applies one of the game mode's bolt-loss rules (a fixed amount or a percentage).
void LoseBoltsPerMode(Frame* frame, EntityRef entity, const GameModeSpec* spec, std::int32_t fixedAmount,
                      std::int32_t percentage) {
    if (spec->boltLossMode == GameModeSpec::BoltsLossMode::Percentage) {
        BoltSignalSystem::LoseBoltsPercentage(frame, entity, FP::FromInt(percentage));
    } else if (spec->boltLossMode == GameModeSpec::BoltsLossMode::FixedAmount) {
        BoltSignalSystem::LoseBoltsFixedAmount(frame, entity, fixedAmount);
    }
}
} // namespace

void BoltSignalSystem::Killable_OnKill(Frame* frame, EntityRef killedEntity, const KilledByData& killedBy) {
    (void)killedBy;
    GameModeSpec* spec = nullptr;
    if (frame->Has<BoltGathererActor>(killedEntity) && GameModeHandlerLib::TryGetGameModeSpec(frame, spec)) {
        LoseBoltsPerMode(frame, killedEntity, spec, spec->boltsLostOnDeath, spec->boltsLostOnDeathPercentage);
    }
}

void BoltSignalSystem::Killable_OnRevive(Frame* frame, EntityRef toRevive, const FPVector3& position,
                                         const FPQuaternion& rotation) {
    (void)position;
    (void)rotation;
    GameModeSpec* spec = nullptr;
    if (GameModeHandlerLib::TryGetGameModeSpec(frame, spec)) {
        LoseBoltsPerMode(frame, toRevive, spec, spec->boltsLostOnRespawn, spec->boltsLostOnRespawnPercentage);
    }
}

void BoltSignalSystem::LoseBoltsFixedAmount(Frame* frame, EntityRef playerEntity, std::int32_t amount) {
    if (amount < 1 || PlayerActorLib::IsGameModeFinished(frame, playerEntity)) {
        return;
    }
    BoltGathererActor* gatherer = nullptr;
    if (frame->TryGetPointer<BoltGathererActor>(playerEntity, gatherer)) {
        LoseBolts(frame, playerEntity, gatherer, amount);
    }
}

void BoltSignalSystem::LoseBoltsPercentage(Frame* frame, EntityRef entity, FP percentage) {
    if (percentage.RawValue() < 1 || PlayerActorLib::IsGameModeFinished(frame, entity)) {
        return;
    }
    BoltGathererActor* gatherer = nullptr;
    if (frame->TryGetPointer<BoltGathererActor>(entity, gatherer)) {
        LoseBolts(frame, entity, gatherer, (FP::FromInt(gatherer->BoltsCollected) / 100 * percentage).AsInt());
    }
}

void BoltSignalSystem::LoseBolts(Frame* frame, EntityRef entity, BoltGathererActor* boltGatherer,
                                 std::int32_t amount) {
    const std::int32_t lost = std::min(amount, boltGatherer->BoltsCollected);
    if (lost < 1) {
        return;
    }
    boltGatherer->BoltsCollected -= lost;
    frame->Events.OnBoltLost(entity, -lost);
    frame->Signals.Bolt_OnLost(entity, -lost);
}

} // namespace Quantum
