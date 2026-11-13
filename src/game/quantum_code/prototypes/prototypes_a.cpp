// prototypes_a.cpp - component prototypes AIBlackboardComponent to HoloshieldGloveWeapon
// (quantum.code.dll).
#include "quantum_code/prototypes/prototypes.h"

#include "quantum_code/prototypes/component_prototype_visitor.h"
#include "quantum_code/prototypes/prototype_support.h"

namespace Quantum::Prototypes {

bool AIBlackboardComponent_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                  const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AIBlackboardComponent>(*this, f, entity, context);
}

void AIBlackboardComponent_Prototype::Materialize(
    Frame* frame, AIBlackboardComponent& result,
    const PrototypeMaterializationContext& context) const {
    result.Board = Board;
    if (Entries.empty()) {
        result.set_Entries({});
    } else {
        auto ptr = result.get_Entries();
        auto dictionary = frame->AllocateDictionary(ptr, static_cast<int>(Entries.size()));
        for (const auto& entry : Entries) {
            BlackboardEntry value{};
            entry.Value.Materialize(frame, value, context);
            PrototypeValidator::AddToDictionary(dictionary, entry.Key, value, context);
        }
        result.set_Entries(ptr);
    }
}

void AIBlackboardComponent_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AIMoveForwardBehaviour_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AIMoveForwardBehaviour>(*this, f, entity, context);
}

void AIMoveForwardBehaviour_Prototype::Materialize(Frame*, AIMoveForwardBehaviour& result,
                                                   const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void AIMoveForwardBehaviour_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AINavigationInitialiser_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AINavigationInitialiser>(*this, f, entity, context);
}

void AINavigationInitialiser_Prototype::Materialize(Frame*, AINavigationInitialiser& result,
                                                    const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void AINavigationInitialiser_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AISpawner_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AISpawner>(*this, f, entity, context);
}

void AISpawner_Prototype::Materialize(Frame* frame, AISpawner& result,
                                      const PrototypeMaterializationContext& context) const {
    MaterializeUser(frame, result, context);
}

void AISpawner_Prototype::MaterializeUser(Frame*, AISpawner& result,
                                          const PrototypeMaterializationContext& context) const {
    result.Prototype = context.get_ComponentPrototypeRef();
}

void AISpawner_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ActorInputInitialiser_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                  const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ActorInputInitialiser>(*this, f, entity, context);
}

void ActorInputInitialiser_Prototype::Materialize(Frame*, ActorInputInitialiser&,
                                                  const PrototypeMaterializationContext&) const {}

void ActorInputInitialiser_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ActorManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ActorManager>(*this, f, entity, context);
}

void ActorManager_Prototype::Materialize(Frame*, ActorManager& result,
                                         const PrototypeMaterializationContext&) const {
    result.Config = Config;
}

void ActorManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ActorStatus_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ActorStatus>(*this, f, entity, context);
}

void ActorStatus_Prototype::Materialize(Frame*, ActorStatus& result,
                                        const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void ActorStatus_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ActorStatusModifier_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ActorStatusModifier>(*this, f, entity, context);
}

void ActorStatusModifier_Prototype::Materialize(Frame*, ActorStatusModifier& result,
                                                const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void ActorStatusModifier_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ActorStunned_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ActorStunned>(*this, f, entity, context);
}

void ActorStunned_Prototype::Materialize(Frame*, ActorStunned&,
                                         const PrototypeMaterializationContext&) const {}

void ActorStunned_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AddDamageIncreaseOnTargetedEffect_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AddDamageIncreaseOnTargetedEffect>(*this, f, entity, context);
}

void AddDamageIncreaseOnTargetedEffect_Prototype::Materialize(
    Frame*, AddDamageIncreaseOnTargetedEffect&, const PrototypeMaterializationContext&) const {}

void AddDamageIncreaseOnTargetedEffect_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AddEntitiesToSpawnOnProjectileDeath_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AddEntitiesToSpawnOnProjectileDeath>(*this, f, entity, context);
}

void AddEntitiesToSpawnOnProjectileDeath_Prototype::Materialize(
    Frame*, AddEntitiesToSpawnOnProjectileDeath& result,
    const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void AddEntitiesToSpawnOnProjectileDeath_Prototype::Dispatch(
    ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AgentOfDoom_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AgentOfDoom>(*this, f, entity, context);
}

void AgentOfDoom_Prototype::Materialize(Frame*, AgentOfDoom&,
                                        const PrototypeMaterializationContext&) const {}

void AgentOfDoom_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AgentOfDoomGrouped_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AgentOfDoomGrouped>(*this, f, entity, context);
}

void AgentOfDoomGrouped_Prototype::Materialize(Frame*, AgentOfDoomGrouped&,
                                               const PrototypeMaterializationContext&) const {}

void AgentOfDoomGrouped_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AmoeboidLauncherEnhancementPuddle_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AmoeboidLauncherEnhancementPuddle>(*this, f, entity, context);
}

void AmoeboidLauncherEnhancementPuddle_Prototype::Materialize(
    Frame*, AmoeboidLauncherEnhancementPuddle& result,
    const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void AmoeboidLauncherEnhancementPuddle_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AoeTrail_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                     const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AoeTrail>(*this, f, entity, context);
}

void AoeTrail_Prototype::Materialize(Frame*, AoeTrail& result,
                                     const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void AoeTrail_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AreaOfEffect_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AreaOfEffect>(*this, f, entity, context);
}

void AreaOfEffect_Prototype::Materialize(Frame*, AreaOfEffect& result,
                                         const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void AreaOfEffect_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AreaOfEffectLevelUp_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AreaOfEffectLevelUp>(*this, f, entity, context);
}

void AreaOfEffectLevelUp_Prototype::Materialize(Frame*, AreaOfEffectLevelUp& result,
                                                const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void AreaOfEffectLevelUp_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AreaOfEffectRaycastInfo_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AreaOfEffectRaycastInfo>(*this, f, entity, context);
}

void AreaOfEffectRaycastInfo_Prototype::Materialize(Frame*, AreaOfEffectRaycastInfo&,
                                                    const PrototypeMaterializationContext&) const {}

void AreaOfEffectRaycastInfo_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ArenaCannon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ArenaCannon>(*this, f, entity, context);
}

void ArenaCannon_Prototype::Materialize(Frame*, ArenaCannon&,
                                        const PrototypeMaterializationContext&) const {}

void ArenaCannon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool AttackInfo_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<AttackInfo>(*this, f, entity, context);
}

void AttackInfo_Prototype::Materialize(Frame*, AttackInfo&,
                                       const PrototypeMaterializationContext&) const {}

void AttackInfo_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BTAgent_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                    const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BTAgent>(*this, f, entity, context);
}

void BTAgent_Prototype::Materialize(Frame* frame, BTAgent& result,
                                    const PrototypeMaterializationContext& context) const {
    result.AbortNodeId = AbortNodeId;
    result.Config = Config;
    result.Current = Current;
    result.Tree = Tree;
    if (ActiveServices.empty()) {
        result.set_ActiveServices({});
    } else {
        auto ptr = result.get_ActiveServices();
        auto list = frame->AllocateList(ptr, static_cast<int>(ActiveServices.size()));
        for (const auto& item : ActiveServices) {
            list.Add(item);
        }
        result.set_ActiveServices(ptr);
    }
    if (BTDataValues.empty()) {
        result.set_BTDataValues({});
    } else {
        auto ptr = result.get_BTDataValues();
        auto list = frame->AllocateList(ptr, static_cast<int>(BTDataValues.size()));
        for (const auto& item : BTDataValues) {
            BTDataValue value{};
            item.Materialize(frame, value, context);
            list.Add(value);
        }
        result.set_BTDataValues(ptr);
    }
    if (DynamicComposites.empty()) {
        result.set_DynamicComposites({});
    } else {
        auto ptr = result.get_DynamicComposites();
        auto list = frame->AllocateList(ptr, static_cast<int>(DynamicComposites.size()));
        for (const auto& item : DynamicComposites) {
            list.Add(item);
        }
        result.set_DynamicComposites(ptr);
    }
    if (NodesStatus.empty()) {
        result.set_NodesStatus({});
    } else {
        auto ptr = result.get_NodesStatus();
        auto list = frame->AllocateList(ptr, static_cast<int>(NodesStatus.size()));
        for (const auto& item : NodesStatus) {
            list.Add(item);
        }
        result.set_NodesStatus(ptr);
    }
    if (ServicesEndTimes.empty()) {
        result.set_ServicesEndTimes({});
    } else {
        auto ptr = result.get_ServicesEndTimes();
        auto list = frame->AllocateList(ptr, static_cast<int>(ServicesEndTimes.size()));
        for (const auto& item : ServicesEndTimes) {
            list.Add(item);
        }
        result.set_ServicesEndTimes(ptr);
    }
}

void BTAgent_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BattleStatsActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BattleStatsActor>(*this, f, entity, context);
}

void BattleStatsActor_Prototype::Materialize(Frame*, BattleStatsActor&,
                                             const PrototypeMaterializationContext&) const {}

void BattleStatsActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BigBoomMine_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BigBoomMine>(*this, f, entity, context);
}

void BigBoomMine_Prototype::Materialize(Frame*, BigBoomMine& result,
                                        const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void BigBoomMine_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BigBoomWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BigBoomWeapon>(*this, f, entity, context);
}

void BigBoomWeapon_Prototype::Materialize(Frame*, BigBoomWeapon&,
                                          const PrototypeMaterializationContext&) const {}

void BigBoomWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BlackboardInitializer_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                  const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BlackboardInitializer>(*this, f, entity, context);
}

void BlackboardInitializer_Prototype::Materialize(Frame*, BlackboardInitializer& result,
                                                  const PrototypeMaterializationContext&) const {
    result.BBInitializer = BBInitializer;
}

void BlackboardInitializer_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BlastBall_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BlastBall>(*this, f, entity, context);
}

void BlastBall_Prototype::Materialize(Frame*, BlastBall& result,
                                      const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void BlastBall_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BlastBallGame_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BlastBallGame>(*this, f, entity, context);
}

void BlastBallGame_Prototype::Materialize(Frame*, BlastBallGame&,
                                          const PrototypeMaterializationContext&) const {}

void BlastBallGame_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BlastBallGoal_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BlastBallGoal>(*this, f, entity, context);
}

void BlastBallGoal_Prototype::Materialize(Frame*, BlastBallGoal& result,
                                          const PrototypeMaterializationContext& context) const {
    result.ballInside = ballInside;
    result.OwnerTeam = OwnerTeam;
    PrototypeValidator::FindMapEntity(ballEntity, context, result.ballEntity);
}

void BlastBallGoal_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BlastBallPlayer_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BlastBallPlayer>(*this, f, entity, context);
}

void BlastBallPlayer_Prototype::Materialize(Frame*, BlastBallPlayer&,
                                            const PrototypeMaterializationContext&) const {}

void BlastBallPlayer_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BlasterMoveSpeedEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BlasterMoveSpeedEnhancement>(*this, f, entity, context);
}

void BlasterMoveSpeedEnhancement_Prototype::Materialize(
    Frame*, BlasterMoveSpeedEnhancement&, const PrototypeMaterializationContext&) const {}

void BlasterMoveSpeedEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BlasterSpecialShotEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BlasterSpecialShotEnhancement>(*this, f, entity, context);
}

void BlasterSpecialShotEnhancement_Prototype::Materialize(
    Frame*, BlasterSpecialShotEnhancement&, const PrototypeMaterializationContext&) const {}

void BlasterSpecialShotEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BoltCounterElement_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BoltCounterElement>(*this, f, entity, context);
}

void BoltCounterElement_Prototype::Materialize(Frame*, BoltCounterElement& result,
                                               const PrototypeMaterializationContext&) const {
    result.requiredBolts = requiredBolts;
    result.gameplayElementConfig = gameplayElementConfig;
}

void BoltCounterElement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BoltGathererActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BoltGathererActor>(*this, f, entity, context);
}

void BoltGathererActor_Prototype::Materialize(Frame*, BoltGathererActor& result,
                                              const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void BoltGathererActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BoltPowerActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                           const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BoltPowerActor>(*this, f, entity, context);
}

void BoltPowerActor_Prototype::Materialize(Frame*, BoltPowerActor& result,
                                           const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void BoltPowerActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BombardierDrone_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BombardierDrone>(*this, f, entity, context);
}

void BombardierDrone_Prototype::Materialize(Frame*, BombardierDrone& result,
                                            const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void BombardierDrone_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BombardierWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BombardierWeapon>(*this, f, entity, context);
}

void BombardierWeapon_Prototype::Materialize(Frame*, BombardierWeapon& result,
                                             const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void BombardierWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Bot_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Bot>(*this, f, entity, context);
}

void Bot_Prototype::Materialize(Frame*, Bot&, const PrototypeMaterializationContext&) const {}

void Bot_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Bumper_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Bumper>(*this, f, entity, context);
}

void Bumper_Prototype::Materialize(Frame*, Bumper& result,
                                   const PrototypeMaterializationContext&) const {
    result.config = config;
    result.impulseStrength = impulseStrength;
    result.impulseDirection = impulseDirection;
}

void Bumper_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BumperBar_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BumperBar>(*this, f, entity, context);
}

void BumperBar_Prototype::Materialize(Frame*, BumperBar& result,
                                      const PrototypeMaterializationContext&) const {
    result.Config = Config;
}

void BumperBar_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool BuzzbladesRicochet_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<BuzzbladesRicochet>(*this, f, entity, context);
}

void BuzzbladesRicochet_Prototype::Materialize(Frame*, BuzzbladesRicochet& result,
                                               const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void BuzzbladesRicochet_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool CharacterColliderScaler_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<CharacterColliderScaler>(*this, f, entity, context);
}

void CharacterColliderScaler_Prototype::Materialize(Frame*, CharacterColliderScaler&,
                                                    const PrototypeMaterializationContext&) const {}

void CharacterColliderScaler_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ChargeAttack_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ChargeAttack>(*this, f, entity, context);
}

void ChargeAttack_Prototype::Materialize(Frame*, ChargeAttack& result,
                                         const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void ChargeAttack_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Checkpoint_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Checkpoint>(*this, f, entity, context);
}

void Checkpoint_Prototype::Materialize(Frame*, Checkpoint&,
                                       const PrototypeMaterializationContext&) const {}

void Checkpoint_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ColliderResize_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                           const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ColliderResize>(*this, f, entity, context);
}

void ColliderResize_Prototype::Materialize(Frame*, ColliderResize& result,
                                           const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void ColliderResize_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool CollisionForce_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                           const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<CollisionForce>(*this, f, entity, context);
}

void CollisionForce_Prototype::Materialize(Frame*, CollisionForce& result,
                                           const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void CollisionForce_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool CombatParticipant_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<CombatParticipant>(*this, f, entity, context);
}

void CombatParticipant_Prototype::Materialize(Frame*, CombatParticipant&,
                                              const PrototypeMaterializationContext&) const {}

void CombatParticipant_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool CompoundHFSM_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<CompoundHFSM>(*this, f, entity, context);
}

void CompoundHFSM_Prototype::Materialize(Frame*, CompoundHFSM& result,
                                         const PrototypeMaterializationContext&) const {
    result.spec = spec;
}

void CompoundHFSM_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Cryoslider_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Cryoslider>(*this, f, entity, context);
}

void Cryoslider_Prototype::Materialize(Frame*, Cryoslider& result,
                                       const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void Cryoslider_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool CryosliderWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<CryosliderWeapon>(*this, f, entity, context);
}

void CryosliderWeapon_Prototype::Materialize(Frame*, CryosliderWeapon& result,
                                             const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void CryosliderWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DamageChargeable_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DamageChargeable>(*this, f, entity, context);
}

void DamageChargeable_Prototype::Materialize(Frame*, DamageChargeable& result,
                                             const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void DamageChargeable_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DamageIncreasedOnTargetsEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DamageIncreasedOnTargetsEnhancement>(*this, f, entity, context);
}

void DamageIncreasedOnTargetsEnhancement_Prototype::Materialize(
    Frame*, DamageIncreasedOnTargetsEnhancement&, const PrototypeMaterializationContext&) const {}

void DamageIncreasedOnTargetsEnhancement_Prototype::Dispatch(
    ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DamageMultiplier_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DamageMultiplier>(*this, f, entity, context);
}

void DamageMultiplier_Prototype::Materialize(Frame*, DamageMultiplier& result,
                                             const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void DamageMultiplier_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DamageMultiplierHeroImprovement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DamageMultiplierHeroImprovement>(*this, f, entity, context);
}

void DamageMultiplierHeroImprovement_Prototype::Materialize(
    Frame*, DamageMultiplierHeroImprovement&, const PrototypeMaterializationContext&) const {}

void DamageMultiplierHeroImprovement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DamageSource_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DamageSource>(*this, f, entity, context);
}

void DamageSource_Prototype::Materialize(Frame*, DamageSource& result,
                                         const PrototypeMaterializationContext&) const {
    result.hitTrackingDone = hitTrackingDone;
    result.AmmunitionSpec = AmmunitionSpec;
    result.Spec = Spec;
}

void DamageSource_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DamageToImpulseConverter_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DamageToImpulseConverter>(*this, f, entity, context);
}

void DamageToImpulseConverter_Prototype::Materialize(Frame*, DamageToImpulseConverter& result,
                                                     const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void DamageToImpulseConverter_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Damageable_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Damageable>(*this, f, entity, context);
}

void Damageable_Prototype::Materialize(Frame*, Damageable& result,
                                       const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void Damageable_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DashActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DashActor>(*this, f, entity, context);
}

void DashActor_Prototype::Materialize(Frame*, DashActor& result,
                                      const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void DashActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DrillDash_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DrillDash>(*this, f, entity, context);
}

void DrillDash_Prototype::Materialize(Frame*, DrillDash&,
                                      const PrototypeMaterializationContext&) const {}

void DrillDash_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DroppableCollector_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DroppableCollector>(*this, f, entity, context);
}

void DroppableCollector_Prototype::Materialize(Frame*, DroppableCollector& result,
                                               const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void DroppableCollector_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DroppableEntity_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DroppableEntity>(*this, f, entity, context);
}

void DroppableEntity_Prototype::Materialize(Frame*, DroppableEntity&,
                                            const PrototypeMaterializationContext&) const {}

void DroppableEntity_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool DroppableManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                             const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<DroppableManager>(*this, f, entity, context);
}

void DroppableManager_Prototype::Materialize(Frame*, DroppableManager& result,
                                             const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void DroppableManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool ElementalBuildUpMultiplier_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<ElementalBuildUpMultiplier>(*this, f, entity, context);
}

void ElementalBuildUpMultiplier_Prototype::Materialize(
    Frame*, ElementalBuildUpMultiplier&, const PrototypeMaterializationContext&) const {}

void ElementalBuildUpMultiplier_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool EliminationGameMode_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<EliminationGameMode>(*this, f, entity, context);
}

void EliminationGameMode_Prototype::Materialize(Frame*, EliminationGameMode& result,
                                                const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void EliminationGameMode_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool EnergyCellManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<EnergyCellManager>(*this, f, entity, context);
}

void EnergyCellManager_Prototype::Materialize(Frame*, EnergyCellManager& result,
                                              const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void EnergyCellManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool EnforcerFirstShotBonusEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<EnforcerFirstShotBonusEnhancement>(*this, f, entity, context);
}

void EnforcerFirstShotBonusEnhancement_Prototype::Materialize(
    Frame*, EnforcerFirstShotBonusEnhancement&, const PrototypeMaterializationContext&) const {}

void EnforcerFirstShotBonusEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool EntityDropper_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<EntityDropper>(*this, f, entity, context);
}

void EntityDropper_Prototype::Materialize(Frame* frame, EntityDropper& result,
                                          const PrototypeMaterializationContext&) const {
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

void EntityDropper_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool EntityInfo_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Code::EntityInfo>(*this, f, entity, context);
}

void EntityInfo_Prototype::Materialize(Frame*, Code::EntityInfo& result,
                                       const PrototypeMaterializationContext&) const {
    result.type = type;
    result.id = id;
    result.isActiveObjective = isActiveObjective;
}

void EntityInfo_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool FireReactor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<FireReactor>(*this, f, entity, context);
}

void FireReactor_Prototype::Materialize(Frame*, FireReactor& result,
                                        const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void FireReactor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GOAPAgent_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GOAPAgent>(*this, f, entity, context);
}

void GOAPAgent_Prototype::Materialize(Frame*, GOAPAgent& result,
                                      const PrototypeMaterializationContext&) const {
    result.Config = Config;
    result.Root = Root;
}

void GOAPAgent_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GOAPData_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                     const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GOAPData>(*this, f, entity, context);
}

void GOAPData_Prototype::Materialize(Frame*, GOAPData& result,
                                     const PrototypeMaterializationContext&) const {
    result.HeuristicCost = HeuristicCost;
}

void GOAPData_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GadgetUser_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GadgetUser>(*this, f, entity, context);
}

void GadgetUser_Prototype::Materialize(Frame*, GadgetUser&,
                                       const PrototypeMaterializationContext&) const {}

void GadgetUser_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GameManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GameManager>(*this, f, entity, context);
}

void GameManager_Prototype::Materialize(Frame*, GameManager& result,
                                        const PrototypeMaterializationContext&) const {
    result.nbPlayersSpawned = nbPlayersSpawned;
    result.config = config;
}

void GameManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GameModeActivator_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GameModeActivator>(*this, f, entity, context);
}

void GameModeActivator_Prototype::Materialize(Frame*, GameModeActivator& result,
                                              const PrototypeMaterializationContext&) const {
    result.modificator = modificator;
    result.enabled = enabled;
}

void GameModeActivator_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GameModeHandler_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GameModeHandler>(*this, f, entity, context);
}

void GameModeHandler_Prototype::Materialize(Frame*, GameModeHandler& result,
                                            const PrototypeMaterializationContext&) const {
    result.config = config;
    result.levelConfig = levelConfig;
    result.gameModesTable = gameModesTable;
}

void GameModeHandler_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GameModifiersManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                 const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GameModifiersManager>(*this, f, entity, context);
}

void GameModifiersManager_Prototype::Materialize(Frame*, GameModifiersManager& result,
                                                 const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void GameModifiersManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GatlingWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GatlingWeapon>(*this, f, entity, context);
}

void GatlingWeapon_Prototype::Materialize(Frame*, GatlingWeapon&,
                                          const PrototypeMaterializationContext&) const {}

void GatlingWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GloveOfDoomWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GloveOfDoomWeapon>(*this, f, entity, context);
}

void GloveOfDoomWeapon_Prototype::Materialize(Frame*, GloveOfDoomWeapon&,
                                              const PrototypeMaterializationContext&) const {}

void GloveOfDoomWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GoldenBolt_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GoldenBolt>(*this, f, entity, context);
}

void GoldenBolt_Prototype::Materialize(Frame*, GoldenBolt& result,
                                       const PrototypeMaterializationContext&) const {
    result.isCollected = isCollected;
}

void GoldenBolt_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GoldenBoltManager_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                              const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GoldenBoltManager>(*this, f, entity, context);
}

void GoldenBoltManager_Prototype::Materialize(Frame*, GoldenBoltManager&,
                                              const PrototypeMaterializationContext&) const {}

void GoldenBoltManager_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GrinderActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                         const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GrinderActor>(*this, f, entity, context);
}

void GrinderActor_Prototype::Materialize(Frame*, GrinderActor& result,
                                         const PrototypeMaterializationContext&) const {
    result.config = config;
}

void GrinderActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool GroundDashActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                            const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<GroundDashActor>(*this, f, entity, context);
}

void GroundDashActor_Prototype::Materialize(Frame*, GroundDashActor& result,
                                            const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void GroundDashActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HFSMAgent_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HFSMAgent>(*this, f, entity, context);
}

void HFSMAgent_Prototype::Materialize(Frame* frame, HFSMAgent& result,
                                      const PrototypeMaterializationContext& context) const {
    result.Config = Config;
    Data.Materialize(frame, result.Data, context);
}

void HFSMAgent_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HeadHunterWallHackEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HeadHunterWallHackEnhancement>(*this, f, entity, context);
}

void HeadHunterWallHackEnhancement_Prototype::Materialize(
    Frame*, HeadHunterWallHackEnhancement& result, const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void HeadHunterWallHackEnhancement_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HealSource_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                       const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HealSource>(*this, f, entity, context);
}

void HealSource_Prototype::Materialize(Frame*, HealSource& result,
                                       const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void HealSource_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Health_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Health>(*this, f, entity, context);
}

void Health_Prototype::Materialize(Frame*, Health& result,
                                   const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void Health_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HelipackActor_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HelipackActor>(*this, f, entity, context);
}

void HelipackActor_Prototype::Materialize(Frame*, HelipackActor& result,
                                          const PrototypeMaterializationContext&) const {
    result.Spec = Spec;
}

void HelipackActor_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool Hierarchy_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                      const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<Hierarchy>(*this, f, entity, context);
}

void Hierarchy_Prototype::Materialize(Frame*, Hierarchy& result,
                                      const PrototypeMaterializationContext& context) const {
    PrototypeValidator::FindMapEntity(Parent, context, result.Parent);
    result.Offset = Offset;
}

void Hierarchy_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HoloshieldGloveHealZoneEnhancement_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HoloshieldGloveHealZoneEnhancement>(*this, f, entity, context);
}

void HoloshieldGloveHealZoneEnhancement_Prototype::Materialize(
    Frame*, HoloshieldGloveHealZoneEnhancement&, const PrototypeMaterializationContext&) const {}

void HoloshieldGloveHealZoneEnhancement_Prototype::Dispatch(
    ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HoloshieldGloveShield_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                  const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HoloshieldGloveShield>(*this, f, entity, context);
}

void HoloshieldGloveShield_Prototype::Materialize(Frame*, HoloshieldGloveShield&,
                                                  const PrototypeMaterializationContext&) const {}

void HoloshieldGloveShield_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HoloshieldGloveShields_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                   const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HoloshieldGloveShields>(*this, f, entity, context);
}

void HoloshieldGloveShields_Prototype::Materialize(Frame*, HoloshieldGloveShields&,
                                                   const PrototypeMaterializationContext&) const {}

void HoloshieldGloveShields_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HoloshieldGloveShieldsGrouped_Prototype::AddToEntity(
    Core::FrameBase* f, EntityRef entity, const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HoloshieldGloveShieldsGrouped>(*this, f, entity, context);
}

void HoloshieldGloveShieldsGrouped_Prototype::Materialize(
    Frame*, HoloshieldGloveShieldsGrouped&, const PrototypeMaterializationContext&) const {}

void HoloshieldGloveShieldsGrouped_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

bool HoloshieldGloveWeapon_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                  const PrototypeMaterializationContext& context) {
    return MaterializeAndSet<HoloshieldGloveWeapon>(*this, f, entity, context);
}

void HoloshieldGloveWeapon_Prototype::Materialize(Frame*, HoloshieldGloveWeapon&,
                                                  const PrototypeMaterializationContext&) const {}

void HoloshieldGloveWeapon_Prototype::Dispatch(ComponentPrototypeVisitorBase& visitor) {
    AsGameVisitor(visitor, *this).Visit(*this);
}

} // namespace Quantum::Prototypes
