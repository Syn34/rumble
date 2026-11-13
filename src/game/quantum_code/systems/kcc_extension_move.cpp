// kcc_extension_move.cpp - KccExtension's movement: Move and the steps it runs (jumps,
// steering, pushes and rebounds, ledge grabbing and slowing, wall-run auto jumps).
#include "photon_deterministic/fp_math.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/level_assets.h"
#include "quantum_code/input.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/libs.h"
#include "quantum_code/systems/physics_utility.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/map.h"
#include "quantum_core/physics3d/physics_engine3d.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/query_options.h"
#include "quantum_core/shape3d.h"
#include "quantum_core/transform.h"

#include <algorithm>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;
using Physics3D::Hit3D;
using Physics3D::HitCollection3D;

namespace {

constexpr std::int32_t kGroundLayers = 0x40;     // layer 6
constexpr std::int32_t kAutoJumpLayers = 0x440;  // layers 6 and 10
constexpr std::int32_t kLedgeSlowLayers = 0x41;  // layers 0 and 6
constexpr std::int32_t kStaticsOnly = static_cast<std::int32_t>(QueryOptions::HitStatics);
constexpr std::int32_t kStaticsAndKinematics =
    static_cast<std::int32_t>(QueryOptions::HitStatics | QueryOptions::HitKinematics);
constexpr std::int32_t kSolidDetailed =
    static_cast<std::int32_t>(QueryOptions::HitStatics | QueryOptions::HitKinematics | QueryOptions::ComputeDetailedInfo);
// Everything but triggers (the original passes ~HitTriggers).
constexpr std::int32_t kAllButTriggers = ~static_cast<std::int32_t>(QueryOptions::HitTriggers);

bool IsZero(const FPVector3& v) {
    return v == FPVector3::Zero();
}

// C#'s % on ints (the sign follows the dividend); arm64 gives x % 0 == x.
std::int32_t Modulo(std::int32_t value, std::int32_t divisor) {
    return divisor == 0 ? value : value % divisor;
}

FPVector3 Horizontal(const FPVector3& v) {
    return FPVector3(v.X, FP{}, v.Z);
}

} // namespace

// -----------------------------------------------------------------------------------------
// Pushes and impulses

void KccExtension::AddPush(Frame* f, FPVector3 _push, bool resetPush_) {
    if (resetPush_) {
        resetPush = true;
    }
    hasPushToConsume = true;
    KccExtensionConfig* config = f->FindAsset<KccExtensionConfig>(Config.Id);
    if (config == nullptr) {
        return;
    }
    if (config->PushUpwardLerp.RawValue() >= 1 && !cancelUpwardPushCorrection) {
        const FP angle = FPVector3::Angle(FPVector3::Up(), _push);
        if (angle <= config->PushUpwardAngle_MinLerp) {
            FP upwardLerp;
            const FP aboveMax = angle - config->PushUpwardAngle_MaxLerp;
            if (aboveMax > FP{}) {
                upwardLerp = FPMath::Lerp(config->PushUpwardLerp, FP{},
                                          aboveMax / (config->PushUpwardAngle_MinLerp - config->PushUpwardAngle_MaxLerp));
            } else {
                upwardLerp = FPMath::Lerp(FP{}, config->PushUpwardLerp, angle / config->PushUpwardAngle_MaxLerp);
            }
            _push = FPVector3::Lerp(_push, FPVector3::Up() * _push.Magnitude(), upwardLerp);
        }
    }
    push = push + _push;
}

void KccExtension::AddImpulsionWithInputs(Frame* f, CharacterController3D* pKcc, FPVector3 _impulse, FP _velocityBlend,
                                          FP _inputBlend, FP _inputDuration, FP _inputFallOff, FP _duration,
                                          bool _pushAnim, bool _snapOnGround) {
    (void)f;
    (void)_pushAnim;
    SetForceInput(FPVector2::ClampMagnitude(FPVector2(_impulse.X, _impulse.Z), FP::One()), _inputBlend, _inputDuration,
                  _inputFallOff, _snapOnGround);
    pKcc->SetJumped(true);
    hasImpulseToConsume = true;
    impulse = _impulse;
    impulseTimer = _duration;
    velocityBlend = _velocityBlend;
    const FP strength = _impulse.Magnitude();
    impulseMaxSpeed = strength;
    impulseMaxVerticalSpeed = strength;
}

void KccExtension::AddImpulsionWithInputsAuto(Frame* f, CharacterController3D* pKcc, FPVector3 _impulse) {
    const FP scale = FPMath::Clamp(_impulse.Magnitude() / FP::FromInt(25), FP{}, FP::FromInt(2));
    AddImpulsionWithInputs(f, pKcc, _impulse, FP::FromRaw(49152), FP::One(), scale * FP::FromRaw(49152),
                           scale * FP::FromRaw(16384), scale, false, false);
}

// -----------------------------------------------------------------------------------------
// Jumping

bool KccExtension::CheckFrameSlicing(Frame* frame, EntityRef entity, std::int32_t frameSlicing) {
    if (frameSlicing < 2) {
        return true;
    }
    return Modulo(frame->Number + entity.Index, frameSlicing) == 0;
}

bool KccExtension::IsMovingForward(CharacterController3D* pKcc, Transform3D* pTransform, KccExtensionConfig* configExt) {
    const FPVector3 velocity = Horizontal(pKcc->Velocity) * configExt->AutoJumpVelocityFactor;
    return FPVector3::Angle(velocity, pTransform->Forward()) < configExt->AutoJumpMaxAngle;
}

void KccExtension::Jump(Frame* frame, CharacterController3D* pKcc, KccExtension* kccExt,
                        CharacterController3DConfig* configKcc, KccExtensionConfig* configExt, JumpState jumpState_,
                        bool ignoreGrounded) {
    (void)frame;
    if (!pKcc->Grounded() && !ignoreGrounded) {
        return;
    }
    if (jumpState_ == JumpState::DoubleJumping) {
        pKcc->Velocity.Y = configExt->DoubleJumpImpulse;
    } else if (jumpState_ == JumpState::Jumping) {
        const FP multiplier = kccExt->baseJumpMultiplier > FP{} ? kccExt->baseJumpMultiplier : FP::One();
        pKcc->Velocity.Y = configKcc->BaseJumpImpulse * multiplier;
    }
    pKcc->SetJumped(true);
}

bool KccExtension::UpdateJumpBeforeMove(Frame* f, EntityRef entity, CharacterController3D* pKcc, KccExtension* kccExt,
                                        CharacterController3DConfig* configKcc, KccExtensionConfig* configExt) {
    const bool autoJump = AutoJumpRequested(f, entity, pKcc, configKcc, configExt);

    // Standing on something a jump can start from (with coyote time after leaving it).
    bool onJumpableSurface = grounded;
    if (!onJumpableSurface) {
        WallRunningActor* wallRunner = nullptr;
        GrinderActor* grinder = nullptr;
        if (f->TryGetPointer<WallRunningActor>(entity, wallRunner) && wallRunner->IsWallRunning) {
            onJumpableSurface = true;
        } else if (f->TryGetPointer<GrinderActor>(entity, grinder) && grinder->isGrinding) {
            onJumpableSurface = true;
        } else {
            onJumpableSurface = airTime <= configExt->CoyoteTime;
        }
    }

    if (!jumpRequested && !autoJump) {
        if (onJumpableSurface) {
            if (jumpState != JumpState::None) {
                jumpState = JumpState::None;
                return true;
            }
            return false;
        }
        // Falling off counts as the first jump.
        if (jumpState == JumpState::None) {
            jumpState = JumpState::Jumping;
        }
        return false;
    }

    jumpRequested = false;
    bool canJump = false;
    if (jumpState == JumpState::None) {
        if (groundedTimeSinceJump < configExt->JumpRequiredGroundedTime) {
            return false;
        }
        canJump = PlayerActorInputActionLib::IsInputActionAllowed(f, entity, PlayerActorInputAction::Jump);
    } else if (jumpState == JumpState::Jumping) {
        canJump = UnlockedFeaturesLib::IsFeatureUnlocked(f, entity, UnlockedFeature::Gameplay_DoubleJump, true);
    }
    if (!canJump) {
        return false;
    }
    // Note: the original tests the request flag here, after clearing it above, so a manual
    // jump is never recorded at this point.
    if (jumpRequested) {
        manualJumped = true;
    }
    if (jumpState == JumpState::Jumping) {
        jumpState = JumpState::DoubleJumping;
    } else if (jumpState == JumpState::None) {
        jumpState = JumpState::Jumping;
        groundedTimeSinceJump = FP{};
    }
    Jump(f, pKcc, kccExt, configKcc, configExt, jumpState, true);
    if (pKcc->Jumped()) {
        BattleStatsActor* stats = nullptr;
        if (f->TryGetPointer<BattleStatsActor>(entity, stats)) {
            if (jumpState == JumpState::DoubleJumping) {
                ++stats->numDoubleJumps;
            } else if (jumpState == JumpState::Jumping) {
                ++stats->numJumps;
            }
        }
    }
    return true;
}

bool KccExtension::AutoJumpRequested(Frame* f, EntityRef entity, CharacterController3D* pKcc,
                                     CharacterController3DConfig* configKcc, KccExtensionConfig* configExt) {
    if (!pKcc->Grounded() || manualJumped) {
        return false;
    }
    PlayerActor* actor = nullptr;
    if (!f->TryGetPointer<PlayerActor>(entity, actor) || !actor->get_IsBot()) {
        return false;
    }
    if (!f->GetPointer<AINavigation>(entity)->isOnJumpLink) {
        return false;
    }

    // Cool-downs after a manual jump and after the last automatic one.
    const FP dt = f->get_DeltaTime();
    if (autoJumpDownTimeAfterManualJumpCounter > dt) {
        autoJumpDownTimeAfterManualJumpCounter = autoJumpDownTimeAfterManualJumpCounter - dt;
        return false;
    }
    autoJumpDownTimeAfterManualJumpCounter = FP{};
    if (autoJumpDownTimeCounter > dt) {
        autoJumpDownTimeCounter = autoJumpDownTimeCounter - dt;
        return false;
    }
    autoJumpDownTimeCounter = FP{};

    // Only bots get here, so the human-only checks below never apply; they are kept as in
    // the original.
    const bool isHuman = actor->get_IsHuman();
    const bool airborneNotAllowed = !pKcc->Grounded() && !configExt->CanAutoJumpWhileAirborne;
    bool beingPushed = false;
    bool standingStill = false;
    if (isHuman) {
        const FP threshold = FP::FromRaw(0x1999);
        beingPushed = pushVelocity.SqrMagnitude() > threshold;
        standingStill = pKcc->Velocity.SqrMagnitude() < threshold;
    }
    Cheats cheats{};
    if (f->TryGet<Cheats>(entity, cheats) && cheats.noClipEnabled) {
        return false;
    }
    if (airborneNotAllowed || beingPushed || standingStill) {
        return false;
    }

    Transform3D* transform = f->GetPointer<Transform3D>(entity);
    FPVector3 direction;
    if (isHuman) {
        direction = Horizontal(pKcc->Velocity) * configExt->AutoJumpVelocityFactor;
        if (FPVector3::Angle(direction, transform->Forward()) >= configExt->AutoJumpMaxAngle) {
            return false;
        }
    } else {
        ActorInput* input = nullptr;
        if (f->TryGetPointer<ActorInput>(entity, input)) {
            direction = FPVector3(input->Direction.X, FP{}, input->Direction.Y);
        }
    }

    // Ground below the point ahead...
    const FPVector3 position = transform->Position;
    HitCollection3D* hits = f->GetPersistentHitCollection(0);
    hits->Reset();
    const FPVector3 probe = position + direction + FPVector3::Up() * configExt->AutoJumpHeightCheckStart;
    RaycastNearest(f, hits, probe, FPVector3::Down(), configExt->AutoJumpHeightCheckDistance, kAutoJumpLayers,
                   kStaticsAndKinematics);
    if (hits->Count() < 1) {
        return false;
    }
    const Hit3D groundAhead = (*hits)[0];

    // ...and an obstacle in the way.
    hits->Reset();
    RaycastNearest(f, hits, position + FPVector3::Up() * FP::FromRaw(0x3333), direction, direction.Magnitude(),
                   kAutoJumpLayers, kStaticsAndKinematics);
    if (hits->Count() < 1) {
        return false;
    }
    if (isHuman) {
        const Hit3D obstacle = (*hits)[0];
        if (FPVector3::Angle(obstacle.Normal, -direction) > configKcc->MaxSlope) {
            return false;
        }
    }
    StaticColliderConfig* colliderConfig = GetHitStaticColliderConfig(f, groundAhead);
    if (colliderConfig != nullptr && colliderConfig->IgnoreAutoJump) {
        return false;
    }
    autoJumpDownTimeCounter = configExt->AutoJumpDownTime;
    return true;
}

// -----------------------------------------------------------------------------------------
// Steering

void KccExtension::ComputeRawSteer(Frame* f, EntityRef entity, CharacterController3D* pKcc, Transform3D* pTransform,
                                   CharacterController3DConfig* configKcc, KccExtensionConfig* configExt,
                                   CharacterController3DMovement& movementPack, FP dt) {
    (void)pTransform;
    pKcc->SetGrounded(movementPack.Grounded);
    FPVector3& kccVelocity = pKcc->Velocity;

    // Bouncing off what was hit.
    if (configExt->CanBounce && movementPack.Contacts >= 1 &&
        kccVelocity.SqrMagnitude() > configExt->BounceMinSpeed * configExt->BounceMinSpeed &&
        FPVector3::Dot(kccVelocity, movementPack.AvgNormal) < FP{}) {
        const FPVector3 reflected = FPVector3::Reflect(kccVelocity, movementPack.AvgNormal).Normalized();
        kccVelocity = reflected * kccVelocity.Magnitude() * configExt->BounceRestitution;
        return;
    }

    const bool impulsing = impulseTimer > FP{};
    const FP maxVerticalSpeed = impulseTimer.RawValue() >= 1 ? impulseMaxVerticalSpeed : FP::FromInt(100);
    const FP accelerationModifier = ComputeModifiers(f, get_accelerationModifiers());
    const FP brakingModifier = ComputeModifiers(f, get_brakingModifiers());
    const FP acceleration = impulsing ? configExt->Impulse_Acceleration : configKcc->Acceleration;
    const FP braking = impulsing ? configExt->Impulse_Braking : configKcc->Braking;
    FP maxSpeed = impulsing ? impulseMaxSpeed : pKcc->MaxSpeed;
    FP minVerticalSpeed = FP::FromInt(-100);

    // Fly cheat: free movement along the camera.
    Cheats cheats{};
    PlayerActor* actor = nullptr;
    ActorInput* actorInput = nullptr;
    if (f->TryGet<Cheats>(entity, cheats) && cheats.noClipEnabled && f->TryGetPointer<PlayerActor>(entity, actor) &&
        f->TryGetPointer<ActorInput>(entity, actorInput)) {
        // Note: bots have no player input; the original would fail on it.
        const ::Quantum::Input* input = actor->GetInput(f);
        const FPVector2 inputDirection = input != nullptr ? input->get_Direction() : FPVector2{};
        const FPVector2 cameraFlat = FPVector2(actorInput->CameraSight.X, actorInput->CameraSight.Z).Normalized();
        const FP forwardAmount = FPVector2::Dot(cameraFlat, inputDirection);
        const FPVector2 inputLeft = FPVector2::CalculateLeft(inputDirection).Normalized();
        const FP sideAmount = FPVector2::Dot(cameraFlat, inputLeft);
        const FPVector2 cameraLeft = FPVector2::CalculateLeft(cameraFlat).Normalized();
        const FPVector3 cameraLeft3 = FPVector3(cameraLeft.X, FP{}, cameraLeft.Y);
        const FPVector3 flight = cameraLeft3 * -sideAmount + actorInput->CameraSight * forwardAmount;
        kccVelocity = flight * acceleration * FP::FromInt(10) * dt; // FP x 10 is exact either way
        impulseTimer = FP{};
        pushVelocity = FPVector3::Zero();
        maxSpeed = FP::FromInt(100);
    } else {
        switch (movementPack.Type) {
        case CharacterMovementType::None: {
            if (pushVelocity.Y < FP{}) {
                pushVelocity.Y = FP{};
            }
            const FP decay = braking * brakingModifier * dt;
            const auto brakeTowardsZero = [&](FP value) {
                const FP braked = FPMath::Lerp(value, FP{}, decay);
                return FPMath::Abs(braked) < FP::FromRaw(0x1999) ? FP{} : braked;
            };
            if (kccVelocity.X != FP{} && !Impulsed) {
                kccVelocity.X = brakeTowardsZero(kccVelocity.X);
            }
            if (kccVelocity.Z != FP{} && !Impulsed) {
                kccVelocity.Z = brakeTowardsZero(kccVelocity.Z);
            }
            if (kccVelocity.Y != FP{} && !pKcc->Jumped()) {
                kccVelocity.Y = brakeTowardsZero(kccVelocity.Y);
            }
            minVerticalSpeed = FP{};
            break;
        }
        case CharacterMovementType::FreeFall: {
            const FP airBraking = impulsing ? configExt->Impulse_Braking : configExt->Airborne_Braking;
            const FP airAcceleration = impulsing ? configExt->Impulse_Acceleration : configExt->Airborne_Acceleration;
            const FP gravityModifier = ComputeModifiers(f, get_gravityModifiers());
            // The original keeps the product unshifted until it is used.
            const FP::Raw gravityStepRaw = gravityModifier.RawValue() * (configKcc->GravityStrength() * dt).RawValue();
            const FP gravityStep = FP::FromRaw(gravityStepRaw >> 16);
            const FP maxFallModifier = ComputeModifiers(f, get_maxFallSpeedModifiers());
            const FP maxFallSpeed = impulsing ? impulseMaxVerticalSpeed : configExt->maxFallSpeed * maxFallModifier;

            // Upward pushes absorb gravity first.
            bool applyGravity = true;
            if (pushVelocity.Y.RawValue() >= 1) {
                const FP remaining = pushVelocity.Y - gravityStep;
                pushVelocity.Y = FPMath::Max(remaining, FP{});
                applyGravity = remaining <= FP{};
            }
            if (applyGravity) {
                kccVelocity.Y = kccVelocity.Y - gravityStep;
                if (pushVelocity.Y == FP{} && kccVelocity.Y < -maxFallSpeed) {
                    kccVelocity.Y = -maxFallSpeed;
                }
            }

            const FP steer = airAcceleration * accelerationModifier;
            if (Impulsed || (configKcc->AirControl && !IsZero(movementPack.Tangent))) {
                kccVelocity = kccVelocity + movementPack.Tangent * steer * dt;
            } else {
                const FP decay = airBraking * brakingModifier * dt;
                kccVelocity.X = FPMath::Lerp(kccVelocity.X, FP{}, decay);
                kccVelocity.Z = FPMath::Lerp(kccVelocity.Z, FP{}, decay);
            }
            break;
        }
        case CharacterMovementType::SlopeFall:
            kccVelocity = kccVelocity + movementPack.SlopeTangent * acceleration * dt;
            minVerticalSpeed = -configKcc->MaxSlopeSpeed;
            break;
        case CharacterMovementType::Horizontal: {
            if (pushVelocity.Y < FP{}) {
                pushVelocity.Y = FP{};
            }
            const FPVector3& tangent = movementPack.Tangent;
            kccVelocity = kccVelocity + tangent * (acceleration * accelerationModifier) * dt;
            const FP decay = braking * brakingModifier * dt;
            const FP speed = FPVector3::Dot(kccVelocity, tangent);
            if (!Impulsed) {
                kccVelocity.X = FPMath::Lerp(kccVelocity.X, tangent.X * speed, decay);
                kccVelocity.Z = FPMath::Lerp(kccVelocity.Z, tangent.Z * speed, decay);
            }
            if (!pKcc->Jumped()) {
                kccVelocity.Y = FPMath::Lerp(kccVelocity.Y, tangent.Y * speed, decay);
            }
            const FP excess = FPMath::Abs(speed) - maxSpeed;
            if (excess > FP{}) {
                kccVelocity = kccVelocity - tangent * FPMath::Sign(speed) * excess;
            }
            break;
        }
        default:
            break;
        }
    }

    // Speed limits (on the ground the horizontal steering above already limits it).
    if (movementPack.Type != CharacterMovementType::Horizontal) {
        FPVector2 horizontal(kccVelocity.X, kccVelocity.Z);
        if (horizontal.SqrMagnitude() > maxSpeed * maxSpeed) {
            horizontal = horizontal.Normalized() * maxSpeed;
        }
        kccVelocity.X = horizontal.X;
        kccVelocity.Z = horizontal.Y;
        kccVelocity.Y = FPMath::Clamp(kccVelocity.Y, minVerticalSpeed, maxVerticalSpeed);
    }
    pKcc->SetJumped(false);
    Impulsed = false;
}

// -----------------------------------------------------------------------------------------
// Rebounds

FP KccExtension::CalculateReboundBonus(Frame* f, EntityRef entity, Transform3D* transform,
                                       CharacterController3DConfig* configKcc) {
    HitCollection3D* hits = f->GetPersistentHitCollection(0);
    const Shape3D sphere = Shape3D::CreateSphere(configKcc->Radius + configKcc->Extent);
    GetPhysicsEngine3D(f)->OverlapShape(
        f, hits, transform->Position + configKcc->Offset, FPQuaternion::Identity(), sphere, configKcc->LayerMask,
        static_cast<std::int32_t>(QueryOptions::HitStatics | QueryOptions::ComputeDetailedInfo));
    if (hits->Count() < 1) {
        return FP{};
    }
    FP bonus;
    // The contact limit only decides whether any hit counts; then all of them are summed.
    if (std::min(configKcc->MaxContacts, hits->Count()) >= 1) {
        for (std::int32_t i = 0; i < hits->Count(); ++i) {
            const Hit3D& hit = hits->HitsBuffer()[i];
            if (GetHitEntity(hit) == entity) {
                continue;
            }
            if (StaticColliderConfig* config = GetHitStaticColliderConfig(f, hit)) {
                bonus = config->ReboundBonus + bonus;
            }
        }
    }
    return bonus;
}

void KccExtension::ComputeRebound(Frame* f, EntityRef entity, KccExtensionConfig* configExt, FPVector3 avgNormal,
                                  Transform3D* pTransform, CharacterController3DConfig* configKcc) {
    if (HasModifier_MaxSpeed(f, MOD_ORIGIN::REBOUND) || IsZero(pushVelocity)) {
        return;
    }
    Pushable* pushable = nullptr;
    if (!f->TryGetPointer<Pushable>(entity, pushable) || pushable->pushedBy.frame == 0) {
        return;
    }
    if (f->get_DeltaTime() * (f->Number - pushable->pushedBy.frame) >= configExt->ReboundAfterPushMaxDelay) {
        return;
    }
    if (pushVelocity.SqrMagnitude() < configExt->ReboundPushThreshold * configExt->ReboundPushThreshold) {
        return;
    }
    if (FPVector3::Dot(pushVelocity.Normalized(), avgNormal.Normalized()) > FP::FromRaw(-0x1999)) {
        return;
    }
    const FP bonus = CalculateReboundBonus(f, entity, pTransform, configKcc);
    const FPVector3 rebound = FPVector3::Reflect(pushVelocity, avgNormal) * (configExt->PushReboundFactor + bonus);
    pushVelocity = FPVector3::Zero();
    PhysicsAbstraction physics = PhysicsAbstraction::Create(f, entity);
    physics.AddPush(f, rebound, FPVector3::Zero(), pTransform, EntityRef{});
}

// -----------------------------------------------------------------------------------------
// Ledges and walls

void KccExtension::UpdateLedgeSlow(Frame* f, EntityRef entity, CharacterController3D* pKcc,
                                   KccExtensionConfig* configExt) {
    const FP currentSlow = ComputeModifiersOfType_Additive(f, get_maxSpeedModifiers(), MOD_ORIGIN::LEDGESLOW);
    const auto clearSlow = [&] {
        if (currentSlow != FP{}) {
            ClearModifiersOfTypeInList(f, get_maxSpeedModifiers(), MOD_ORIGIN::LEDGESLOW);
        }
    };
    const bool walking = configExt->LedgeSlowEnable && pKcc->Grounded() &&
                         pushVelocity.SqrMagnitude() <= FP::FromRaw(0x1999) &&
                         pKcc->Velocity.SqrMagnitude() > FP::FromRaw(0x1998);
    if (!walking) {
        clearSlow();
        return;
    }
    Transform3D* transform = f->GetPointer<Transform3D>(entity);
    const FPVector3 ahead = Horizontal(pKcc->Velocity) * configExt->LedgeSlowVelocityFactor;
    if (FPVector3::Angle(ahead, -transform->Forward()) >= configExt->LedgeSlowMaxAngle) {
        clearSlow();
        return;
    }
    // Probed one frame in four.
    if (Modulo(entity.Index, 4) != Modulo(f->Number, 4)) {
        return;
    }

    const FPVector3 position = transform->Position;
    const FPVector3 lift = FPVector3::Up() * FP::FromRaw(0x1999);
    HitCollection3D* hits = f->GetPersistentHitCollection(0);
    hits->Reset();
    RaycastNearest(f, hits, position + ahead + lift, FPVector3::Down(), configExt->LedgeSlowHeightThreshold,
                   kLedgeSlowLayers, kStaticsOnly);
    const bool groundAhead = hits->Count() >= 1;
    hits->Reset();
    RaycastNearest(f, hits, position + lift, ahead, ahead.Magnitude(), kLedgeSlowLayers, kStaticsOnly);
    const bool wallAhead = hits->Count() >= 1;
    if (groundAhead || wallAhead) {
        clearSlow();
        return;
    }
    // A drop ahead: slow down a bit more each frame, down to the limit.
    const FP room = configExt->LedgeSlowMaxSpeedReductionLimit - currentSlow;
    if (room > FP{}) {
        return;
    }
    AddModifier_MaxSpeed(f, MOD_ORIGIN::LEDGESLOW,
                         FPMath::Max(f->get_DeltaTime() * configExt->LedgeSlowMaxSpeedReductionAmount, room),
                         FP::FromRaw(-65536), FP{}, false);
}

namespace {

// What UpdateLedgeGrab's direction probes share (the original's closure).
struct LedgeProbe {
    Frame* f = nullptr;
    FPVector3 origin; // the character's position raised by the spec's height offset
    LedgeGrabbingSpec* spec = nullptr;
    FPVector3 inputDirection;
    Transform3D* transform = nullptr;
    EntityRef entity;
};

// 0x7c54fac (<UpdateLedgeGrab>g__EvaluateDirection) - Looks for a ledge in one direction:
// a wall ahead (or the ground below the point ahead) with a walkable normal, no ground
// close below the character, a thick enough ledge and room above. Signals the ledge when
// found. True stops the search (a ledge was found or the character is too close to the
// ground).
bool EvaluateLedgeDirection(HitCollection3D* hits, const FPVector3& rayDir, const LedgeProbe& probe) {
    Frame* f = probe.f;
    const LedgeGrabbingSpec* spec = probe.spec;
    const std::int32_t layers = spec->LayerMask;
    hits->Reset();
    const FPVector3 ahead = probe.origin + rayDir * spec->ForwardDistance;
    LinecastNearest(f, hits, probe.origin, ahead, layers, kSolidDetailed);
    if (hits->Count() < 1) {
        hits->Reset();
        LinecastNearest(f, hits, ahead, ahead + FPVector3::Down() * spec->DownwardDistance, layers, kSolidDetailed);
        if (hits->Count() != 1) {
            return false;
        }
    }
    const Hit3D ledge = (*hits)[0];
    if (FPVector3::Angle(FPVector3::Up(), ledge.Normal) > spec->LedgeHitNormalMaxAngle) {
        return false;
    }

    // Not when the ground is close below the character.
    const FP minHeight = FPVector3::Dot(probe.inputDirection, rayDir).RawValue() >= 1 ? spec->VoidMinHeight
                                                                                      : spec->LedgeMinHeight;
    const FPVector3 position = probe.transform->Position;
    hits->Reset();
    const FPVector3 voidStart(position.X, position.Y + spec->VoidTestOffsetY, position.Z);
    LinecastNearest(f, hits, voidStart, voidStart + FPVector3::Down() * (spec->VoidTestOffsetY + minHeight), layers,
                    kStaticsAndKinematics);
    if (hits->Count() >= 1) {
        return true;
    }

    // The ledge must be thick enough: a wall just below its top.
    hits->Reset();
    const FPVector3 thicknessStart(position.X, ledge.Point.Y - spec->LedgeMinThickness, position.Z);
    LinecastNearest(f, hits, thicknessStart, thicknessStart + rayDir * spec->ForwardDistance, layers, kSolidDetailed);
    if (hits->Count() < 1) {
        return false;
    }
    const Hit3D wall = (*hits)[0];

    // And nothing above the character.
    hits->Reset();
    LinecastNearest(f, hits, position, position + FPVector3::Up() * FP::FromInt(2), layers, kStaticsOnly);
    if (hits->Count() >= 1) {
        return false;
    }
    f->Signals.KccExtension_OnValidLedgeDetected(probe.entity, wall, ledge.Point);
    return true;
}

} // namespace

void KccExtension::UpdateLedgeGrab(Frame* f, EntityRef entity, CharacterController3D* pKcc, Transform3D* pTransform,
                                   FPVector3 inputDirection) {
    LedgeProbe probe;
    probe.f = f;
    probe.inputDirection = inputDirection;
    probe.transform = pTransform;
    probe.entity = entity;

    ParabolaFollower* parabola = nullptr;
    if (f->TryGetPointer<ParabolaFollower>(entity, parabola)) {
        return;
    }
    LedgeGrabbingActor* ledgeActor = nullptr;
    if (!f->TryGetPointer<LedgeGrabbingActor>(entity, ledgeActor) || ledgeActor->IsLedgeGrabbing) {
        return;
    }
    if (pKcc->Grounded() || pushVelocity.SqrMagnitude() > FP::FromRaw(0x1999)) {
        return;
    }
    if (!f->TryFindAssetInternal<LedgeGrabbingSpec>(ledgeActor->Spec.Id, true, probe.spec)) {
        return;
    }
    ActorStatus status{};
    if (f->TryGet<ActorStatus>(entity, status)) {
        const auto& allowed = probe.spec->AuthorizedActorStatusTypes;
        if (std::find(allowed.begin(), allowed.end(), status.currentType) == allowed.end()) {
            return;
        }
    }
    // Bots on a navigation jump link probe every frame; everyone else on a schedule.
    bool botOnJumpLink = false;
    PlayerActor* actor = nullptr;
    AINavigation* navigation = nullptr;
    if (f->TryGetPointer<PlayerActor>(entity, actor) && actor->get_IsBot() &&
        f->TryGetPointer<AINavigation>(entity, navigation)) {
        if (!navigation->isOnJumpLink) {
            return;
        }
        botOnJumpLink = true;
    }
    if (!botOnJumpLink && Modulo(entity.Index, probe.spec->SchedulePeriod) != Modulo(f->Number, probe.spec->SchedulePeriod)) {
        return;
    }

    probe.origin = pTransform->Position;
    probe.origin.Y = probe.origin.Y + probe.spec->HeightOffset;
    const FPVector3 forward = pTransform->Forward();
    HitCollection3D* hits = f->GetPersistentHitCollection(0);
    if (EvaluateLedgeDirection(hits, forward, probe)) {
        return;
    }
    if (probe.spec->ForwardMaxAngle.RawValue() < 1) {
        return;
    }
    const FPVector3 left = FPQuaternion::Euler(FPVector3::Up() * -probe.spec->ForwardMaxAngle) * forward;
    if (EvaluateLedgeDirection(hits, left, probe)) {
        return;
    }
    const FPVector3 right = FPQuaternion::Euler(FPVector3::Up() * probe.spec->ForwardMaxAngle) * forward;
    EvaluateLedgeDirection(hits, right, probe);
}

void KccExtension::UpdateWallRunAutoJump(Frame* frame, EntityRef entity, bool isHuman, CharacterController3D* pKcc,
                                         KccExtension* kccExt, Transform3D* pTransform,
                                         CharacterController3DConfig* configKcc, KccExtensionConfig* configExt) {
    (void)configKcc;
    if (!isHuman || !configExt->AutoJumpOnWallRun || !pKcc->Grounded()) {
        return;
    }
    if (!CheckFrameSlicing(frame, entity, configExt->AutoJumpOnWallRunFrameSlicing)) {
        return;
    }
    const FP horizontalSpeedSqr = pKcc->Velocity.Z * pKcc->Velocity.Z + pKcc->Velocity.X * pKcc->Velocity.X;
    if (horizontalSpeedSqr < FP::FromRaw(0x4000)) {
        return;
    }
    if (!IsMovingForward(pKcc, pTransform, configExt)) {
        return;
    }
    WallRunningActor wallRunner{};
    if (frame->TryGet<WallRunningActor>(entity, wallRunner) && !wallRunner.CanWallRun) {
        return;
    }
    HitCollection3D* hits = frame->GetPersistentHitCollection(0);
    hits->Reset();
    const FPVector3 origin = pTransform->Position + FPVector3::Up() * (configExt->AutoJumpOnWallRunHeight + FP::One());
    RaycastNearest(frame, hits, origin, pTransform->Forward(), configExt->AutoJumpOnWallRunDistance, 1,
                   static_cast<std::int32_t>(QueryOptions::HitAll));
    if (hits->Count() < 1) {
        return;
    }
    const EntityRef wall = GetHitEntity((*hits)[0]);
    RunnableWallTriggerZone zone{};
    if (!frame->TryGet<RunnableWallTriggerZone>(wall, zone)) {
        return;
    }
    const Transform3D wallTransform = frame->Get<Transform3D>(wall);
    const FP facing = FPVector3::Dot(wallTransform.Forward(), pTransform->Forward());
    if (FPMath::Abs(facing) > configExt->AutoJumpOnWallRunFacingThreshold) {
        kccExt->jumpRequested = true;
    }
}

// -----------------------------------------------------------------------------------------
// The step

void KccExtension::Move(Frame* f, EntityRef entity, Transform3D* pTransform, CharacterController3D* pKcc,
                        KccExtension* kccExt, FPVector3 direction, CharacterController3DConfig* configKcc,
                        KccExtensionConfig* configExt, IKCCCallbacks3D* callback) {
    const FP dt = f->get_DeltaTime();
    PlayerActor* actor = nullptr;
    const bool isHuman = f->TryGetPointer<PlayerActor>(entity, actor) && !actor->get_IsBot();
    const bool jumpedBeforeMove = pKcc->Jumped();

    UpdateLedgeGrab(f, entity, pKcc, pTransform, direction);
    UpdateWallRunAutoJump(f, entity, isHuman, pKcc, kccExt, pTransform, configKcc, configExt);
    if (isHuman) {
        UpdateLedgeSlow(f, entity, pKcc, configExt);
    }

    // Max speed: config x input strength x modifiers.
    maxSpeedModifier = ComputeModifiers(f, get_maxSpeedModifiers());
    FP inputFactor = FP::One();
    if (configExt->limitVelocityBasedOnInput && !IsZero(direction)) {
        inputFactor = FPMath::Max(direction.Magnitude(), configExt->limitVelocityMinValue);
    }
    pKcc->MaxSpeed = configKcc->MaxSpeed * inputFactor * maxSpeedModifier;

    // Push velocity decays horizontally; it dies once too weak.
    const FPVector3 pushDirection = pushVelocity.Normalized();
    const FPVector3 reduction(pushDirection.X * configExt->PushReductionSpeed, FP{},
                              pushDirection.Z * configExt->PushReductionSpeed);
    const FPVector3 deceleration = -reduction * dt;
    if (deceleration.SqrMagnitude() > pushVelocity.SqrMagnitude() ||
        pushVelocity.Magnitude() < configExt->PushKillThreshold) {
        pushVelocity = FPVector3::Zero();
    } else {
        pushVelocity = pushVelocity + deceleration;
        if (pushVelocity.Y < FP{}) {
            if (pushVelocity.Y <= -configExt->PushKillThreshold) {
                const FP fall = pushVelocity.Y + -pushDirection.Y * configExt->PushReductionSpeed * dt;
                pushVelocity.Y = FPMath::Min(fall, FP{});
            } else {
                pushVelocity.Y = FP{};
            }
        }
    }

    // The contacts are computed with the push velocity included.
    pKcc->Velocity = pKcc->Velocity + pushVelocity;
    CharacterController3DMovementExt movement{};
    MovementPackCache* cache = nullptr;
    f->TryGetPointer<MovementPackCache>(entity, cache);
    bool computeMovement = true;
    if (cache != nullptr) {
        // While pushed the pack is recomputed every frame.
        const std::int32_t frequency = IsZero(pushVelocity) ? cache->updateFrequency : 1;
        computeMovement = Modulo(entity.Index, frequency) == Modulo(f->Number, frequency);
    }
    if (computeMovement) {
        movement = CharacterController3DLib::ComputeRawMovement(f, entity, pTransform, pKcc, configKcc, direction, callback);
        if (cache != nullptr) {
            cache->tangent = movement.baseMovementPack.Tangent;
            cache->slopeTangent = movement.baseMovementPack.SlopeTangent;
        }
    } else {
        // Between updates: walking on flat ground along the cached tangents.
        CharacterController3DMovement& pack = movement.baseMovementPack;
        pack.Type = CharacterMovementType::Horizontal;
        pack.NearestNormal = FPVector3::Up();
        pack.AvgNormal = FPVector3::Up();
        pack.GroundNormal = FPVector3::Up();
        pack.Tangent = cache->tangent;
        pack.SlopeTangent = cache->slopeTangent;
        pack.Penetration = FP{};
        pack.Contacts = 0;
        pack.Correction = FPVector3::Zero();
        pack.Grounded = true;
    }
    pKcc->Velocity = pKcc->Velocity - pushVelocity;
    f->Signals.KccExtension_OnMovementPackComputed(entity, &movement);
    CharacterController3DMovement& pack = movement.baseMovementPack;
    ComputeRawSteer(f, entity, pKcc, pTransform, configKcc, configExt, pack, dt);

    moveSpeedModifier = ignoreMoveSpeedModifier ? FP::One() : ComputeModifiers(f, get_speedModifiers());
    velocity = pKcc->Velocity * moveSpeedModifier;

    // A push replaces the controller's velocity along its direction (capped at the max
    // speed while the push is slower than it).
    const FP pushSpeed = pushVelocity.Magnitude();
    if (pushSpeed.RawValue() >= 1) {
        const FPVector3 alongPush = FPVector3::Project(pKcc->Velocity, pushDirection);
        if (alongPush.SqrMagnitude().RawValue() >= 1) {
            if (pushSpeed > pKcc->MaxSpeed) {
                velocity = velocity - alongPush;
            } else {
                velocity = velocity - (alongPush + pushVelocity) + pushDirection * pKcc->MaxSpeed;
            }
        }
    }

    FPVector3 displacement = pushVelocity * dt + velocity * dt;
    if (pack.Penetration.RawValue() >= 0x42) {
        CharacterController3DConfig* config = f->FindAsset<CharacterController3DConfig>(pKcc->Config().Id);
        FPVector3 correction = pack.Correction;
        if (FPVector3::Angle(FPVector3::Down(), correction) <= config->MaxSlope && pack.Grounded) {
            correction.Y = FPMath::Max(correction.Y, FP{});
        }
        if (pack.Penetration <= config->MaxPenetration) {
            correction = correction * config->PenetrationCorrection;
        }
        displacement = displacement + correction;
        ComputeRebound(f, entity, configExt, pack.AvgNormal, pTransform, configKcc);
    }

    const FPVector3 previousPosition = pTransform->Position;
    pTransform->Position = previousPosition + displacement;

    // Fast moves are checked against walls they could have skipped through.
    if (displacement.SqrMagnitude() >= configExt->GoThroughWallCheckThreshold) {
        const FPVector3 offset = pTransform->Rotation * configKcc->Offset;
        const FPVector3 moveDirection = displacement.Normalized();
        HitCollection3D* hits = f->GetPersistentHitCollection(0);
        hits->Reset();
        const FPVector3 start = previousPosition + offset;
        const FPVector3 end = pTransform->Position + offset + moveDirection * configKcc->Radius;
        LinecastNearest(f, hits, start, end, configExt->WallCheckLayerMask, kAllButTriggers);
        if (hits->Count() >= 1) {
            const Hit3D wall = (*hits)[0];
            Collections::QList<EntityRef> exclusions = f->ResolveList(get_wallCheckExclusions());
            if (!exclusions.Contains(GetHitEntity(wall))) {
                const FPVector3 back = (end - start).Normalized();
                pTransform->Position = wall.Point - (offset + back * configKcc->Radius);
                pKcc->Velocity = FPVector3::Zero();
                if (impulseTimer.RawValue() >= 1) {
                    hasImpulseToConsume = false;
                    impulseTimer = FP{};
                    impulse = FPVector3::Zero();
                }
            }
        }
    }

    groundNormal = pack.GroundNormal;
    if (pKcc->Grounded()) {
        Bot* bot = nullptr;
        if (f->TryGetPointer<Bot>(entity, bot)) {
            bot->lastGroundedPosition = pTransform->Position;
        }
    }

    if (jumpedBeforeMove || pushVelocity.Y.RawValue() >= 1) {
        grounded = false;
    } else {
        const bool walkable = pack.Type == CharacterMovementType::Horizontal || pack.Type == CharacterMovementType::None;
        const bool nowGrounded = walkable && pack.Grounded;
        if (!grounded && pack.Grounded) {
            // Landing.
            if (manualJumped) {
                autoJumpDownTimeAfterManualJumpCounter = configExt->AutoJumpDownTimeAfterManualJump;
            }
            manualJumped = false;
        }
        grounded = nowGrounded;

        // A forced input that snaps to the ground keeps the character on it.
        if (forceInputCounter.RawValue() >= 1 && !grounded && forceInputSnapOnGround) {
            HitCollection3D* hits = f->GetPersistentHitCollection(0);
            const FPVector3 start = pTransform->Position + FPVector3::Up() * FP::One();
            LinecastNearest(f, hits, start, start + FPVector3::Down() * FP::FromRaw(163840), kGroundLayers, kStaticsOnly);
            if (hits->Count() == 1) {
                pTransform->Position = (*hits)[0].Point;
                grounded = true;
            }
        }
        // Close enough to the ground counts as grounded.
        if (!grounded) {
            HitCollection3D* hits = f->GetPersistentHitCollection(0);
            hits->Reset();
            const FPVector3 start = pTransform->Position + FPVector3::Up() * FP::FromRaw(16384);
            LinecastNearest(f, hits, start, start + FPVector3::Down() * FP::FromRaw(32768), kGroundLayers, kStaticsOnly);
            if (hits->Count() == 1 && ((*hits)[0].Point - pTransform->Position).Magnitude() <= FP::FromRaw(0x2e11)) {
                grounded = true;
                if (manualJumped) {
                    autoJumpDownTimeAfterManualJumpCounter = configExt->AutoJumpDownTimeAfterManualJump;
                }
                manualJumped = false;
            }
        }
    }

    if (grounded) {
        groundedTime = groundedTime + dt;
        groundedTimeSinceJump = groundedTimeSinceJump + dt;
        lastGroundedPos = pTransform->Position;
        airTime = FP{};
    } else {
        groundedTime = FP{};
        airTime = airTime + dt;
    }
    if (impulseTimer.RawValue() >= 1) {
        impulseTimer = impulseTimer - dt;
    }
}

} // namespace Quantum
