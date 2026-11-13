// Methods of the game mode assets (quantum.code.dll).
#include "quantum_code/assets/game_mode_specs.h"

namespace Quantum {

BOT_AGGRO_BEHAVIOUR GameModeSpec::GetBotAggroBehaviour(Frame* frame, std::int32_t customBotIndex) const {
    (void)frame;
    if (customBotIndex >= 0 && static_cast<std::size_t>(customBotIndex) < BotAggroBehaviourByIndex.size())
        return BotAggroBehaviourByIndex[static_cast<std::size_t>(customBotIndex)];
    return DefaultAggroBehaviour;
}

bool TrainingGameModeSpec::RespawnPointIsValid(Frame* f, EntityRef entityToRespawn,
                                               const MapCustomData::SpawnPointData& spData,
                                               const SpawnPointDynamicData& spDynData) const {
    (void)f, (void)entityToRespawn, (void)spDynData;
    // Only the points flagged as initial spawn points.
    return (static_cast<std::int32_t>(spData.options) & static_cast<std::int32_t>(SpawnPointOptions::Initial)) != 0;
}

BATTLE_RESULT BlastBallGameModeSpec::GetBattleResult(std::int32_t rank, std::int32_t nbPlayersInBattle,
                                                     std::int32_t localScore, std::int32_t opponentScore) {
    (void)rank, (void)nbPlayersInBattle;
    if (localScore > opponentScore) return BATTLE_RESULT::WIN;
    return localScore == opponentScore ? BATTLE_RESULT::DRAW : BATTLE_RESULT::LOSE;
}

namespace {

// Free-for-all style ranking: the better half (rank below half the players) wins.
BATTLE_RESULT RankingResult(std::int32_t rank, std::int32_t nbPlayersInBattle) {
    return rank > nbPlayersInBattle / 2 ? BATTLE_RESULT::LOSE : BATTLE_RESULT::WIN;
}

} // namespace

BATTLE_RESULT BoltKingGameModeSpec::GetBattleResult(std::int32_t rank, std::int32_t nbPlayersInBattle,
                                                    std::int32_t localScore, std::int32_t opponentScore) {
    (void)localScore, (void)opponentScore;
    return RankingResult(rank, nbPlayersInBattle);
}

BATTLE_RESULT FreeForAllGameModeSpec::GetBattleResult(std::int32_t rank, std::int32_t nbPlayersInBattle,
                                                      std::int32_t localScore, std::int32_t opponentScore) {
    (void)localScore, (void)opponentScore;
    return RankingResult(rank, nbPlayersInBattle);
}

BATTLE_RESULT TeamTotalRumbleGameModeSpec::GetBattleResult(std::int32_t localScore, std::int32_t opponentScore) {
    if (localScore == opponentScore) return BATTLE_RESULT::DRAW;
    return localScore > opponentScore ? BATTLE_RESULT::WIN : BATTLE_RESULT::LOSE;
}

FP ZoneControlGameModeSpec::GetZoneLoadingTime(Frame* f, const ZoneControlGameMode& gameMode) const {
    (void)f;
    // The first zone (no team has scored yet) takes its own, usually longer, time.
    const bool noPointsYet = gameMode.Scoring.TeamA.VictoryPoints == 0 && gameMode.Scoring.TeamB.VictoryPoints == 0;
    return noPointsYet ? ControlZoneInitialLoadingTime : ControlZoneLoadingTime;
}

} // namespace Quantum
