// movement_systems.h - MovementSystem: drives every kinematic character (controller plus
// KccExtension) from its actor input each frame, and filters the controller's contacts.
#pragma once

#include "quantum_code/systems/system_main_thread.h"
#include "quantum_core/physics3d/character_controller3d.h"
#include "photon_deterministic/deterministic_command.h"

#include <memory>

namespace Quantum {

class KccExtensionConfig;

struct MovementSystemFilter {
    EntityRef Entity;
    CharacterController3D* pKcc = nullptr;
    KccExtension* pKccExt = nullptr;
    Transform3D* pTransform = nullptr;
    ActorInput* pActorInput = nullptr;
    static constexpr auto FilterEntity = &MovementSystemFilter::Entity;
    static constexpr auto FilterComponents =
        std::make_tuple(&MovementSystemFilter::pKcc, &MovementSystemFilter::pKccExt, &MovementSystemFilter::pTransform,
                        &MovementSystemFilter::pActorInput);
};

class MovementSystem final : public SystemMainThreadFilter<MovementSystemFilter>,
                             public IKCCCallbacks3D,
                             public ISignalOnComponentAdded<KccExtension>,
                             public ISignalOnComponentAdded<KccModifier>,
                             public ISignalOnComponentAdded<MovementPackCache>,
                             public ISignalKccExtension_OnConfigChanged,
                             public ISignalKillable_OnRevive,
                             public ISignalKillable_OnKill {
  public:
    // 0x7bdebc8
    MovementSystem() = default;

    // 0x7bdd258 - A new extension starts enabled, with the config's orientation sharpness.
    void OnAdded(Frame* f, EntityRef entity, KccExtension* component) override;
    // 0x7bdd310 - Picks up the new config's orientation sharpness.
    void KccExtension_OnConfigChanged(Frame* f, EntityRef entity, KccExtension* component) override;
    // 0x7bdd3b4 - A modifier without a valid spec is logged and removed.
    void OnAdded(Frame* f, EntityRef entity, KccModifier* component) override;
    // 0x7bdd530 - The movement pack is recomputed every frame by default.
    void OnAdded(Frame* f, EntityRef entity, MovementPackCache* component) override;
    // 0x7bdd548
    void Killable_OnRevive(Frame* frame, EntityRef entity, const FPVector3& position,
                           const FPQuaternion& rotation) override;
    // 0x7bdd554 - Drops pending impulses, pushes and forced input; the velocity is cleared.
    static void ResetKccMotion(Frame* frame, EntityRef entity);
    // 0x7bdd6d8 - Only looks the extension up (nothing else is left in the original).
    void Killable_OnKill(Frame* frame, EntityRef entity, const KilledByData& killedBy) override;
    // 0x7bdd760 - Delayed physics shut-down, expired modifiers, rotation, consumed
    // impulses and pushes, impulse target homing, lock timers, jumps, the input direction
    // (blended with any forced input), then KccExtension.Move.
    void Update(Frame* f, Filter& filter) override;
    // 0x7bde1dc - Turns the character towards the input (or camera, or forced) direction,
    // smoothed by the orientation sharpness.
    void UpdateRotation(Frame* f, Filter& filter, ActorInput& input, KccExtensionConfig* configExt);
    // 0x7bde664 - Lets other systems ignore a contact; walls and static surfaces are
    // reported, dynamic bodies get pushed by the character.
    bool OnCharacterCollision3D(Core::FrameBase* fbase, EntityRef character, Physics3D::Hit3D hit) override;
    // 0x7bdebc4 - Nothing.
    void OnCharacterTrigger3D(Core::FrameBase* f, EntityRef character, Physics3D::Hit3D hit) override {
        (void)f, (void)character, (void)hit;
    }
};

// Quantum.CommandActorLockInputs: a player command locking (or unlocking) the player's
// inputs from outside the game (ExternalCommand lock flag).
class CommandActorLockInputs final : public Photon::Deterministic::DeterministicCommand {
  public:
    bool locked = false;

    // 0x7cb282c
    CommandActorLockInputs() = default;
    // 0x7cb280c
    void Serialize(Photon::Deterministic::BitStream& stream) override { stream.Serialize(locked); }

  protected:
    std::shared_ptr<DeterministicCommand> CreateInstance() const override {
        return std::make_shared<CommandActorLockInputs>();
    }
};

struct ActorInputConverterSystemFilter {
    EntityRef entity;
    PlayerActor* actor = nullptr;
    ActorInput* actorInput = nullptr;
    Transform3D* transform = nullptr;
    static constexpr auto FilterEntity = &ActorInputConverterSystemFilter::entity;
    static constexpr auto FilterComponents =
        std::make_tuple(&ActorInputConverterSystemFilter::actor, &ActorInputConverterSystemFilter::actorInput,
                        &ActorInputConverterSystemFilter::transform);
};

// Copies each player's input of the frame into the actor's ActorInput (bots fill theirs
// themselves), then applies the no-action time after a push and the input locks.
class ActorInputConverterSystem final : public SystemMainThreadFilter<ActorInputConverterSystemFilter> {
  public:
    // 0x7cb23c8
    ActorInputConverterSystem() = default;
    // 0x7cb201c
    void Update(Frame* f, Filter& filter) override;
};

struct ActorInputUpdateSystemFilter {
    EntityRef entity;
    ActorInput* actorInput = nullptr;
    static constexpr auto FilterEntity = &ActorInputUpdateSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&ActorInputUpdateSystemFilter::actorInput);
};

// Turns the wanted states into button states (pressed/released this frame), honouring the
// features the player has unlocked, and applies lock commands.
class ActorInputUpdateSystem final : public SystemMainThreadFilter<ActorInputUpdateSystemFilter> {
  public:
    // 0x7cb27c4
    ActorInputUpdateSystem() = default;
    // 0x7cb2410
    void Update(Frame* f, Filter& filter) override;
    // 0x7cb263c - During the action phase a CommandActorLockInputs from the player sets or
    // clears the ExternalCommand lock.
    static void HandlePlayerCommands(Frame* frame, EntityRef entity);
};

} // namespace Quantum
