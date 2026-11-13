// combat_libs.h - helpers around killing, reviving, ownership and physics shut-down
// (KillableLib, PhysicsAbstraction, Ownership, FrameUtility and the small lookups they
// rely on).
#pragma once

#include "quantum_code/frame.h"
#include "quantum_core/physics_components.h"

namespace Quantum {

// Quantum.PhysicsAbstraction: the moving parts of an entity, whichever it has - a
// physics body, or a kinematic character (KccExtension, with its controller and grind
// state), or a level button.
struct PhysicsAbstraction {
    EntityRef physicsEntity;
    PhysicsBody3D* physicsBody = nullptr;
    KccExtension* kccExtension = nullptr;
    CharacterController3D* characterController = nullptr;
    GrinderActor* grinderActor = nullptr;
    LevelButton* levelButton = nullptr;

    // 0x7bdde94
    static PhysicsAbstraction Create(Frame* frame, EntityRef entity);
    // 0x7bdf0d8 - Keeps the character's controller off for `delay` (whole frames).
    void DisablePhysics(Frame* frame, FP delay);
    // 0x7bde074 - Stops the character (velocities cleared, extension disabled) and drops
    // a pending impulse.
    void DisablePhysics(Frame* frame);
    // 0x7bdf130 - Re-enables the character, cleared of leftover motion.
    void EnablePhysics(Frame* frame);
    // 0x7bded30 - A physics body takes the force as an impulse at the hit point; a
    // character queues it as a push (the hit point and pusher are not used then).
    void AddPush(Frame* f, FPVector3 force, FPVector3 hitpoint, Transform3D* transform, EntityRef pusherActor);
    // TODO(0x7bdee20, Quantum.PhysicsAbstraction.GetAnyComponentSet), TODO(0x7bdeea8,
    // Quantum.PhysicsAbstraction.GetVelocity), TODO(0x7bdefdc,
    // Quantum.PhysicsAbstraction.ApplyPushStop): not reimplemented yet.
};

class FrameUtility {
  public:
    // 0x7be2ecc - A prototype instance that is destroyed when the map changes.
    static EntityRef CreateDynamic(Frame* f, AssetRefEntityPrototype prototype);
    // 0x7be2f54 - Placed, owned (when an owner is given) and in a team (unless -1).
    static EntityRef CreateDynamic(Frame* f, AssetRefEntityPrototype prototype, FPVector3 spawnPos,
                                   FPQuaternion spawnRot, EntityRef owner, EntityRef ownerWeapon,
                                   EntityRef ownerSource, std::int32_t teamID, bool isOwnerUltimate);
};

class PushableLib {
  public:
    // 0x7bc43b0 - The latest push, if the entity is pushable and has been pushed.
    static bool TryGetPushedBy(Frame* frame, EntityRef entity, PushedByData& pushedBy);
};

class DamageLib {
  public:
    // 0x7bba854 - The latest hit, if the entity is damageable (see the overload).
    static bool TryGetHitBy(Frame* frame, EntityRef entity, HitData& hitBy);
    // 0x7bba910 - The latest hit while it is recent enough to count (the damageable's
    // spec says how long a hit is remembered).
    static bool TryGetHitBy(Frame* frame, EntityRef entity, Damageable* damageable, HitData& hitBy);

    // 0x7bb9b58 - The filter has the damage type's bit.
    static bool Contains(DAMAGE_FILTER filters, DamageType targetType) {
        return ((static_cast<std::uint32_t>(filters) >> static_cast<std::uint32_t>(targetType)) & 1U) != 0;
    }
    // 0x7bb9b64 - The two filters share a flag (ALL shares every one).
    static bool Contains(INVULNERABILITY_FILTER thisFilter, INVULNERABILITY_FILTER otherFilter) {
        return (static_cast<std::int32_t>(otherFilter) & static_cast<std::int32_t>(thisFilter)) > 0;
    }
    // 0x7bba5f0 - Damageable.SetInvulnerable on the entity, if damageable.
    static void SetInvulnerable(Frame* frame, EntityRef entity, FP duration, INVULNERABILITY_FILTER invulnerabilityFilter);
    // 0x7bb9b70 - Fully (filter ALL) invulnerable right now.
    static bool IsInvulnerable(Frame* frame, EntityRef entity);
    // 0x7bba6bc
    static bool IsInvulnerableToDamage(Frame* frame, EntityRef entity);
    // 0x7bba404
    static bool IsInvulnerableToPush(Frame* frame, EntityRef entity);
    // 0x7bba76c - Zero for non-damageables and when not invulnerable.
    static FP GetInvulnerabilityRemainingDuration(Frame* frame, EntityRef entity);
    // 0x7bbaa20 - A multiplier that lasts until cleared.
    static void SetDamageMultiplier(Frame* frame, EntityRef entity, FP value, DAMAGE_MULTIPLIER_ORIGIN origin) {
        SetDamageMultiplier(frame, entity, value, origin, FP::FromRaw(-65536));
    }
    // 0x7bbaa28 - Sets the damage source's multiplier from `origin`; a positive duration
    // makes it expire (an existing entry keeps the later end frame), otherwise it never
    // expires.
    static void SetDamageMultiplier(Frame* frame, EntityRef entity, FP value, DAMAGE_MULTIPLIER_ORIGIN origin,
                                    FP duration);
    // 0x7bb99b4 - Drops every multiplier from `origin`.
    static void ClearDamageMultiplier(Frame* frame, EntityRef entity, DAMAGE_MULTIPLIER_ORIGIN origin);
    // 0x7bbac4c - The product of the entity's multipliers from `origin` (1 if none).
    static bool TryGetDamageMultiplier(Frame* frame, EntityRef entity, DAMAGE_MULTIPLIER_ORIGIN origin,
                                       FP& damageMultiplier);
    // 0x7bbad3c
    static bool TryGetDamageMultiplier(Frame* frame, const DamageSource& damageSource, DAMAGE_MULTIPLIER_ORIGIN origin,
                                       FP& damageMultiplier);
    // 0x7bbaf48 - The product of the live multipliers; expired ones are removed.
    static FP GetTotalDamageMultiplier(Frame* frame, EntityRef entity);
    // TODO(0x7bb9c7c, Quantum.DamageLib.Hit): queues a hit built by DamageSpec.CreateHitData
    //   on a damageable target (filters, Damageable_OnBeforeHit, area-of-effect continuity);
    //   waits for DamageSpec.CreateHitData.
    // TODO(0x7bba074, Quantum.DamageLib.Push): pushes a pushable target with a pusher spec.
    // TODO(0x7bb5d40, Quantum.DamageLib.HitAndPush): Hit then Push.
    // TODO(0x7bbb0cc, Quantum.DamageLib.DamageableCanBuildUpDamageOfType): elemental build-up
    //   check (fire damage effects).
};

class EntityInfoLib {
  public:
    // 0x7ccba64
    static bool FilterContains(ENTITY_INFO_FILTER filters, EntityInfoType targetType) {
        return ((static_cast<std::uint32_t>(filters) >> static_cast<std::uint32_t>(targetType)) & 1U) != 0;
    }
    // 0x7cd35cc
    static bool EntityHasTypeMatching(Frame* frame, EntityRef entity, ENTITY_INFO_FILTER filter) {
        Code::EntityInfo entityInfo{};
        return EntityHasTypeMatching(frame, entity, filter, entityInfo);
    }
    // 0x7cda7f4 - The entity has an EntityInfo whose type is in the filter.
    static bool EntityHasTypeMatching(Frame* frame, EntityRef entity, ENTITY_INFO_FILTER filter, Code::EntityInfo& entityInfo);
    // 0x7cda87c - Entities without EntityInfo always match.
    static bool EntityHasNoTypeOrMatches(Frame* frame, EntityRef entity, ENTITY_INFO_FILTER filter);
    // 0x7cd7f00 - The first entity (in component order) with that id and a matching type.
    static bool TryFindEntity(Frame* frame, ENTITY_INFO_FILTER filter, std::int32_t entityId, EntityRef& entity);
    // 0x7cdaa0c - -1 without EntityInfo.
    static bool TryGetId(Frame* frame, EntityRef entity, std::int32_t& id);
    // 0x7cdaab8 - None without EntityInfo.
    static bool TryGetType(Frame* frame, EntityRef entity, EntityInfoType& type);
    // 0x7cdab64
    static bool HasId(Frame* frame, EntityRef spawnerEntity, std::int32_t id);
    // TODO(0x7cd8fb0, Quantum.EntityInfoLib.EntityPrototypeHasTypeMatching): type test on an
    //   entity prototype's EntityInfo prototype; needs FrameUtility.GetComponentPrototype.
    // TODO(0x7cda948, Quantum.EntityInfoLib.GetEntities): lazily enumerates the entities whose
    //   type matches a filter.
};

class WeaponLoadoutLib {
  public:
    // 0x7c0251c - The unique id of the loadout slot that spawned `weapon`.
    static bool TryGetWeaponIdInLoadout(Frame* frame, EntityRef entity, EntityRef weapon, GENERATED_WEAPON_ID& weaponId);
};

class PlayerActorLib {
  public:
    // 0x7cb8060 - The player's game mode has finished (false for non-players).
    static bool IsGameModeFinished(Frame* frame, EntityRef entity);
    // 0x7cb8360
    static bool IsBot(Frame* frame, EntityRef entity);
    // 0x7cb8628 - Runs OnInit of the hero's first `talentLevel` talents that are hero
    // enhancements.
    static void InitializeHeroEnhancementsFromTalentLevel(Frame* frame, EntityRef entity, std::int32_t talentLevel);
    // 0x7cb81c0 - Players get a respawn timer from the actor manager.
    static void PrepareForRespawn(Frame* frame, EntityRef entity);
    // 0x7cb8284 - The player's remaining respawn time.
    static bool TryGetRespawnTimer(Frame* frame, EntityRef entity, FP& timer);
};

class KillableLib {
  public:
    // 0x7bc3bb0
    static void KillNextFrame(Frame* f, EntityRef entity, KillType killType, EntityRef killerEntity) {
        KillNextFrame(f, entity, FP{}, killType, killerEntity);
    }
    // 0x7bc3bc0 - Marks the entity to be killed once `destructionDelay` has passed; an
    // entity already marked keeps its first mark.
    static void KillNextFrame(Frame* f, EntityRef entity, FP destructionDelay, KillType killType,
                              EntityRef killerEntity);
    // 0x7bc3c74 - Marks the entity for destruction and reports the kill (no killer).
    static void Destroy(Frame* frame, EntityRef entity, KillType killType);
    // 0x7bb4cdc - Kills a Killable: works out who killed it, records it, counts the death,
    // applies the prototype's on-kill behaviour (physics/collider off, delayed
    // destruction, an entity spawned in its place) and reports the kill. Entities without
    // a Killable, and dead ones, are just destroyed / ignored.
    static void Kill(Frame* frame, EntityRef killedEntity, KillType killType, EntityRef killerSource);
    // 0x7bc3fe8 - Brings a dead entity back at a position, unless its game mode has
    // finished.
    static void Revive(Frame* frame, EntityRef entity, FPVector3 position, FPQuaternion rotation);
    // 0x7bc3d24 - Without a known source, the latest push or hit names it; a source's
    // ownership then gives the owning player, weapon and whether it came from an
    // ultimate.
    static void ResolveKillSource(Frame* frame, EntityRef killedEntity, EntityRef& killerSource,
                                  EntityRef& killerWeapon, EntityRef& killerOwner, bool& isOwnerUltimate);
    // 0x7bc4304 - False without a Killable.
    static bool IsAlive(Frame* frame, EntityRef actorEntity);
    // 0x7bb8a44
    static bool IsDead(Frame* frame, EntityRef actorEntity) { return !IsAlive(frame, actorEntity); }
    // 0x7bbfad0 - False without a Killable.
    static bool IsKillableDead(Frame* frame, EntityRef entity);
    // 0x7bc446c
    static bool IsKillableAlive(Frame* frame, EntityRef entity);
    // 0x7bc4518 - 1 for entities with unlimited lives (or no Killable).
    static std::int32_t GetLivesLeft(Frame* frame, EntityRef actorEntity);
    // 0x7bc45c4 - Only entities with limited lives get more.
    static void AddExtraLives(Frame* frame, EntityRef actorEntity, std::int32_t extraLives);
    // 0x7bc3f48
    static void IncrementDeathCount(Frame* frame, EntityRef actorEntity);
    // 0x7bc4670 - The record of the last kill, if there was one.
    static bool TryGetKilledBy(Frame* frame, EntityRef killedEntity, KilledByData& killedBy);
};

} // namespace Quantum
