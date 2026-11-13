// game_mode_systems.cpp
#include "quantum_code/systems/game_mode_systems.h"

#include "quantum_code/systems/asset_decls.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/libs.h"

#include "photon_deterministic/fp_math.h"

namespace Quantum {

void GameModeActivatorSystem::OnAdded(Frame* f, EntityRef entity, GameModeActivator* component) {
    const bool sessionRunsIt = f->get_RuntimeConfig()->modificator == static_cast<std::int32_t>(component->modificator);
    if (sessionRunsIt != static_cast<bool>(component->enabled)) {
        KillableLib::KillNextFrame(f, entity, KillType::Undefined, EntityRef{});
    }
}

void GameModeBoltCountSystem::Bolt_OnCollected(Frame* f, EntityRef collector, std::int32_t amount) {
    (void)amount;
    if (GameModeHandlerLib::IsGameModeFinished(f)) {
        return;
    }
    BoltKingGameModeSpec* spec = nullptr;
    if (!BoltKingGameModeLib::TryGetGameModeSpec(f, spec)) {
        return;
    }
    if (BoltLib::GetCollectedBolts(f, collector) >= spec->ScoreToBecomeKing) {
        GameModeHandlerLib::FinishGameMode(f, spec->EndGameDelay);
    }
}

// --- GameTimerLib / GameTimerSystem ---------------------------------------------------------

FP GameTimerLib::GetGameDuration(Frame* frame) {
    GameModeHandler handler{};
    GameModeSpec* spec = nullptr;
    if (frame->TryGetSingleton<GameModeHandler>(handler) &&
        frame->TryFindAssetInternal(handler.gameModeSpecRef.Id, true, spec)) {
        return spec->gameDuration;
    }
    return -FP::One();
}

FP GameTimerLib::GetElapsedTime(Frame* frame) {
    GameModeHandler handler{};
    if (frame->TryGetSingleton<GameModeHandler>(handler) && handler.state == GAME_MODE_HANDLER_STATE::MainLoop) {
        return handler.GetCurrentStateElapsedTime(frame);
    }
    return FP{};
}

FP GameTimerLib::GetTimeLeft(Frame* frame, bool& isOverTime) {
    isOverTime = false;
    GameModeHandler handler{};
    if (!frame->TryGetSingleton<GameModeHandler>(handler) || handler.state != GAME_MODE_HANDLER_STATE::MainLoop ||
        handler.finishFrame > 0) {
        return -FP::One();
    }
    GameModeSpec* spec = nullptr;
    if (!frame->TryFindAssetInternal(handler.gameModeSpecRef.Id, true, spec) || spec->gameDuration.RawValue() < 1) {
        return -FP::One();
    }
    FP left = spec->gameDuration - handler.GetCurrentStateElapsedTime(frame);
    if (left <= FP{} && spec->overTimeDuration.RawValue() >= 1) {
        left = spec->overTimeDuration + left;
        isOverTime = true;
    }
    return Photon::Deterministic::FPMath::Max(left, FP{});
}

void GameTimerSystem::Update(Frame* frame) {
    GameModeHandler* handler = nullptr;
    GameModeSpec* spec = nullptr;
    if (frame->TryGetPointerSingleton<GameModeHandler>(handler) &&
        frame->TryFindAssetInternal(handler->gameModeSpecRef.Id, true, spec) &&
        handler->state == GAME_MODE_HANDLER_STATE::MainLoop) {
        CheckGameTime(frame);
    }
}

void GameTimerSystem::CheckGameTime(Frame* frame) {
    bool isOverTime = false;
    if (GameTimerLib::GetTimeLeft(frame, isOverTime) == FP{}) {
        GameModeHandlerLib::FinishGameMode(frame, FP{});
    }
}

} // namespace Quantum
