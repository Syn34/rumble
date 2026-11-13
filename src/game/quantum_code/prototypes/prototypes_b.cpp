// prototypes_b.cpp - component prototypes HoverBootsWeapon to ZoneControlGameMode
// (quantum.code.dll).
#include "quantum_code/prototypes/prototypes.h"

#include "quantum_code/prototypes/component_prototype_visitor.h"
#include "quantum_code/prototypes/prototype_support.h"

namespace Quantum::Prototypes {

bool HoverBootsWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HoverBootsWeapon>(*this, f, entity, context);
}

void HoverBootsWeapon_Prototype::Materialize(Frame*, HoverBootsWeapon&,
                                             const PrototypeMaterializationContext&) const {}

void HoverBootsWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ItemSpawner_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ItemSpawner>(*this, f, entity, context);
}

void ItemSpawner_Prototype::Materialize(Frame*, ItemSpawner& result,
                                        const PrototypeMaterializationContext&) const {
    result.spawnedItemId = spawnedItemId;
    result.Config = Config;
}

void ItemSpawner_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool JumpAttack_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<JumpAttack>(*this, f, entity, context);
}

void JumpAttack_Prototype::Materialize(Frame*, JumpAttack& result,
                                       const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void JumpAttack_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool KccExtension_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<KccExtension>(*this, f, entity, context);
}

void KccExtension_Prototype::Materialize(Frame*, KccExtension& result,
                                         const PrototypeMaterializationContext&) const {
    result.Config = Config;
}

void KccExtension_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool KccModifier_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<KccModifier>(*this, f, entity, context);
}

void KccModifier_Prototype::Materialize(Frame*, KccModifier& result,
                                        const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void KccModifier_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool KillZone_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                     const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<KillZone>(*this, f, entity, context);
}

void KillZone_Prototype::Materialize(Frame* frame, KillZone& result,
                                     const PrototypeMaterializationContext& context) const {
    result.verticalThreshold = verticalThreshold;
    if (areas.empty()) {
        result.set_areas({});
    } else {
        auto ptr = result.get_areas();
        auto list = frame->AllocateList(ptr, static_cast<int>(areas.size()));
        for (const auto& item : areas) {
            KillZoneArea value{};
            item.Materialize(frame, value, context);
            list.Add(value);
        }
        result.set_areas(ptr);
    }
}

void KillZone_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Killable_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                     const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Killable>(*this, f, entity, context);
}

void Killable_Prototype::Materialize(Frame* frame, Killable& result,
                                     const PrototypeMaterializationContext& context) const {
    result.deathAnimationSpec = deathAnimationSpec;
    MaterializeUser(frame, result, context);
}

void Killable_Prototype::MaterializeUser(Frame*, Killable& result,
                                         const PrototypeMaterializationContext& context) const {
    result.Prototype = context.get_ComponentPrototypeRef();
}

void Killable_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool LedgeGrabbingActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<LedgeGrabbingActor>(*this, f, entity, context);
}

void LedgeGrabbingActor_Prototype::Materialize(Frame*, LedgeGrabbingActor& result,
                                               const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void LedgeGrabbingActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool LevelButton_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<LevelButton>(*this, f, entity, context);
}

void LevelButton_Prototype::Materialize(Frame*, LevelButton& result,
                                        const PrototypeMaterializationContext& context) const {
    result.Config = Config;
    PrototypeValidator::FindMapEntity(gameplayElementEntity, context, result.gameplayElementEntity);
}

void LevelButton_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool LevelButtonGameplayElement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<LevelButtonGameplayElement>(*this, f, entity, context);
}

void LevelButtonGameplayElement_Prototype::Materialize(
    Frame*, LevelButtonGameplayElement& result, const PrototypeMaterializationContext&) const {
    result.Config = Config;
}

void LevelButtonGameplayElement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool LevelHolder_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<LevelHolder>(*this, f, entity, context);
}

void LevelHolder_Prototype::Materialize(Frame*, LevelHolder&,
                                        const PrototypeMaterializationContext&) const {}

void LevelHolder_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool LevelUpActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<LevelUpActor>(*this, f, entity, context);
}

void LevelUpActor_Prototype::Materialize(Frame*, LevelUpActor& result,
                                         const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void LevelUpActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Lifetime_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                     const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Lifetime>(*this, f, entity, context);
}

void Lifetime_Prototype::Materialize(Frame*, Lifetime& result,
                                     const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void Lifetime_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MeleeAttack_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MeleeAttack>(*this, f, entity, context);
}

void MeleeAttack_Prototype::Materialize(Frame*, MeleeAttack& result,
                                        const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void MeleeAttack_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MeleeJumpAttack_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MeleeJumpAttack>(*this, f, entity, context);
}

void MeleeJumpAttack_Prototype::Materialize(Frame*, MeleeJumpAttack& result,
                                            const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void MeleeJumpAttack_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MineLauncherEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MineLauncherEnhancement>(*this, f, entity, context);
}

void MineLauncherEnhancement_Prototype::Materialize(Frame*, MineLauncherEnhancement& result,
                                                    const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void MineLauncherEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MineMerger_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MineMerger>(*this, f, entity, context);
}

void MineMerger_Prototype::Materialize(Frame*, MineMerger& result,
                                       const PrototypeMaterializationContext&) const {
    result.mergeCount = mergeCount;
    result.wasMergeChecked = wasMergeChecked;
}

void MineMerger_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MobGroup_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                     const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MobGroup>(*this, f, entity, context);
}

void MobGroup_Prototype::Materialize(Frame*, MobGroup& result,
                                     const PrototypeMaterializationContext&) const {
    result.NumToSpawn = NumToSpawn;
    result.SwarmerConfig = SwarmerConfig;
}

void MobGroup_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MobGroupMember_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                           const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MobGroupMember>(*this, f, entity, context);
}

void MobGroupMember_Prototype::Materialize(Frame*, MobGroupMember&,
                                           const PrototypeMaterializationContext&) const {}

void MobGroupMember_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MobInfo_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                    const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MobInfo>(*this, f, entity, context);
}

void MobInfo_Prototype::Materialize(Frame*, MobInfo& result,
                                    const PrototypeMaterializationContext&) const {
    result.Type = Type;
    result.UseOwnerLevel = UseOwnerLevel;
    result.Spec = Spec;
}

void MobInfo_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MoveTo_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MoveTo>(*this, f, entity, context);
}

void MoveTo_Prototype::Materialize(Frame* frame, MoveTo& result,
                                   const PrototypeMaterializationContext&) const {
    result.moveToSpec = moveToSpec;
    if (onMoveFinished.empty()) {
        result.set_onMoveFinished({});
    } else {
        auto ptr = result.get_onMoveFinished();
        auto list = frame->AllocateList(ptr, static_cast<int>(onMoveFinished.size()));
        for (const auto& item : onMoveFinished) {
            list.Add(item);
        }
        result.set_onMoveFinished(ptr);
    }
}

void MoveTo_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MovementPackCache_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MovementPackCache>(*this, f, entity, context);
}

void MovementPackCache_Prototype::Materialize(Frame*, MovementPackCache&,
                                              const PrototypeMaterializationContext&) const {}

void MovementPackCache_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool MovingWall_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<MovingWall>(*this, f, entity, context);
}

void MovingWall_Prototype::Materialize(Frame*, MovingWall& result,
                                       const PrototypeMaterializationContext&) const {
    result.navmeshRegionID = navmeshRegionID;
    result.Config = Config;
}

void MovingWall_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool OnKillCallbacks_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<OnKillCallbacks>(*this, f, entity, context);
}

void OnKillCallbacks_Prototype::Materialize(Frame* frame, OnKillCallbacks& result,
                                            const PrototypeMaterializationContext&) const {
    if (onPlayerKilled.empty()) {
        result.set_onPlayerKilled({});
    } else {
        auto ptr = result.get_onPlayerKilled();
        auto list = frame->AllocateList(ptr, static_cast<int>(onPlayerKilled.size()));
        for (const auto& item : onPlayerKilled) {
            list.Add(item);
        }
        result.set_onPlayerKilled(ptr);
    }
}

void OnKillCallbacks_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool OnTriggerEnter_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                           const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<OnTriggerEnter>(*this, f, entity, context);
}

void OnTriggerEnter_Prototype::Materialize(Frame* frame, OnTriggerEnter& result,
                                           const PrototypeMaterializationContext& context) const {
    if (onTriggerEnter.empty()) {
        result.set_onTriggerEnter({});
    } else {
        auto ptr = result.get_onTriggerEnter();
        auto list = frame->AllocateList(ptr, static_cast<int>(onTriggerEnter.size()));
        for (const auto& item : onTriggerEnter) {
            list.Add(item);
        }
        result.set_onTriggerEnter(ptr);
    }
    MaterializeUser(frame, result, context);
}

void OnTriggerEnter_Prototype::MaterializeUser(
    Frame*, OnTriggerEnter& result, const PrototypeMaterializationContext& context) const {
    result.Prototype = context.get_ComponentPrototypeRef();
}

void OnTriggerEnter_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool PlayerActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<PlayerActor>(*this, f, entity, context);
}

void PlayerActor_Prototype::Materialize(Frame*, PlayerActor& result,
                                        const PrototypeMaterializationContext&) const {
    result.durationBeforeCombatReset = durationBeforeCombatReset;
    result.maxNoActionTimeAfterPush = maxNoActionTimeAfterPush;
    result.noActionMaxPushForce = noActionMaxPushForce;
    result.noActionPushForceThreshold = noActionPushForceThreshold;
    result.noActionTimeAfterPush = noActionTimeAfterPush;
}

void PlayerActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool PowerUp_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                    const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<PowerUp>(*this, f, entity, context);
}

void PowerUp_Prototype::Materialize(Frame*, PowerUp& result,
                                    const PrototypeMaterializationContext&) const {
    result.IsPickedUp = IsPickedUp;
    result.Spec = Spec;
}

void PowerUp_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool PowerUpUser_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<PowerUpUser>(*this, f, entity, context);
}

void PowerUpUser_Prototype::Materialize(Frame*, PowerUpUser&,
                                        const PrototypeMaterializationContext&) const {}

void PowerUpUser_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool PreciseHitReceiver_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<PreciseHitReceiver>(*this, f, entity, context);
}

void PreciseHitReceiver_Prototype::Materialize(Frame*, PreciseHitReceiver& result,
                                               const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void PreciseHitReceiver_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool PrioritisedTargetHandler_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<PrioritisedTargetHandler>(*this, f, entity, context);
}

void PrioritisedTargetHandler_Prototype::Materialize(Frame*, PrioritisedTargetHandler& result,
                                                     const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void PrioritisedTargetHandler_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Projectile_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Projectile>(*this, f, entity, context);
}

void Projectile_Prototype::Materialize(Frame*, Projectile& result,
                                       const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void Projectile_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ProjectileSpawner_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ProjectileSpawner>(*this, f, entity, context);
}

void ProjectileSpawner_Prototype::Materialize(Frame*, ProjectileSpawner& result,
                                              const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void ProjectileSpawner_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Pushable_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                     const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Pushable>(*this, f, entity, context);
}

void Pushable_Prototype::Materialize(Frame*, Pushable& result,
                                     const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void Pushable_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool PushableTrigger_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<PushableTrigger>(*this, f, entity, context);
}

void PushableTrigger_Prototype::Materialize(Frame*, PushableTrigger&,
                                            const PrototypeMaterializationContext&) const {}

void PushableTrigger_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Pusher_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Pusher>(*this, f, entity, context);
}

void Pusher_Prototype::Materialize(Frame*, Pusher& result,
                                   const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void Pusher_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Rail_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                 const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Rail>(*this, f, entity, context);
}

void Rail_Prototype::Materialize(Frame*, Rail& result,
                                 const PrototypeMaterializationContext&) const {
    result.bakedRailIndex = bakedRailIndex;
}

void Rail_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool RailCollider_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<RailCollider>(*this, f, entity, context);
}

void RailCollider_Prototype::Materialize(Frame*, RailCollider& result,
                                         const PrototypeMaterializationContext& context) const {
    PrototypeValidator::FindMapEntity(nearbyRailColLeft, context, result.nearbyRailColLeft);
    PrototypeValidator::FindMapEntity(nearbyRailColRight, context, result.nearbyRailColRight);
    PrototypeValidator::FindMapEntity(railEntity, context, result.railEntity);
    result.distanceOnRail = distanceOnRail;
}

void RailCollider_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool RankingManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                           const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<RankingManager>(*this, f, entity, context);
}

void RankingManager_Prototype::Materialize(Frame*, RankingManager&,
                                           const PrototypeMaterializationContext&) const {}

void RankingManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool RecoverAmmoOnDashEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<RecoverAmmoOnDashEnhancement>(*this, f, entity, context);
}

void RecoverAmmoOnDashEnhancement_Prototype::Materialize(
    Frame*, RecoverAmmoOnDashEnhancement& result, const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void RecoverAmmoOnDashEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ReplaceMultiShotConfigsEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ReplaceMultiShotConfigsEnhancement>(*this, f, entity, context);
}

void ReplaceMultiShotConfigsEnhancement_Prototype::Materialize(
    Frame*, ReplaceMultiShotConfigsEnhancement&, const PrototypeMaterializationContext&) const {}

void ReplaceMultiShotConfigsEnhancement_Prototype::Dispatch(
    ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool RewardReceiver_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                           const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<RewardReceiver>(*this, f, entity, context);
}

void RewardReceiver_Prototype::Materialize(Frame*, RewardReceiver& result,
                                           const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void RewardReceiver_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Roll_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                 const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Roll>(*this, f, entity, context);
}

void Roll_Prototype::Materialize(Frame*, Roll& result,
                                 const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void Roll_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool RunnableWall_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<RunnableWall>(*this, f, entity, context);
}

void RunnableWall_Prototype::Materialize(Frame*, RunnableWall&,
                                         const PrototypeMaterializationContext&) const {}

void RunnableWall_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool RunnableWallTriggerZone_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<RunnableWallTriggerZone>(*this, f, entity, context);
}

void RunnableWallTriggerZone_Prototype::Materialize(Frame*, RunnableWallTriggerZone&,
                                                    const PrototypeMaterializationContext&) const {}

void RunnableWallTriggerZone_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool RynoWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<RynoWeapon>(*this, f, entity, context);
}

void RynoWeapon_Prototype::Materialize(Frame*, RynoWeapon&,
                                       const PrototypeMaterializationContext&) const {}

void RynoWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ShapeOverlapper_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ShapeOverlapper>(*this, f, entity, context);
}

void ShapeOverlapper_Prototype::Materialize(Frame*, ShapeOverlapper& result,
                                            const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void ShapeOverlapper_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ShieldedActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ShieldedActor>(*this, f, entity, context);
}

void ShieldedActor_Prototype::Materialize(Frame*, ShieldedActor& result,
                                          const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void ShieldedActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ShotgunBerserkerEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ShotgunBerserkerEnhancement>(*this, f, entity, context);
}

void ShotgunBerserkerEnhancement_Prototype::Materialize(
    Frame*, ShotgunBerserkerEnhancement&, const PrototypeMaterializationContext&) const {}

void ShotgunBerserkerEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ShotgunEnhancement_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ShotgunEnhancement>(*this, f, entity, context);
}

void ShotgunEnhancement_Prototype::Materialize(Frame*, ShotgunEnhancement&,
                                               const PrototypeMaterializationContext&) const {}

void ShotgunEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ShrinkingZone_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ShrinkingZone>(*this, f, entity, context);
}

void ShrinkingZone_Prototype::Materialize(Frame*, ShrinkingZone& result,
                                          const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void ShrinkingZone_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ShrinkingZoneActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ShrinkingZoneActor>(*this, f, entity, context);
}

void ShrinkingZoneActor_Prototype::Materialize(Frame*, ShrinkingZoneActor& result,
                                               const PrototypeMaterializationContext&) const {
    result.IgnoreShrinkingZone = IgnoreShrinkingZone;
}

void ShrinkingZoneActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool SpawnProjectilesOnDestroy_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<SpawnProjectilesOnDestroy>(*this, f, entity, context);
}

void SpawnProjectilesOnDestroy_Prototype::Materialize(
    Frame* frame, SpawnProjectilesOnDestroy& result, const PrototypeMaterializationContext&) const {
    if (Specs.empty()) {
        result.set_Specs({});
    } else {
        auto ptr = result.get_Specs();
        auto list = frame->AllocateList(ptr, static_cast<int>(Specs.size()));
        for (const auto& item : Specs) {
            list.Add(item);
        }
        result.set_Specs(ptr);
    }
}

void SpawnProjectilesOnDestroy_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Speeder_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                    const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Speeder>(*this, f, entity, context);
}

void Speeder_Prototype::Materialize(Frame*, Speeder& result,
                                    const PrototypeMaterializationContext&) const {
    result.UsedFrame = UsedFrame;
    result.Team = Team;
    result.Spec = Spec;
    result.Duration = Duration;
    result.SpeedModifier = SpeedModifier;
}

void Speeder_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool StatsManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<StatsManager>(*this, f, entity, context);
}

void StatsManager_Prototype::Materialize(Frame*, StatsManager&,
                                         const PrototypeMaterializationContext&) const {}

void StatsManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool StreamEffect_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<StreamEffect>(*this, f, entity, context);
}

void StreamEffect_Prototype::Materialize(Frame*, StreamEffect& result,
                                         const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void StreamEffect_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool StreamEffectSpawner_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<StreamEffectSpawner>(*this, f, entity, context);
}

void StreamEffectSpawner_Prototype::Materialize(Frame*, StreamEffectSpawner& result,
                                                const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void StreamEffectSpawner_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool SweepAttack_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<SweepAttack>(*this, f, entity, context);
}

void SweepAttack_Prototype::Materialize(Frame*, SweepAttack& result,
                                        const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void SweepAttack_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TargetFinder_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TargetFinder>(*this, f, entity, context);
}

void TargetFinder_Prototype::Materialize(Frame*, TargetFinder& result,
                                         const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void TargetFinder_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TargetHolder_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TargetHolder>(*this, f, entity, context);
}

void TargetHolder_Prototype::Materialize(Frame*, TargetHolder&,
                                         const PrototypeMaterializationContext&) const {}

void TargetHolder_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Targetable_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Targetable>(*this, f, entity, context);
}

void Targetable_Prototype::Materialize(Frame*, Targetable& result,
                                       const PrototypeMaterializationContext&) const {
    result.IgnoreAutoShoot = IgnoreAutoShoot;
    result.IgnoreAutoTargetInsideBounds = IgnoreAutoTargetInsideBounds;
    result.IgnoreCameraAiming = IgnoreCameraAiming;
    result.BoundsSizeMultiplier = BoundsSizeMultiplier;
}

void Targetable_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TargetedEffect_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                           const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TargetedEffect>(*this, f, entity, context);
}

void TargetedEffect_Prototype::Materialize(Frame*, TargetedEffect& result,
                                           const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void TargetedEffect_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Team_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                 const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Team>(*this, f, entity, context);
}

void Team_Prototype::Materialize(Frame*, Team& result,
                                 const PrototypeMaterializationContext&) const {
    result.TeamID = TeamID;
}

void Team_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TeslaClawMoreBeam_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TeslaClawMoreBeam>(*this, f, entity, context);
}

void TeslaClawMoreBeam_Prototype::Materialize(Frame*, TeslaClawMoreBeam& result,
                                              const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void TeslaClawMoreBeam_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TimeScale_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TimeScale>(*this, f, entity, context);
}

void TimeScale_Prototype::Materialize(Frame*, TimeScale&,
                                      const PrototypeMaterializationContext&) const {}

void TimeScale_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TrailSpawner_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TrailSpawner>(*this, f, entity, context);
}

void TrailSpawner_Prototype::Materialize(Frame*, TrailSpawner& result,
                                         const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void TrailSpawner_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TravolatorActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TravolatorActor>(*this, f, entity, context);
}

void TravolatorActor_Prototype::Materialize(Frame*, TravolatorActor&,
                                            const PrototypeMaterializationContext&) const {}

void TravolatorActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TurnTo_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TurnTo>(*this, f, entity, context);
}

void TurnTo_Prototype::Materialize(Frame*, TurnTo& result,
                                   const PrototypeMaterializationContext&) const {
    result.turnSpeed = turnSpeed;
}

void TurnTo_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool TutorialGameMode_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<TutorialGameMode>(*this, f, entity, context);
}

void TutorialGameMode_Prototype::Materialize(Frame*, TutorialGameMode& result,
                                             const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void TutorialGameMode_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool UTAgent_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                    const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<UTAgent>(*this, f, entity, context);
}

void UTAgent_Prototype::Materialize(Frame* frame, UTAgent& result,
                                    const PrototypeMaterializationContext& context) const {
    result.Config = Config;
    UtilityReasoner.Materialize(frame, result.UtilityReasoner, context);
}

void UTAgent_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool UltimateAbilityUser_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<UltimateAbilityUser>(*this, f, entity, context);
}

void UltimateAbilityUser_Prototype::Materialize(Frame*, UltimateAbilityUser& result,
                                                const PrototypeMaterializationContext&) const {
    result.userSpec = userSpec;
}

void UltimateAbilityUser_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool UltimateEvolution_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<UltimateEvolution>(*this, f, entity, context);
}

void UltimateEvolution_Prototype::Materialize(Frame*, UltimateEvolution&,
                                              const PrototypeMaterializationContext&) const {}

void UltimateEvolution_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool UltimateTank_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<UltimateTank>(*this, f, entity, context);
}

void UltimateTank_Prototype::Materialize(Frame*, UltimateTank&,
                                         const PrototypeMaterializationContext&) const {}

void UltimateTank_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool UnlockedFeaturesManager_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<UnlockedFeaturesManager>(*this, f, entity, context);
}

void UnlockedFeaturesManager_Prototype::Materialize(
    Frame*, UnlockedFeaturesManager& result, const PrototypeMaterializationContext& context) const {
    {
        const int count =
            PrototypeValidator::CheckLength(static_cast<int>(unlockedFeatures.size()), 19, context);
        auto target = result.get_unlockedFeatures();
        for (int i = 0; i < count; ++i)
            target[static_cast<std::size_t>(i)] = unlockedFeatures[static_cast<std::size_t>(i)];
    }
}

void UnlockedFeaturesManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool VoidRepulserShield_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<VoidRepulserShield>(*this, f, entity, context);
}

void VoidRepulserShield_Prototype::Materialize(Frame*, VoidRepulserShield&,
                                               const PrototypeMaterializationContext&) const {}

void VoidRepulserShield_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool VoidRepulserSpeedBoostEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<VoidRepulserSpeedBoostEnhancement>(*this, f, entity, context);
}

void VoidRepulserSpeedBoostEnhancement_Prototype::Materialize(
    Frame*, VoidRepulserSpeedBoostEnhancement&, const PrototypeMaterializationContext&) const {}

void VoidRepulserSpeedBoostEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool VoidRepulserVoidWaveEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<VoidRepulserVoidWaveEnhancement>(*this, f, entity, context);
}

void VoidRepulserVoidWaveEnhancement_Prototype::Materialize(
    Frame*, VoidRepulserVoidWaveEnhancement&, const PrototypeMaterializationContext&) const {}

void VoidRepulserVoidWaveEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool VoidRepulserWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<VoidRepulserWeapon>(*this, f, entity, context);
}

void VoidRepulserWeapon_Prototype::Materialize(Frame*, VoidRepulserWeapon&,
                                               const PrototypeMaterializationContext&) const {}

void VoidRepulserWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WakeUpSurroundingsOnPushOrDestroy_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WakeUpSurroundingsOnPushOrDestroy>(*this, f, entity, context);
}

void WakeUpSurroundingsOnPushOrDestroy_Prototype::Materialize(
    Frame*, WakeUpSurroundingsOnPushOrDestroy& result,
    const PrototypeMaterializationContext&) const {
    result.radius = radius;
}

void WakeUpSurroundingsOnPushOrDestroy_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WallRunningActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WallRunningActor>(*this, f, entity, context);
}

void WallRunningActor_Prototype::Materialize(Frame*, WallRunningActor& result,
                                             const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void WallRunningActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WarmongerMiniRocketsEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WarmongerMiniRocketsEnhancement>(*this, f, entity, context);
}

void WarmongerMiniRocketsEnhancement_Prototype::Materialize(
    Frame*, WarmongerMiniRocketsEnhancement& result, const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void WarmongerMiniRocketsEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WaveElement_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WaveElement>(*this, f, entity, context);
}

void WaveElement_Prototype::Materialize(Frame*, WaveElement&,
                                        const PrototypeMaterializationContext&) const {}

void WaveElement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WaveManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WaveManager>(*this, f, entity, context);
}

void WaveManager_Prototype::Materialize(Frame*, WaveManager& result,
                                        const PrototypeMaterializationContext&) const {
    result.SquadSiegeConfig = SquadSiegeConfig;
    result.Spec = Spec;
    result.referencesAsset = referencesAsset;
}

void WaveManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WaveSpawner_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WaveSpawner>(*this, f, entity, context);
}

void WaveSpawner_Prototype::Materialize(Frame* frame, WaveSpawner& result,
                                        const PrototypeMaterializationContext& context) const {
    result.index = index;
    result.spawnInterval = spawnInterval;
    result.wanderRadius = wanderRadius;
    result.waypointRadius = waypointRadius;
    spawnConfig.Materialize(frame, result.spawnConfig, context);
    if (waypoints.empty()) {
        result.set_waypoints({});
    } else {
        auto ptr = result.get_waypoints();
        auto list = frame->AllocateList(ptr, static_cast<int>(waypoints.size()));
        for (const auto& item : waypoints) {
            list.Add(item);
        }
        result.set_waypoints(ptr);
    }
}

void WaveSpawner_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Weapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Weapon>(*this, f, entity, context);
}

void Weapon_Prototype::Materialize(Frame*, Weapon& result,
                                   const PrototypeMaterializationContext&) const {
    result.WeaponData = WeaponData;
}

void Weapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WeaponBehaviour_Charging_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WeaponBehaviour_Charging>(*this, f, entity, context);
}

void WeaponBehaviour_Charging_Prototype::Materialize(Frame*, WeaponBehaviour_Charging& result,
                                                     const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void WeaponBehaviour_Charging_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WeaponLoadoutActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WeaponLoadoutActor>(*this, f, entity, context);
}

void WeaponLoadoutActor_Prototype::Materialize(Frame*, WeaponLoadoutActor& result,
                                               const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void WeaponLoadoutActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WeaponProjectileSwitcherEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WeaponProjectileSwitcherEnhancement>(*this, f, entity, context);
}

void WeaponProjectileSwitcherEnhancement_Prototype::Materialize(
    Frame*, WeaponProjectileSwitcherEnhancement&, const PrototypeMaterializationContext&) const {}

void WeaponProjectileSwitcherEnhancement_Prototype::Dispatch(
    ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WeaponShowcaseGameMode_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WeaponShowcaseGameMode>(*this, f, entity, context);
}

void WeaponShowcaseGameMode_Prototype::Materialize(Frame*, WeaponShowcaseGameMode&,
                                                   const PrototypeMaterializationContext&) const {}

void WeaponShowcaseGameMode_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WeaponUser_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WeaponUser>(*this, f, entity, context);
}

void WeaponUser_Prototype::Materialize(Frame* frame, WeaponUser& result,
                                       const PrototypeMaterializationContext& context) const {
    MaterializeUser(frame, result, context);
}

void WeaponUser_Prototype::MaterializeUser(Frame*, WeaponUser& result,
                                           const PrototypeMaterializationContext& context) const {
    result.Prototype = context.get_ComponentPrototypeRef();
}

void WeaponUser_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WrenchThrowProjectile_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                  const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WrenchThrowProjectile>(*this, f, entity, context);
}

void WrenchThrowProjectile_Prototype::Materialize(Frame*, WrenchThrowProjectile&,
                                                  const PrototypeMaterializationContext&) const {}

void WrenchThrowProjectile_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WrenchThrowWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WrenchThrowWeapon>(*this, f, entity, context);
}

void WrenchThrowWeapon_Prototype::Materialize(Frame*, WrenchThrowWeapon&,
                                              const PrototypeMaterializationContext&) const {}

void WrenchThrowWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool WrenchWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<WrenchWeapon>(*this, f, entity, context);
}

void WrenchWeapon_Prototype::Materialize(Frame*, WrenchWeapon&,
                                         const PrototypeMaterializationContext&) const {}

void WrenchWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ZoneControlGameMode_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ZoneControlGameMode>(*this, f, entity, context);
}

void ZoneControlGameMode_Prototype::Materialize(Frame*, ZoneControlGameMode& result,
                                                const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void ZoneControlGameMode_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

} // namespace Quantum::Prototypes
