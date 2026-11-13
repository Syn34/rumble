// combat_libs.cpp - KillableLib, PhysicsAbstraction, FrameUtility, Ownership and the
// lookups they use.
#include "quantum_code/systems/combat_libs.h"

#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/killable_prototype.h"
#include "quantum_core/physics_components.h"

namespace Quantum {

// --- small component helpers ---------------------------------------------------------------

bool PlayerActor::get_IsHuman() const { return !isBot; }

Prototypes::Killable_Prototype* Killable::GetPrototype(Frame* frame) const {
    return frame->FindPrototype<Prototypes::Killable_Prototype>(Prototype);
}

std::int32_t Team::GetTeamID(Frame* f, EntityRef teamMember) {
    Team* team = nullptr;
    return f->TryGetPointer<Team>(teamMember, team) ? team->TeamID : -1;
}

void Ownership::SetOwnerShip(Frame* frame, EntityRef entity, EntityRef owner, EntityRef weapon,
                             EntityRef directSource, bool isOwnerUltimate) {
    Ownership* ownership = nullptr;
    if (!frame->AddOrGet<Ownership>(entity, ownership)) {
        return;
    }
    if (owner != EntityRef{}) {
        ownership->Owner = owner;
    }
    if (weapon != EntityRef{}) {
        ownership->Weapon = weapon;
    }
    if (directSource != EntityRef{}) {
        ownership->Source = directSource;
    }
    ownership->IsOwnerUltimate = isOwnerUltimate;
}

bool Ownership::TryGetOwnership(Frame* frame, EntityRef entity, Ownership*& ownership) {
    return frame->TryGetPointer<Ownership>(entity, ownership);
}

// --- PhysicsAbstraction ----------------------------------------------------------------------

PhysicsAbstraction PhysicsAbstraction::Create(Frame* frame, EntityRef entity) {
    PhysicsAbstraction physics;
    physics.physicsEntity = entity;
    if (frame->TryGetPointer<PhysicsBody3D>(entity, physics.physicsBody)) {
        return physics;
    }
    if (frame->TryGetPointer<KccExtension>(entity, physics.kccExtension)) {
        frame->TryGetPointer<CharacterController3D>(entity, physics.characterController);
        frame->TryGetPointer<GrinderActor>(entity, physics.grinderActor);
        return physics;
    }
    frame->TryGetPointer<LevelButton>(entity, physics.levelButton);
    return physics;
}

void PhysicsAbstraction::DisablePhysics(Frame* frame, FP delay) {
    if (kccExtension == nullptr) {
        return;
    }
    kccExtension->disableFrame = frame->Number + (delay / frame->get_DeltaTime()).AsInt();
}

void PhysicsAbstraction::DisablePhysics(Frame* frame) {
    if (characterController != nullptr) {
        characterController->Velocity = FPVector3::Zero();
    }
    if (kccExtension != nullptr) {
        kccExtension->disableFrame = 0;
        kccExtension->enabled = false;
        kccExtension->pushVelocity = FPVector3::Zero();
        kccExtension->velocity = FPVector3::Zero();
    }
    frame->Remove<PendingPhysicsImpulse>(physicsEntity);
}

void PhysicsAbstraction::EnablePhysics(Frame* frame) {
    if (characterController != nullptr) {
        characterController->Velocity = FPVector3::Zero();
        // Drops the controller's broad-phase query registration.
        characterController->_broadphaseQueryIndexPlusOne = 0;
    }
    if (kccExtension != nullptr) {
        kccExtension->disableFrame = 0;
        kccExtension->enabled = true;
        kccExtension->pushVelocity = FPVector3::Zero();
    }
    frame->Remove<PendingPhysicsImpulse>(physicsEntity);
}

// --- FrameUtility ------------------------------------------------------------------------------

EntityRef FrameUtility::CreateDynamic(Frame* f, AssetRefEntityPrototype prototype) {
    const EntityRef entity = f->Create(AssetRefEntityPrototype{prototype.Id});
    f->Add<AutoDestroyOnMapChanged>(entity);
    return entity;
}

EntityRef FrameUtility::CreateDynamic(Frame* f, AssetRefEntityPrototype prototype, FPVector3 spawnPos,
                                      FPQuaternion spawnRot, EntityRef owner, EntityRef ownerWeapon,
                                      EntityRef ownerSource, std::int32_t teamID, bool isOwnerUltimate) {
    const EntityRef entity = CreateDynamic(f, prototype);
    Transform3D* transform = nullptr;
    if (f->TryGetPointer<Transform3D>(entity, transform)) {
        transform->Position = spawnPos;
        transform->Rotation = spawnRot;
    }
    if (owner != EntityRef::None()) {
        Ownership::SetOwnerShip(f, entity, owner, ownerWeapon, ownerSource, isOwnerUltimate);
    }
    if (teamID != -1) {
        Team::JoinTeam(f, entity, teamID);
    }
    return entity;
}

// --- PushableLib / DamageLib / PlayerActorLib --------------------------------------------------

bool PushableLib::TryGetPushedBy(Frame* frame, EntityRef entity, PushedByData& pushedBy) {
    Pushable* pushable = nullptr;
    if (frame->TryGetPointer<Pushable>(entity, pushable) && pushable->pushedBy.frame != 0) {
        pushedBy = pushable->pushedBy;
        return true;
    }
    pushedBy = PushedByData{};
    return false;
}

bool DamageLib::TryGetHitBy(Frame* frame, EntityRef entity, HitData& hitBy) {
    Damageable* damageable = nullptr;
    if (frame->TryGetPointer<Damageable>(entity, damageable)) {
        return TryGetHitBy(frame, entity, damageable, hitBy);
    }
    hitBy = HitData{};
    return false;
}

bool DamageLib::TryGetHitBy(Frame* frame, EntityRef entity, Damageable* damageable, HitData& hitBy) {
    (void)entity;
    if (damageable->hitBy.frame != 0) {
        DamageableSpec* spec = nullptr;
        if (frame->TryFindAssetInternal(damageable->Spec.Id, true, spec)) {
            const FP sinceHit = frame->get_DeltaTime() * (frame->Number - damageable->hitBy.frame);
            if (sinceHit < spec->LastHitSaveDuration) {
                hitBy = damageable->hitBy;
                return true;
            }
        }
    }
    hitBy = HitData{};
    return false;
}

bool PlayerActorLib::IsGameModeFinished(Frame* frame, EntityRef entity) {
    PlayerActor* actor = nullptr;
    return frame->TryGetPointer<PlayerActor>(entity, actor) && actor->gameModeFinished;
}

// --- Action_KillSelf (asset) ------------------------------------------------------------------

void Action_KillSelf::Execute(Frame* frame, EntityRef entity) {
    KillableLib::Kill(frame, entity, KillType::Undefined, EntityRef{});
}

// --- KillableLib ---------------------------------------------------------------------------------

void KillableLib::KillNextFrame(Frame* f, EntityRef entity, FP destructionDelay, KillType killType,
                                EntityRef killerEntity) {
    ::Quantum::KillNextFrame* mark = nullptr;
    if (f->Add<::Quantum::KillNextFrame>(entity, ::Quantum::KillNextFrame{}, mark) != AddResult::ComponentAdded) {
        return;
    }
    mark->MarkedForKillFrame = f->Number;
    mark->killType = killType;
    mark->KillDelay = destructionDelay;
    mark->killer = killerEntity;
}

void KillableLib::Destroy(Frame* frame, EntityRef entity, KillType killType) {
    KillNextFrame(frame, entity, FP{}, KillType::Undefined, EntityRef{});
    frame->Signals.Killable_OnKill(entity, KilledByData{});
    frame->Events.KillEvent(entity, killType, KillZoneSurfaceType::Default);
}

void KillableLib::Kill(Frame* frame, EntityRef killedEntity, KillType killType, EntityRef killerSource) {
    Killable* killable = nullptr;
    if (!frame->TryGetPointer<Killable>(killedEntity, killable)) {
        Destroy(frame, killedEntity, killType);
        return;
    }
    if (!killable->IsAlive) {
        return;
    }
    Prototypes::Killable_Prototype* prototype = killable->GetPrototype(frame);
    killable->IsAlive = false;
    // A kill zone always disables physics; otherwise the prototype decides.
    const bool disablePhysics = killType == KillType::KillZone || prototype->OnKillDisablePhysics;

    EntityRef weapon;
    EntityRef owner;
    bool isOwnerUltimate = false;
    ResolveKillSource(frame, killedEntity, killerSource, weapon, owner, isOwnerUltimate);
    killable->killedBy.frame = frame->Number;
    killable->killedBy.killType = killType;
    killable->killedBy.isOwnerUltimate = isOwnerUltimate;
    killable->killedBy.owner = owner;
    killable->killedBy.source = killerSource;
    killable->killedBy.weapon = weapon;

    // In the tutorial only the human player's deaths count.
    bool countDeath = true;
    if (frame->get_RuntimeConfig()->get_IsTutorial()) {
        PlayerActor player{};
        if (frame->TryGet<PlayerActor>(killedEntity, player)) {
            PlayerActor* actor = nullptr;
            countDeath = frame->TryGetPointer<PlayerActor>(killable->killedBy.owner, actor) && actor->get_IsHuman();
        }
    }
    if (countDeath) {
        IncrementDeathCount(frame, killedEntity);
    }

    if (disablePhysics) {
        // Physics stay off for one second when the kill came from losing all HP.
        const FP delay = killType == KillType::ZeroHP ? FP::One() : FP{};
        PhysicsAbstraction::Create(frame, killedEntity).DisablePhysics(frame, delay);
    }
    if (prototype->OnKillDisableCollider) {
        PhysicsCollider3D* collider = nullptr;
        if (frame->TryGetPointer<PhysicsCollider3D>(killedEntity, collider)) {
            collider->_disabled = 1;
        }
    }
    if (prototype->OnKillBehaviour == KillableOnKillBehaviour::DestroyEntity) {
        KillNextFrame(frame, killedEntity, prototype->DelayBeforeDestruction, KillType::Undefined, EntityRef{});
    }
    if (prototype->EntityToSpawnOnDeath.Id.IsValid()) {
        Transform3D transform{};
        Ownership* ownership = nullptr;
        if (frame->TryGet<Transform3D>(killedEntity, transform) &&
            Ownership::TryGetOwnership(frame, killedEntity, ownership)) {
            FrameUtility::CreateDynamic(frame, prototype->EntityToSpawnOnDeath, transform.Position, transform.Rotation,
                                        ownership->Owner, ownership->Weapon, EntityRef{},
                                        Team::GetTeamID(frame, ownership->Owner), false);
        }
    }
    frame->Signals.Killable_OnKill(killedEntity, killable->killedBy);
    frame->Events.KillEvent(killedEntity, killType, killable->lastKillzoneSurfaceType);
}

void KillableLib::Revive(Frame* frame, EntityRef entity, FPVector3 position, FPQuaternion rotation) {
    if (IsAlive(frame, entity) || PlayerActorLib::IsGameModeFinished(frame, entity)) {
        return;
    }
    Killable* killable = nullptr;
    if (frame->TryGetPointer<Killable>(entity, killable)) {
        killable->IsAlive = true;
        killable->killedBy = KilledByData{};
        killable->lastKillzoneSurfaceType = KillZoneSurfaceType::Default;
    }
    PhysicsAbstraction::Create(frame, entity).EnablePhysics(frame);
    KccExtension* kcc = nullptr;
    if (frame->TryGetPointer<KccExtension>(entity, kcc)) {
        kcc->enabled = true;
    }
    PhysicsCollider3D* collider = nullptr;
    if (frame->TryGetPointer<PhysicsCollider3D>(entity, collider)) {
        collider->_disabled = 0;
    }
    FPVector3 forward = FPVector3::Forward();
    Transform3D* transform = nullptr;
    if (frame->TryGetPointer<Transform3D>(entity, transform)) {
        transform->Position = position;
        transform->Rotation = rotation;
        forward = transform->Forward();
    }
    frame->Signals.Killable_OnRevive(entity, position, rotation);
    frame->Events.ReviveEvent(entity, forward);
}

void KillableLib::ResolveKillSource(Frame* frame, EntityRef killedEntity, EntityRef& killerSource,
                                    EntityRef& killerWeapon, EntityRef& killerOwner, bool& isOwnerUltimate) {
    killerWeapon = EntityRef{};
    killerOwner = EntityRef{};
    isOwnerUltimate = false;
    if (killerSource == EntityRef{}) {
        PushedByData pushedBy{};
        if (PushableLib::TryGetPushedBy(frame, killedEntity, pushedBy)) {
            killerSource = pushedBy.source;
            killerWeapon = pushedBy.weapon;
            killerOwner = pushedBy.owner;
            return;
        }
    }
    if (killerSource == EntityRef{}) {
        HitData hitBy{};
        if (DamageLib::TryGetHitBy(frame, killedEntity, hitBy)) {
            killerSource = hitBy.source;
            killerWeapon = hitBy.weapon;
            killerOwner = hitBy.owner;
            return;
        }
    }
    if (killerSource == EntityRef{}) {
        return;
    }
    Ownership* ownership = nullptr;
    if (frame->TryGetPointer<Ownership>(killerSource, ownership) && frame->Has<PlayerActor>(ownership->Owner)) {
        killerWeapon = ownership->Weapon;
        killerOwner = ownership->Owner;
        isOwnerUltimate = ownership->IsOwnerUltimate;
    } else if (frame->Has<PlayerActor>(killerSource)) {
        killerOwner = killerSource;
    }
}

bool KillableLib::IsAlive(Frame* frame, EntityRef actorEntity) {
    Killable* killable = nullptr;
    return frame->TryGetPointer<Killable>(actorEntity, killable) && killable->IsAlive;
}

bool KillableLib::IsKillableDead(Frame* frame, EntityRef entity) {
    Killable* killable = nullptr;
    return frame->TryGetPointer<Killable>(entity, killable) && !killable->IsAlive;
}

bool KillableLib::IsKillableAlive(Frame* frame, EntityRef entity) {
    Killable* killable = nullptr;
    return frame->TryGetPointer<Killable>(entity, killable) && killable->IsAlive;
}

std::int32_t KillableLib::GetLivesLeft(Frame* frame, EntityRef actorEntity) {
    Killable* killable = nullptr;
    if (frame->TryGetPointer<Killable>(actorEntity, killable) && killable->maxDeath != 0) {
        return killable->maxDeath - killable->deathCount;
    }
    return 1;
}

void KillableLib::AddExtraLives(Frame* frame, EntityRef actorEntity, std::int32_t extraLives) {
    Killable* killable = nullptr;
    if (frame->TryGetPointer<Killable>(actorEntity, killable) && killable->maxDeath >= 1) {
        killable->maxDeath += extraLives;
    }
}

void KillableLib::IncrementDeathCount(Frame* frame, EntityRef actorEntity) {
    Killable* killable = nullptr;
    if (frame->TryGetPointer<Killable>(actorEntity, killable)) {
        ++killable->deathCount;
    }
}

bool KillableLib::TryGetKilledBy(Frame* frame, EntityRef killedEntity, KilledByData& killedBy) {
    Killable* killable = nullptr;
    if (frame->TryGetPointer<Killable>(killedEntity, killable) && killable->killedBy.frame != 0) {
        killedBy = killable->killedBy;
        return true;
    }
    killedBy = KilledByData{};
    return false;
}

} // namespace Quantum
