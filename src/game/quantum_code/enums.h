// enums.h - the enumerations of the gameplay simulation (quantum.code.dll).
//
// Values and underlying types are the originals': they are stored in components and
// serialised, so they are part of the deterministic state.
#pragma once

#include <cstdint>

namespace Quantum {

enum class AIBlackboardValueType : std::int32_t {
    Boolean = 0,
    Byte = 1,
    Integer = 2,
    FP = 3,
    Vector2 = 4,
    Vector3 = 5,
    EntityRef = 6,
    AssetRef = 7,
};

enum class AIParamSource : std::int32_t {
    None = 0,
    Value = 1,
    Config = 2,
    Blackboard = 3,
    Function = 4,
};

enum class AI_NAVIGATION_TYPE : std::uint8_t {
    TO_ENTITY = 0,
    TO_POSITION = 1,
};

enum class AccelerationType : std::int32_t {
    Linear = 0,
    Curve = 1,
};

enum class ActorInputLockFlags : std::int32_t {
    None = 0,
    Movement = 1,
    Abilities = 2,
    Melee = 4,
    Loadout = 8,
    FiringAndUltimate = 16,
    ExternalCommand = 32,
    FiringUltimateAndLoadout = 24,
    All = 31,
    AllExceptLoadout = 23,
};

enum class ActorStatusType : std::int32_t {
    None = 0,
    Normal = 1,
    Iced = 2,
    Sheeped = 3,
    Stunned = 4,
    Tank = 5,
};

enum class AgentOfDoomEventType : std::int32_t {
    SPAWN = 0,
    IDLE = 1,
    RUN = 2,
    FOUNDTARGET = 3,
    DEATH = 4,
};

enum class BALL_HOLDING_BREAKS_REASON : std::int32_t {
    SHOOT = 0,
    HEALTH = 1,
    MELEE = 2,
    DASH = 3,
    SCORED = 4,
};

enum class BOT_AGGRO_BEHAVIOUR : std::int32_t {
    None = 0,
    AttackBots = 1,
    AttackHumans = 2,
};

enum class BTAbort : std::int32_t {
    None = 0,
    Self = 1,
    LowerPriority = 2,
    Both = 3,
};

enum class BTStatus : std::int32_t {
    Inactive = 0,
    Success = 1,
    Failure = 2,
    Running = 3,
    Abort = 4,
};

enum class BlastBallBehaviorState : std::int32_t {
    OFFENSIVE = 0,
    DEFENSIVE = 1,
};

enum class BlastBallGameState : std::int32_t {
    Playing = 0,
    Scored = 1,
    OverTime = 2,
    Restarting = 3,
    GameOver = 4,
};

enum class BlastBallShootingZone : std::int32_t {
    NONE = 0,
    MAIN = 1,
    SECONDARY = 2,
};

enum class BlastBallTeamState : std::int32_t {
    Defending = 0,
    Attacking = 1,
};

enum class BombDropSide : std::int32_t {
    Left = 0,
    Right = 1,
};

enum class BombardierDroneState : std::int32_t {
    TakingOff = 0,
    Flying = 1,
    BeingDestroyed = 2,
};

enum class BumpType : std::int32_t {
    BumpReductionPercentage = 0,
    FlatBumpStrength = 1,
};

enum class CONTROL_ZONE_STATE : std::int32_t {
    DISABLED = 0,
    LOADING = 1,
    NEUTRAL = 2,
    CONTROLLED = 3,
    CONTESTED = 4,
    COMPLETED = 5,
};

enum class ChargeAttackState : std::int32_t {
    None = 0,
    Anticipation = 1,
    Movement = 2,
    Cooldown = 3,
};

enum class CollectionSource : std::int32_t {
    Default = 0,
    Droppable = 1,
};

enum class ColliderResizeStatus : std::int32_t {
    NONE = 0,
    SPAWNED = 1,
    RESIZE = 2,
    COMPLETED = 3,
};

enum class DAMAGE_FILTER : std::int32_t {
    None = 0,
    Normal = 2,
    Electric = 4,
    Fire = 8,
    Acid = 16,
    Ice = 32,
    All = -1,
};

enum class DAMAGE_MULTIPLIER_ORIGIN : std::int32_t {
    Spawner = 0,
    CriticalHit = 1,
};

enum class DASH_DISABLED_REASON : std::int32_t {
    NONE = 0,
    ACTOR_STATUS = 1,
    BUMPER = 2,
    FEATURE_LOCK = 4,
};

enum class DamageType : std::int32_t {
    None = 0,
    Normal = 1,
    Electric = 2,
    Fire = 3,
    Acid = 4,
    Ice = 5,
};

enum class DeathAnimationState : std::int32_t {
    INACTIVE = 0,
    GROUNDED_LOOP = 1,
    GROUNDED_END = 2,
    IN_AIR = 3,
};

enum class DrillDashState : std::int32_t {
    Anticipation = 0,
    Dash = 1,
    Attack = 2,
    LastHit = 3,
    Recovery = 4,
};

enum class DroppableState : std::uint8_t {
    NotUsed = 0,
    WillDrop = 1,
    Dropping = 2,
    Static = 3,
    Collecting = 4,
    ToDestroy = 5,
};

enum class DroppableType : std::uint8_t {
    Undefined = 0,
    Bolt = 1,
    EnergyCellLow = 2,
    EnergyCellMedium = 3,
    EnergyCellHigh = 4,
    EventToken = 5,
    ExtraLife = 6,
    RegenHP = 7,
    ReviveTotem = 8,
    PowerUp_Overhealth = 9,
    PowerUp_ExtraDamage = 10,
    PowerUp_SpeedBoost = 11,
    PowerUp_PermanentOverhealth = 12,
    PowerUp_ExtraDamage_PVE = 13,
};

enum class ENTITY_INFO_FILTER : std::int32_t {
    None = 0,
    Player = 2,
    BasicMob = 4,
    PopcornMob = 8,
    Boss = 16,
    Crate = 32,
    BounceBalloon = 64,
    BlastBall = 128,
    MiscMob = 256,
    MiscEnvironment = 512,
    ObjectiveEntity = 1024,
    MiniBoss = 4096,
    Holoshield = 8192,
    PowerUp = 16384,
    PowerUpCrate = 32768,
    ControlZone = 65536,
    PLAYER = 2,
    MOB = 268,
    NPC = 4380,
    BOSS = 4112,
    ENVIRONMENT = 58080,
    All = -1,
};

enum class ETargetType : std::int32_t {
    None = 0,
    Player = 1,
    NPC = 2,
    Environment = 3,
};

enum class EValueComparison : std::int32_t {
    None = 0,
    LessThan = 1,
    MoreThan = 2,
    EqualTo = 3,
};

enum class EWorldState : std::uint32_t {
    None = 0u,
    Idle = 1u,
    CarryingCollectible = 2u,
    ColletibleDelivered = 4u,
};

enum class EasingType : std::int32_t {
    Unset = 0,
    Linear = 1,
    InSine = 2,
    OutSine = 3,
    InOutSine = 4,
    InQuad = 5,
    OutQuad = 6,
    InOutQuad = 7,
    InCubic = 8,
    OutCubic = 9,
    InOutCubic = 10,
    InQuart = 11,
    OutQuart = 12,
    InOutQuart = 13,
    InQuint = 14,
    OutQuint = 15,
    InOutQuint = 16,
    InExpo = 17,
    OutExpo = 18,
    InOutExpo = 19,
    InCirc = 20,
    OutCirc = 21,
    InOutCirc = 22,
    InElastic = 23,
    OutElastic = 24,
    InOutElastic = 25,
    InBack = 26,
    OutBack = 27,
    InOutBack = 28,
    InBounce = 29,
    OutBounce = 30,
    InOutBounce = 31,
};

enum class ElementType : std::int32_t {
    None = 0,
    Normal = 1,
    Electric = 2,
    Fire = 3,
    Acid = 4,
    Ice = 5,
    Energetic = 6,
    Bio = 7,
    Water = 8,
};

enum class EntityInfoType : std::int32_t {
    None = 0,
    Player = 1,
    BasicMob = 2,
    PopcornMob = 3,
    Boss = 4,
    Crate = 5,
    BounceBalloon = 6,
    BlastBall = 7,
    MiscMob = 8,
    MiscEnvironment = 9,
    ObjectiveEntity = 10,
    GoldBolt = 11,
    MiniBoss = 12,
    Holoshield = 13,
    PowerUp = 14,
    PowerUpCrate = 15,
    ControlZone = 16,
    COUNT = 17,
};

enum class EntityPrefabsAmount : std::int32_t {
    Single = 0,
    Multiple = 1,
};

enum class FIRE_REQUEST_TYPE : std::uint8_t {
    None = 0,
    Wrench = 1,
    LongPress = 2,
};

enum class GAME_MODE : std::int32_t {
    UNDEFINED = 0,
    FREE_FOR_ALL = 1,
    BLAST_BALL = 2,
    BOLT_KING = 3,
    SQUAD_SIEGE = 4,
    TUTORIAL = 5,
    WEAPON_SHOWCASE = 6,
    TRAINING = 7,
    TEAM_TOTAL_RUMBLE = 8,
    QWARK_BALL = 9,
    ZONE_CONTROL = 10,
    ELIMINATION = 11,
    COUNT = 12,
};

enum class GAME_MODE_HANDLER_STATE : std::int32_t {
    WaitConnections = 0,
    WaitForPrefabsLoaded = 1,
    WaitAfterSpawnPlayersAndBots = 2,
    Cinematic = 3,
    PresentPlayers = 4,
    LevelIntro = 5,
    GameModeIntro = 6,
    Countdown = 7,
    MainLoop = 8,
    GameModeOutro = 9,
    Ended = 10,
    None = 11,
};

enum class GAME_MODE_STATUS : std::int32_t {
    VISIBLE = 0,
    HIDDEN_EXCLUDED_FROM_RELEASES = 1,
    HIDDEN = 2,
};

enum class GAME_MODIFICATOR : std::int32_t {
    NONE = 0,
    TUTORIAL = 1,
    PERFORMANCE_TEST = 2,
    TRAINING = 3,
};

enum class GENERATED_CHARACTER_ID : std::int32_t {
    NONE = 0,
    CLANK_BAG = 1,
    SANDY = 2,
    RAIDEN = 3,
    LIL_ANN = 4,
    CELESTE = 5,
    CHIP = 6,
    STEELJAW = 7,
    FUSE = 8,
    LUMP = 9,
    MARKUS = 10,
    CHANCE = 11,
    MOLLY = 12,
    HELEN = 13,
    SPROCKET = 14,
    ZEDONE = 15,
    AVATAR = 16,
    GRIMSHOT = 17,
    FLARE = 18,
    NAKARI = 19,
    TERRY = 20,
    CRUMBY = 21,
    RATCHET = 22,
    COUNT = 23,
};

enum class GENERATED_GADGET_ID : std::int32_t {
    NONE = 0,
    DRILL_DASH = 1,
    ELECTRICGRENADE = 2,
    LILANNMELEEGADGET = 3,
    CRYOSLIDER = 4,
    GLOVEOFDOOM = 5,
    MINELAUNCHER = 6,
    AMOEBOIDLAUNCHER = 7,
    VOIDREPULSER = 8,
    HOVERBOOTS = 9,
    BOMBARDIER = 10,
    HOLOSHIELDGLOVE = 11,
    WRENCHTHROW = 12,
    COUNT = 13,
};

enum class GENERATED_ULTIMATE_ID : std::int32_t {
    NONE = 0,
    RYNO = 1,
    NEGATRON = 2,
    SABRE = 3,
    HOVERBOARD = 4,
    SHEEPINATOR = 5,
    BIGBOOM = 6,
    EVOLUTION = 7,
    CANNON = 8,
    SLAM = 9,
    TANK = 10,
    SMOKESCREEN = 11,
    RATCHETRYNO = 12,
    COUNT = 13,
};

enum class GENERATED_WEAPON_ID : std::int32_t {
    NONE = 0,
    BLASTER = 1,
    GRENADELAUNCHER = 2,
    MINELAUNCHER = 3,
    GATLING = 4,
    SHOTGUN = 5,
    VOIDREPULSER = 6,
    RYNO = 7,
    NEGATRON = 8,
    SCORPIONFLAIL = 9,
    GLOVEOFDOOM = 10,
    HEADHUNTER = 11,
    SUCKER = 12,
    COLDSNAP = 13,
    WARMONGER = 14,
    SHEEPINATOR = 15,
    HOVERBOOTS = 16,
    HOVERBOARD = 17,
    SABRE = 18,
    ENFORCER = 19,
    BUZZBLADES = 20,
    PLASMASTRIKER = 21,
    BOMBARDIER = 22,
    HOLOSHIELDGLOVE = 23,
    BIGBOOM = 24,
    TESLACLAW = 25,
    AMOEBOIDLAUNCHER = 26,
    PYROCITOR = 27,
    CELESTEMELEE = 28,
    CHANCEMELEE = 29,
    CHIPMELEE = 30,
    FUSEMELEE = 31,
    LILANNMELEE = 32,
    LUMPMELEE = 33,
    MARKUSMELEE = 34,
    MOLLYMELEE = 35,
    MOPZMELEE = 36,
    STEELJAWMELEE = 37,
    WIDGETMELEE = 38,
    LAVAGUN = 39,
    CANNON = 40,
    CRYOSLIDER = 41,
    TANKCANNON = 42,
    ZEDONEMELEE = 43,
    SPROCKETMELEE = 44,
    HELENMELEE = 45,
    GRIMSHOTMELEE = 46,
    FLAREMELEE = 47,
    NAKARIMELEE = 48,
    TERRYMELEE = 49,
    WRENCHMELEE = 50,
    WIDGETMELEEGADGET = 51,
    ELECTRICGRENADE = 52,
    LILANNMELEEGADGET = 53,
    RATCHETBLASTER = 54,
    RATCHETRYNO = 55,
    WRENCHTHROW = 56,
    COUNT = 57,
};

enum class GOTO_DESTINATION : std::int32_t {
    UNKNOWN = 0,
    TARGET_CRATE = 1,
    WANDER_DESTINATION = 2,
    ATTACK_PLAYER = 3,
    TARGET_DROPPABLE = 4,
    TARGET_GOLDEN_BOLT = 5,
    TARGET_REVIVE_TOTEM = 6,
    TARGET_TEAM = 7,
    TARGET_STRIKE_POS = 8,
    TARGET_KING_POS = 9,
    TARGET_OBJECTIVE = 10,
    TARGET_ENEMY_BB_GOAL = 11,
    TARGET_BB_POSITION = 12,
    TARGET_BB_DEFENSE_POSITION = 13,
    TARGET_BB_PASSING = 14,
    TARGET_ZONE_CONTROL = 15,
};

enum class GameModeIntroFlags : std::int32_t {
    None = 0,
    LevelIntro = 1,
    GameModeIntro = 2,
    Countdown = 4,
    Default = 7,
};

enum class GoldenBoltState : std::int32_t {
    Descending = 0,
    Arrived = 1,
    Collected = 2,
};

enum class GroundDashEndReason : std::int32_t {
    Undefined = 0,
    GroundImpact = 1,
    ActorKilled = 2,
};

enum class HEALTH_MULTIPLIER_SOURCE : std::int32_t {
    BOLT_POWER = 0,
    ULTIMATE_EVOLUTION = 1,
    KING_CHASE = 2,
    SPAWNER = 3,
    ENHANCEMENT = 4,
};

enum class HealthSpecMode : std::int32_t {
    USE_AVERAGE_HUMAN_WEAPONS_LEVEL = 0,
    HERO = 1,
    USE_OWNER_WEAPON_LEVEL = 2,
    USE_PVE_SCALING = 3,
    FIXED = 4,
    USE_SELF_LEVEL = 5,
};

enum class INVULNERABILITY_FILTER : std::int32_t {
    NONE = 0,
    DAMAGE = 1,
    PUSH = 2,
    ALL = -1,
};

enum class InputButtons : std::int32_t {
    Melee = 1,
    Dash = 2,
    Jump = 4,
    Fire = 8,
    Ultimate = 16,
    Gadget = 32,
    Reload = 64,
};

enum class ItemSpawnerStatusType : std::int32_t {
    StartRespawnTimer = 0,
    StopRespawnTimer = 1,
    FinishSpawning = 2,
    SpawnerEnabledChanged = 3,
};

enum class JumpState : std::int32_t {
    None = 0,
    Jumping = 1,
    DoubleJumping = 2,
};

enum class KccSurfaceSetPriority : std::int32_t {
    None = 0,
    Static = 1,
    Dynamic = 2,
};

enum class KccSurfaceType : std::int32_t {
    Normal = 0,
    Ice = 1,
};

enum class KillType : std::int32_t {
    Undefined = 1,
    ZeroHP = 2,
    Push = 4,
    ShrinkingZone = 8,
    InstaKill = 16,
    KillZone = 32,
    GameRule = 64,
    Silent = 128,
};

enum class KillZoneSurfaceType : std::int32_t {
    Default = 0,
    SwampWater = 1,
    Water = 2,
};

enum class KillableOnKillBehaviour : std::int32_t {
    PreserveEntity = 0,
    DestroyEntity = 1,
};

enum class LookPointHitType : std::int32_t {
    None = 0,
    Wall = 1,
    Ground = 2,
};

enum class MOD_COMPUTE_MODE : std::int32_t {
    NONE = 0,
    ADDITIVE = 1,
    MINIMUM = 2,
};

enum class MOD_ORIGIN : std::int32_t {
    NONE = 0,
    ENEMY = 1,
    SELF_WEAPON = 2,
    SELF_WEAPON_EXOTIC = 3,
    SELF_WEAPON_ENHANCEMENT_PRIMARY = 4,
    SELF_WEAPON_ENHANCEMENT_SECONDARY = 5,
    SELF_WEAPON_ENHANCEMENT_ULTIMATE = 6,
    SELF_WRENCH = 7,
    SELF_GADGET = 8,
    SELF_AI_ACTION = 9,
    SELF_AI_DECISION = 10,
    LEDGESLOW = 11,
    SHIELD = 12,
    PUSHSTOP = 13,
    ZONE = 14,
    HELIPACK = 15,
    REBOUND = 16,
    ULTIMATE = 17,
    LEVELPERK_BONUSMOVESPEED = 18,
    SPEEDER = 19,
    KING_CHASE = 20,
    BALL_HOLDER = 21,
    TRAVOLATOR = 22,
    DASH = 23,
    GAME_MODE = 24,
    HERO_IMPROVEMENT = 25,
    HIT_RECEIVED = 26,
};

enum class MOD_TYPE : std::int32_t {
    Speed = 0,
    MaxSpeed = 1,
    Gravity = 2,
    MaxFallSpeed = 3,
    Acceleration = 4,
    Braking = 5,
};

enum class MeleeJumpAttackState : std::int32_t {
    Waiting = 0,
    Charging = 1,
    Performing = 2,
    Cooldown = 3,
};

enum class MobSlamAttackState : std::int32_t {
    None = 0,
    Prepare = 1,
    Jump = 2,
    Slam = 3,
};

enum class MobType : std::int32_t {
    None = 0,
    AgentOfDoom = 1,
    Amoeboid_Small = 2,
    BuryBlossom = 3,
    Duchess = 4,
    HornyToad = 5,
    Juggernaut = 6,
    Lobber = 7,
    Robomutt = 8,
    Sandshark = 9,
    Shooter = 10,
    Slammer = 11,
    Spinner = 12,
    Tank = 13,
    Amoeboid_Medium = 14,
    Amoeboid_Large = 15,
};

enum class MobsEventType : std::int32_t {
    TargetDetectedStart = 0,
    TargetDetectedEnd = 1,
    CooldownStart = 2,
    CooldownEnd = 3,
    ChargeAttackStart = 4,
    ChargeAttackEnd = 5,
    SweepAttackStart = 6,
    SweepAttackEnd = 7,
    Spawn = 8,
    IdleStart = 9,
    IdleEnd = 10,
    Walk = 11,
    Run = 12,
    Jump = 13,
    MeleeAttackStart = 14,
    MeleeAttackEnd = 15,
    AlertedStart = 16,
    AlertedEnd = 17,
    AttackPhaseStart = 18,
    AttackPhaseEnd = 19,
};

enum class MoveMode : std::int32_t {
    Offset = 0,
    Position = 1,
    Entity = 2,
};

enum class MoveOffsetOrientation : std::int32_t {
    World = 0,
    Local = 1,
};

enum class MoveState : std::int32_t {
    Waiting = 0,
    Moving = 1,
    Finished = 2,
};

enum class MoveToSpecMoveModes : std::int32_t {
    Offset = 0,
    Position = 1,
    Entity = 2,
};

enum class NAV_REGION : std::uint8_t {
    LONGJUMP = 0,
    JUMP = 1,
    RAIL = 2,
    AUTO_JUMPER = 3,
};

enum class PATH_FINDING_STATUS : std::uint8_t {
    STOPPED = 0,
    PATH_REQUESTED = 1,
    PATH_COMPUTING = 2,
    PATH_FOUND = 3,
    PATH_NOT_FOUND = 4,
};

enum class PUSH_FILTERS : std::int32_t {
    NONE = 0,
    PLAYER = 1,
    NPC = 2,
    ENVIRONMENT = 4,
};

enum class PlayerActorInputAction : std::int32_t {
    None = 0,
    Fire = 1,
    Melee = 2,
    Jump = 3,
    Dash = 4,
    Ultimate = 5,
    Gadget = 6,
    Reload = 7,
};

enum class PlayerActorInputActionFilter : std::int32_t {
    None = 0,
    Fire = 2,
    Melee = 4,
    Jump = 8,
    Dash = 16,
    Ultimate = 32,
    Gadget = 64,
    All = -1,
};

enum class PowerUpType : std::int32_t {
    None = 0,
    Overhealth = 1,
    ExtraDamage = 2,
    SpeedBoost = 3,
    EnergyCell = 4,
};

enum class PrioTargetType : std::int32_t {
    None = -1,
    Objective = 0,
    RecentThreat = 1,
    VisibleThreat = 2,
    AttackedByThreat = 3,
    GroupTarget = 4,
};

enum class ProjectileDeathType : std::int32_t {
    None = 0,
    Impact = 1,
    ImpactHit = 2,
    Lifetime = 3,
    Killed = 4,
};

enum class ProjectileHitShapeUserTag : std::int32_t {
    Default = 0,
    Headshot = 1,
};

enum class ProjectileImpactType : std::int32_t {
    None = 0,
    Die = 1,
};

enum class ProjectileTargetedPart : std::int32_t {
    None = 0,
    Feet = 1,
    Body = 2,
    Head = 3,
};

enum class ProjectileType : std::int32_t {
    Linear = 0,
    Grenade_DEPRECATED = 1,
    Curve = 2,
    Lob = 3,
    LobFixedFlightTime = 4,
};

enum class PushOnShieldBehavior : std::int32_t {
    DoPush = 0,
    CancelPush = 1,
};

enum class REMOVE_REASON : std::int32_t {
    REACHED_END = 1,
    KCC_GROUNDED = 2,
    HIT_RAIL = 4,
    HIT_BALLOON = 8,
    ENTITY_KILLED = 16,
    HIT_AUTO_JUMPER = 32,
    ENTITY_PUSHED = 64,
};

enum class RecordingFlags : std::int32_t {
    None = 0,
    Input = 1,
    Checksums = 2,
    Default = 3,
    All = 255,
};

enum class RewardReason : std::int32_t {
    Kill = 0,
    Killed = 1,
    GoldenBolt = 2,
    LevelPerk = 3,
    ShieldBreak = 4,
};

enum class STAT_ID : std::int32_t {
    N_LINE_OF_SIGHT_QUERIES = 0,
    COUNT = 1,
};

enum class ShieldSpecMode : std::int32_t {
    USE_AVERAGE_HUMAN_WEAPONS_LEVEL = 0,
    HERO = 1,
};

enum class ShieldState : std::int32_t {
    WAIT_FOR_REGEN = 0,
    REGENERATING = 1,
    FULL = 2,
};

enum class ShootingEnemyEventType : std::int32_t {
    Aggressive = 0,
    Tired = 1,
    StartShooting = 2,
    StopShooting = 3,
    Rebound = 4,
    CoolDownStarts = 5,
    CoolDownStops = 6,
    Pushed = 7,
    Dead = 8,
    TargetDetected = 9,
};

enum class SimulationConfigChecksumErrorDumpOptions : std::int32_t {
    SendAssetDBChecksums = 1,
    ReadableDynamicDB = 2,
    RawFPValues = 4,
    ComponentChecksums = 8,
};

enum class SimulationUpdateTime : std::int32_t {
    Default = 0,
    EngineDeltaTime = 1,
    EngineUnscaledDeltaTime = 2,
};

enum class SpawnPointOptions : std::int32_t {
    None = 0,
    Initial = 1,
    Respawn = 2,
    DebugStart = 4,
    Podium = 8,
    Tutorial = 16,
    LocalPlayerFirstSpawn = 32,
};

enum class SpinningEnemyEventType : std::int32_t {
    StartSpinning = 0,
    StopSpinning = 1,
    Rebound = 2,
    CoolDownStops = 3,
    Pushed = 4,
    Die = 5,
    SlamAttack_Prepare = 6,
    SlamAttack_Jump = 7,
    SlamAttack_Slam = 8,
    SlamAttack_Attack = 9,
    CoolDownStarts = 10,
    TargetDetected = 11,
};

enum class SweepAttackState : std::int32_t {
    None = 0,
    Prepare = 1,
    Sweeping = 2,
    Finished = 3,
};

enum class TeamFlag : std::int32_t {
    NEUTRAL = -2,
    NONE = -1,
    A = 0,
    B = 1,
    C = 2,
    D = 3,
    E = 4,
    Count = 5,
};

enum class TutorialGameModeStep : std::int32_t {
    None = 0,
    Tuto3C = 1,
    TutoShoot = 2,
    TutoGadget = 3,
    Fight = 4,
    Ultimate = 5,
};

enum class UltimateAbilityState : std::int32_t {
    Activated = 0,
    Charging = 1,
    FullyCharged = 2,
};

enum class UnlockedFeature : std::int32_t {
    Gameplay_CharacterMovement = 0,
    Gameplay_CameraMovement = 1,
    Gameplay_Jump = 2,
    Gameplay_DoubleJump = 3,
    Gameplay_Melee = 4,
    Gameplay_Fire = 5,
    Gameplay_FirstWeapon = 6,
    Gameplay_Dash = 7,
    Gameplay_Helipack = 8,
    Gameplay_SecondWeapon_Deprecated = 9,
    Gameplay_ThirdWeapon_Deprecated = 10,
    Gameplay_UltimateWeapon = 11,
    Gameplay_GadgetWeapon = 12,
    HUD_BoltCounter = 13,
    HUD_Health = 14,
    HUD_Shield = 15,
    HUD_Ranking = 16,
    HUD_Timer = 17,
    HUD_BoltLevel = 18,
};

enum class VoidRepulserState : std::int32_t {
    Normal = 0,
    Broken = 1,
    Expired = 2,
};

enum class WaveCompletionReason : std::int32_t {
    Undefined = 0,
    Cleared = 1,
    TimedOut = 2,
};

enum class WeaponFilterType : std::int32_t {
    Agility = 0,
    Versatility = 1,
    Artillery = 2,
    Glove = 3,
    Utility = 4,
};

enum class WeaponLoadoutSlot : std::int32_t {
    FIRST = 0,
    ULTIMATE = 1,
    MELEE = 2,
    GADGET = 3,
};

enum class WeaponLoadoutSlotType : std::int32_t {
    NORMAL = 0,
    ULTIMATE = 1,
    MELEE = 2,
};

enum class WeaponMoveSpeedModType : std::int32_t {
    OnUse = 0,
    OnButtonPress = 1,
    WhileFiring = 2,
    None = 3,
    WhileAmmoIsAlive = 4,
};

enum class WeaponStartUseType : std::int32_t {
    WhileButtonPressed = 0,
    OnButtonRelease = 1,
    OnButtonPressed = 2,
    Always = 3,
};

enum class WeaponStopUseType : std::int32_t {
    Manual = 0,
    FinishedFiring = 1,
};

enum class WeaponTag : std::int32_t {
    NONE = 0,
    SHOOT_BLANKS_FOR_MISSED_SHOTS = 1,
    CANNOT_MISS_SHOTS = 2,
    STRONG_HOMING = 4,
    SHOOTS_MOBS = 8,
    CANT_HIT_AT_CLOSE_RANGE = 16,
};

enum class WeaponType : std::int32_t {
    Projectile = 0,
    Effect = 1,
    EffectOneShot = 2,
};

} // namespace Quantum
