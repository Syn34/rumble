// actor_manager_systems.cpp - the ActorManager singleton's spawn point and respawn logic,
// ActorManagerSystem, AISystemsLib, and the small lookups respawning relies on
// (GameModeSpec.CanRespawn, Team.TryGetTeammate, ShrinkingZone position, player lookups).
#include "quantum_code/systems/actor_manager_systems.h"

#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/log.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/game_mode_systems.h"
#include "quantum_code/systems/libs.h"
#include "quantum_code/systems/list_extensions.h"
#include "quantum_code/systems/pending_systems.h"
#include "quantum_core/physics_components.h"

#include <limits>
#include <stdexcept>
#include <vector>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;
using Photon::Deterministic::Log;

namespace {

// Helper (unnamed in the method list; code at 0x7c49304): a random direction on the unit
// circle, as (cos, sin) of a random angle in [0, 2*pi).
FPVector2 RandomUnitCircleDirection(Frame* frame) {
    const FP angle = frame->get_RNG()->Next() * FP::PiTimes2();
    FP sin;
    FP cos;
    FPMath::SinCos(angle, sin, cos);
    return FPVector2(cos, sin);
}

} // namespace

// --- small lookups ------------------------------------------------------------------------------

bool GameModeSpec::CanRespawn(Frame* frame, EntityRef killedEntity, std::int32_t startFrame) const {
    if (PlayerActorLib::IsGameModeFinished(frame, killedEntity) || !playersCanRespawn) {
        return false;
    }
    if (!PlayersCanRespawnForASetPeriodOfTime) {
        return true;
    }
    return frame->get_DeltaTime() * (frame->Number - startFrame) <= RespawnSetTime;
}

GameModeHandlerLevelConfig* GameModeHandler::GetLevelConfig(Frame* frame) const {
    return frame->FindAsset<GameModeHandlerLevelConfig>(levelConfig.Id);
}

bool Team::TryGetTeammate(Frame* f, EntityRef playerEntity, EntityRef& teammate) {
    std::int32_t teamId = 0;
    if (TryGetTeamID(f, playerEntity, teamId)) {
        for (const auto& pair : f->Unsafe.GetComponentBlockIterator<PlayerActor>().Range()) {
            std::int32_t otherTeamId = 0;
            if (pair.Entity != playerEntity && TryGetTeamID(f, pair.Entity, otherTeamId) && otherTeamId == teamId) {
                teammate = pair.Entity;
                return true;
            }
        }
    }
    teammate = EntityRef{};
    return false;
}

bool ShrinkingZone::TryGet(Frame* f, ShrinkingZone& zone) {
    return f->TryGetSingleton<ShrinkingZone>(zone);
}

std::pair<FPVector3, FP> ShrinkingZone::GetPositionAndRadiusAt(Frame* f, FP time) const {
    FPVector3 centre = startPosition;
    FP zoneRadius = startRadius;
    Map* map = f->get_Map();
    if (map == nullptr) {
        throw std::runtime_error("ShrinkingZone: no map");
    }
    MapCustomData* customData = nullptr;
    if (!f->TryFindAssetInternal(map->UserAsset.Id, true, customData)) {
        return {centre, zoneRadius};
    }
    const MapCustomData::ClosingZoneData& zone = customData->ClosingZones.at(closingZoneIndex);
    for (const MapCustomData::ClosingPointData& point : zone.closingPoints) {
        if (time < point.startTime) {
            break;
        }
        if (point.endTime > time) {
            // Inside this closing window.
            const FP progress = (time - point.startTime) / (point.endTime - point.startTime);
            centre = FPVector3::Lerp(centre, point.targetPosition, progress);
            zoneRadius = FPMath::Lerp(zoneRadius, point.targetRadius, progress);
            return {centre, zoneRadius};
        }
        centre = point.targetPosition;
        zoneRadius = point.targetRadius;
    }
    return {centre, zoneRadius};
}

bool PlayerActorLib::IsBot(Frame* frame, EntityRef entity) {
    PlayerActor* actor = nullptr;
    return frame->TryGetPointer<PlayerActor>(entity, actor) && actor->isBot;
}

void PlayerActorLib::InitializeHeroEnhancementsFromTalentLevel(Frame* frame, EntityRef entity, std::int32_t talentLevel) {
    const PlayerActor actor = frame->Get<PlayerActor>(entity);
    HeroSpec* heroSpec = nullptr;
    if (!BalancingLib::TryGetHeroSpec(frame, static_cast<GENERATED_CHARACTER_ID>(actor.characterIndex), heroSpec)) {
        return;
    }
    if (heroSpec == nullptr) {
        throw std::runtime_error("PlayerActorLib: the player's hero has no spec");
    }
    for (std::int32_t i = 0; i < talentLevel && i < static_cast<std::int32_t>(heroSpec->TalentsInUnlockOrder.size()); ++i) {
        const AssetRef talent = heroSpec->TalentsInUnlockOrder[static_cast<std::size_t>(i)];
        HeroEnhancementSpec* enhancement = nullptr;
        if (talent.Id.IsValid() && frame->TryFindAssetInternal(talent.Id, true, enhancement)) {
            enhancement->OnInit(frame, entity);
        }
    }
}

void PlayerActorLib::PrepareForRespawn(Frame* frame, EntityRef entity) {
    ActorManager* manager = nullptr;
    if (frame->Has<PlayerActor>(entity) && frame->Unsafe.TryGetPointerSingleton<ActorManager>(manager)) {
        manager->AddRespawnTimer(frame, entity);
    }
}

bool PlayerActorLib::TryGetRespawnTimer(Frame* frame, EntityRef entity, FP& timer) {
    timer = FP{};
    ActorManager* manager = nullptr;
    if (frame->Has<PlayerActor>(entity) && frame->Unsafe.TryGetPointerSingleton<ActorManager>(manager)) {
        return manager->TryGetRespawnTimer(frame, entity, timer);
    }
    return false;
}

void AISystemsLib::SetEnabled(Frame* f, bool enabled) {
    f->get_Global()->AI_BotsInactive = !enabled;
    if (enabled) {
        f->SystemEnable<AIBotSDKSystem>();
        f->SystemEnable<AINavigationSystem>();
        f->SystemEnable<AIMoveForwardSystem>();
        return;
    }
    // Path finding of every AINavigation should stop here (see the TODO in the header).
    f->SystemDisable<AIBotSDKSystem>();
    f->SystemDisable<AINavigationSystem>();
    f->SystemDisable<AIMoveForwardSystem>();
}

// --- ActorManager: spawn points ------------------------------------------------------------------

template <class TMapCustomData>
typename TMapCustomData::SpawnPointData ActorManager::GetStaticSpawnPointData(Frame* frame, std::int32_t index) {
    Map* map = frame->get_Map();
    if (map == nullptr) {
        throw std::runtime_error("ActorManager: no map");
    }
    const TMapCustomData* customData = frame->FindAsset<TMapCustomData>(map->UserAsset.Id);
    if (customData == nullptr) {
        throw std::runtime_error("ActorManager: the map has no custom data");
    }
    // A negative index fails the bounds check too, as in the original.
    return customData->SpawnPoints.at(static_cast<std::size_t>(static_cast<std::uint32_t>(index)));
}
template MapCustomData::SpawnPointData ActorManager::GetStaticSpawnPointData<MapCustomData>(Frame*, std::int32_t);

void ActorManager::GetStaticSpawnPointAt(Frame* frame, std::int32_t index, FPVector3& position, FPQuaternion& rotation,
                                         std::uint16_t& introTrackId) {
    const MapCustomData::SpawnPointData data = GetStaticSpawnPointData(frame, index);
    position = data.position;
    rotation = FPQuaternion::Euler(FP{}, data.rotY, FP{});
    introTrackId = data.introTrackId;
}

void ActorManager::GetRespawnableSpawnPointAt(Frame* frame, std::int32_t index, FPVector3& position,
                                              FPQuaternion& rotation) const {
    std::uint16_t introTrackId = 0;
    const std::int16_t spawnPoint = frame->ResolveList(get_respawnablePoints()).get_Item(index);
    GetStaticSpawnPointAt(frame, spawnPoint, position, rotation, introTrackId);
}

void ActorManager::InitializeFromMapData(Frame* f, SpawnPointOptions excludeSpawnPoints) {
    Map* map = f->get_Map();
    if (map == nullptr) {
        throw std::runtime_error("ActorManager: no map");
    }
    MapCustomData* customData = f->FindAsset<MapCustomData>(map->UserAsset.Id);
    if (customData == nullptr) {
        Log::Error("ActorManager: the map has no custom data, no spawn points");
        return;
    }
    std::uint32_t maxInitialPoints = 100;
    GameModeHandler handler{};
    if (f->TryGetSingleton<GameModeHandler>(handler)) {
        const GameModeHandlerLevelConfig* levelConfig = handler.GetLevelConfig(f);
        if (levelConfig == nullptr) {
            throw std::runtime_error("ActorManager: the game mode handler has no level config");
        }
        maxInitialPoints = levelConfig->maxPlayers;
    }
    const auto& spawnPoints = customData->SpawnPoints;
    const std::int32_t pointCount = static_cast<std::int32_t>(spawnPoints.size());

    Collections::QList<std::int16_t> initial = f->ResolveList(get_initialSpawnPoints());
    initial.SetCapacity(static_cast<std::int32_t>(maxInitialPoints));
    initial.Clear();
    Collections::QList<std::int16_t> respawnable = f->ResolveList(get_respawnablePoints());
    respawnable.SetCapacity(pointCount);
    respawnable.Clear();
    Collections::QList<std::int16_t> podium = f->ResolveList(get_podiumPoints());

    const auto exclude = static_cast<std::int32_t>(excludeSpawnPoints);
    for (std::int16_t i = 0; i < pointCount; ++i) {
        const auto options = static_cast<std::int32_t>(spawnPoints.at(static_cast<std::size_t>(i)).options);
        if (exclude != 0 && (options & exclude) != 0) {
            continue;
        }
        if ((options & static_cast<std::int32_t>(SpawnPointOptions::Initial)) != 0 &&
            static_cast<std::int64_t>(initial.get_Count()) < static_cast<std::int64_t>(maxInitialPoints)) {
            initial.Add(i);
        }
        if ((options & static_cast<std::int32_t>(SpawnPointOptions::Respawn)) != 0) {
            respawnable.Add(i);
        }
        if ((options & static_cast<std::int32_t>(SpawnPointOptions::Podium)) != 0) {
            podium.Add(i);
        }
    }
    nbInitialSpawnPoints = static_cast<std::int16_t>(initial.get_Count());
    IListExtensions::Shuffle(initial, f);
    HandleForcedLocalPlayerSpawnPoint(customData, initial);

    const std::int32_t respawnableCount = respawnable.get_Count();
    if (respawnableCount != 0) {
        Collections::QList<SpawnPointDynamicData> dynamicData = f->ResolveList(get_spawnPointDynamicData());
        dynamicData.SetCapacity(respawnableCount);
        dynamicData.Clear();
        for (std::int32_t i = 0; i < respawnableCount; ++i) {
            dynamicData.Add(SpawnPointDynamicData{});
        }
    }
    f->ResolveList(get_activeCheckpoints()).Clear();
}

void ActorManager::HandleForcedLocalPlayerSpawnPoint(const MapCustomData* mapCustomData,
                                                     Collections::QList<std::int16_t>& availableList) {
    constexpr auto forced = static_cast<std::int32_t>(SpawnPointOptions::Initial) |
                            static_cast<std::int32_t>(SpawnPointOptions::LocalPlayerFirstSpawn);
    const auto& spawnPoints = mapCustomData->SpawnPoints;
    for (std::int16_t i = 0; i < static_cast<std::int32_t>(spawnPoints.size()); ++i) {
        if ((static_cast<std::int32_t>(spawnPoints[static_cast<std::size_t>(i)].options) & forced) != forced) {
            continue;
        }
        const std::int32_t position = availableList.IndexOf(i);
        if (position < availableList.get_Count()) {
            // A forced point missing from the list (IndexOf -1) fails like the original.
            IListExtensions::Swap(availableList, 0, position);
        }
        return;
    }
}

std::int32_t ActorManager::GetCurrentPlayerCount(Frame* frame) const {
    std::int32_t count = 0;
    for (const auto& pair : frame->Unsafe.GetComponentBlockIterator<PlayerActor>().Range()) {
        if (!pair.Component->gameModeFinished) {
            ++count;
        }
    }
    return count;
}

bool ActorManager::ProvidePlayerInitialSpawnPoint(Frame* f, PlayerRef playerRef, std::int32_t teamId,
                                                  std::int32_t playerIndex, FPVector3& spawnPosition,
                                                  FPQuaternion& spawnRotation, std::uint16_t& introTrackId) {
    spawnPosition = FPVector3::Zero();
    spawnRotation = FPQuaternion::Identity();
    introTrackId = 0;
    Collections::QList<std::int16_t> initial = f->ResolveList(get_initialSpawnPoints());
    if (initial.get_Count() <= 0) {
        Log::Error("ActorManager: no initial spawn point left");
        return false;
    }
    std::int32_t pick = 0;
    GameModeSpec* spec = nullptr;
    if (GameModeHandlerLib::TryGetGameModeSpec(f, spec)) {
        pick = spec->ProvideInitialSpawnPointIndex(f, playerRef, teamId, playerIndex, initial);
        if (pick < 0) {
            Log::Error("ActorManager: the game mode gave no initial spawn point for player {}", playerIndex);
            return false;
        }
    }
    GetStaticSpawnPointAt(f, initial.get_Item(pick), spawnPosition, spawnRotation, introTrackId);
    initial.RemoveAt(pick);
    return true;
}

void ActorManager::SetActiveCheckpoint(Frame* frame, EntityRef actor, EntityRef checkpoint) {
    Collections::QList<ActiveCheckpoint> checkpoints = frame->ResolveList(get_activeCheckpoints());
    for (std::int32_t i = 0; i < checkpoints.get_Count(); ++i) {
        const ActiveCheckpoint entry = checkpoints.get_Item(i);
        if (entry.Actor == actor) {
            if (entry.Checkpoint != checkpoint) {
                checkpoints.set_Item(i, ActiveCheckpoint{actor, checkpoint});
            }
            return;
        }
    }
    checkpoints.Add(ActiveCheckpoint{actor, checkpoint});
}

// --- ActorManager: respawning --------------------------------------------------------------------

void ActorManager::ClearAllRespawnTimers(Frame* frame) {
    ActorManager* manager = nullptr;
    if (frame->Unsafe.TryGetPointerSingleton<ActorManager>(manager)) {
        frame->ResolveList(manager->get_respawnTimers()).Clear();
    }
}

void ActorManager::AddRespawnTimer(Frame* frame, EntityRef entity) {
    GameModeSpec* spec = nullptr;
    if (GameModeHandlerLib::TryGetGameModeSpec(frame, spec) && !spec->CanRespawn(frame, entity, 0)) {
        return;
    }
    Collections::QList<RespawnTimers> timers = frame->ResolveList(get_respawnTimers());
    for (std::int32_t i = 0; i < timers.get_Count(); ++i) {
        if (timers.get_Item(i).ToRespawn == entity) {
            return;
        }
    }
    RespawnTimers timer{};
    timer.UnspawnedFrame = frame->Number;
    timer.IsRespawnPointCalculated = false;
    timer.ToRespawn = entity;
    timers.Add(timer);
    ActorManagerConfig* config = nullptr;
    if (frame->TryFindAssetInternal(Config.Id, true, config)) {
        frame->Events.RespawnTimerCreated(entity, config->RespawnPointCalculationDelay);
    }
}

bool ActorManager::TryGetRespawnTimer(Frame* frame, EntityRef entity, FP& timer) const {
    timer = FP{};
    ActorManagerConfig* config = nullptr;
    if (!frame->TryFindAssetInternal(Config.Id, true, config)) {
        return false;
    }
    Collections::QList<RespawnTimers> timers = frame->ResolveList(get_respawnTimers());
    const FP total = config->GetTotalRespawnDuration(PlayerActorLib::IsBot(frame, entity));
    for (std::int32_t i = 0; i < timers.get_Count(); ++i) {
        const RespawnTimers entry = timers.get_Item(i);
        if (entry.ToRespawn == entity) {
            timer = total - frame->get_DeltaTime() * (frame->Number - entry.UnspawnedFrame);
            return true;
        }
    }
    return false;
}

void ActorManager::RemoveRespawnTimer(Frame* frame, EntityRef entity) {
    Collections::QList<RespawnTimers> timers = frame->ResolveList(get_respawnTimers());
    for (std::int32_t i = 0; i < timers.get_Count(); ++i) {
        if (timers.get_Item(i).ToRespawn == entity) {
            timers.RemoveAt(i);
            return;
        }
    }
}

bool ActorManager::TryGetTargetRespawnPositionAndRotation(Frame* frame, EntityRef target,
                                                          Collections::QList<ActiveCheckpoint> activeCheckpointsList,
                                                          Collections::QList<std::int16_t> respawnableList,
                                                          FP respawnDistance, FPVector3& position,
                                                          FPQuaternion& rotation) const {
    (void)respawnDistance;
    position = FPVector3::Zero();
    rotation = FPQuaternion::Identity();
    if (KillableLib::IsDead(frame, target)) {
        return false;
    }
    Transform3D* transform = nullptr;
    KccExtension* kcc = nullptr;
    if (!frame->TryGetPointer<Transform3D>(target, transform) || !frame->TryGetPointer<KccExtension>(target, kcc)) {
        return false;
    }
    if (kcc->lastGroundedPos != FPVector3::Zero()) {
        position = kcc->lastGroundedPos;
    } else if (respawnableList.get_Count() == 0 && activeCheckpointsList.get_Count() == 0) {
        position = transform->Position;
    } else {
        return false;
    }
    rotation = transform->Rotation;
    return true;
}

void ActorManager::GetRespawnPositionAndRotation(Frame* frame, EntityRef toRespawn, FPVector3& position,
                                                 FPQuaternion& rotation) {
    Collections::QList<ActiveCheckpoint> checkpoints = frame->ResolveList(get_activeCheckpoints());
    Collections::QList<std::int16_t> respawnable = frame->ResolveList(get_respawnablePoints());
    const std::int32_t respawnableCount = respawnable.get_Count();
    GameModeSpec* spec = nullptr;
    GameModeHandlerLib::TryGetGameModeSpec(frame, spec);
    if (spec == nullptr) {
        throw std::runtime_error("ActorManager: respawning without a game mode spec");
    }

    // Near the team leader, then near any teammate, when the mode asks for it.
    if (spec->RespawnNearTeamLeader) {
        EntityRef leader;
        if (Team::TryGetPlayerTeamLeader(frame, toRespawn, leader) &&
            TryGetTargetRespawnPositionAndRotation(frame, leader, checkpoints, respawnable, spec->RespawnDistance,
                                                   position, rotation)) {
            return;
        }
    }
    if (spec->RespawnNearTeammate) {
        EntityRef teammate;
        if (Team::TryGetTeammate(frame, toRespawn, teammate) &&
            TryGetTargetRespawnPositionAndRotation(frame, teammate, checkpoints, respawnable, spec->RespawnDistance,
                                                   position, rotation)) {
            return;
        }
    }

    // Around the actor's active checkpoint.
    EntityRef checkpoint = EntityRef::None();
    for (std::int32_t i = 0; i < checkpoints.get_Count(); ++i) {
        const ActiveCheckpoint entry = checkpoints.get_Item(i);
        if (entry.Actor == toRespawn) {
            checkpoint = entry.Checkpoint;
            break;
        }
    }
    Transform3D checkpointTransform{};
    if (checkpoint != EntityRef::None() && frame->TryGet<Transform3D>(checkpoint, checkpointTransform)) {
        const FPVector2 offset = RandomUnitCircleDirection(frame);
        position = checkpointTransform.Position + FPVector3(offset.X, FP{}, offset.Y);
        rotation = checkpointTransform.Rotation;
        return;
    }

    if (respawnableCount < 1) {
        position = FPVector3::Zero();
        rotation = FPQuaternion::Identity();
        Log::Error("ActorManager: the map has no respawn point");
        return;
    }

    // Sort the respawn points the mode accepts and that are off cooldown: timed points
    // outside the shrinking zone apart; otherwise every candidate, those with nobody in the
    // inner range, and those with nobody in either range.
    std::vector<std::int32_t> unwatched;
    std::vector<std::int32_t> innerFree;
    std::vector<std::int32_t> candidates;
    std::vector<std::int32_t> timed;
    std::vector<MapCustomData::SpawnPointData> pointData;
    Collections::QList<SpawnPointDynamicData> dynamicData = frame->ResolveList(get_spawnPointDynamicData());
    FP cooldown;
    ActorManagerConfig* config = nullptr;
    if (frame->TryFindAssetInternal(Config.Id, true, config)) {
        cooldown = config->RespawnPointCooldownDuration;
    }
    for (std::int32_t i = 0; i < dynamicData.get_Count(); ++i) {
        const SpawnPointDynamicData dynamic = dynamicData.get_Item(i);
        const MapCustomData::SpawnPointData data = GetStaticSpawnPointData(frame, respawnable.get_Item(i));
        pointData.push_back(data);
        if (!spec->RespawnPointIsValid(frame, toRespawn, data, dynamic)) {
            continue;
        }
        if (frame->get_DeltaTime() * (frame->Number - dynamic.lastRespawnFrame) < cooldown) {
            continue;
        }
        if (data.availabilityDuration.RawValue() >= 1 && dynamic.isOutsideShrinkZone) {
            timed.push_back(i);
            continue;
        }
        candidates.push_back(i);
        if (dynamic.numActorsInInnerRange != 0) {
            continue;
        }
        innerFree.push_back(i);
        if (dynamic.numActorsInOuterRange != 0) {
            continue;
        }
        unwatched.push_back(i);
    }

    std::vector<std::int32_t> allPoints;
    const std::vector<std::int32_t>* pool = nullptr;
    if (!unwatched.empty()) {
        pool = &unwatched;
    } else if (!innerFree.empty()) {
        pool = &innerFree;
    } else if (!candidates.empty()) {
        pool = &candidates;
    } else if (!timed.empty()) {
        Log::Error("ActorManager: only timed respawn points are left for {}", toRespawn.ToString());
        pool = &timed;
    } else {
        for (std::size_t i = 0; i < pointData.size(); ++i) {
            allPoints.push_back(static_cast<std::int32_t>(i));
        }
        Log::Error("ActorManager: no acceptable respawn point for {}, using any", toRespawn.ToString());
        pool = &allPoints;
    }

    // The shortest non-zero availability wins; else the point farthest from the shrinking
    // zone's centre; else a random one.
    std::int32_t chosen = -1;
    std::int64_t shortestAvailability = std::numeric_limits<std::int64_t>::max();
    for (const std::int32_t index : *pool) {
        const std::int64_t availability = pointData[static_cast<std::size_t>(index)].availabilityDuration.RawValue();
        if (availability != 0 && availability < shortestAvailability) {
            chosen = index;
            shortestAvailability = availability;
        }
    }
    if (chosen == -1) {
        ShrinkingZone zone{};
        if (ShrinkingZone::TryGet(frame, zone)) {
            const FPVector3 centre = zone.GetPositionAndRadiusAt(frame, GameTimerLib::GetElapsedTime(frame)).first;
            chosen = pool->at(0);
            FP farthest = FPVector3::DistanceSquared(pointData[static_cast<std::size_t>(chosen)].position, centre);
            for (std::size_t k = 1; k < pool->size(); ++k) {
                const std::int32_t index = (*pool)[k];
                const FP distance = FPVector3::DistanceSquared(pointData[static_cast<std::size_t>(index)].position, centre);
                if (distance > farthest) {
                    chosen = index;
                    farthest = distance;
                }
            }
        } else {
            const std::int32_t pick = frame->get_RNG()->Next(0, static_cast<std::int32_t>(pool->size()));
            chosen = pool->at(static_cast<std::size_t>(pick));
        }
    }

    std::uint16_t introTrackId = 0;
    GetStaticSpawnPointAt(frame, respawnable.get_Item(chosen), position, rotation, introTrackId);
    SpawnPointDynamicData used = dynamicData.get_Item(chosen);
    used.lastRespawnFrame = frame->Number;
    dynamicData.set_Item(chosen, used);
}

void ActorManager::UpdateRespawnTimers(Frame* frame) {
    Collections::QList<RespawnTimers> timers = frame->ResolveList(get_respawnTimers());
    if (timers.get_Count() == 0) {
        return;
    }
    ActorManagerConfig* config = nullptr;
    if (!frame->TryFindAssetInternal(Config.Id, true, config)) {
        return;
    }
    for (std::int32_t i = timers.get_Count() - 1; i >= 0; --i) {
        RespawnTimers timer = timers.get_Item(i);
        const FP waited = frame->get_DeltaTime() * (frame->Number - timer.UnspawnedFrame);
        if (!timer.IsRespawnPointCalculated && waited >= config->RespawnPointCalculationDelay) {
            GetRespawnPositionAndRotation(frame, timer.ToRespawn, timer.Position, timer.Rotation);
            timer.IsRespawnPointCalculated = true;
            timers.set_Item(i, timer);
            Transform3D* transform = nullptr;
            if (frame->TryGetPointer<Transform3D>(timer.ToRespawn, transform)) {
                transform->Position = timer.Position;
                transform->Rotation = timer.Rotation;
            }
            frame->Events.RespawnPointSet(timer.ToRespawn, timer.Position, timer.Rotation);
            continue;
        }
        const EntityRef entity = timer.ToRespawn;
        const FP total = config->GetTotalRespawnDuration(PlayerActorLib::IsBot(frame, entity));
        if (frame->get_DeltaTime() * (frame->Number - timer.UnspawnedFrame) >= total) {
            KillableLib::Revive(frame, entity, timer.Position, timer.Rotation);
            timers.RemoveAtUnordered(i);
            frame->Events.RespawnTimerDestroyed(entity);
        }
    }
}

// --- ActorManagerSystem --------------------------------------------------------------------------

void ActorManagerSystem::OnInit(Frame* f) {
    OnMapChanged(f, AssetRefMap{});
    EntityRef handler;
    if (!f->TryGetSingletonEntityRef<GameModeHandler>(handler)) {
        AISystemsLib::SetEnabled(f, true);
    }
}

void ActorManagerSystem::Update(Frame* frame) {
    frame->Unsafe.GetPointerSingleton<ActorManager>()->Update(frame);
}

void ActorManagerSystem::OnMapChanged(Frame* f, AssetRefMap previousMap) {
    (void)previousMap;
    f->Unsafe.GetPointerSingleton<ActorManager>()->InitializeFromMapData(f, SpawnPointOptions::None);
}

void ActorManagerSystem::OnAdded(Frame* f, EntityRef entity, PlayerActor* pActor) {
    (void)pActor;
    f->Add<ActorInput>(entity);
}

void ActorManagerSystem::OnAdded(Frame* f, EntityRef entity, ActorManager* component) {
    (void)component;
    f->SetCullable(entity, false);
}

void ActorManagerSystem::GameMode_OnPlayerFinished(Frame* f, EntityRef entity) {
    ActorManager* manager = nullptr;
    if (f->Unsafe.TryGetPointerSingleton<ActorManager>(manager)) {
        manager->RemoveRespawnTimer(f, entity);
    }
}

void ActorManagerSystem::PlayerActor_OnAfterInitialized(Frame* f, EntityRef actorEntity, PlayerActor* actor) {
    PlayerActorLib::InitializeHeroEnhancementsFromTalentLevel(f, actorEntity, actor->heroTalentLevel);
}

} // namespace Quantum
