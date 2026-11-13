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

struct TurnToSystemFilter {
    EntityRef entity;
    Transform3D* transform = nullptr;
    TurnTo* turn = nullptr;
    static constexpr auto FilterEntity = &TurnToSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&TurnToSystemFilter::transform, &TurnToSystemFilter::turn);
};

// Turns entities with a TurnTo towards its horizontal direction at its turn speed.
class TurnToSystem final : public SystemMainThreadFilter<TurnToSystemFilter> {
  public:
    // 0x7bd28ec
    TurnToSystem() = default;
    // 0x7bd260c
    bool get_StartEnabled() const override { return false; }
    // 0x7bd2614
    void Update(Frame* f, Filter& filter) override;
};

struct LedgeGrabbingActorSystemFilter {
    EntityRef entity;
    Transform3D* transform = nullptr;
    LedgeGrabbingActor* ledgeGrabbingActor = nullptr;
    static constexpr auto FilterEntity = &LedgeGrabbingActorSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&LedgeGrabbingActorSystemFilter::transform,
                                                             &LedgeGrabbingActorSystemFilter::ledgeGrabbingActor);
};

// Climbs grabbed ledges: snaps the actor from where it grabbed to the top of the ledge along
// the spec's curves.
class LedgeGrabbingActorSystem final : public SystemMainThreadFilter<LedgeGrabbingActorSystemFilter>,
                                       public ISignalKccExtension_OnValidLedgeDetected {
  public:
    // 0x7cb6f74
    LedgeGrabbingActorSystem() = default;
    // 0x7cb69ac
    void Update(Frame* f, Filter& filter) override;
    // 0x7cb6ca8 - Starts the grab: the snap point in front of the wall below the ledge top,
    // facing the wall.
    void KccExtension_OnValidLedgeDetected(Frame* f, EntityRef character, const Physics3D::Hit3D& hit,
                                           const FPVector3& aboveLedgePoint) override;
};

struct WallRunningSystemFilter {
    EntityRef entity;
    CharacterController3D* pKcc = nullptr;
    KccExtension* pKccExt = nullptr;
    Transform3D* pTransform = nullptr;
    WallRunningActor* pWallRunningActor = nullptr;
    DashActor* pDashActor = nullptr;
    ActorInput* pActorInput = nullptr;
    static constexpr auto FilterEntity = &WallRunningSystemFilter::entity;
    static constexpr auto FilterComponents =
        std::make_tuple(&WallRunningSystemFilter::pKcc, &WallRunningSystemFilter::pKccExt,
                        &WallRunningSystemFilter::pTransform, &WallRunningSystemFilter::pWallRunningActor,
                        &WallRunningSystemFilter::pDashActor, &WallRunningSystemFilter::pActorInput);
};

// Runs actors along runnable walls (WallRunningActor does the work).
class WallRunningSystem final : public SystemMainThreadFilter<WallRunningSystemFilter>,
                                public ISignalOnComponentAdded<WallRunningActor>,
                                public ISignalPushable_OnPushed,
                                public ISignalDashActor_OnDashed,
                                public ISignalKillable_OnRevive,
                                public ISignalBlastBallGame_StateChanged {
  public:
    // 0x7bd9a4c
    WallRunningSystem() = default;
    // 0x7bd9324 - New actors may wall run.
    void OnAdded(Frame* f, EntityRef entity, WallRunningActor* wallRunningActor) override;
    // 0x7bd934c - Prepares the getter that fills the filter for one entity.
    void OnInit(Frame* f) override;
    // 0x7bd93f8
    void Update(Frame* f, Filter& filter) override;
    // 0x7bd9464 - A pushed runner leaves the wall with the push's impulse.
    void Pushable_OnPushed(Frame* f, const PushData& pushParams, const OnPushedData& onPushedData) override;
    // 0x7bd95e4
    void DashActor_OnDashed(Frame* f, EntityRef entity) override;
    // 0x7bd96b4 - Revived runners are off the wall and at rest.
    void Killable_OnRevive(Frame* f, EntityRef toRevive, const FPVector3& position,
                           const FPQuaternion& rotation) override;
    // 0x7bd983c - No wall running while a blast ball round restarts.
    void BlastBallGame_StateChanged(Frame* f, BlastBallGameState newState) override;

  private:
    ComponentGetter<WallRunningSystemFilter> _filterGetter;
};

} // namespace Quantum
