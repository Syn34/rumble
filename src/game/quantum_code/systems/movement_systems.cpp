// movement_systems.cpp
#include "quantum_code/systems/movement_systems.h"

#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/log.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/level_assets.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/fp_math_utility.h"
#include "quantum_code/input.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/libs.h"
#include "quantum_code/systems/physics_utility.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/transform.h"

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

namespace {

void ApplyConfigOrientationSharpness(Frame* f, KccExtension* kccExt) {
    KccExtensionConfig* config = nullptr;
    if (f->TryFindAssetInternal<KccExtensionConfig>(kccExt->Config.Id, true, config)) {
        kccExt->orientationSharpness = config->OrientationSharpness;
    }
}

} // namespace

void MovementSystem::OnAdded(Frame* f, EntityRef entity, KccExtension* component) {
    (void)entity;
    component->enabled = true;
    component->disableFrame = 0;
    ApplyConfigOrientationSharpness(f, component);
}

void MovementSystem::KccExtension_OnConfigChanged(Frame* f, EntityRef entity, KccExtension* component) {
    (void)entity;
    ApplyConfigOrientationSharpness(f, component);
}

void MovementSystem::OnAdded(Frame* f, EntityRef entity, KccModifier* component) {
    KccModifierSpec* spec = nullptr;
    if (f->TryFindAssetInternal<KccModifierSpec>(component->Spec.Id, true, spec)) {
        return;
    }
    Photon::Deterministic::Log::Error("MovementSystem: a KccModifier has no valid spec; the component is removed");
    f->Remove<KccModifier>(entity);
}

void MovementSystem::OnAdded(Frame* f, EntityRef entity, MovementPackCache* component) {
    (void)f;
    (void)entity;
    component->updateFrequency = 1;
}

void MovementSystem::Killable_OnRevive(Frame* frame, EntityRef entity, const FPVector3& position,
                                       const FPQuaternion& rotation) {
    (void)position;
    (void)rotation;
    ResetKccMotion(frame, entity);
}

void MovementSystem::ResetKccMotion(Frame* frame, EntityRef entity) {
    KccExtension* kccExt = nullptr;
    if (!frame->TryGetPointer<KccExtension>(entity, kccExt)) {
        return;
    }
    kccExt->ResetImpulse();
    kccExt->velocity = FPVector3::Zero();
    kccExt->pushVelocity = FPVector3::Zero();
    kccExt->hasImpulseToConsume = false;
    kccExt->hasPushToConsume = false;
    kccExt->resetPush = false;
    kccExt->forceInput = FPVector2::Zero();
    kccExt->forceInputCounter = FP{};
}

void MovementSystem::Killable_OnKill(Frame* frame, EntityRef entity, const KilledByData& killedBy) {
    (void)killedBy;
    KccExtension* kccExt = nullptr;
    frame->TryGetPointer<KccExtension>(entity, kccExt);
}

void MovementSystem::Update(Frame* f, Filter& filter) {
    KccExtension* kccExt = filter.pKccExt;
    CharacterController3D* kcc = filter.pKcc;
    ActorInput* input = filter.pActorInput;

    if (kccExt->disableFrame != 0 && f->Number > kccExt->disableFrame) {
        PhysicsAbstraction physics = PhysicsAbstraction::Create(f, filter.Entity);
        physics.DisablePhysics(f);
    }
    if (!kccExt->enabled) {
        return;
    }
    CharacterController3DConfig* configKcc = f->FindAsset<CharacterController3DConfig>(kcc->Config().Id);
    KccExtensionConfig* configExt = f->FindAsset<KccExtensionConfig>(kccExt->Config.Id);
    kccExt->RemoveExpiredModifiers(f);

    if (!kccExt->lockRotation) {
        UpdateRotation(f, filter, *input, configExt);
    }

    // A queued impulse replaces part of the velocity.
    if (kccExt->hasImpulseToConsume) {
        kcc->Velocity = FPVector3::Lerp(kcc->Velocity, FPVector3::Zero(), kccExt->velocityBlend);
        kcc->Velocity = kcc->Velocity + kccExt->impulse;
        kccExt->hasImpulseToConsume = false;
        kccExt->velocityBlend = FP{};
        kccExt->Impulsed = true;
    }

    // Homing on an impulse target: the velocity turns towards it (keeping its speed).
    Transform3D targetTransform{};
    if (f->Exists(kccExt->impulseTarget) && f->TryGet<Transform3D>(kccExt->impulseTarget, targetTransform)) {
        const FPVector3 toTarget = (targetTransform.Position - filter.pTransform->Position).Normalized();
        const FPVector3 homing = FPVector3(toTarget.X, FP{}, toTarget.Z) * kcc->Velocity.Magnitude();
        kcc->Velocity = FPVector3::Lerp(kcc->Velocity, homing,
                                        FPMath::Clamp01(f->get_DeltaTime() * kccExt->impulseTargetSnapLerpStrength));
    }

    // A queued push goes into the push velocity.
    if (kccExt->hasPushToConsume) {
        kccExt->hasPushToConsume = false;
        if (kccExt->resetPush) {
            kcc->Velocity = FPVector3::Zero();
            kccExt->resetPush = false;
        }
        kccExt->velocityBlend = FP{};
        kccExt->pushVelocity = kccExt->pushVelocity + kccExt->push;
        kccExt->push = FPVector3::Zero();
    }

    const FP dt = f->get_DeltaTime();
    if (kccExt->lockJumpTimer.RawValue() >= 1) {
        kccExt->lockJumpTimer = kccExt->lockJumpTimer - dt;
        if (kccExt->lockJumpTimer <= FP{}) {
            kccExt->UnlockJump();
        }
    }
    if (kccExt->lockRotationTimer.RawValue() >= 1) {
        kccExt->lockRotationTimer = kccExt->lockRotationTimer - dt;
        if (kccExt->lockRotationTimer <= FP{}) {
            kccExt->UnlockRotation();
        }
    }

    bool jumped = false;
    if (configExt->CanJump && !kccExt->lockJump) {
        if (input->JumpButton.WasPressed()) {
            kccExt->jumpRequested = true;
        }
        if (kccExt->UpdateJumpBeforeMove(f, filter.Entity, kcc, kccExt, configKcc, configExt)) {
            f->Events.KccExtension_OnJumpStateChanged(filter.Entity, kccExt->jumpState);
            jumped = true;
        }
    }

    // The input direction, blended towards any forced input (fading over its fall-off).
    FPVector3 direction = FPVector3::Zero();
    if (configExt->CanMoveFromInputs) {
        direction = FPVector3(input->Direction.X, FP{}, input->Direction.Y);
        kccExt->UpdateForceInput(f);
        if (kccExt->forceInputBlend.RawValue() >= 1) {
            direction = FPVector3::Lerp(direction, FPVector3(kccExt->forceInput.X, FP{}, kccExt->forceInput.Y),
                                        kccExt->forceInputBlend);
        }
    }

    kccExt->Move(f, filter.Entity, filter.pTransform, kcc, kccExt, direction, configKcc, configExt, this);
    kccExt->UpdateJumpAfterMove(f, kcc, jumped);
}

void MovementSystem::UpdateRotation(Frame* f, Filter& filter, ActorInput& input, KccExtensionConfig* configExt) {
    (void)configExt;
    KccExtension* kccExt = filter.pKccExt;
    Transform3D* transform = filter.pTransform;

    FPVector3 lookDirection(input.Direction.X, FP{}, input.Direction.Y);

    // Movement locked, or stunned after a push: the camera does not steer the character.
    bool cameraIgnored = (static_cast<std::int32_t>(input.inputsLocked) & 1) != 0;
    if (!cameraIgnored) {
        PlayerActor* actor = nullptr;
        cameraIgnored = f->TryGetPointer<PlayerActor>(filter.Entity, actor) && actor->noActionCounter > FP{};
    }
    if (!cameraIgnored) {
        if (kccExt->lookTowardsCamera && input.LookInputVector != FPVector3::Zero()) {
            lookDirection = FPVector3(input.LookInputVector.X, FP{}, input.LookInputVector.Z);
        } else if (kccExt->lookTowardsCameraSight && input.CameraSight != FPVector3::Zero()) {
            lookDirection = FPVector3(input.CameraSight.X, FP{}, input.CameraSight.Z);
        }
    }
    if (kccExt->forceOrientation || kccExt->forceLookInput != FPVector3::Zero()) {
        lookDirection = kccExt->forceLookInput;
    }

    if (lookDirection.SqrMagnitude().RawValue() >= 1) {
        FPVector3 facing = lookDirection.Normalized();
        const FPVector3 up = FPVector3::Up();
        if (!kccExt->forceOrientation && kccExt->orientationSharpness.RawValue() >= 1) {
            // Exponential smoothing towards the wanted facing.
            const FPQuaternion from = FPQuaternion::LookRotation(transform->Forward(), false);
            const FPQuaternion to = FPQuaternion::LookRotation(facing, false);
            const FP t = FPMath::Clamp01(FP::One() - FPMath::Exp(-kccExt->orientationSharpness * f->get_DeltaTime()));
            facing = FPQuaternion::LerpUnclamped(from, to, t) * FPVector3::Forward();
        }
        transform->Rotation = FPQuaternion::LookRotation(facing.Normalized(), up, false);
    }

    if (!kccExt->preserveForceLookInput) {
        kccExt->forceOrientation = false;
        kccExt->forceLookInput = FPVector3::Zero();
    }
}

bool MovementSystem::OnCharacterCollision3D(Core::FrameBase* fbase, EntityRef character, Physics3D::Hit3D hit) {
    auto* f = dynamic_cast<Frame*>(fbase);
    if (f == nullptr) {
        return true;
    }
    KccExtension* kccExt = f->GetPointer<KccExtension>(character);
    bool ignoreHit = false;
    f->Signals.KccExtension_OnFilterCollision3D(character, hit, &ignoreHit);
    if (ignoreHit) {
        return false;
    }

    if (PhysicsUtility::IsStatic(hit)) {
        if (PhysicsUtility::IsWall(hit)) {
            f->Signals.KccExtension_OnStaticWallCollision3D(character, hit);
            return true;
        }
        const StaticColliderConfig* config = GetHitStaticColliderConfig(f, hit);
        kccExt->TrySetCurrentSurfaceType(config != nullptr ? config->SurfaceType : KccSurfaceType::Normal,
                                         KccSurfaceSetPriority::Static);
        return true;
    }

    // A dynamic body: the character pushes it along the contact normal by how fast it walks
    // into it (scaled per kind of entity).
    const EntityRef other = GetHitEntity(hit);
    if (!f->Exists(other)) {
        return true;
    }
    CharacterController3D* kcc = nullptr;
    Transform3D* otherTransform = nullptr;
    PhysicsBody3D* body = nullptr;
    if (f->TryGetPointer<CharacterController3D>(character, kcc) && f->TryGetPointer<Transform3D>(other, otherTransform) &&
        f->TryGetPointer<PhysicsBody3D>(other, body)) {
        const FPVector2 walk(kcc->Velocity.X, kcc->Velocity.Z);
        const FP into = FPVector2::Dot(FPVector2(hit.Normal.X, hit.Normal.Z).Normalized(), walk);
        FPVector3 force = hit.Normal * (into * 2);
        Code::EntityInfo info{};
        KccExtensionConfig* config = nullptr;
        FP multiplier;
        if (f->TryGet<Code::EntityInfo>(other, info) &&
            f->TryFindAssetInternal<KccExtensionConfig>(kccExt->Config.Id, true, config) &&
            config->TryGetWalkingPushMultiplier(info.type, multiplier)) {
            force = force * multiplier;
        }
        body->WakeUp();
        body->AddForceAtPosition(force, hit.Point, otherTransform);
    }
    f->Signals.KccExtension_OnCharacterCollision3D(character, hit);
    return true;
}

// -----------------------------------------------------------------------------------------
// Actor input

void ActorInputConverterSystem::Update(Frame* f, Filter& filter) {
    ActorInput* actorInput = filter.actorInput;
    PlayerActor* actor = filter.actor;
    const ::Quantum::Input* input = actor->GetInput(f);
    actorInput->areHumanInputs = false;
    if (input != nullptr) {
        actorInput->areHumanInputs = true;
        if (UnlockedFeaturesLib::IsFeatureUnlocked(f, filter.entity, UnlockedFeature::Gameplay_CharacterMovement, true)) {
            actorInput->Direction = input->get_Direction();
        }
        if (UnlockedFeaturesLib::IsFeatureUnlocked(f, filter.entity, UnlockedFeature::Gameplay_CameraMovement, true)) {
            actorInput->LookInputVector = input->get_LookInputVector();
            actorInput->CameraPosition = input->GetCameraPosition(filter.transform);
            actorInput->CameraSight = input->get_CameraSight();
        }
        actorInput->MeleeStateWanted = input->Melee.IsDown();
        actorInput->DashStateWanted = input->Dash.IsDown();
        actorInput->JumpStateWanted = input->Jump.IsDown();
        actorInput->FireStateWanted = input->Fire.IsDown();
        actorInput->UltimateStateWanted = input->Ultimate.IsDown();
        actorInput->GadgetStateWanted = input->Gadget.IsDown();
        actorInput->ReloadStateWanted = input->Reload.IsDown();
    }

    // After a push the actor stays inactive for a while.
    if (actor->noActionCounter.RawValue() >= 1) {
        const FP remaining = actor->noActionCounter - f->get_DeltaTime();
        actor->noActionCounter = FPMath::Max(remaining, FP{});
        if (actorInput->inputsLocked == ActorInputLockFlags::None) {
            if (remaining.RawValue() >= 1) {
                actorInput->Direction = FPVector2::Zero();
                actorInput->MeleeStateWanted = false;
                actorInput->DashStateWanted = false;
                actorInput->JumpStateWanted = false;
                actorInput->FireStateWanted = false;
                actorInput->UltimateStateWanted = false;
                actorInput->GadgetStateWanted = false;
                actorInput->ReloadStateWanted = false;
            }
            return;
        }
    } else if (actorInput->inputsLocked == ActorInputLockFlags::None) {
        return;
    }

    // Locked inputs are cleared.
    const bool humanInputs = actorInput->areHumanInputs;
    const ActorInputLockFlags locks = actorInput->inputsLocked;
    const auto flags = static_cast<std::int32_t>(locks);
    constexpr std::int32_t movement = static_cast<std::int32_t>(ActorInputLockFlags::Movement);
    constexpr std::int32_t abilities = static_cast<std::int32_t>(ActorInputLockFlags::Abilities);
    constexpr std::int32_t firing = static_cast<std::int32_t>(ActorInputLockFlags::FiringAndUltimate);
    constexpr std::int32_t external = static_cast<std::int32_t>(ActorInputLockFlags::ExternalCommand);
    if (flags == 0x1f || (flags & external) != 0) {
        *actorInput = ActorInput{};
        f->Signals.PlayerActor_OnInputLocked(filter.entity, true);
    } else {
        if ((flags & movement) != 0) {
            actorInput->Direction = FPVector2::Zero();
            actorInput->LookInputVector = FPVector3::Zero();
        }
        if ((flags & abilities) != 0) {
            actorInput->DashButton = Button{};
            actorInput->DashStateWanted = false;
            actorInput->JumpButton = Button{};
            actorInput->JumpStateWanted = false;
        }
        if ((flags & firing) != 0) {
            actorInput->UltimateButton = Button{};
            actorInput->FireButton = Button{};
            actorInput->GadgetButton = Button{};
            actorInput->FireStateWanted = false;
            actorInput->GadgetStateWanted = false;
            // Note: the original clears the reload wish here too.
            actorInput->ReloadStateWanted = false;
            actorInput->UltimateStateWanted = false;
        }
    }
    actorInput->inputsLocked = locks;
    actorInput->areHumanInputs = humanInputs;
}

void ActorInputUpdateSystem::Update(Frame* f, Filter& filter) {
    ActorInput* input = filter.actorInput;
    const auto wanted = [&](QBoolean state, UnlockedFeature feature) {
        return state && UnlockedFeaturesLib::IsFeatureUnlocked(f, filter.entity, feature, true);
    };
    input->MeleeButton.Update(f->Number, wanted(input->MeleeStateWanted, UnlockedFeature::Gameplay_Melee));
    input->DashButton.Update(f->Number, wanted(input->DashStateWanted, UnlockedFeature::Gameplay_Dash));
    input->JumpButton.Update(f->Number, wanted(input->JumpStateWanted, UnlockedFeature::Gameplay_Jump));
    input->FireButton.Update(f->Number, wanted(input->FireStateWanted, UnlockedFeature::Gameplay_Fire));
    input->UltimateButton.Update(f->Number,
                                 wanted(input->UltimateStateWanted, UnlockedFeature::Gameplay_UltimateWeapon));
    input->GadgetButton.Update(f->Number, wanted(input->GadgetStateWanted, UnlockedFeature::Gameplay_GadgetWeapon));
    input->ReloadButton.Update(f->Number, input->ReloadStateWanted);
    HandlePlayerCommands(f, filter.entity);
}

void ActorInputUpdateSystem::HandlePlayerCommands(Frame* frame, EntityRef entity) {
    PlayerActor* actor = nullptr;
    if (!frame->TryGetPointer<PlayerActor>(entity, actor) || !GameModeHandlerLib::IsInActionPhase(frame)) {
        return;
    }
    const std::shared_ptr<Photon::Deterministic::DeterministicCommand> command =
        frame->GetPlayerCommand(actor->playerRef.ToIndex());
    if (command == nullptr) {
        return;
    }
    ActorInput* input = nullptr;
    if (!frame->TryGetPointer<ActorInput>(entity, input)) {
        return;
    }
    const auto* lockCommand = dynamic_cast<const CommandActorLockInputs*>(command.get());
    if (lockCommand == nullptr) {
        return;
    }
    const auto flags = static_cast<std::int32_t>(input->inputsLocked);
    constexpr std::int32_t external = static_cast<std::int32_t>(ActorInputLockFlags::ExternalCommand);
    input->inputsLocked = static_cast<ActorInputLockFlags>(lockCommand->locked ? (flags | external) : (flags & ~external));
}

// -----------------------------------------------------------------------------------------
// Turning and ledge grabbing

void TurnToSystem::Update(Frame* f, Filter& filter) {
    const TurnTo* turn = filter.turn;
    if (turn->lookAtDirection == FPVector3::Zero()) {
        return;
    }
    Transform3D* transform = filter.transform;
    const FPVector3 target = FPVector3(turn->lookAtDirection.X, FP{}, turn->lookAtDirection.Z).Normalized();
    const FPVector3 forward = transform->Forward();
    const FPVector3 current = FPVector3(forward.X, FP{}, forward.Z).Normalized();
    const FPQuaternion from = FPQuaternion::LookRotation(current, false);
    const FPQuaternion to = FPQuaternion::LookRotation(target, false);
    const FP t = FPMath::Clamp01(f->get_DeltaTime() * turn->turnSpeed);
    const FPVector3 facing = FPQuaternion::LerpUnclamped(from, to, t) * FPVector3::Forward();
    if (!FPMathUtility::EqualWithEpsilon(facing, transform->Forward())) {
        transform->Rotation = FPQuaternion::LookRotation(facing, false);
    }
}

void LedgeGrabbingActorSystem::Update(Frame* f, Filter& filter) {
    LedgeGrabbingActor* grab = filter.ledgeGrabbingActor;
    if (!grab->IsLedgeGrabbing) {
        return;
    }
    LedgeGrabbingSpec* spec = nullptr;
    if (!f->TryFindAssetInternal<LedgeGrabbingSpec>(grab->Spec.Id, true, spec)) {
        return;
    }
    const FP t = f->get_DeltaTime() * (f->Number - grab->LedgeGrabbingStartFrame) / spec->snapDuration;
    const FPVector3& start = grab->GrabStartPosition;
    const FPVector3& end = grab->Destination;
    const FP x = FPMath::Lerp(start.X, end.X, spec->snapPositionCurveXZ.Evaluate(t));
    const FP y = FPMath::Lerp(start.Y, end.Y, spec->snapPositionCurveY.Evaluate(t));
    const FP z = FPMath::Lerp(start.Z, end.Z, spec->snapPositionCurveXZ.Evaluate(t));
    filter.transform->Position = FPVector3(x, y, z);
    filter.transform->Rotation = FPQuaternion::LerpUnclamped(grab->GrabStartRotation, grab->SnapRotation,
                                                             FPMath::Clamp01(spec->snapRotationCurve.Evaluate(t)));
    if (f->get_DeltaTime() * (f->Number - grab->LedgeGrabbingStartFrame) >= spec->snapDuration) {
        filter.transform->Position = grab->Destination;
        grab->IsLedgeGrabbing = false;
        f->Events.OnLedgeGrabEnd(filter.entity);
    }
}

void LedgeGrabbingActorSystem::KccExtension_OnValidLedgeDetected(Frame* f, EntityRef character,
                                                                 const Physics3D::Hit3D& hit,
                                                                 const FPVector3& aboveLedgePoint) {
    LedgeGrabbingActor* grab = nullptr;
    Transform3D* transform = nullptr;
    LedgeGrabbingSpec* spec = nullptr;
    if (!f->TryGetPointer<LedgeGrabbingActor>(character, grab) || !f->TryGetPointer<Transform3D>(character, transform) ||
        !f->TryFindAssetInternal<LedgeGrabbingSpec>(grab->Spec.Id, true, spec)) {
        return;
    }
    grab->IsLedgeGrabbing = true;
    grab->LedgeGrabbingStartFrame = f->Number;
    const FPVector3 towardsWall = (hit.Point - transform->Position).Normalized();
    grab->GrabStartPosition = transform->Position;
    grab->SnapPosition = FPVector3(hit.Point.X - spec->snapPosForwardOffset * towardsWall.X,
                                   aboveLedgePoint.Y - spec->snapPosHeightOffset,
                                   hit.Point.Z - spec->snapPosForwardOffset * towardsWall.Z);
    grab->GrabStartRotation = transform->Rotation;
    grab->SnapRotation = FPQuaternion::LookRotation(FPVector3(towardsWall.X, FP{}, towardsWall.Z), false);
    grab->Destination = aboveLedgePoint;
    f->Events.OnLedgeGrabStart(character, spec->cameraDistanceOffset, spec->cameraVerticalAngle);
    f->Signals.LedgeGrabbingActor_OnLedgeGrabStart(character);
}

// -----------------------------------------------------------------------------------------
// Wall running

void WallRunningActor::Init(Frame* f, const WallRunningSystemFilter& filter) {
    WallRunningSpec* spec = nullptr;
    Transform3D* wall = nullptr;
    if (!f->TryFindAssetInternal<WallRunningSpec>(Spec.Id, true, spec) ||
        !f->TryGetPointer<Transform3D>(RunnableWallEntity, wall)) {
        return;
    }
    // Run along the wall the way the actor moves, or else looks, or else forwards.
    FP alignment = FPVector3::Dot(filter.pKcc->Velocity.Normalized(), wall->Forward());
    FP side = FP::One();
    if (FPMath::Abs(alignment) > spec->PerpendicularThreshold) {
        side = FPMath::Sign(alignment);
    } else {
        alignment = FPVector3::Dot(filter.pActorInput->CameraSight, wall->Forward());
        if (FPMath::Abs(alignment) > spec->PerpendicularThreshold) {
            side = FPMath::Sign(alignment);
        }
    }
    const FPVector3 fromWall = filter.pTransform->Position - wall->Position;
    const FPVector3 right = wall->Right();
    PreviousPosition = wall->Position + FPVector3::ProjectOnPlane(fromWall, right) + right * spec->SnapOffset;
    RunningDirection = wall->Forward() * side;
    const FP horizontalSpeedSqr =
        filter.pKcc->Velocity.Z * filter.pKcc->Velocity.Z + filter.pKcc->Velocity.X * filter.pKcc->Velocity.X;
    const FP horizontalSpeed = FP::FromRaw(FPMath::SqrtRaw(horizontalSpeedSqr.RawValue()));
    // Note: the original adds the perpendicular threshold here, not a speed.
    RunningSpeed = FPMath::Abs(spec->PerpendicularThreshold + horizontalSpeed * alignment);
    RunningLeft = side > FP{};
    IsInitialized = true;
    f->Events.OnWallRunningStart(filter.entity, RunningLeft);
    f->Signals.WallRunningActor_OnWallRunningStart(filter.entity);
}

void WallRunningActor::Update(Frame* f, const WallRunningSystemFilter& filter) {
    if (!IsInitialized) {
        Init(f, filter);
    }
    WallRunningSpec* spec = nullptr;
    Transform3D wall{};
    if (!f->TryFindAssetInternal<WallRunningSpec>(Spec.Id, true, spec) ||
        !f->TryGet<Transform3D>(RunnableWallEntity, wall)) {
        return;
    }
    const FP dt = f->get_DeltaTime();
    const FPVector3 step = RunningDirection * RunningSpeed;
    FP y = PreviousPosition.Y + dt * step.Y;
    PhysicsCollider3D wallCollider{};
    if (f->TryGet<PhysicsCollider3D>(RunnableWallEntity, wallCollider)) {
        // The height is pulled towards the wall collider's centre.
        const FPVector3 centroid = wall.TransformPoint(wallCollider.Shape.LocalTransform.Position);
        y = FPMath::Lerp(y, centroid.Y + spec->HeightCorrectionCentroidOffset, spec->HeightCorrectionInterpolationSpeed);
    }
    const FPVector3 position(PreviousPosition.X + dt * step.X, y, PreviousPosition.Z + dt * step.Z);
    filter.pTransform->Position = position;
    filter.pTransform->Rotation = FPQuaternion::LookRotation(RunningDirection, FPVector3::Up(), false);
    PreviousPosition = position;

    if (RunningSpeed < spec->Speed) {
        RunningSpeed = FPMath::Clamp(RunningSpeed + dt * spec->Acceleration, RunningSpeed, spec->Speed);
    } else if (RunningSpeed > spec->Speed) {
        RunningSpeed = FPMath::Clamp(RunningSpeed - dt * spec->Deceleration, spec->Speed, RunningSpeed);
    }
    filter.pKcc->Velocity = FPVector3::Zero();

    if (filter.pActorInput->JumpButton.WasPressed()) {
        // Off the wall: away from it and up.
        Stop(f, filter, true, (wall.Right() + wall.Up()) * spec->JumpSpeed);
    }
}

void WallRunningActor::OnActorDashed(Frame* f, EntityRef entity, const WallRunningSystemFilter& filter) {
    (void)entity;
    WallRunningSpec* spec = nullptr;
    DashActorConfig* dashConfig = nullptr;
    if (!f->TryFindAssetInternal<WallRunningSpec>(filter.pWallRunningActor->Spec.Id, true, spec) ||
        !f->TryFindAssetInternal<DashActorConfig>(filter.pDashActor->Spec.Id, true, dashConfig)) {
        return;
    }
    FPVector3 dashDirection = filter.pTransform->Forward();
    const FPVector2& input = filter.pActorInput->Direction;
    if (input.Y * input.Y + input.X * input.X > dashConfig->validInputThreshold) {
        dashDirection = FPVector3(input.X, FP{}, input.Y).Normalized();
    }
    if (FPVector3::Dot(dashDirection, RunningDirection) < spec->PerpendicularThreshold) {
        Stop(f, filter, true, std::nullopt);
    } else {
        RunningSpeed = dashConfig->force + RunningSpeed;
    }
}

void WallRunningActor::Stop(Frame* f, const WallRunningSystemFilter& filter, bool isJumping,
                            std::optional<FPVector3> impulse) {
    IsWallRunning = false;
    RunnableWallEntity = EntityRef::None();
    if (impulse.has_value()) {
        filter.pKccExt->AddImpulsionWithInputsAuto(f, filter.pKcc, *impulse);
    }
    if (isJumping) {
        filter.pKccExt->jumpState = JumpState::Jumping;
    }
    f->Events.OnWallRunningStop(filter.entity);
}

void WallRunningSystem::OnAdded(Frame* f, EntityRef entity, WallRunningActor* wallRunningActor) {
    (void)f;
    (void)entity;
    wallRunningActor->CanWallRun = true;
}

void WallRunningSystem::OnInit(Frame* f) {
    (void)f;
    _filterGetter = ComponentGetter<WallRunningSystemFilter>{};
}

void WallRunningSystem::Update(Frame* f, Filter& filter) {
    if (filter.pWallRunningActor->IsWallRunning) {
        filter.pWallRunningActor->Update(f, filter);
    }
}

void WallRunningSystem::Pushable_OnPushed(Frame* f, const PushData& pushParams, const OnPushedData& onPushedData) {
    (void)onPushedData;
    WallRunningSystemFilter filter{};
    if (_filterGetter.TryGet(f, pushParams.pushed, filter) && filter.pWallRunningActor->IsWallRunning) {
        filter.pWallRunningActor->Stop(f, filter, true, pushParams.impulse);
    }
}

void WallRunningSystem::DashActor_OnDashed(Frame* f, EntityRef entity) {
    WallRunningSystemFilter filter{};
    if (_filterGetter.TryGet(f, entity, filter) && filter.pWallRunningActor->IsWallRunning) {
        filter.pWallRunningActor->OnActorDashed(f, entity, filter);
    }
}

void WallRunningSystem::Killable_OnRevive(Frame* f, EntityRef toRevive, const FPVector3& position,
                                          const FPQuaternion& rotation) {
    (void)position;
    (void)rotation;
    WallRunningActor* runner = nullptr;
    WallRunningSystemFilter filter{};
    if (f->TryGetPointer<WallRunningActor>(toRevive, runner) && runner->IsWallRunning &&
        _filterGetter.TryGet(f, toRevive, filter)) {
        runner->Stop(f, filter, false, std::nullopt);
        filter.pKcc->Velocity = FPVector3::Zero();
    }
}

void WallRunningSystem::BlastBallGame_StateChanged(Frame* f, BlastBallGameState newState) {
    if (newState != BlastBallGameState::Playing && newState != BlastBallGameState::Restarting) {
        return;
    }
    const bool canWallRun = newState == BlastBallGameState::Playing;
    for (const auto& pair : f->Unsafe.GetComponentBlockIterator<WallRunningActor>().Range()) {
        pair.Component->CanWallRun = canWallRun;
    }
}

} // namespace Quantum
