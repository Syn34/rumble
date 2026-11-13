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
    // TODO(0x7bdee20, Quantum.PhysicsAbstraction.GetAnyComponentSet), TODO(0x7bded30,
    // Quantum.PhysicsAbstraction.AddPush), TODO(0x7bdeea8,
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
};

class PlayerActorLib {
  public:
    // 0x7cb8060 - The player's game mode has finished (false for non-players).
    static bool IsGameModeFinished(Frame* frame, EntityRef entity);
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
