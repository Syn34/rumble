// game_mode_libs.cpp - GameModeHandlerLib, BoltLib and the game mode handler / energy
// cell manager component helpers.
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/libs.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_core/component_filter_struct.h"

#include <algorithm>
#include <limits>

namespace Quantum {

// --- GameModeHandler (component) --------------------------------------------------------

FP GameModeHandler::GetCurrentStateElapsedTime(Frame* f) const {
    return FP::FromRaw(f->get_UnscaledDeltaTime().RawValue() * (f->Number - stateStartFrame) - pausedTime.RawValue());
}

FP GameModeHandler::GetCurrentStateElapsedTimeNoPause(Frame* f) const {
    return FP::FromRaw(f->get_UnscaledDeltaTime().RawValue() * (f->Number - stateStartFrame));
}

// --- GameModeHandlerLib ------------------------------------------------------------------

bool GameModeHandlerLib::TryGetGameModeSpec(Frame* f, GameModeSpec*& gameModeSpec) {
    gameModeSpec = nullptr;
    GameModeHandler* handler = nullptr;
    if (!f->TryGetPointerSingleton<GameModeHandler>(handler)) {
        return false;
    }
    gameModeSpec = f->FindAsset<GameModeSpec>(handler->gameModeSpecRef.Id);
    return gameModeSpec != nullptr;
}

GAME_MODE_HANDLER_STATE GameModeHandlerLib::GetGameModeState(Frame* f) {
    GameModeHandler* handler = nullptr;
    if (!f->TryGetPointerSingleton<GameModeHandler>(handler)) {
        return GAME_MODE_HANDLER_STATE::WaitConnections;
    }
    return handler->state;
}

bool GameModeHandlerLib::IsInActionPhase(Frame* f) {
    GameModeHandler* handler = nullptr;
    if (!f->TryGetPointerSingleton<GameModeHandler>(handler)) {
        return true;
    }
    return handler->state == GAME_MODE_HANDLER_STATE::MainLoop;
}

FP GameModeHandlerLib::GetCurrentStateElapsedTime(Frame* frame) {
    GameModeHandler* handler = nullptr;
    if (!frame->TryGetPointerSingleton<GameModeHandler>(handler)) {
        return FP{};
    }
    return handler->GetCurrentStateElapsedTime(frame);
}

bool GameModeHandlerLib::IsGameModeFinished(Frame* frame) {
    GameModeHandler* handler = nullptr;
    return frame->TryGetPointerSingleton<GameModeHandler>(handler) && handler->finishFrame > 0;
}

void GameModeHandlerLib::FinishGameMode(Frame* frame, FP delay) {
    GameModeHandler* handler = nullptr;
    if (!frame->TryGetPointerSingleton<GameModeHandler>(handler) || handler->finishFrame > 0) {
        return;
    }
    handler->finishFrame = frame->Number + (delay / frame->get_DeltaTime()).AsInt();
}

// --- BoltLib -------------------------------------------------------------------------------

namespace {
// Quantum.BoltLib.BoltGathererTeamFilter
struct BoltGathererTeamFilter {
    EntityRef entity;
    BoltGathererActor* boltGathererActor = nullptr;
    Team* team = nullptr;
    static constexpr auto FilterEntity = &BoltGathererTeamFilter::entity;
    static constexpr auto FilterComponents =
        std::make_tuple(&BoltGathererTeamFilter::boltGathererActor, &BoltGathererTeamFilter::team);
};
} // namespace

std::int32_t BoltLib::GetCollectedBolts(Frame* frame, EntityRef entity) {
    BoltGathererActor* gatherer = nullptr;
    if (!frame->TryGetPointer<BoltGathererActor>(entity, gatherer)) {
        return 0;
    }
    Team team{};
    if (gatherer->hasTeamCounter && frame->TryGet<Team>(entity, team)) {
        return GetTeamCollectedBolts(frame, team.TeamID);
    }
    return gatherer->BoltsCollected;
}

std::int32_t BoltLib::GetTeamCollectedBolts(Frame* frame, std::int32_t teamId) {
    std::int32_t total = 0;
    auto filter = FilterStruct<BoltGathererTeamFilter>(frame);
    BoltGathererTeamFilter current;
    while (filter.Next(current)) {
        if (current.team->TeamID == teamId) {
            total += current.boltGathererActor->BoltsCollected;
        }
    }
    return total;
}

void BoltLib::TryChangeBolts(Frame* f, EntityRef gathererEntity, std::int32_t changeAmount) {
    BoltGathererActor* gatherer = nullptr;
    if (f->TryGetPointer<BoltGathererActor>(gathererEntity, gatherer)) {
        TryChangeBolts(f, gathererEntity, gatherer, changeAmount);
    }
}

void BoltLib::TryChangeBolts(Frame* frame, EntityRef entity, BoltGathererActor* boltGathererActor,
                             std::int32_t changeAmount) {
    if (PlayerActorLib::IsGameModeFinished(frame, entity)) {
        return;
    }
    std::int32_t maximum = std::numeric_limits<std::int32_t>::max();
    BoltGathererActorSpec* spec = nullptr;
    if (frame->TryFindAssetInternal(boltGathererActor->Spec.Id, true, spec) && spec->MaxBoltsCollectable != 0) {
        maximum = spec->MaxBoltsCollectable;
    }
    const std::int32_t previous = boltGathererActor->BoltsCollected;
    const std::int32_t current = std::clamp(previous + changeAmount, 0, maximum);
    boltGathererActor->BoltsCollected = current;
    const std::int32_t delta = current - previous;
    if (delta >= 1) {
        frame->Events.OnBoltCollected(entity, delta);
        frame->Signals.Bolt_OnCollected(entity, delta);
        return;
    }
    if (changeAmount < 0 || current != previous) {
        frame->Events.OnBoltLost(entity, -delta);
        frame->Signals.Bolt_OnLost(entity, -delta);
    }
}

// --- EnergyCellManager (component) --------------------------------------------------------

void EnergyCellManager::TryGiveEnergyCell(Frame* frame, EntityRef collectorEntity, DroppableType energyCellType) {
    EnergyCellManager* manager = nullptr;
    if (!frame->TryGetPointerSingleton<EnergyCellManager>(manager)) {
        return;
    }
    EnergyCellManagerSpec* spec = nullptr;
    if (!frame->TryFindAssetInternal(manager->Spec.Id, true, spec)) {
        return;
    }
    TryGiveEnergyCell(frame, collectorEntity, GetCharge(energyCellType, spec));
}

void EnergyCellManager::TryGiveEnergyCell(Frame* frame, EntityRef collectorEntity, std::int32_t charge) {
    EnergyCellManager* manager = nullptr;
    const bool hasManager = frame->TryGetPointerSingleton<EnergyCellManager>(manager);
    if (charge < 1 || !hasManager) {
        return;
    }
    UltimateAbilityUser* user = nullptr;
    if (!frame->TryGetPointer<UltimateAbilityUser>(collectorEntity, user)) {
        return;
    }
    UltimateAbilitySpec* spec = nullptr;
    if (!frame->TryFindAssetInternal(user->abilitySpec.Id, true, spec)) {
        return;
    }
    frame->Events.EnergyCell_OnCollected(collectorEntity, charge);
    UltimateAbilityLib::TryGiveCharge(frame, collectorEntity, user, spec, FP::FromInt(charge));
}

std::int32_t EnergyCellManager::GetCharge(DroppableType energyCellType, const EnergyCellManagerSpec* spec) {
    switch (energyCellType) {
    case DroppableType::EnergyCellLow:
        return spec->LowCharge;
    case DroppableType::EnergyCellMedium:
        return spec->MediumCharge;
    case DroppableType::EnergyCellHigh:
        return spec->HighCharge;
    default:
        return 0;
    }
}

} // namespace Quantum
