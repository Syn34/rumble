// killable_systems.h - death and lifetimes: KillableSystem (delayed kills, death
// animation state, extra lives), OnKillCallbacksSystem, LifetimeLib, LifetimeSystem and
// LifetimeInitSystem.
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

class LifetimeSpec;

struct KillableSystemFilter {
    EntityRef entity;
    KillNextFrame* killNextFrame = nullptr;
    static constexpr auto FilterEntity = &KillableSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&KillableSystemFilter::killNextFrame);
};

class KillableSystem final : public SystemMainThreadFilter<KillableSystemFilter>,
                             public ISignalOnComponentAdded<Killable>,
                             public ISignalReward_OnCollected,
                             public ISignalKillable_OnKill,
                             public ISignalParabolaFollower_OnReachedEnd,
                             public ISignalKillable_OnRevive {
  public:
    // 0x7bc552c
    KillableSystem() = default;
    // 0x7bc4738 - Once a kill mark's delay has passed it is removed and the entity is
    // killed (players) or destroyed (everything else).
    void Update(Frame* f, Filter& filter) override;
    // 0x7bc4880 - New killables are alive (and announced).
    void OnAdded(Frame* f, EntityRef entity, Killable* component) override;
    // 0x7bc48c8 - Extra-life pickups.
    void Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type, std::uint16_t amount) override;
    // TODO(0x7bc48ec, Quantum.KillableSystem.Killable_OnKill): clears pending pushes and,
    // for zero-HP kills with a death animation spec, launches the corpse on a parabola
    // (scene ray/shape casts pick the landing point); needs the 3D scene queries.
    void Killable_OnKill(Frame* f, EntityRef toKill, const KilledByData& killedBy) override {
        (void)f;
        (void)toKill;
        (void)killedBy;
    }
    // 0x7bc53ec - A death flight that lands moves the death animation to its next state.
    void ParabolaFollower_OnReachedEnd(Frame* f, EntityRef entity) override;
    // 0x7bc5494 - Revived entities have no death animation running.
    void Killable_OnRevive(Frame* f, EntityRef toRevive, const FPVector3& position,
                           const FPQuaternion& rotation) override;
};

// Runs the killer's "on player killed" callbacks.
class OnKillCallbacksSystem final : public SystemSignalsOnly, public ISignalKillable_OnKill {
  public:
    // 0x7bc7960
    OnKillCallbacksSystem() = default;
    // 0x7bc7718 - Only kills of players count; the callbacks are the killer's (owner's).
    void Killable_OnKill(Frame* f, EntityRef toKill, const KilledByData& killedBy) override;
};

class LifetimeLib {
  public:
    // 0x7bc6288 - Once: the duration from the spec, or per the owner's weapon level when
    // the spec says so.
    static void TryInitialiseStats(Frame* frame, EntityRef entity, Lifetime* lifetime, LifetimeSpec* lifetimeSpec);
};

struct LifetimeSystemFilter {
    EntityRef entity;
    Lifetime* lifetime = nullptr;
    static constexpr auto FilterEntity = &LifetimeSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&LifetimeSystemFilter::lifetime);
};

class LifetimeSystem final : public SystemMainThreadFilter<LifetimeSystemFilter>,
                             public ISignalKillable_OnKill,
                             public ISignalOnComponentAdded<Lifetime>,
                             public ISignalOnComponentRemoved<Lifetime> {
  public:
    // 0x7bc6e54
    LifetimeSystem() = default;
    // 0x7bc6400 - Entities already marked for a kill are left alone.
    ComponentSet get_Without() const override { return ComponentSet::Create<KillNextFrame>(); }
    // 0x7bc6488 - A started lifetime that has run out kills its entity.
    void Update(Frame* frame, Filter& filter) override;
    // TODO(0x7bc6584, Quantum.LifetimeSystem.Killable_OnKill): spawns the lifetime's
    // entities on death (offset, optional ground ray cast and orientation, ownership and
    // team carried over); needs the 3D scene ray casts.
    void Killable_OnKill(Frame* frame, EntityRef toKill, const KilledByData& killedBy) override {
        (void)frame;
        (void)toKill;
        (void)killedBy;
    }
    // 0x7bc6b9c - Copies the spec's spawn-on-death list into the component.
    void OnAdded(Frame* frame, EntityRef entity, Lifetime* lifetime) override;
    // 0x7bc6dd0 - Frees that list.
    void OnRemoved(Frame* f, EntityRef entity, Lifetime* lifetime) override;
};

class LifetimeInitSystem final : public SystemSignalsOnly, public ISignalOnComponentAdded<Lifetime> {
  public:
    // 0x7bc63f8
    LifetimeInitSystem() = default;
    // 0x7bc5f88 - Starts the lifetime now if the spec says so; a lifetime without a spec
    // is reported and removed.
    void OnAdded(Frame* f, EntityRef entity, Lifetime* lifetime) override;
    // TODO(0x7bc612c, Quantum.LifetimeInitSystem.Projectile_OnSpawnEntityOnDeath): adjusts
    // the lifetime of an entity a projectile spawned on death (shortened after hitting a
    // damageable); not reimplemented yet.
};

// Projectiles remember when they were spawned; one without a spec is removed.
class ProjectileInitSystem final : public SystemSignalsOnly, public ISignalOnComponentAdded<Projectile> {
  public:
    // 0x7c075ac
    ProjectileInitSystem() = default;
    // 0x7c0741c
    void OnAdded(Frame* f, EntityRef entity, Projectile* component) override;
};

} // namespace Quantum
