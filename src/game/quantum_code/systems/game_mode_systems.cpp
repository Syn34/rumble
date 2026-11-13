// game_mode_systems.cpp
#include "quantum_code/systems/game_mode_systems.h"

#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/libs.h"
#include "quantum_core/component_filter_struct.h"

#include "photon_deterministic/fp_math.h"

#include <utility>

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

// --- GameModeKillCountSystem -----------------------------------------------------------------

namespace {
// Quantum.Team.PlayerTeamFilter
struct PlayerTeamFilter {
    EntityRef entity;
    Team* team = nullptr;
    PlayerActor* playerActor = nullptr;
    static constexpr auto FilterEntity = &PlayerTeamFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&PlayerTeamFilter::team, &PlayerTeamFilter::playerActor);
};
} // namespace

void GameModeKillCountSystem::Killable_OnKill(Frame* f, EntityRef toKill, const KilledByData& killedBy) {
    if (!f->Has<PlayerActor>(toKill)) {
        return;
    }
    PlayerActor* killer = nullptr;
    if (!f->TryGetPointer<PlayerActor>(killedBy.owner, killer)) {
        return;
    }
    FreeForAllGameModeSpec* freeForAll = nullptr;
    if (GameModeHandlerLib::TryGetGameModeSpec<FreeForAllGameModeSpec>(f, freeForAll) &&
        static_cast<std::int64_t>(killer->killCount) >= static_cast<std::int64_t>(freeForAll->killsToWin)) {
        GameModeHandlerLib::FinishGameMode(f, FP{});
        return;
    }
    TeamTotalRumbleGameModeSpec* teamMode = nullptr;
    std::int32_t teamId = -1;
    if (!GameModeHandlerLib::TryGetGameModeSpec<TeamTotalRumbleGameModeSpec>(f, teamMode) ||
        !Team::TryGetTeamID(f, killedBy.owner, teamId)) {
        return;
    }
    std::int32_t teamKills = 0;
    auto filter = FilterStruct<PlayerTeamFilter>(f);
    PlayerTeamFilter current;
    while (filter.Next(current)) {
        if (current.team->TeamID == teamId) {
            teamKills += static_cast<std::uint16_t>(current.playerActor->killCount);
        }
    }
    if (static_cast<std::int64_t>(teamMode->killsToWin) <= static_cast<std::int16_t>(teamKills)) {
        GameModeHandlerLib::FinishGameMode(f, FP{});
    }
}

// --- TimeScaleLib / TimeScaleSystem ------------------------------------------------------------

namespace {
// Whole frames of `duration` at the unscaled delta time (truncated, as the original).
std::int32_t FramesOf(Frame* frame, FP duration) { return (duration / frame->get_UnscaledDeltaTime()).AsInt(); }
} // namespace

void TimeScaleLib::SetProgressiveTimeScale(Frame* frame, FP targetTimeScale, FP duration) {
    TimeScale* timeScale = nullptr;
    if (!frame->TryGetPointerSingleton<TimeScale>(timeScale)) {
        return;
    }
    timeScale->startFrame = frame->Number;
    timeScale->startTimeScale = frame->get_DeltaTime() / frame->get_UnscaledDeltaTime();
    timeScale->endFrame = frame->Number + FramesOf(frame, duration);
    timeScale->endTimeScale = targetTimeScale;
}

void TimeScaleLib::DoBackAndForthTimeScale(Frame* frame, FP targetTimeScale, FP halfDuration) {
    TimeScale* timeScale = nullptr;
    if (!frame->TryGetPointerSingleton<TimeScale>(timeScale)) {
        return;
    }
    timeScale->startFrame = frame->Number;
    timeScale->startTimeScale = frame->get_DeltaTime() / frame->get_UnscaledDeltaTime();
    timeScale->endFrame = frame->Number + FramesOf(frame, halfDuration);
    timeScale->endTimeScale = targetTimeScale;
    timeScale->autoReverse = true;
}

void TimeScaleLib::ResetTimeScale(Frame* frame) {
    frame->set_DeltaTime(frame->get_UnscaledDeltaTime());
    TimeScale* timeScale = nullptr;
    if (!frame->TryGetPointerSingleton<TimeScale>(timeScale)) {
        return;
    }
    timeScale->startFrame = 0;
    timeScale->endFrame = 0;
    timeScale->startTimeScale = FP{};
    timeScale->endTimeScale = FP{};
}

bool TimeScaleLib::IsTimeScaleActive(Frame* frame) {
    TimeScale* timeScale = nullptr;
    return frame->TryGetPointerSingleton<TimeScale>(timeScale) && timeScale->endFrame > 0;
}

void TimeScaleSystem::OnInit(Frame* frame) { frame->GetOrAddSingleton<TimeScale>(); }

void TimeScaleSystem::Update(Frame* frame, Filter& filter) {
    TimeScale* timeScale = filter.timeScale;
    if (timeScale->endFrame < 1) {
        return;
    }
    if (frame->Number < timeScale->endFrame) {
        const FP t = FP::FromInt(frame->Number - timeScale->startFrame) /
                     FP::FromInt(timeScale->endFrame - timeScale->startFrame);
        const FP scale = Photon::Deterministic::FPMath::Lerp(timeScale->startTimeScale, timeScale->endTimeScale, t);
        frame->set_DeltaTime(scale * frame->get_UnscaledDeltaTime());
        return;
    }
    if (timeScale->autoReverse) {
        timeScale->autoReverse = false;
        std::swap(timeScale->startTimeScale, timeScale->endTimeScale);
        const std::int32_t frames = timeScale->endFrame - timeScale->startFrame;
        timeScale->endFrame = frame->Number + FramesOf(frame, frame->get_UnscaledDeltaTime() * frames);
        timeScale->startFrame = frame->Number;
        return;
    }
    timeScale->startFrame = 0;
    timeScale->startTimeScale = FP{};
    timeScale->endFrame = 0;
    timeScale->endTimeScale = FP{};
}

void TimeScaleSystem::GameMode_OnStateChanged(Frame* frame, GAME_MODE_HANDLER_STATE prevState,
                                              GAME_MODE_HANDLER_STATE newState) {
    (void)prevState;
    GameModeSpec* spec = nullptr;
    if (!GameModeHandlerLib::TryGetGameModeSpec(frame, spec)) {
        return;
    }
    if (newState == GAME_MODE_HANDLER_STATE::Ended) {
        TimeScaleLib::ResetTimeScale(frame);
    } else if (newState == GAME_MODE_HANDLER_STATE::GameModeOutro) {
        TimeScaleLib::SetProgressiveTimeScale(frame, spec->outroTimeScale, spec->outroTimeScalingDuration);
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
