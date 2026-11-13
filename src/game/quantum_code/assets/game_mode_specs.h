// game_mode_specs.h - game modes, their handlers and level configurations, waves and spawners (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/assets/enhancement_specs.h"

namespace Quantum {

class AIBotSDKConfig : public AssetObject {
  public:
    FP attackPosSearchRadius = FP::FromInt(1);
    FP attackPosIdealRangeFactor = FP::FromRaw(52427); // 0.799973
    bool allowRepositioningDuringAttack = true;
    FP repositioningMinDelay = FP::FromInt(2);
    FP repositioningMaxDelay = FP::FromInt(5);
    FP safetyMarginForCratesAndCollectables = FP::FromInt(4);
    FP wanderDangerZoneMargin = FP::FromInt(3);
    FP wanderMinDistZoneRadiusFactor = FP::FromRaw(16384); // 0.25
    FP wanderDestinationSafetyMargin = FP::FromInt(4);
    FP seekTimeoutOnPathNotFound = FP::FromInt(5);
    bool allowRandomJump = true;
    FP randomJumpMinDelay = FP::FromInt(3);
    FP randomJumpMaxDelay = FP::FromInt(10);
    DifficultyIntervalInt shootMissChance;
    DifficultyIntervalInt shootMissChanceBehindHuman;
    FP shootMissAmplitudeHorizontal = FP::FromInt(3);
    FP shootMissAmplitudeVertical = FP::FromInt(1);
    FP blankShootMinCoef = FP::FromRaw(13107); // 0.199997
    DifficultyIntervalInt shootCooldownMsMin;
    DifficultyIntervalInt shootCooldownMsMax;
    DifficultyIntervalInt shootCooldownMsMinForNoMissWeapon;
    DifficultyIntervalInt shootCooldownMsMaxForNoMissWeapon;

    // 0x7c8e0a8
    AIBotSDKConfig() = default;
};

class AISpawnerConfig : public AssetObject {
  public:
    AssetRefEntityPrototype EntityToSpawn;
    FP InitialSpawnDelaySeconds{};
    FP SpawnIntervalSeconds = FP::FromInt(2);
    bool UseRandomSpawnTransform = true;
    FP SpawnPositionOffsetDistance = FP::FromInt(1);
    bool AllowRespawnAfterDeath = true;
    FP AfterDeathSpawnDelaySeconds = FP::FromInt(3);

    // 0x7c8dc70
    AISpawnerConfig() = default;
};

class GameManagerConfig : public AssetObject {
  public:
    FPAnimationCurve BotWinStreakRandomCurve;
    std::uint32_t BotWinStreakRandomMinValueIncl{};
    std::uint32_t BotWinStreakRandomMaxValueIncl{};

    // 0x7c91054
    GameManagerConfig() = default;
};

class GameModeHandlerConfig : public AssetObject {
  public:
    std::string referenceDate = "2025-1-1T0:0:0";
    FP FFAHoursDuration = FP::FromInt(8);
    FP BBHoursDuration = FP::FromInt(8);
    FP SSDaysDuration = FP::FromInt(7);
    FP SSDaysStart = FP::FromInt(21);
    FP levelLoadingScreenDuration = FP::FromInt(7);
    FP LevelIntroDuration = FP::FromInt(5);
    FP CountDownDuration = FP::FromInt(3);
    FP GameOverDuration = FP::FromInt(5);
    bool GameModeIntroEnabledInEditor = true;
    GENERATED_WEAPON_ID ForcedBotsWeapon{};

    // 0x7c9107c
    GameModeHandlerConfig() = default;

    // 0x7c9105c - editor-only switch; always on in the game.
    bool DebugRunnerSkipIntroInEditor() const { return true; }
    // 0x7c91064 - editor-only switch; always on in the game.
    bool LevelIntroEnabledInEditor() const { return true; }
    // 0x7c9106c - editor-only switch; always on in the game.
    bool CountdownEnabledInEditor() const { return true; }
    // 0x7c91074 - editor-only switch; always on in the game.
    bool LevelLoadingScreenEnabledInEditor() const { return true; }
};

class GameModeHandlerLevelConfig : public AssetObject {
  public:
    std::uint32_t maxPlayers = 8u;
    std::string sceneName;
    AssetRefMap mapRef;

    // 0x7c9110c
    GameModeHandlerLevelConfig() = default;
};

class GameModeSpec : public AssetObject {
  public:
    enum class BoltsLossMode : std::int32_t {
        FixedAmount = 0,
        Percentage = 1,
    };

    GAME_MODE id{};
    GameModeIntroFlags introFlags = GameModeIntroFlags::Default;
    FP introDuration = FP::FromInt(5);
    FP gameDuration = FP::FromInt(180);
    FP overTimeDuration{};
    FP outroDuration = FP::FromInt(5);
    FP lastManStandingExtraTime = FP::FromInt(3);
    std::int32_t closeToWinScoreGap = 1;
    bool triggerSlowMotionOnlyOnce = true;
    FP closeToWinSlowMotionDuration = FP::FromInt(2);
    FP closeToWinSlowMotionModifier = FP::FromRaw(32768); // 0.5
    FP playerMaxSpeedModifier{};
    bool showPlayerNameOnCanvasMark = true;
    bool showBoltAmountOnCanvasMark = true;
    bool showKillAmountOnCanvasMark = true;
    bool showRankOnCanvasMark = true;
    bool showPermanentOverhealthCount{};
    EntityPrototypeRef BotPrototype;
    AssetRefEntityPrototype UTMoveAIPrototype;
    AssetRefEntityPrototype UTActionAIPrototype;
    AssetRefHealthSpec FTUEBotHealthSpec;
    BOT_AGGRO_BEHAVIOUR DefaultAggroBehaviour = static_cast<BOT_AGGRO_BEHAVIOUR>(3);
    std::vector<BOT_AGGRO_BEHAVIOUR> BotAggroBehaviourByIndex;
    bool allowGoldenBolt{};
    ENTITY_INFO_FILTER allowedEntityTypesForMinimap = ENTITY_INFO_FILTER::All;
    AssetRefGoldenBoltConfig goldenBoltConfig;
    GameModeSpec::BoltsLossMode boltLossMode{};
    std::int32_t boltsLostOnRespawn{};
    std::int32_t boltsLostOnRespawnPercentage{};
    std::int32_t boltsLostOnDeath{};
    std::int32_t boltsLostOnDeathPercentage{};
    std::int32_t boltsLostOnElimination{};
    std::int32_t boltsLostOnEliminationPercentage{};
    AssetRefRewardSpec rewardOnKillSpec;
    AssetRefRewardSpec rewardOnShieldBreakSpec;
    bool isCoop{};
    bool ShowLocalPlayerTeamatesSilhouettes{};
    bool ShowTeamatesAnnounces{};
    bool ShowTeamatesHalo = true;
    bool ShowTeammatesMarks = true;
    bool playersCanRespawn{};
    bool AutoRespawn = true;
    bool RespawnNearTeamLeader{};
    bool RespawnNearTeammate{};
    FP RespawnDistance = FP::FromInt(3);
    bool PlayersCanRespawnForASetPeriodOfTime{};
    FP RespawnSetTime = FP::FromInt(60);
    bool allowShrinkingZone{};
    FP shrinkingZoneSpeed = FP::FromInt(1);
    FP shrinkingZoneDelay = FP::FromInt(60);
    FP shrinkingZoneMinRadius = FP::FromInt(1);
    bool allowMapDisappearance{};
    FP disappearanceBufferRadius = FP::FromInt(10);
    FPVector3 mapCenter;
    bool autoFinishGameOnLastPlayerStanding = true;
    FP outroTimeScale = FP::FromRaw(6553); // 0.0999908
    FP outroTimeScalingDuration = FP::FromInt(2);
    AssetRefBoltDropperSpec boltDropperSpec;
    AssetRefBoltGathererActorSpec boltGathererSpec;
    AssetRefDroppableCollectorSpec droppableCollectorSpecRef;
    AssetRefDashActorConfig dashActorConfig;
    bool useBoltPowerAndPerks = true;
    AssetRefBoltPowerActorSpec boltPowerActorSpec;
    std::int32_t ultimateBoostMinRank = 6;
    bool useLives{};
    std::int32_t maxLives = 3;
    bool useLivesForBots = true;
    std::int32_t maxLivesForBots = 3;
    bool showKillFeed = true;
    bool autoShootEnabled = true;
    FP autoShootDelay = FP::FromRaw(16384); // 0.25

    // 0x7c91fa4
    GameModeSpec() = default;

    // 0x7c91124
    virtual FP GetIntroDuration() const { return introDuration; }
    // 0x7c9112c
    virtual FP GetOutroDuration(Frame* frame) const {
        (void)frame;
        return outroDuration;
    }
    // TODO(0x7c91134, Quantum.GameModeSpec.GetTeamRepartition): Queue<int>>
    //   GetTeamRepartition(Frame frame)
    // 0x7c91364 - 1 unless a mode says otherwise.
    virtual FP GetBoltsDropMultiplier(Frame* frame) const {
        (void)frame;
        return FP::FromInt(1);
    }
    // 0x7c9136c - whether a player trailing by `playerScoreGap` is within closeToWinScoreGap.
    virtual bool IsCloseToWin(std::int32_t playerScoreGap) const { return closeToWinScoreGap >= playerScoreGap; }
    // TODO(0x7c9137c, Quantum.GameModeSpec.OnPlayerInitialSpawn): void OnPlayerInitialSpawn(Frame
    //   frame, EntityRef playerEntity, PlayerRef playerRef, int playerIndex)
    // TODO(0x7c913fc, Quantum.GameModeSpec.SetupPlayerMaxSpeedModifier): void
    //   SetupPlayerMaxSpeedModifier(Frame frame, EntityRef playerEntity)
    // TODO(0x7c91498, Quantum.GameModeSpec.SetupPlayerBoltDropper): void
    //   SetupPlayerBoltDropper(Frame frame, EntityRef playerEntity)
    // TODO(0x7c9193c, Quantum.GameModeSpec.SetupPlayerBoltGatherer): void
    //   SetupPlayerBoltGatherer(Frame frame, EntityRef playerEntity)
    // TODO(0x7c91be8, Quantum.GameModeSpec.SetupPlayerDroppableCollector): void
    //   SetupPlayerDroppableCollector(Frame frame, EntityRef playerEntity)
    // TODO(0x7c91c90, Quantum.GameModeSpec.SetupPlayerDashActor): void SetupPlayerDashActor(Frame
    //   frame, EntityRef playerEntity)
    // TODO(0x7c91d38, Quantum.GameModeSpec.SetupPlayerBoltPower): void SetupPlayerBoltPower(Frame
    //   frame, EntityRef playerEntity)
    // TODO(0x7c91de0, Quantum.GameModeSpec.SetupFTUEBotHealthSpec): void
    //   SetupFTUEBotHealthSpec(Frame frame, EntityRef playerEntity)
    // 0x7c91ecc - the first spawn point unless a mode picks another.
    virtual std::int32_t ProvideInitialSpawnPointIndex(Frame* f, PlayerRef playerRef, std::int32_t teamId, std::int32_t playerIndex,
                                               Collections::QList<std::int16_t> availableSpawnPointsIndices) const {
        (void)f, (void)playerRef, (void)teamId, (void)playerIndex, (void)availableSpawnPointsIndices;
        return 0;
    }
    // Abstract in the original.
    virtual bool RespawnPointIsValid(Frame* f, EntityRef entityToRespawn, const MapCustomData::SpawnPointData& spData,
                             const SpawnPointDynamicData& spDynData) const {
        (void)f, (void)entityToRespawn, (void)spData, (void)spDynData;
        NotReimplemented("Quantum.GameModeSpec.RespawnPointIsValid");
    }
    // 0x7c91ed4 - nothing by default.
    virtual void OnPlayerKilled(Frame* frame, EntityRef killedEntity) const { (void)frame, (void)killedEntity; }
    // 0x7c91ed8 - the custom bot's entry of BotAggroBehaviourByIndex, DefaultAggroBehaviour outside it.
    virtual BOT_AGGRO_BEHAVIOUR GetBotAggroBehaviour(Frame* frame, std::int32_t customBotIndex) const;
    // TODO(0x7c91f08, Quantum.GameModeSpec.CanRespawn): bool CanRespawn(Frame frame, EntityRef
    //   killedEntity, int startFrame)
    virtual bool CanRespawn(Frame* frame, EntityRef killedEntity, std::int32_t startFrame) const {
        (void)frame, (void)killedEntity, (void)startFrame;
        NotReimplemented("Quantum.GameModeSpec.CanRespawn");
    }
    // 0x7c91f9c - autoShootEnabled, whatever the target.
    virtual bool CanAutoShootTarget(Frame* frame, EntityRef localPlayerEntityRef, EntityRef targetEntityRef) const {
        (void)frame, (void)localPlayerEntityRef, (void)targetEntityRef;
        return autoShootEnabled;
    }
};

class GameModesTable : public AssetObject {
  public:
    std::vector<GameModesTableEntry> prototypes;

    // 0x7c9111c
    GameModesTable() = default;
};

class GameModifiersSpec : public AssetObject {
  public:
    RandomWeaponsModifierSpec randomWeapon;

    // 0x7c920dc
    GameModifiersSpec() = default;
};

class GoldenBoltConfig : public AssetObject {
  public:
    AssetRefEntityPrototype GoldenBoltToSpawn;
    std::uint16_t BoltsEarned = 100;
    std::vector<FP> SpawnDelaysSeconds;
    FP DropDurationSeconds = FP::FromInt(10);
    FP DropHeight = FP::FromInt(20);
    FP ArrivalHeightOffset = FP::FromInt(1);
    FP SafeDurationAfterArrivalInClosingZone = FP::FromInt(15);

    // 0x7c920e4
    GoldenBoltConfig() = default;
};

class MobGroupConfig : public AssetObject {
  public:
    AssetRefEntityPrototype MemberEntityPrototype;

    // 0x7c934a4
    MobGroupConfig() = default;
};

class MobInfoSpec : public AssetObject {
  public:
    std::vector<::Quantum::MobLevelMappingData> MobLevelMappingData;

    // 0x7c93738
    MobInfoSpec() = default;

    // TODO(0x7c934ac, Quantum.MobInfoSpec.TryGetMobLevel): bool TryGetMobLevel(MobType type, FP
    //   pveScaling, ushort resultLevel)
};

class SimulationHeroesTable : public AssetObject {
  public:
    struct Entry {
        GENERATED_CHARACTER_ID uniqueID{};
        AssetRefHeroSpec spec;

        // 0x7c926f8
        Entry() = default;
    };

    AssetRefHeroSpec defaultSpec;
    std::vector<SimulationHeroesTable::Entry> entries;

    // 0x7c92670
    SimulationHeroesTable() = default;
};

class SquadSiegeConfig : public AssetObject {
  public:
    std::vector<AssetRefSquadSiegeLevelConfig> Levels;
    ENTITY_INFO_FILTER AttackerDestructionEntityInfoType = ENTITY_INFO_FILTER::MOB;
    FP AttackerDestructionRange = FP::FromInt(5);
    FP MobAutoDestructRange = FP::FromInt(3);
    FP MobAutoDestructStuckDelay = FP::FromInt(10);
    FP MobAutoDestructAfterLastSpawnDelay = FP::FromInt(90);

    // 0x7c94e7c
    SquadSiegeConfig() = default;
};

class SquadSiegeLevelConfig : public AssetObject {
  public:
    std::string SceneName;
    FP PveScaling = FP::FromInt(1);
    std::int32_t PowerRequired = 2000;
    std::vector<WaveData> Waves;

    // 0x7c94ea0
    SquadSiegeLevelConfig() = default;
};

class WaveManagerSpec : public AssetObject {
  public:
    FP InitialWaveDelay = FP::FromInt(3);
    FP WaveStartDelay = FP::FromInt(2);
    bool AutoStartOnMainLoopStart = true;
    bool FinishGameOnAllWavesComplete = true;
    FP FinishGameDelay = FP::FromInt(2);
    FP MobInitialInvulnerabilityDuration = FP::FromInt(4);

    // 0x7bd2934
    WaveManagerSpec() = default;
};

class WaveReferences : public AssetObject {
  public:
    struct DroperReference {
        std::string ID;
        std::vector<AssetRefEntityDropperSpec> DropperSpecs;
    };

    struct WaveMobReference {
        std::string ID;
        AssetRefEntityPrototype EntityToSpawn;
    };

    std::vector<WaveReferences::WaveMobReference> MobReferenceArray;
    std::vector<WaveReferences::DroperReference> DropperReferenceArray;

    // 0x7c94e74
    WaveReferences() = default;

    // TODO(0x7c94cc8, Quantum.WaveReferences.TryGetMobEntityPrototype): bool
    //   TryGetMobEntityPrototype(string id, AssetRefEntityPrototype entityToSpawn)
    // TODO(0x7c94d94, Quantum.WaveReferences.TryGetDropperSpecs): bool TryGetDropperSpecs(string
    //   id, AssetRefEntityDropperSpec[] dropperSpec)
};

class BlastBallGameModeSpec : public GameModeSpec {
  public:
    AssetRefEntityPrototype footballPrototype;
    std::uint16_t scoreToWin = 3;
    FP scoringSequenceDuration = FP::FromInt(5);
    FP resetPositionsDelay = FP::FromInt(2);
    FP overtimeTransitionDuration = FP::FromInt(1);
    FP overtimeTimeScaleDuration = FP::FromInt(1);
    FP restartDelay = FP::FromInt(3);
    FP maxHitToGoalDelay = FP::FromInt(5);
    FP autoShootMinBallToGoalDistance = FP::FromInt(25);
    FP autoShootMaxDot{};
    FP goalSize = FP::FromInt(4);
    FP mainZoneGoalSizeMultiplier = FP::FromRaw(32768); // 0.5

    // 0x7cde2e0
    BlastBallGameModeSpec() = default;

    // TODO(0x7cddbc4, Quantum.BlastBallGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7cddc34, Quantum.BlastBallGameModeSpec.DeInit): void DeInit(Frame f)
    // TODO(0x7cddca4, Quantum.BlastBallGameModeSpec.OnPlayerInitialSpawn): void
    //   OnPlayerInitialSpawn(Frame frame, EntityRef playerEntity, PlayerRef playerRef, int
    //   playerIndex)
    // TODO(0x7cddd34, Quantum.BlastBallGameModeSpec.ProvideInitialSpawnPointIndex): int
    //   ProvideInitialSpawnPointIndex(Frame f, PlayerRef playerRef, int teamId, int playerIndex,
    //   QList<short> availableSpawnPointsIndices)
    // TODO(0x7cdde88, Quantum.BlastBallGameModeSpec.RespawnPointIsValid): bool
    //   RespawnPointIsValid(Frame frame, EntityRef entityToRespawn, SpawnPointData spData,
    //   SpawnPointDynamicData spDynData)
    // 0x7cddefc - always.
    bool CanRespawn(Frame* frame, EntityRef killedEntity, std::int32_t startFrame) const override {
        (void)frame, (void)killedEntity, (void)startFrame;
        return true;
    }
    // TODO(0x7cddf04, Quantum.BlastBallGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
    // 0x7cde044 - win/draw/lose from the local and opposing scores.
    static BATTLE_RESULT GetBattleResult(std::int32_t rank, std::int32_t nbPlayersInBattle, std::int32_t localScore,
                                         std::int32_t opponentScore);
    // TODO(0x7cde058, Quantum.BlastBallGameModeSpec.CanAutoShootTarget): bool
    //   CanAutoShootTarget(Frame frame, EntityRef localPlayerEntityRef, EntityRef targetEntityRef)
};

class BoltKingGameModeSpec : public GameModeSpec {
  public:
    FP FirstPhaseDelay = FP::FromInt(1);
    FP NextPhaseDelay = FP::FromInt(3);
    FP EndGameDelay = FP::FromInt(3);
    std::int32_t ScoreToBecomeKing = 100;
    FP KingChaseDuration = FP::FromInt(20);
    FP KingSpeedModifier = FP::FromRaw(-21845); // -0.333328
    FP KingMaxHpMultiplier = FP::FromRaw(98304); // 1.5
    FP KingDamageMultiplier = FP::FromRaw(98304); // 1.5
    FP KingSizeMultiplier = FP::FromInt(2);
    FP KingAnimatorPlaybackSpeed = FP::FromRaw(32768); // 0.5
    AssetRefOverhealthPowerUpSpec overHealthSpec;
    FP ChaserSpeedModifier = FP::FromRaw(87381); // 1.33333
    std::vector<std::int32_t> BotChasersCountPerHumanPlayerCount;
    bool AllBotChaseHumanKing{};

    // 0x7ba1c64
    BoltKingGameModeSpec() = default;

    // TODO(0x7ba1514, Quantum.BoltKingGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7ba1584, Quantum.BoltKingGameModeSpec.DeInit): void DeInit(Frame f)
    // TODO(0x7ba15f4, Quantum.BoltKingGameModeSpec.ProvideInitialSpawnPointIndex): int
    //   ProvideInitialSpawnPointIndex(Frame f, PlayerRef playerRef, int teamId, int playerIndex,
    //   QList<short> availableSpawnPointsIndices)
    // TODO(0x7ba174c, Quantum.BoltKingGameModeSpec.OnPlayerInitialSpawn): void
    //   OnPlayerInitialSpawn(Frame frame, EntityRef playerEntity, PlayerRef playerRef, int
    //   playerIndex)
    // 0x7ba19ac - every spawn point is.
    bool RespawnPointIsValid(Frame* f, EntityRef entityToRespawn, const MapCustomData::SpawnPointData& spData,
                             const SpawnPointDynamicData& spDynData) const override {
        (void)f, (void)entityToRespawn, (void)spData, (void)spDynData;
        return true;
    }
    // TODO(0x7ba19b4, Quantum.BoltKingGameModeSpec.CanRespawn): bool CanRespawn(Frame frame,
    //   EntityRef killedEntity, int startFrame)
    // TODO(0x7ba19fc, Quantum.BoltKingGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
    // 0x7ba1c4c - a win for the better half of the ranking (rank below half the player count), else a loss.
    static BATTLE_RESULT GetBattleResult(std::int32_t rank, std::int32_t nbPlayersInBattle, std::int32_t localScore,
                                         std::int32_t opponentScore);
};

class EliminationGameModeSpec : public GameModeSpec {
  public:
    struct BotAggroSettings {
        FTUE_STEP_ID FtueStep{};
        BOT_AGGRO_BEHAVIOUR DefaultBotAggroBehaviour{};
        std::vector<BOT_AGGRO_BEHAVIOUR> BotAggroBehaviourByIndex;

        // 0x7ba2a48
        BotAggroSettings() = default;
    };

    AssetRefEntityDropperSpec PowerUpEntityDropperSpecAsset;
    std::vector<EliminationGameModeSpec::BotAggroSettings> BotAggroSettingsByFtueStep;

    // 0x7ba2a40
    EliminationGameModeSpec() = default;

    // TODO(0x7ba2430, Quantum.EliminationGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7ba2480, Quantum.EliminationGameModeSpec.DeInit): void DeInit(Frame f)
    // TODO(0x7ba24d0, Quantum.EliminationGameModeSpec.OnPlayerInitialSpawn): void
    //   OnPlayerInitialSpawn(Frame frame, EntityRef playerEntity, PlayerRef playerRef, int
    //   playerIndex)
    // TODO(0x7ba2694, Quantum.EliminationGameModeSpec.GetBotAggroBehaviour): BOT_AGGRO_BEHAVIOUR
    //   GetBotAggroBehaviour(Frame frame, int customBotIndex)
    // 0x7ba2848 - every spawn point is.
    bool RespawnPointIsValid(Frame* f, EntityRef entityToRespawn, const MapCustomData::SpawnPointData& spData,
                             const SpawnPointDynamicData& spDynData) const override {
        (void)f, (void)entityToRespawn, (void)spData, (void)spDynData;
        return true;
    }
    // TODO(0x7ba2850, Quantum.EliminationGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
};

class FreeForAllGameModeSpec : public GameModeSpec {
  public:
    std::uint32_t killsToWin = 12u;

    // 0x7ba3138
    FreeForAllGameModeSpec() = default;

    // TODO(0x7ba2de0, Quantum.FreeForAllGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7ba2e50, Quantum.FreeForAllGameModeSpec.DeInit): void DeInit(Frame f)
    // 0x7ba2ec0 - every spawn point is.
    bool RespawnPointIsValid(Frame* f, EntityRef entityToRespawn, const MapCustomData::SpawnPointData& spData,
                             const SpawnPointDynamicData& spDynData) const override {
        (void)f, (void)entityToRespawn, (void)spData, (void)spDynData;
        return true;
    }
    // 0x7ba2ec8 - always.
    bool CanRespawn(Frame* frame, EntityRef killedEntity, std::int32_t startFrame) const override {
        (void)frame, (void)killedEntity, (void)startFrame;
        return true;
    }
    // TODO(0x7ba2ed0, Quantum.FreeForAllGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
    // 0x7ba3120 - a win for the better half of the ranking (rank below half the player count), else a loss.
    static BATTLE_RESULT GetBattleResult(std::int32_t rank, std::int32_t nbPlayersInBattle, std::int32_t localScore,
                                         std::int32_t opponentScore);
};

class SquadSiegeGameModeSpec : public GameModeSpec {
  public:

    // 0x7ba8214
    SquadSiegeGameModeSpec() = default;

    // TODO(0x7ba7e34, Quantum.SquadSiegeGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7ba7e84, Quantum.SquadSiegeGameModeSpec.DeInit): void DeInit(Frame f)
    // TODO(0x7ba7ef4, Quantum.SquadSiegeGameModeSpec.ProvideInitialSpawnPointIndex): int
    //   ProvideInitialSpawnPointIndex(Frame f, PlayerRef playerRef, int teamId, int playerIndex,
    //   QList<short> availableSpawnPointsIndices)
    // TODO(0x7ba8048, Quantum.SquadSiegeGameModeSpec.RespawnPointIsValid): bool
    //   RespawnPointIsValid(Frame frame, EntityRef entityToRespawn, SpawnPointData spData,
    //   SpawnPointDynamicData spDynData)
    // 0x7ba80bc - always.
    bool CanRespawn(Frame* frame, EntityRef killedEntity, std::int32_t startFrame) const override {
        (void)frame, (void)killedEntity, (void)startFrame;
        return true;
    }
    // TODO(0x7ba80c4, Quantum.SquadSiegeGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
};

class TeamTotalRumbleGameModeSpec : public GameModeSpec {
  public:
    std::uint32_t killsToWin = 12u;

    // 0x7baa948
    TeamTotalRumbleGameModeSpec() = default;

    // TODO(0x7baa3a4, Quantum.TeamTotalRumbleGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7baa414, Quantum.TeamTotalRumbleGameModeSpec.DeInit): void DeInit(Frame f)
    // TODO(0x7baa484, Quantum.TeamTotalRumbleGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
    // 0x7baa760 - win/draw/lose from the team scores.
    static BATTLE_RESULT GetBattleResult(std::int32_t localScore, std::int32_t opponentScore);
    // 0x7baa778 - always.
    bool CanRespawn(Frame* frame, EntityRef killedEntity, std::int32_t startFrame) const override {
        (void)frame, (void)killedEntity, (void)startFrame;
        return true;
    }
    // TODO(0x7baa780, Quantum.TeamTotalRumbleGameModeSpec.ProvideInitialSpawnPointIndex): int
    //   ProvideInitialSpawnPointIndex(Frame f, PlayerRef playerRef, int teamId, int playerIndex,
    //   QList<short> availableSpawnPointsIndices)
    // TODO(0x7baa8d4, Quantum.TeamTotalRumbleGameModeSpec.RespawnPointIsValid): bool
    //   RespawnPointIsValid(Frame f, EntityRef entityToRespawn, SpawnPointData spData,
    //   SpawnPointDynamicData spDynData)
};

class TrainingGameModeSpec : public GameModeSpec {
  public:

    // 0x7bab640
    TrainingGameModeSpec() = default;

    // TODO(0x7bab430, Quantum.TrainingGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7bab4e0, Quantum.TrainingGameModeSpec.DeInit): void DeInit(Frame f)
    // 0x7bab598 - no intro.
    FP GetIntroDuration() const override { return FP::FromRaw(0); }
    // TODO(0x7bab5a0, Quantum.TrainingGameModeSpec.OnPlayerInitialSpawn): void
    //   OnPlayerInitialSpawn(Frame frame, EntityRef playerEntity, PlayerRef playerRef, int
    //   playerIndex)
    // 0x7bab630 - a spawn point whose dynamic data marks it valid (lowest bit).
    bool RespawnPointIsValid(Frame* f, EntityRef entityToRespawn, const MapCustomData::SpawnPointData& spData,
                             const SpawnPointDynamicData& spDynData) const override;
    // TODO(0x7bab63c, Quantum.TrainingGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
};

class TutorialGameModeSpec : public GameModeSpec {
  public:
    struct StepBehaviour {
        TutorialGameModeStep step{};
        std::vector<UnlockedFeature> featuresToUnlock;

        // 0x7bac028
        StepBehaviour() = default;
    };

    std::vector<TutorialGameModeSpec::StepBehaviour> stepBehaviours;
    std::string playerName = "Player";
    AssetRefWeaponLoadoutSpec playerLoadoutSpec;
    AssetRefWeaponSpec overrideMeleeWeaponSpec;
    AssetRefWeaponSpec overrideUltimateWeaponSpec;
    AssetRefGadgetSpec overrideGadgetSpec;

    // 0x7babf7c
    TutorialGameModeSpec() = default;

    // TODO(0x7bab904, Quantum.TutorialGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7bab954, Quantum.TutorialGameModeSpec.DeInit): void DeInit(Frame f)
    // 0x7bab9a4 - no intro.
    FP GetIntroDuration() const override { return FP::FromRaw(0); }
    // TODO(0x7bab9ac, Quantum.TutorialGameModeSpec.OnPlayerInitialSpawn): void
    //   OnPlayerInitialSpawn(Frame frame, EntityRef playerEntity, PlayerRef playerRef, int
    //   playerIndex)
    // TODO(0x7babc8c, Quantum.TutorialGameModeSpec.ProvideInitialSpawnPointIndex): int
    //   ProvideInitialSpawnPointIndex(Frame frame, PlayerRef playerRef, int teamId, int
    //   playerIndex, QList<short> availableSpawnPointsIndices)
    // 0x7babdf4 - always.
    bool CanRespawn(Frame* frame, EntityRef killedEntity, std::int32_t startFrame) const override {
        (void)frame, (void)killedEntity, (void)startFrame;
        return true;
    }
    // 0x7babdfc - every spawn point is.
    bool RespawnPointIsValid(Frame* f, EntityRef entityToRespawn, const MapCustomData::SpawnPointData& spData,
                             const SpawnPointDynamicData& spDynData) const override {
        (void)f, (void)entityToRespawn, (void)spData, (void)spDynData;
        return true;
    }
    // TODO(0x7babe04, Quantum.TutorialGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
    // TODO(0x7babe08, Quantum.TutorialGameModeSpec.TryGetStepBehaviour): bool
    //   TryGetStepBehaviour(TutorialGameModeStep step, StepBehaviour behaviour)
};

class WeaponShowcaseGameModeSpec : public GameModeSpec {
  public:
    AssetRefEntityPrototype weaponUserPrototype;
    FP isUnlockingAnimationDelay{};
    FP moveToShootPositionDelay{};
    FP moveToShootPositionDuration{};
    FPAnimationCurve moveToShootAnimationCurve;
    FP triggerPressedDelay{};
    FP moveBackToSpawnPositionDelay{};
    FP moveBackToSpawnPositionDuration{};
    FPAnimationCurve moveBackToSpawnAnimationCurve;
    std::vector<WeaponVisualParams> allWeaponParams;
    FP upgradeXPos{};
    bool moveWeaponsManually{};
    bool blockFirstRotation{};
    bool blockShoot{};
    bool blockLastRotation{};

    // 0x7bae548
    WeaponShowcaseGameModeSpec() = default;

    // TODO(0x7bae37c, Quantum.WeaponShowcaseGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7bae3cc, Quantum.WeaponShowcaseGameModeSpec.DeInit): void DeInit(Frame f)
    // 0x7bae41c - no intro.
    FP GetIntroDuration() const override { return FP::FromRaw(0); }
    // 0x7bae424 - every spawn point is.
    bool RespawnPointIsValid(Frame* f, EntityRef entityToRespawn, const MapCustomData::SpawnPointData& spData,
                             const SpawnPointDynamicData& spDynData) const override {
        (void)f, (void)entityToRespawn, (void)spData, (void)spDynData;
        return true;
    }
    // TODO(0x7bae42c, Quantum.WeaponShowcaseGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
    // TODO(0x7bae430, Quantum.WeaponShowcaseGameModeSpec.GetWeaponParamsFromID): WeaponVisualParams
    //   GetWeaponParamsFromID(GENERATED_WEAPON_ID weaponID)
};

class ZoneControlGameModeSpec : public GameModeSpec {
  public:
    std::uint16_t VictoryPointsToWin = 2;
    FP ControlZoneInitialLoadingTime = FP::FromInt(15);
    FP ControlZoneLoadingTime = FP::FromInt(10);
    FP ControlZoneRequiredTime = FP::FromInt(5);
    FP TotalDurationForCompletion = FP::FromInt(10);
    FP ScoringIncrementFrequency = FP::FromRaw(32768); // 0.5
    FP ComeBackToNeutralDuration = FP::FromInt(10);
    FP DelayBeforeLosingControl = FP::FromInt(1);

    // 0x7baf9c0
    ZoneControlGameModeSpec() = default;

    // TODO(0x7baf5f0, Quantum.ZoneControlGameModeSpec.Init): void Init(Frame f)
    // TODO(0x7baf640, Quantum.ZoneControlGameModeSpec.DeInit): void DeInit(Frame f)
    // TODO(0x7baf690, Quantum.ZoneControlGameModeSpec.ComputeBattleResult): void
    //   ComputeBattleResult(QuantumGame game, EntityRef localPlayerEntityRef, BattleResultInfo
    //   battleResultInfo)
    // TODO(0x7baf7d4, Quantum.ZoneControlGameModeSpec.RespawnPointIsValid): bool
    //   RespawnPointIsValid(Frame f, EntityRef entityToRespawn, SpawnPointData spData,
    //   SpawnPointDynamicData spDynData)
    // TODO(0x7baf848, Quantum.ZoneControlGameModeSpec.ProvideInitialSpawnPointIndex): int
    //   ProvideInitialSpawnPointIndex(Frame f, PlayerRef playerRef, int teamId, int playerIndex,
    //   QList<short> availableSpawnPointsIndices)
    // 0x7baf99c
    FP GetZoneLoadingTime(Frame* f, const ZoneControlGameMode& gameMode) const;
};

} // namespace Quantum
