// prototypes.h - the prototypes of the gameplay simulation's components and of the value
// types stored in them (Quantum.Prototypes, quantum.code.dll).
//
// A component prototype holds the editor-authored values of one component; materialising
// it into a frame builds the component (Materialize) and adds it to an entity
// (AddToEntity, which always replaces an existing component and reports whether the
// component was new). The original code generator wrote one class per component:
//
//   * fields that exist in the component are copied; map entity links (MapEntityId) are
//     resolved to the entities created for the map; arrays become frame-heap lists or
//     dictionaries (allocated in the component's field order) or fill fixed-size buffers;
//     nested prototypes materialise into the nested value;
//   * a few prototypes carry fields the component does not have. Their component stores
//     the ComponentPrototypeRef of its prototype instead (MaterializeUser), and systems
//     read those fields from the prototype at run time;
//   * Dispatch hands the prototype to a ComponentPrototypeVisitor (double dispatch).
//
// The C# classes' default values are reproduced as member initialisers.
#pragma once

#include "quantum_code/components.h"
#include "quantum_code/frame.h"
#include "quantum_core/component_prototype.h"
#include "quantum_core/component_prototypes.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Quantum {

class ComponentPrototypeVisitor;

namespace Prototypes {

class UTMomentumData_Prototype final : public StructPrototype {
  public:
    std::int32_t Value{};
    std::uint8_t DecayAmount{};

    // 0x7c2b974
    UTMomentumData_Prototype() = default;
    // 0x7c2b960
    void Materialize(Frame* frame, UTMomentumData& result,
                     const PrototypeMaterializationContext& context) const;
};

class UTMomentumPack_Prototype final : public StructPrototype {
  public:
    AssetRefConsideration ConsiderationRef{};
    UTMomentumData_Prototype MomentumData{};

    // 0x7c2b9a8
    UTMomentumPack_Prototype() = default;
    // 0x7c2b97c
    void Materialize(Frame* frame, UTMomentumPack& result,
                     const PrototypeMaterializationContext& context) const;
};

class DictionaryEntry_AssetRefConsideration_FP_Prototype final : public DictionaryEntryPrototype {
  public:
    AssetRefConsideration Key{};
    FP Value{};

    // 0x7c1710c
    DictionaryEntry_AssetRefConsideration_FP_Prototype() = default;
};

class UtilityReasoner_Prototype final : public StructPrototype {
  public:
    AssetRefUTRoot UTRoot{};
    std::vector<AssetRefConsideration> Considerations{};
    std::vector<AssetRefConsideration> HighRankConsiderations{};
    std::vector<UTMomentumPack_Prototype> MomentumList{};
    FP TimeToTick{};
    std::vector<DictionaryEntry_AssetRefConsideration_FP_Prototype> CooldownsDict{};
    std::vector<AssetRefConsideration> PreviousExecution{};

    // 0x7c2c258
    UtilityReasoner_Prototype() = default;
    // 0x7c2b3cc
    void Materialize(Frame* frame, UtilityReasoner& result,
                     const PrototypeMaterializationContext& context) const;
};

class SpawnConfig_Prototype final : public StructPrototype {
  public:
    QBoolean useRandomSpawnTransform{};
    FP spawnPositionOffsetDistance{};
    std::int32_t findPositionAttempts{};
    FP freePositionRadius{};
    FP fallbackYOffset{};

    // 0x7c291b4
    SpawnConfig_Prototype() = default;
    // 0x7c29188
    void Materialize(Frame* frame, SpawnConfig& result,
                     const PrototypeMaterializationContext& context) const;
};

// Prototype of a KillZoneSurfaceType value (the enum as its underlying integer).
struct KillZoneSurfaceType_Prototype {
    std::int32_t Value{};

    // 0x7c170fc
    operator KillZoneSurfaceType() const { return static_cast<KillZoneSurfaceType>(Value); }
};

class KillZoneArea_Prototype final : public StructPrototype {
  public:
    FPVector3 corner{};
    FPVector2 size{};
    KillZoneSurfaceType_Prototype surfaceType{};

    // 0x7c23a90
    KillZoneArea_Prototype() = default;
    // 0x7c2395c
    void Materialize(Frame* frame, KillZoneArea& result,
                     const PrototypeMaterializationContext& context) const;
};

class HFSMData_Prototype final : public StructPrototype {
  public:
    AssetRefHFSMRoot Root{};
    AssetRefHFSMState CurrentState{};
    FP Time{};
    std::vector<FP> Times = std::vector<FP>(8);
    QBoolean UsesStateHistory{};
    std::uint8_t StatesHistory{};

    // 0x7c21b18
    HFSMData_Prototype() = default;
    // 0x7c21944
    void Materialize(Frame* frame, HFSMData& result,
                     const PrototypeMaterializationContext& context) const;
};

class BTDataValue_Prototype final : public UnionPrototype {
  public:
    std::string _field_used_{};
    FP FPValue{};
    std::int32_t IntValue{};

    // 0x7c1a298
    BTDataValue_Prototype() = default;
    // 0x7c19fd0
    void Materialize(Frame* frame, BTDataValue& result,
                     const PrototypeMaterializationContext& context) const;
};

class BlackboardValue_Prototype final : public UnionPrototype {
  public:
    std::string _field_used_{};
    QBoolean BooleanValue{};
    std::uint8_t ByteValue{};
    std::int32_t IntegerValue{};
    FP FPValue{};
    FPVector2 FPVector2Value{};
    FPVector3 FPVector3Value{};
    MapEntityId EntityRefValue{};
    AssetRef AssetRefValue{};

    // 0x7c1ad58
    BlackboardValue_Prototype() = default;
    // 0x7c1a80c
    void Materialize(Frame* frame, BlackboardValue& result,
                     const PrototypeMaterializationContext& context) const;
};

class BlackboardEntry_Prototype final : public StructPrototype {
  public:
    BlackboardValue_Prototype Value{};
    std::vector<AssetRefBTDecorator> ReactiveDecorators{};

    // 0x7c1ab54
    BlackboardEntry_Prototype() = default;
    // 0x7c17478
    void Materialize(Frame* frame, BlackboardEntry& result,
                     const PrototypeMaterializationContext& context) const;
};

class DictionaryEntry_Int32_BlackboardEntry_Prototype final : public DictionaryEntryPrototype {
  public:
    std::int32_t Key{};
    BlackboardEntry_Prototype Value{};

    // 0x7c17104
    DictionaryEntry_Int32_BlackboardEntry_Prototype() = default;
};

// Prototype of a EntityInfoType value (the enum as its underlying integer).
struct EntityInfoType_Prototype {
    std::int32_t Value{};

    // 0x7c170f4
    operator EntityInfoType() const { return static_cast<EntityInfoType>(Value); }
};

// Prototype of a GAME_MODIFICATOR value (the enum as its underlying integer).
struct GAME_MODIFICATOR_Prototype {
    std::int32_t Value{};

    // 0x7c170f8
    operator GAME_MODIFICATOR() const { return static_cast<GAME_MODIFICATOR>(Value); }
};

// Prototype of a MobType value (the enum as its underlying integer).
struct MobType_Prototype {
    std::int32_t Value{};

    // 0x7c17100
    operator MobType() const { return static_cast<MobType>(Value); }
};

class AIBlackboardComponent_Prototype final
    : public TypedComponentPrototype<AIBlackboardComponent> {
  public:
    AssetRefAIBlackboard Board{};
    std::vector<DictionaryEntry_Int32_BlackboardEntry_Prototype> Entries{};

    // 0x7c1765c
    AIBlackboardComponent_Prototype() = default;

    // 0x7c17114
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c17250
    void Materialize(Frame* frame, AIBlackboardComponent& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c175cc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AIMoveForwardBehaviour_Prototype final
    : public TypedComponentPrototype<AIMoveForwardBehaviour> {
  public:
    AssetRefAIMoveForwardBehaviourSpec Spec{};

    // 0x7c1788c
    AIMoveForwardBehaviour_Prototype() = default;

    // 0x7c176dc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c177f0
    void Materialize(Frame* frame, AIMoveForwardBehaviour& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c177fc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AINavigationInitialiser_Prototype final
    : public TypedComponentPrototype<AINavigationInitialiser> {
  public:
    AssetRefAINavigationSpec spec{};

    // 0x7c17a2c
    AINavigationInitialiser_Prototype() = default;

    // 0x7c178d4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c17990
    void Materialize(Frame* frame, AINavigationInitialiser& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1799c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AISpawner_Prototype final : public TypedComponentPrototype<AISpawner> {
  public:
    AssetRefEntityPrototype EntityToSpawn{};
    bool StartEnabled = true;
    std::int32_t NumToSpawn{};
    FP WanderRadius{};
    FP InitialSpawnDelaySeconds{};
    FP SpawnIntervalSeconds = FP::FromRaw(2 << 16);
    ::Quantum::SpawnConfig SpawnConfig{};
    bool AllowRespawnAfterDeath = true;
    FP AfterDeathSpawnDelaySeconds = FP::FromRaw(3 << 16);
    bool configWasDeprecated{};

    // 0x7c17c70
    AISpawner_Prototype() = default;

    // 0x7c17a94
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c17bc0
    void Materialize(Frame* frame, AISpawner& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c17a74 - the component keeps a reference to this prototype.
    void MaterializeUser(Frame* frame, AISpawner& result,
                         const PrototypeMaterializationContext& context) const;
    // 0x7c17be0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ActorInputInitialiser_Prototype final
    : public TypedComponentPrototype<ActorInputInitialiser> {
  public:
    // 0x7c17e1c
    ActorInputInitialiser_Prototype() = default;

    // 0x7c17cd8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c17d88
    void Materialize(Frame* frame, ActorInputInitialiser& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c17d8c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ActorManager_Prototype final : public TypedComponentPrototype<ActorManager> {
  public:
    AssetRefActorManagerConfig Config{};

    // 0x7c18014
    ActorManager_Prototype() = default;

    // 0x7c17e64
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c17f78
    void Materialize(Frame* frame, ActorManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c17f84
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ActorStatus_Prototype final : public TypedComponentPrototype<ActorStatus> {
  public:
    AssetRefActorStatusSpec spec{};

    // 0x7c18208
    ActorStatus_Prototype() = default;

    // 0x7c1805c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1816c
    void Materialize(Frame* frame, ActorStatus& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c18178
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ActorStatusModifier_Prototype final : public TypedComponentPrototype<ActorStatusModifier> {
  public:
    AssetRefActorStatusModifierSpec spec{};

    // 0x7c183a8
    ActorStatusModifier_Prototype() = default;

    // 0x7c18250
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1830c
    void Materialize(Frame* frame, ActorStatusModifier& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c18318
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ActorStunned_Prototype final : public TypedComponentPrototype<ActorStunned> {
  public:
    // 0x7c18534
    ActorStunned_Prototype() = default;

    // 0x7c183f0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c184a0
    void Materialize(Frame* frame, ActorStunned& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c184a4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AddDamageIncreaseOnTargetedEffect_Prototype final
    : public TypedComponentPrototype<AddDamageIncreaseOnTargetedEffect> {
  public:
    // 0x7c186c0
    AddDamageIncreaseOnTargetedEffect_Prototype() = default;

    // 0x7c1857c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1862c
    void Materialize(Frame* frame, AddDamageIncreaseOnTargetedEffect& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c18630
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AddEntitiesToSpawnOnProjectileDeath_Prototype final
    : public TypedComponentPrototype<AddEntitiesToSpawnOnProjectileDeath> {
  public:
    AssetRefAddEntitiesToSpawnOnProjectileDeathSpec Spec{};

    // 0x7c18860
    AddEntitiesToSpawnOnProjectileDeath_Prototype() = default;

    // 0x7c18708
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c187c4
    void Materialize(Frame* frame, AddEntitiesToSpawnOnProjectileDeath& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c187d0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AgentOfDoom_Prototype final : public TypedComponentPrototype<AgentOfDoom> {
  public:
    // 0x7c189ec
    AgentOfDoom_Prototype() = default;

    // 0x7c188a8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c18958
    void Materialize(Frame* frame, AgentOfDoom& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1895c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AgentOfDoomGrouped_Prototype final : public TypedComponentPrototype<AgentOfDoomGrouped> {
  public:
    // 0x7c18b78
    AgentOfDoomGrouped_Prototype() = default;

    // 0x7c18a34
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c18ae4
    void Materialize(Frame* frame, AgentOfDoomGrouped& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c18ae8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AmoeboidLauncherEnhancementPuddle_Prototype final
    : public TypedComponentPrototype<AmoeboidLauncherEnhancementPuddle> {
  public:
    AssetRefAmoeboidLauncherEnhancementPuddleSpec Spec{};

    // 0x7c18d18
    AmoeboidLauncherEnhancementPuddle_Prototype() = default;

    // 0x7c18bc0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c18c7c
    void Materialize(Frame* frame, AmoeboidLauncherEnhancementPuddle& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c18c88
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AoeTrail_Prototype final : public TypedComponentPrototype<AoeTrail> {
  public:
    AssetRefAoeTrailSpec Spec{};

    // 0x7c18f1c
    AoeTrail_Prototype() = default;

    // 0x7c18d60
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c18e80
    void Materialize(Frame* frame, AoeTrail& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c18e8c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AreaOfEffect_Prototype final : public TypedComponentPrototype<AreaOfEffect> {
  public:
    AssetRefAreaOfEffectSpec Spec{};

    // 0x7c19128
    AreaOfEffect_Prototype() = default;

    // 0x7c18f64
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1908c
    void Materialize(Frame* frame, AreaOfEffect& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c19098
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AreaOfEffectLevelUp_Prototype final : public TypedComponentPrototype<AreaOfEffectLevelUp> {
  public:
    AssetRefAreaOfEffectLevelUpSpec Spec{};

    // 0x7c192c8
    AreaOfEffectLevelUp_Prototype() = default;

    // 0x7c19170
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1922c
    void Materialize(Frame* frame, AreaOfEffectLevelUp& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c19238
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AreaOfEffectRaycastInfo_Prototype final
    : public TypedComponentPrototype<AreaOfEffectRaycastInfo> {
  public:
    // 0x7c194b0
    AreaOfEffectRaycastInfo_Prototype() = default;

    // 0x7c19310
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1941c
    void Materialize(Frame* frame, AreaOfEffectRaycastInfo& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c19420
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ArenaCannon_Prototype final : public TypedComponentPrototype<ArenaCannon> {
  public:
    // 0x7c1963c
    ArenaCannon_Prototype() = default;

    // 0x7c194f8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c195a8
    void Materialize(Frame* frame, ArenaCannon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c195ac
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class AttackInfo_Prototype final : public TypedComponentPrototype<AttackInfo> {
  public:
    // 0x7c197cc
    AttackInfo_Prototype() = default;

    // 0x7c19684
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c19738
    void Materialize(Frame* frame, AttackInfo& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1973c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BTAgent_Prototype final : public TypedComponentPrototype<BTAgent> {
  public:
    AssetRefBTRoot Tree{};
    AssetRefBTNode Current{};
    AssetRefAIConfig Config{};
    std::vector<std::uint8_t> NodesStatus{};
    std::vector<BTDataValue_Prototype> BTDataValues{};
    std::vector<AssetRefBTService> ActiveServices{};
    std::vector<FP> ServicesEndTimes{};
    std::vector<AssetRefBTComposite> DynamicComposites{};
    std::int32_t AbortNodeId{};

    // 0x7c1a148
    BTAgent_Prototype() = default;

    // 0x7c19814
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c19968
    void Materialize(Frame* frame, BTAgent& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1a0b8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BattleStatsActor_Prototype final : public TypedComponentPrototype<BattleStatsActor> {
  public:
    // 0x7c1a444
    BattleStatsActor_Prototype() = default;

    // 0x7c1a2a0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1a3b0
    void Materialize(Frame* frame, BattleStatsActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1a3b4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BigBoomMine_Prototype final : public TypedComponentPrototype<BigBoomMine> {
  public:
    AssetRefBigBoomMineSpec Spec{};

    // 0x7c1a638
    BigBoomMine_Prototype() = default;

    // 0x7c1a48c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1a59c
    void Materialize(Frame* frame, BigBoomMine& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1a5a8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BigBoomWeapon_Prototype final : public TypedComponentPrototype<BigBoomWeapon> {
  public:
    // 0x7c1a7c4
    BigBoomWeapon_Prototype() = default;

    // 0x7c1a680
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1a730
    void Materialize(Frame* frame, BigBoomWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1a734
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BlackboardInitializer_Prototype final
    : public TypedComponentPrototype<BlackboardInitializer> {
  public:
    AssetRefAIBlackboardInitializer BBInitializer{};

    // 0x7c1ad10
    BlackboardInitializer_Prototype() = default;

    // 0x7c1abb8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1ac74
    void Materialize(Frame* frame, BlackboardInitializer& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1ac80
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BlastBall_Prototype final : public TypedComponentPrototype<BlastBall> {
  public:
    AssetRefBlastBallSpec Spec{};

    // 0x7c1af2c
    BlastBall_Prototype() = default;

    // 0x7c1ad60
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1ae90
    void Materialize(Frame* frame, BlastBall& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1ae9c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BlastBallGame_Prototype final : public TypedComponentPrototype<BlastBallGame> {
  public:
    // 0x7c1b118
    BlastBallGame_Prototype() = default;

    // 0x7c1af74
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1b084
    void Materialize(Frame* frame, BlastBallGame& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1b088
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BlastBallGoal_Prototype final : public TypedComponentPrototype<BlastBallGoal> {
  public:
    TeamFlag OwnerTeam{};
    MapEntityId ballEntity{};
    QBoolean ballInside{};

    // 0x7c1b36c
    BlastBallGoal_Prototype() = default;

    // 0x7c1b160
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1b29c
    void Materialize(Frame* frame, BlastBallGoal& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1b2dc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BlastBallPlayer_Prototype final : public TypedComponentPrototype<BlastBallPlayer> {
  public:
    // 0x7c1b4f8
    BlastBallPlayer_Prototype() = default;

    // 0x7c1b3b4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1b464
    void Materialize(Frame* frame, BlastBallPlayer& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1b468
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BlasterMoveSpeedEnhancement_Prototype final
    : public TypedComponentPrototype<BlasterMoveSpeedEnhancement> {
  public:
    // 0x7c1b688
    BlasterMoveSpeedEnhancement_Prototype() = default;

    // 0x7c1b540
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1b5f4
    void Materialize(Frame* frame, BlasterMoveSpeedEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1b5f8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BlasterSpecialShotEnhancement_Prototype final
    : public TypedComponentPrototype<BlasterSpecialShotEnhancement> {
  public:
    // 0x7c1b86c
    BlasterSpecialShotEnhancement_Prototype() = default;

    // 0x7c1b6d0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1b7d8
    void Materialize(Frame* frame, BlasterSpecialShotEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1b7dc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BoltCounterElement_Prototype final : public TypedComponentPrototype<BoltCounterElement> {
  public:
    std::int32_t requiredBolts{};
    AssetRefLevelButtonGameplayElementConfig gameplayElementConfig{};

    // 0x7c1ba6c
    BoltCounterElement_Prototype() = default;

    // 0x7c1b8b4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1b9c8
    void Materialize(Frame* frame, BoltCounterElement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1b9dc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BoltGathererActor_Prototype final : public TypedComponentPrototype<BoltGathererActor> {
  public:
    AssetRefBoltGathererActorSpec Spec{};

    // 0x7c1bc60
    BoltGathererActor_Prototype() = default;

    // 0x7c1bab4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1bbc4
    void Materialize(Frame* frame, BoltGathererActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1bbd0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BoltPowerActor_Prototype final : public TypedComponentPrototype<BoltPowerActor> {
  public:
    AssetRefBoltPowerActorSpec Spec{};

    // 0x7c1be04
    BoltPowerActor_Prototype() = default;

    // 0x7c1bca8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1bd68
    void Materialize(Frame* frame, BoltPowerActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1bd74
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BombardierDrone_Prototype final : public TypedComponentPrototype<BombardierDrone> {
  public:
    AssetRefBombardierDroneSpec Spec{};

    // 0x7c1c000
    BombardierDrone_Prototype() = default;

    // 0x7c1be4c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1bf64
    void Materialize(Frame* frame, BombardierDrone& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1bf70
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BombardierWeapon_Prototype final : public TypedComponentPrototype<BombardierWeapon> {
  public:
    AssetRefBombardierWeaponAutoTargetSpec Spec{};

    // 0x7c1c1a0
    BombardierWeapon_Prototype() = default;

    // 0x7c1c048
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1c104
    void Materialize(Frame* frame, BombardierWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1c110
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Bot_Prototype final : public TypedComponentPrototype<Bot> {
  public:
    // 0x7c1c38c
    Bot_Prototype() = default;

    // 0x7c1c1e8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1c2f8
    void Materialize(Frame* frame, Bot& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1c2fc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Bumper_Prototype final : public TypedComponentPrototype<Bumper> {
  public:
    AssetRefBumperConfig config{};
    FP impulseStrength{};
    FPVector3 impulseDirection{};

    // 0x7c1c5a8
    Bumper_Prototype() = default;

    // 0x7c1c3d4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1c4f4
    void Materialize(Frame* frame, Bumper& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1c518
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BumperBar_Prototype final : public TypedComponentPrototype<BumperBar> {
  public:
    AssetRefBumperBarConfig Config{};

    // 0x7c1c74c
    BumperBar_Prototype() = default;

    // 0x7c1c5f0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1c6b0
    void Materialize(Frame* frame, BumperBar& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1c6bc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class BuzzbladesRicochet_Prototype final : public TypedComponentPrototype<BuzzbladesRicochet> {
  public:
    AssetRefBuzzbladesRicochetSpec Spec{};

    // 0x7c1c8f0
    BuzzbladesRicochet_Prototype() = default;

    // 0x7c1c794
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1c854
    void Materialize(Frame* frame, BuzzbladesRicochet& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1c860
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class CharacterColliderScaler_Prototype final
    : public TypedComponentPrototype<CharacterColliderScaler> {
  public:
    // 0x7c1cad8
    CharacterColliderScaler_Prototype() = default;

    // 0x7c1c938
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1ca44
    void Materialize(Frame* frame, CharacterColliderScaler& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1ca48
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ChargeAttack_Prototype final : public TypedComponentPrototype<ChargeAttack> {
  public:
    AssetRefChargeAttackSpec Spec{};

    // 0x7c1ccd0
    ChargeAttack_Prototype() = default;

    // 0x7c1cb20
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1cc34
    void Materialize(Frame* frame, ChargeAttack& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1cc40
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Checkpoint_Prototype final : public TypedComponentPrototype<Checkpoint> {
  public:
    // 0x7c1ce5c
    Checkpoint_Prototype() = default;

    // 0x7c1cd18
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1cdc8
    void Materialize(Frame* frame, Checkpoint& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1cdcc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ColliderResize_Prototype final : public TypedComponentPrototype<ColliderResize> {
  public:
    AssetRefColliderResizeSpec Spec{};

    // 0x7c1d068
    ColliderResize_Prototype() = default;

    // 0x7c1cea4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1cfcc
    void Materialize(Frame* frame, ColliderResize& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1cfd8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class CollisionForce_Prototype final : public TypedComponentPrototype<CollisionForce> {
  public:
    AssetRefCollisionForceSpec Spec{};

    // 0x7c1d264
    CollisionForce_Prototype() = default;

    // 0x7c1d0b0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1d1c8
    void Materialize(Frame* frame, CollisionForce& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1d1d4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class CombatParticipant_Prototype final : public TypedComponentPrototype<CombatParticipant> {
  public:
    // 0x7c1d3f0
    CombatParticipant_Prototype() = default;

    // 0x7c1d2ac
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1d35c
    void Materialize(Frame* frame, CombatParticipant& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1d360
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class CompoundHFSM_Prototype final : public TypedComponentPrototype<CompoundHFSM> {
  public:
    AssetRefCompoundHFSMSpec spec{};

    // 0x7c1d5fc
    CompoundHFSM_Prototype() = default;

    // 0x7c1d438
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1d560
    void Materialize(Frame* frame, CompoundHFSM& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1d56c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Cryoslider_Prototype final : public TypedComponentPrototype<Cryoslider> {
  public:
    AssetRefCryosliderSpec Spec{};

    // 0x7c1d7f0
    Cryoslider_Prototype() = default;

    // 0x7c1d644
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1d754
    void Materialize(Frame* frame, Cryoslider& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1d760
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class CryosliderWeapon_Prototype final : public TypedComponentPrototype<CryosliderWeapon> {
  public:
    AssetRefCryosliderWeaponAutoTargetSpec Spec{};

    // 0x7c1dbf4
    CryosliderWeapon_Prototype() = default;

    // 0x7c1da9c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1db58
    void Materialize(Frame* frame, CryosliderWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1db64
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DamageChargeable_Prototype final : public TypedComponentPrototype<DamageChargeable> {
  public:
    AssetRefDamageChargeableSpec Spec{};

    // 0x7c1dde8
    DamageChargeable_Prototype() = default;

    // 0x7c1dc3c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1dd4c
    void Materialize(Frame* frame, DamageChargeable& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1dd58
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DamageIncreasedOnTargetsEnhancement_Prototype final
    : public TypedComponentPrototype<DamageIncreasedOnTargetsEnhancement> {
  public:
    // 0x7c1df78
    DamageIncreasedOnTargetsEnhancement_Prototype() = default;

    // 0x7c1de30
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1dee4
    void Materialize(Frame* frame, DamageIncreasedOnTargetsEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1dee8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DamageMultiplier_Prototype final : public TypedComponentPrototype<DamageMultiplier> {
  public:
    AssetRefDamageMultiplierSpec Spec{};

    // 0x7c1e118
    DamageMultiplier_Prototype() = default;

    // 0x7c1dfc0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1e07c
    void Materialize(Frame* frame, DamageMultiplier& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1e088
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DamageMultiplierHeroImprovement_Prototype final
    : public TypedComponentPrototype<DamageMultiplierHeroImprovement> {
  public:
    // 0x7c1e2a4
    DamageMultiplierHeroImprovement_Prototype() = default;

    // 0x7c1e160
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1e210
    void Materialize(Frame* frame, DamageMultiplierHeroImprovement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1e214
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DamageSource_Prototype final : public TypedComponentPrototype<DamageSource> {
  public:
    AssetRefDamageSpec Spec{};
    AssetRefAmmunitionDamageSpec AmmunitionSpec{};
    QBoolean hitTrackingDone{};

    // 0x7c1e4b0
    DamageSource_Prototype() = default;

    // 0x7c1e2ec
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1e404
    void Materialize(Frame* frame, DamageSource& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1e420
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DamageToImpulseConverter_Prototype final
    : public TypedComponentPrototype<DamageToImpulseConverter> {
  public:
    AssetRefDamageToImpulseConverterSpec Spec{};

    // 0x7c1e6a8
    DamageToImpulseConverter_Prototype() = default;

    // 0x7c1e4f8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1e60c
    void Materialize(Frame* frame, DamageToImpulseConverter& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1e618
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Damageable_Prototype final : public TypedComponentPrototype<Damageable> {
  public:
    AssetRefDamageableSpec Spec{};

    // 0x7c1e8bc
    Damageable_Prototype() = default;

    // 0x7c1e6f0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1e820
    void Materialize(Frame* frame, Damageable& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1e82c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DashActor_Prototype final : public TypedComponentPrototype<DashActor> {
  public:
    AssetRefDashActorConfig Spec{};

    // 0x7c1eac8
    DashActor_Prototype() = default;

    // 0x7c1e904
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1ea2c
    void Materialize(Frame* frame, DashActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1ea38
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DrillDash_Prototype final : public TypedComponentPrototype<DrillDash> {
  public:
    // 0x7c1ecb0
    DrillDash_Prototype() = default;

    // 0x7c1eb10
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1ec1c
    void Materialize(Frame* frame, DrillDash& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1ec20
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DroppableCollector_Prototype final : public TypedComponentPrototype<DroppableCollector> {
  public:
    AssetRefDroppableCollectorSpec Spec{};

    // 0x7c1eea4
    DroppableCollector_Prototype() = default;

    // 0x7c1ecf8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1ee08
    void Materialize(Frame* frame, DroppableCollector& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1ee14
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DroppableEntity_Prototype final : public TypedComponentPrototype<DroppableEntity> {
  public:
    // 0x7c1f090
    DroppableEntity_Prototype() = default;

    // 0x7c1eeec
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1effc
    void Materialize(Frame* frame, DroppableEntity& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1f000
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class DroppableManager_Prototype final : public TypedComponentPrototype<DroppableManager> {
  public:
    AssetRefDroppableManagerSpec Spec{};

    // 0x7c1f284
    DroppableManager_Prototype() = default;

    // 0x7c1f0d8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1f1e8
    void Materialize(Frame* frame, DroppableManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1f1f4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ElementalBuildUpMultiplier_Prototype final
    : public TypedComponentPrototype<ElementalBuildUpMultiplier> {
  public:
    // 0x7c1f468
    ElementalBuildUpMultiplier_Prototype() = default;

    // 0x7c1f2cc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1f3d4
    void Materialize(Frame* frame, ElementalBuildUpMultiplier& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1f3d8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class EliminationGameMode_Prototype final : public TypedComponentPrototype<EliminationGameMode> {
  public:
    AssetRefEliminationGameModeSpec Spec{};

    // 0x7c1f60c
    EliminationGameMode_Prototype() = default;

    // 0x7c1f4b0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1f570
    void Materialize(Frame* frame, EliminationGameMode& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1f57c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class EnergyCellManager_Prototype final : public TypedComponentPrototype<EnergyCellManager> {
  public:
    AssetRefEnergyCellManagerSpec Spec{};

    // 0x7c1f7ac
    EnergyCellManager_Prototype() = default;

    // 0x7c1f654
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1f710
    void Materialize(Frame* frame, EnergyCellManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1f71c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class EnforcerFirstShotBonusEnhancement_Prototype final
    : public TypedComponentPrototype<EnforcerFirstShotBonusEnhancement> {
  public:
    // 0x7c1f938
    EnforcerFirstShotBonusEnhancement_Prototype() = default;

    // 0x7c1f7f4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1f8a4
    void Materialize(Frame* frame, EnforcerFirstShotBonusEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1f8a8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class EntityDropper_Prototype final : public TypedComponentPrototype<EntityDropper> {
  public:
    std::vector<AssetRefEntityDropperSpec> Specs{};

    // 0x7c1fc80
    EntityDropper_Prototype() = default;

    // 0x7c1f980
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1faa8
    void Materialize(Frame* frame, EntityDropper& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1fbf0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class EntityInfo_Prototype final : public TypedComponentPrototype<Code::EntityInfo> {
  public:
    EntityInfoType_Prototype type{};
    std::int32_t id{};
    QBoolean isActiveObjective{};

    // 0x7c1fe6c
    EntityInfo_Prototype() = default;

    // 0x7c1fd00
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1fdc0
    void Materialize(Frame* frame, Code::EntityInfo& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1fddc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class FireReactor_Prototype final : public TypedComponentPrototype<FireReactor> {
  public:
    AssetRefFireReactorSpec Spec{};

    // 0x7c20060
    FireReactor_Prototype() = default;

    // 0x7c1feb4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c1ffc4
    void Materialize(Frame* frame, FireReactor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c1ffd0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GOAPAgent_Prototype final : public TypedComponentPrototype<GOAPAgent> {
  public:
    AssetRefGOAPRoot Root{};
    AssetRefAIConfig Config{};

    // 0x7c20274
    GOAPAgent_Prototype() = default;

    // 0x7c200a8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c201d0
    void Materialize(Frame* frame, GOAPAgent& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c201e4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GOAPData_Prototype final : public TypedComponentPrototype<GOAPData> {
  public:
    AssetRefGOAPHeuristic HeuristicCost{};

    // 0x7c20414
    GOAPData_Prototype() = default;

    // 0x7c202bc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c20378
    void Materialize(Frame* frame, GOAPData& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c20384
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GadgetUser_Prototype final : public TypedComponentPrototype<GadgetUser> {
  public:
    // 0x7c205f8
    GadgetUser_Prototype() = default;

    // 0x7c2045c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c20564
    void Materialize(Frame* frame, GadgetUser& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c20568
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GameManager_Prototype final : public TypedComponentPrototype<GameManager> {
  public:
    std::int32_t nbPlayersSpawned{};
    AssetRefGameManagerConfig config{};

    // 0x7c207a4
    GameManager_Prototype() = default;

    // 0x7c20640
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c20700
    void Materialize(Frame* frame, GameManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c20714
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GameModeActivator_Prototype final : public TypedComponentPrototype<GameModeActivator> {
  public:
    GAME_MODIFICATOR_Prototype modificator{};
    QBoolean enabled{};

    // 0x7c2094c
    GameModeActivator_Prototype() = default;

    // 0x7c207ec
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c208a8
    void Materialize(Frame* frame, GameModeActivator& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c208bc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GameModeHandler_Prototype final : public TypedComponentPrototype<GameModeHandler> {
  public:
    AssetRefGameModeHandlerConfig config{};
    AssetRefGameModeHandlerLevelConfig levelConfig{};
    AssetRefGameModesTable gameModesTable{};

    // 0x7c20b8c
    GameModeHandler_Prototype() = default;

    // 0x7c20994
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c20ae0
    void Materialize(Frame* frame, GameModeHandler& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c20afc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GameModifiersManager_Prototype final : public TypedComponentPrototype<GameModifiersManager> {
  public:
    AssetRefGameModifiersSpec Spec{};

    // 0x7c20d2c
    GameModifiersManager_Prototype() = default;

    // 0x7c20bd4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c20c90
    void Materialize(Frame* frame, GameModifiersManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c20c9c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GatlingWeapon_Prototype final : public TypedComponentPrototype<GatlingWeapon> {
  public:
    // 0x7c20ebc
    GatlingWeapon_Prototype() = default;

    // 0x7c20d74
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c20e28
    void Materialize(Frame* frame, GatlingWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c20e2c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GloveOfDoomWeapon_Prototype final : public TypedComponentPrototype<GloveOfDoomWeapon> {
  public:
    // 0x7c21048
    GloveOfDoomWeapon_Prototype() = default;

    // 0x7c20f04
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c20fb4
    void Materialize(Frame* frame, GloveOfDoomWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c20fb8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GoldenBolt_Prototype final : public TypedComponentPrototype<GoldenBolt> {
  public:
    QBoolean isCollected{};

    // 0x7c211e8
    GoldenBolt_Prototype() = default;

    // 0x7c21090
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2114c
    void Materialize(Frame* frame, GoldenBolt& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c21158
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GoldenBoltManager_Prototype final : public TypedComponentPrototype<GoldenBoltManager> {
  public:
    // 0x7c21374
    GoldenBoltManager_Prototype() = default;

    // 0x7c21230
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c212e0
    void Materialize(Frame* frame, GoldenBoltManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c212e4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GrinderActor_Prototype final : public TypedComponentPrototype<GrinderActor> {
  public:
    AssetRefGrinderActorConfig config{};

    // 0x7c2158c
    GrinderActor_Prototype() = default;

    // 0x7c213bc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c214f0
    void Materialize(Frame* frame, GrinderActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c214fc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class GroundDashActor_Prototype final : public TypedComponentPrototype<GroundDashActor> {
  public:
    AssetRefGroundDashActorConfig Spec{};

    // 0x7c21780
    GroundDashActor_Prototype() = default;

    // 0x7c215d4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c216e4
    void Materialize(Frame* frame, GroundDashActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c216f0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HFSMAgent_Prototype final : public TypedComponentPrototype<HFSMAgent> {
  public:
    HFSMData_Prototype Data{};
    AssetRefAIConfig Config{};

    // 0x7c21ad0
    HFSMAgent_Prototype() = default;

    // 0x7c217c8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c21924
    void Materialize(Frame* frame, HFSMAgent& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c21a40
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HeadHunterWallHackEnhancement_Prototype final
    : public TypedComponentPrototype<HeadHunterWallHackEnhancement> {
  public:
    AssetRefHeadHunterWallHackEnhancementSpec Spec{};

    // 0x7c21cd4
    HeadHunterWallHackEnhancement_Prototype() = default;

    // 0x7c21b7c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c21c38
    void Materialize(Frame* frame, HeadHunterWallHackEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c21c44
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HealSource_Prototype final : public TypedComponentPrototype<HealSource> {
  public:
    AssetRefHealSourceSpec Spec{};

    // 0x7c21e74
    HealSource_Prototype() = default;

    // 0x7c21d1c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c21dd8
    void Materialize(Frame* frame, HealSource& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c21de4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Health_Prototype final : public TypedComponentPrototype<Health> {
  public:
    AssetRefHealthSpec Spec{};

    // 0x7c22080
    Health_Prototype() = default;

    // 0x7c21ebc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c21fe4
    void Materialize(Frame* frame, Health& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c21ff0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HelipackActor_Prototype final : public TypedComponentPrototype<HelipackActor> {
  public:
    AssetRefHelipackActorSpec Spec{};

    // 0x7c22278
    HelipackActor_Prototype() = default;

    // 0x7c220c8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c221dc
    void Materialize(Frame* frame, HelipackActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c221e8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Hierarchy_Prototype final : public TypedComponentPrototype<Hierarchy> {
  public:
    MapEntityId Parent{};
    FPVector3 Offset{};

    // 0x7c224e4
    Hierarchy_Prototype() = default;

    // 0x7c222c0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c22434
    void Materialize(Frame* frame, Hierarchy& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c22454
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HoloshieldGloveHealZoneEnhancement_Prototype final
    : public TypedComponentPrototype<HoloshieldGloveHealZoneEnhancement> {
  public:
    // 0x7c22670
    HoloshieldGloveHealZoneEnhancement_Prototype() = default;

    // 0x7c2252c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c225dc
    void Materialize(Frame* frame, HoloshieldGloveHealZoneEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c225e0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HoloshieldGloveShield_Prototype final
    : public TypedComponentPrototype<HoloshieldGloveShield> {
  public:
    // 0x7c22800
    HoloshieldGloveShield_Prototype() = default;

    // 0x7c226b8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2276c
    void Materialize(Frame* frame, HoloshieldGloveShield& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c22770
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HoloshieldGloveShields_Prototype final
    : public TypedComponentPrototype<HoloshieldGloveShields> {
  public:
    // 0x7c2298c
    HoloshieldGloveShields_Prototype() = default;

    // 0x7c22848
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c228f8
    void Materialize(Frame* frame, HoloshieldGloveShields& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c228fc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HoloshieldGloveShieldsGrouped_Prototype final
    : public TypedComponentPrototype<HoloshieldGloveShieldsGrouped> {
  public:
    // 0x7c22b18
    HoloshieldGloveShieldsGrouped_Prototype() = default;

    // 0x7c229d4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c22a84
    void Materialize(Frame* frame, HoloshieldGloveShieldsGrouped& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c22a88
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HoloshieldGloveWeapon_Prototype final
    : public TypedComponentPrototype<HoloshieldGloveWeapon> {
  public:
    // 0x7c22ca4
    HoloshieldGloveWeapon_Prototype() = default;

    // 0x7c22b60
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c22c10
    void Materialize(Frame* frame, HoloshieldGloveWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c22c14
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class HoverBootsWeapon_Prototype final : public TypedComponentPrototype<HoverBootsWeapon> {
  public:
    // 0x7c22e30
    HoverBootsWeapon_Prototype() = default;

    // 0x7c22cec
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c22d9c
    void Materialize(Frame* frame, HoverBootsWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c22da0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ItemSpawner_Prototype final : public TypedComponentPrototype<ItemSpawner> {
  public:
    AssetRefItemSpawnerConfig Config{};
    std::int32_t spawnedItemId{};

    // 0x7c2303c
    ItemSpawner_Prototype() = default;

    // 0x7c22e78
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c22f98
    void Materialize(Frame* frame, ItemSpawner& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c22fac
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class JumpAttack_Prototype final : public TypedComponentPrototype<JumpAttack> {
  public:
    AssetRefJumpAttackSpec Spec{};

    // 0x7c23240
    JumpAttack_Prototype() = default;

    // 0x7c23084
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c231a4
    void Materialize(Frame* frame, JumpAttack& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c231b0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class KccExtension_Prototype final : public TypedComponentPrototype<KccExtension> {
  public:
    AssetRefKccExtensionConfig Config{};

    // 0x7c23460
    KccExtension_Prototype() = default;

    // 0x7c23288
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c233c4
    void Materialize(Frame* frame, KccExtension& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c233d0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class KccModifier_Prototype final : public TypedComponentPrototype<KccModifier> {
  public:
    AssetRefKccModifierSpec Spec{};

    // 0x7c23600
    KccModifier_Prototype() = default;

    // 0x7c234a8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c23564
    void Materialize(Frame* frame, KccModifier& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c23570
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class KillZone_Prototype final : public TypedComponentPrototype<KillZone> {
  public:
    std::vector<KillZoneArea_Prototype> areas{};
    FP verticalThreshold{};

    // 0x7c23a10
    KillZone_Prototype() = default;

    // 0x7c23648
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c23770
    void Materialize(Frame* frame, KillZone& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c23980
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Killable_Prototype final : public TypedComponentPrototype<Killable> {
  public:
    AssetRefDeathAnimationSpec deathAnimationSpec{};
    KillableOnKillBehaviour OnKillBehaviour{};
    FP DelayBeforeDestruction{};
    bool OnKillDisablePhysics = true;
    bool OnKillDisableCollider = true;
    AssetRefEntityPrototype EntityToSpawnOnDeath{};

    // 0x7c23cac
    Killable_Prototype() = default;

    // 0x7c23ab8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c23bf4
    void Materialize(Frame* frame, Killable& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c23a98 - the component keeps a reference to this prototype.
    void MaterializeUser(Frame* frame, Killable& result,
                         const PrototypeMaterializationContext& context) const;
    // 0x7c23c1c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class LedgeGrabbingActor_Prototype final : public TypedComponentPrototype<LedgeGrabbingActor> {
  public:
    AssetRefLedgeGrabbingSpec Spec{};

    // 0x7c23ebc
    LedgeGrabbingActor_Prototype() = default;

    // 0x7c23d00
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c23e20
    void Materialize(Frame* frame, LedgeGrabbingActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c23e2c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class LevelButton_Prototype final : public TypedComponentPrototype<LevelButton> {
  public:
    MapEntityId gameplayElementEntity{};
    AssetRefLevelButtonConfig Config{};

    // 0x7c24120
    LevelButton_Prototype() = default;

    // 0x7c23f04
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c24074
    void Materialize(Frame* frame, LevelButton& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c24090
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class LevelButtonGameplayElement_Prototype final
    : public TypedComponentPrototype<LevelButtonGameplayElement> {
  public:
    AssetRefLevelButtonGameplayElementConfig Config{};

    // 0x7c242c0
    LevelButtonGameplayElement_Prototype() = default;

    // 0x7c24168
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c24224
    void Materialize(Frame* frame, LevelButtonGameplayElement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c24230
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class LevelHolder_Prototype final : public TypedComponentPrototype<LevelHolder> {
  public:
    // 0x7c2444c
    LevelHolder_Prototype() = default;

    // 0x7c24308
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c243b8
    void Materialize(Frame* frame, LevelHolder& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c243bc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class LevelUpActor_Prototype final : public TypedComponentPrototype<LevelUpActor> {
  public:
    AssetRefLevelUpActorSpec Spec{};

    // 0x7c245ec
    LevelUpActor_Prototype() = default;

    // 0x7c24494
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c24550
    void Materialize(Frame* frame, LevelUpActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2455c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Lifetime_Prototype final : public TypedComponentPrototype<Lifetime> {
  public:
    AssetRefLifetimeSpec Spec{};

    // 0x7c247e0
    Lifetime_Prototype() = default;

    // 0x7c24634
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c24744
    void Materialize(Frame* frame, Lifetime& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c24750
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MeleeAttack_Prototype final : public TypedComponentPrototype<MeleeAttack> {
  public:
    AssetRefMeleeAttackSpec Spec{};

    // 0x7c249dc
    MeleeAttack_Prototype() = default;

    // 0x7c24828
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c24940
    void Materialize(Frame* frame, MeleeAttack& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2494c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MeleeJumpAttack_Prototype final : public TypedComponentPrototype<MeleeJumpAttack> {
  public:
    AssetRefMeleeJumpAttackSpec spec{};

    // 0x7c24b80
    MeleeJumpAttack_Prototype() = default;

    // 0x7c24a24
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c24ae4
    void Materialize(Frame* frame, MeleeJumpAttack& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c24af0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MineLauncherEnhancement_Prototype final
    : public TypedComponentPrototype<MineLauncherEnhancement> {
  public:
    AssetRefWeaponEnhancementSpec Spec{};

    // 0x7c24d20
    MineLauncherEnhancement_Prototype() = default;

    // 0x7c24bc8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c24c84
    void Materialize(Frame* frame, MineLauncherEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c24c90
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MineMerger_Prototype final : public TypedComponentPrototype<MineMerger> {
  public:
    std::int32_t mergeCount{};
    QBoolean wasMergeChecked{};

    // 0x7c24ec8
    MineMerger_Prototype() = default;

    // 0x7c24d68
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c24e24
    void Materialize(Frame* frame, MineMerger& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c24e38
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MobGroup_Prototype final : public TypedComponentPrototype<MobGroup> {
  public:
    AssetRefMobGroupConfig SwarmerConfig{};
    std::int32_t NumToSpawn{};

    // 0x7c250cc
    MobGroup_Prototype() = default;

    // 0x7c24f10
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c25028
    void Materialize(Frame* frame, MobGroup& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2503c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MobGroupMember_Prototype final : public TypedComponentPrototype<MobGroupMember> {
  public:
    // 0x7c25258
    MobGroupMember_Prototype() = default;

    // 0x7c25114
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c251c4
    void Materialize(Frame* frame, MobGroupMember& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c251c8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MobInfo_Prototype final : public TypedComponentPrototype<MobInfo> {
  public:
    MobType_Prototype Type{};
    AssetRefMobInfoSpec Spec{};
    QBoolean UseOwnerLevel{};

    // 0x7c25414
    MobInfo_Prototype() = default;

    // 0x7c252a0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c25368
    void Materialize(Frame* frame, MobInfo& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c25384
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MoveTo_Prototype final : public TypedComponentPrototype<MoveTo> {
  public:
    AssetRefMoveToSpec moveToSpec{};
    std::vector<AssetRefCallbackAction> onMoveFinished{};

    // 0x7c25788
    MoveTo_Prototype() = default;

    // 0x7c2545c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c255a8
    void Materialize(Frame* frame, MoveTo& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c256f8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MovementPackCache_Prototype final : public TypedComponentPrototype<MovementPackCache> {
  public:
    // 0x7c259ac
    MovementPackCache_Prototype() = default;

    // 0x7c25808
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c25918
    void Materialize(Frame* frame, MovementPackCache& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2591c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class MovingWall_Prototype final : public TypedComponentPrototype<MovingWall> {
  public:
    AssetRefMovingWallConfig Config{};
    std::int32_t navmeshRegionID{};

    // 0x7c25bbc
    MovingWall_Prototype() = default;

    // 0x7c259f4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c25b18
    void Materialize(Frame* frame, MovingWall& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c25b2c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class OnKillCallbacks_Prototype final : public TypedComponentPrototype<OnKillCallbacks> {
  public:
    std::vector<AssetRefCallbackAction> onPlayerKilled{};

    // 0x7c25ebc
    OnKillCallbacks_Prototype() = default;

    // 0x7c25c04
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c25ce4
    void Materialize(Frame* frame, OnKillCallbacks& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c25e2c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class OnTriggerEnter_Prototype final : public TypedComponentPrototype<OnTriggerEnter> {
  public:
    std::vector<AssetRefCallbackAction> onTriggerEnter{};
    ENTITY_INFO_FILTER EntityFilter = static_cast<ENTITY_INFO_FILTER>(-1);
    bool CanBeTriggeredByEnemies{};
    bool CanBeTriggeredByTeammates{};
    bool CanBeTriggeredByInstigator{};
    bool SendTriggeringEntityAsParameter{};
    FP ActivationDelay{};

    // 0x7c2628c
    OnTriggerEnter_Prototype() = default;

    // 0x7c25f5c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c260ac
    void Materialize(Frame* frame, OnTriggerEnter& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c25f3c - the component keeps a reference to this prototype.
    void MaterializeUser(Frame* frame, OnTriggerEnter& result,
                         const PrototypeMaterializationContext& context) const;
    // 0x7c261fc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class PlayerActor_Prototype final : public TypedComponentPrototype<PlayerActor> {
  public:
    FP noActionPushForceThreshold{};
    FP noActionMaxPushForce{};
    FP noActionTimeAfterPush{};
    FP maxNoActionTimeAfterPush{};
    FP durationBeforeCombatReset{};

    // 0x7c26510
    PlayerActor_Prototype() = default;

    // 0x7c26314
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c26454
    void Materialize(Frame* frame, PlayerActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c26480
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class PowerUp_Prototype final : public TypedComponentPrototype<PowerUp> {
  public:
    AssetRefPowerUpSpec Spec{};
    QBoolean IsPickedUp{};

    // 0x7c266bc
    PowerUp_Prototype() = default;

    // 0x7c26558
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c26618
    void Materialize(Frame* frame, PowerUp& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2662c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class PowerUpUser_Prototype final : public TypedComponentPrototype<PowerUpUser> {
  public:
    // 0x7c26848
    PowerUpUser_Prototype() = default;

    // 0x7c26704
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c267b4
    void Materialize(Frame* frame, PowerUpUser& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c267b8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class PreciseHitReceiver_Prototype final : public TypedComponentPrototype<PreciseHitReceiver> {
  public:
    AssetRefPreciseHitReceiverSpec Spec{};

    // 0x7c269e8
    PreciseHitReceiver_Prototype() = default;

    // 0x7c26890
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2694c
    void Materialize(Frame* frame, PreciseHitReceiver& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c26958
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class PrioritisedTargetHandler_Prototype final
    : public TypedComponentPrototype<PrioritisedTargetHandler> {
  public:
    AssetRefPrioritisedTargetHandlerSpec spec{};

    // 0x7c26bf4
    PrioritisedTargetHandler_Prototype() = default;

    // 0x7c26a30
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c26b58
    void Materialize(Frame* frame, PrioritisedTargetHandler& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c26b64
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Projectile_Prototype final : public TypedComponentPrototype<Projectile> {
  public:
    AssetRefProjectileSpec Spec{};

    // 0x7c26e0c
    Projectile_Prototype() = default;

    // 0x7c26c3c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c26d70
    void Materialize(Frame* frame, Projectile& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c26d7c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ProjectileSpawner_Prototype final : public TypedComponentPrototype<ProjectileSpawner> {
  public:
    AssetRefProjectileSpawnerSpec Spec{};

    // 0x7c27010
    ProjectileSpawner_Prototype() = default;

    // 0x7c26e54
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c26f74
    void Materialize(Frame* frame, ProjectileSpawner& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c26f80
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Pushable_Prototype final : public TypedComponentPrototype<Pushable> {
  public:
    AssetRefPushableSpec Spec{};

    // 0x7c27214
    Pushable_Prototype() = default;

    // 0x7c27058
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c27178
    void Materialize(Frame* frame, Pushable& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c27184
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class PushableTrigger_Prototype final : public TypedComponentPrototype<PushableTrigger> {
  public:
    // 0x7c273a0
    PushableTrigger_Prototype() = default;

    // 0x7c2725c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2730c
    void Materialize(Frame* frame, PushableTrigger& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c27310
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Pusher_Prototype final : public TypedComponentPrototype<Pusher> {
  public:
    AssetRefPusherSpec Spec{};

    // 0x7c27544
    Pusher_Prototype() = default;

    // 0x7c273e8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c274a8
    void Materialize(Frame* frame, Pusher& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c274b4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Rail_Prototype final : public TypedComponentPrototype<Rail> {
  public:
    std::int32_t bakedRailIndex{};

    // 0x7c276e8
    Rail_Prototype() = default;

    // 0x7c2758c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2764c
    void Materialize(Frame* frame, Rail& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c27658
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class RailCollider_Prototype final : public TypedComponentPrototype<RailCollider> {
  public:
    MapEntityId railEntity{};
    FP distanceOnRail{};
    MapEntityId nearbyRailColLeft{};
    MapEntityId nearbyRailColRight{};

    // 0x7c2796c
    RailCollider_Prototype() = default;

    // 0x7c27730
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2787c
    void Materialize(Frame* frame, RailCollider& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c278dc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class RankingManager_Prototype final : public TypedComponentPrototype<RankingManager> {
  public:
    // 0x7c27afc
    RankingManager_Prototype() = default;

    // 0x7c279b4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c27a68
    void Materialize(Frame* frame, RankingManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c27a6c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class RecoverAmmoOnDashEnhancement_Prototype final
    : public TypedComponentPrototype<RecoverAmmoOnDashEnhancement> {
  public:
    AssetRefRecoverAmmoOnDashEnhancementSpec Spec{};

    // 0x7c27c9c
    RecoverAmmoOnDashEnhancement_Prototype() = default;

    // 0x7c27b44
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c27c00
    void Materialize(Frame* frame, RecoverAmmoOnDashEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c27c0c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ReplaceMultiShotConfigsEnhancement_Prototype final
    : public TypedComponentPrototype<ReplaceMultiShotConfigsEnhancement> {
  public:
    // 0x7c27e28
    ReplaceMultiShotConfigsEnhancement_Prototype() = default;

    // 0x7c27ce4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c27d94
    void Materialize(Frame* frame, ReplaceMultiShotConfigsEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c27d98
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class RewardReceiver_Prototype final : public TypedComponentPrototype<RewardReceiver> {
  public:
    AssetRefRewardReceiverSpec Spec{};

    // 0x7c27fcc
    RewardReceiver_Prototype() = default;

    // 0x7c27e70
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c27f30
    void Materialize(Frame* frame, RewardReceiver& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c27f3c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Roll_Prototype final : public TypedComponentPrototype<Roll> {
  public:
    AssetRefRollSpec Spec{};

    // 0x7c281c8
    Roll_Prototype() = default;

    // 0x7c28014
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2812c
    void Materialize(Frame* frame, Roll& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c28138
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class RunnableWall_Prototype final : public TypedComponentPrototype<RunnableWall> {
  public:
    // 0x7c28354
    RunnableWall_Prototype() = default;

    // 0x7c28210
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c282c0
    void Materialize(Frame* frame, RunnableWall& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c282c4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class RunnableWallTriggerZone_Prototype final
    : public TypedComponentPrototype<RunnableWallTriggerZone> {
  public:
    // 0x7c284e0
    RunnableWallTriggerZone_Prototype() = default;

    // 0x7c2839c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2844c
    void Materialize(Frame* frame, RunnableWallTriggerZone& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c28450
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class RynoWeapon_Prototype final : public TypedComponentPrototype<RynoWeapon> {
  public:
    // 0x7c286c8
    RynoWeapon_Prototype() = default;

    // 0x7c28528
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c28634
    void Materialize(Frame* frame, RynoWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c28638
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ShapeOverlapper_Prototype final : public TypedComponentPrototype<ShapeOverlapper> {
  public:
    AssetRefShapeOverlapperSpec Spec{};

    // 0x7c2886c
    ShapeOverlapper_Prototype() = default;

    // 0x7c28710
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c287d0
    void Materialize(Frame* frame, ShapeOverlapper& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c287dc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ShieldedActor_Prototype final : public TypedComponentPrototype<ShieldedActor> {
  public:
    AssetRefShieldedActorSpec spec{};

    // 0x7c28a74
    ShieldedActor_Prototype() = default;

    // 0x7c288b4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c289d8
    void Materialize(Frame* frame, ShieldedActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c289e4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ShotgunBerserkerEnhancement_Prototype final
    : public TypedComponentPrototype<ShotgunBerserkerEnhancement> {
  public:
    // 0x7c28c04
    ShotgunBerserkerEnhancement_Prototype() = default;

    // 0x7c28abc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c28b70
    void Materialize(Frame* frame, ShotgunBerserkerEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c28b74
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ShotgunEnhancement_Prototype final : public TypedComponentPrototype<ShotgunEnhancement> {
  public:
    // 0x7c28d94
    ShotgunEnhancement_Prototype() = default;

    // 0x7c28c4c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c28d00
    void Materialize(Frame* frame, ShotgunEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c28d04
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ShrinkingZone_Prototype final : public TypedComponentPrototype<ShrinkingZone> {
  public:
    AssetRefShrinkingZoneSpec Spec{};

    // 0x7c28f9c
    ShrinkingZone_Prototype() = default;

    // 0x7c28ddc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c28f00
    void Materialize(Frame* frame, ShrinkingZone& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c28f0c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ShrinkingZoneActor_Prototype final : public TypedComponentPrototype<ShrinkingZoneActor> {
  public:
    QBoolean IgnoreShrinkingZone{};

    // 0x7c29140
    ShrinkingZoneActor_Prototype() = default;

    // 0x7c28fe4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c290a4
    void Materialize(Frame* frame, ShrinkingZoneActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c290b0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class SpawnProjectilesOnDestroy_Prototype final
    : public TypedComponentPrototype<SpawnProjectilesOnDestroy> {
  public:
    std::vector<AssetRefProjectileSpawnerSpec> Specs{};

    // 0x7c29474
    SpawnProjectilesOnDestroy_Prototype() = default;

    // 0x7c291bc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2929c
    void Materialize(Frame* frame, SpawnProjectilesOnDestroy& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c293e4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Speeder_Prototype final : public TypedComponentPrototype<Speeder> {
  public:
    AssetRefSpeederSpec Spec{};
    FP SpeedModifier{};
    FP Duration{};
    TeamFlag Team{};
    std::int32_t UsedFrame{};

    // 0x7c296d4
    Speeder_Prototype() = default;

    // 0x7c294f4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c29618
    void Materialize(Frame* frame, Speeder& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c29644
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class StatsManager_Prototype final : public TypedComponentPrototype<StatsManager> {
  public:
    // 0x7c29864
    StatsManager_Prototype() = default;

    // 0x7c2971c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c297d0
    void Materialize(Frame* frame, StatsManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c297d4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class StreamEffect_Prototype final : public TypedComponentPrototype<StreamEffect> {
  public:
    AssetRefStreamEffectSpec Spec{};

    // 0x7c29a68
    StreamEffect_Prototype() = default;

    // 0x7c298ac
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c299cc
    void Materialize(Frame* frame, StreamEffect& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c299d8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class StreamEffectSpawner_Prototype final : public TypedComponentPrototype<StreamEffectSpawner> {
  public:
    AssetRefStreamEffectSpawnerSpec Spec{};

    // 0x7c29c0c
    StreamEffectSpawner_Prototype() = default;

    // 0x7c29ab0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c29b70
    void Materialize(Frame* frame, StreamEffectSpawner& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c29b7c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class SweepAttack_Prototype final : public TypedComponentPrototype<SweepAttack> {
  public:
    AssetRefSweepAttackSpec Spec{};

    // 0x7c29e08
    SweepAttack_Prototype() = default;

    // 0x7c29c54
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c29d6c
    void Materialize(Frame* frame, SweepAttack& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c29d78
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TargetFinder_Prototype final : public TypedComponentPrototype<TargetFinder> {
  public:
    AssetRefTargetFinderSpec Spec{};

    // 0x7c2a020
    TargetFinder_Prototype() = default;

    // 0x7c29e50
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c29f84
    void Materialize(Frame* frame, TargetFinder& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c29f90
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TargetHolder_Prototype final : public TypedComponentPrototype<TargetHolder> {
  public:
    // 0x7c2a1ac
    TargetHolder_Prototype() = default;

    // 0x7c2a068
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2a118
    void Materialize(Frame* frame, TargetHolder& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2a11c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Targetable_Prototype final : public TypedComponentPrototype<Targetable> {
  public:
    QBoolean IgnoreAutoShoot{};
    QBoolean IgnoreAutoTargetInsideBounds{};
    QBoolean IgnoreCameraAiming{};
    FP BoundsSizeMultiplier{};

    // 0x7c2a3c4
    Targetable_Prototype() = default;

    // 0x7c2a1f4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2a310
    void Materialize(Frame* frame, Targetable& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2a334
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TargetedEffect_Prototype final : public TypedComponentPrototype<TargetedEffect> {
  public:
    AssetRefTargetedEffectSpec spec{};

    // 0x7c2a5c8
    TargetedEffect_Prototype() = default;

    // 0x7c2a40c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2a52c
    void Materialize(Frame* frame, TargetedEffect& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2a538
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Team_Prototype final : public TypedComponentPrototype<Team> {
  public:
    std::int32_t TeamID{};

    // 0x7c2a76c
    Team_Prototype() = default;

    // 0x7c2a610
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2a6d0
    void Materialize(Frame* frame, Team& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2a6dc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TeslaClawMoreBeam_Prototype final : public TypedComponentPrototype<TeslaClawMoreBeam> {
  public:
    AssetRefTeslaClawMoreBeamEnhancementSpec Spec{};

    // 0x7c2a90c
    TeslaClawMoreBeam_Prototype() = default;

    // 0x7c2a7b4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2a870
    void Materialize(Frame* frame, TeslaClawMoreBeam& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2a87c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TimeScale_Prototype final : public TypedComponentPrototype<TimeScale> {
  public:
    // 0x7c2aaf0
    TimeScale_Prototype() = default;

    // 0x7c2a954
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2aa5c
    void Materialize(Frame* frame, TimeScale& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2aa60
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TrailSpawner_Prototype final : public TypedComponentPrototype<TrailSpawner> {
  public:
    AssetRefTrailSpawnerSpec Spec{};

    // 0x7c2ac94
    TrailSpawner_Prototype() = default;

    // 0x7c2ab38
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2abf8
    void Materialize(Frame* frame, TrailSpawner& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2ac04
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TravolatorActor_Prototype final : public TypedComponentPrototype<TravolatorActor> {
  public:
    // 0x7c2ae78
    TravolatorActor_Prototype() = default;

    // 0x7c2acdc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2ade4
    void Materialize(Frame* frame, TravolatorActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2ade8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TurnTo_Prototype final : public TypedComponentPrototype<TurnTo> {
  public:
    FP turnSpeed{};

    // 0x7c2b06c
    TurnTo_Prototype() = default;

    // 0x7c2aec0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2afd0
    void Materialize(Frame* frame, TurnTo& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2afdc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class TutorialGameMode_Prototype final : public TypedComponentPrototype<TutorialGameMode> {
  public:
    AssetRefTutorialGameModeSpec Spec{};

    // 0x7c2b210
    TutorialGameMode_Prototype() = default;

    // 0x7c2b0b4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2b174
    void Materialize(Frame* frame, TutorialGameMode& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2b180
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class UTAgent_Prototype final : public TypedComponentPrototype<UTAgent> {
  public:
    UtilityReasoner_Prototype UtilityReasoner{};
    AssetRefAIConfig Config{};

    // 0x7c2b918
    UTAgent_Prototype() = default;

    // 0x7c2b258
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2b3ac
    void Materialize(Frame* frame, UTAgent& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2b888
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class UltimateAbilityUser_Prototype final : public TypedComponentPrototype<UltimateAbilityUser> {
  public:
    AssetRefUltimateAbilityUserSpec userSpec{};

    // 0x7c2bb6c
    UltimateAbilityUser_Prototype() = default;

    // 0x7c2b9b0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2bad0
    void Materialize(Frame* frame, UltimateAbilityUser& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2badc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class UltimateEvolution_Prototype final : public TypedComponentPrototype<UltimateEvolution> {
  public:
    // 0x7c2bcf8
    UltimateEvolution_Prototype() = default;

    // 0x7c2bbb4
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2bc64
    void Materialize(Frame* frame, UltimateEvolution& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2bc68
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class UltimateTank_Prototype final : public TypedComponentPrototype<UltimateTank> {
  public:
    // 0x7c2bedc
    UltimateTank_Prototype() = default;

    // 0x7c2bd40
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2be48
    void Materialize(Frame* frame, UltimateTank& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2be4c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class UnlockedFeaturesManager_Prototype final
    : public TypedComponentPrototype<UnlockedFeaturesManager> {
  public:
    std::vector<QBoolean> unlockedFeatures = std::vector<QBoolean>(19);

    // 0x7c2c1d8
    UnlockedFeaturesManager_Prototype() = default;

    // 0x7c2bf24
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2c074
    void Materialize(Frame* frame, UnlockedFeaturesManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2c148
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class VoidRepulserShield_Prototype final : public TypedComponentPrototype<VoidRepulserShield> {
  public:
    // 0x7c2c4a0
    VoidRepulserShield_Prototype() = default;

    // 0x7c2c35c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2c40c
    void Materialize(Frame* frame, VoidRepulserShield& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2c410
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class VoidRepulserSpeedBoostEnhancement_Prototype final
    : public TypedComponentPrototype<VoidRepulserSpeedBoostEnhancement> {
  public:
    // 0x7c2c62c
    VoidRepulserSpeedBoostEnhancement_Prototype() = default;

    // 0x7c2c4e8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2c598
    void Materialize(Frame* frame, VoidRepulserSpeedBoostEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2c59c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class VoidRepulserVoidWaveEnhancement_Prototype final
    : public TypedComponentPrototype<VoidRepulserVoidWaveEnhancement> {
  public:
    // 0x7c2c7bc
    VoidRepulserVoidWaveEnhancement_Prototype() = default;

    // 0x7c2c674
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2c728
    void Materialize(Frame* frame, VoidRepulserVoidWaveEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2c72c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class VoidRepulserWeapon_Prototype final : public TypedComponentPrototype<VoidRepulserWeapon> {
  public:
    // 0x7c2c9a0
    VoidRepulserWeapon_Prototype() = default;

    // 0x7c2c804
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2c90c
    void Materialize(Frame* frame, VoidRepulserWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2c910
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WakeUpSurroundingsOnPushOrDestroy_Prototype final
    : public TypedComponentPrototype<WakeUpSurroundingsOnPushOrDestroy> {
  public:
    FP radius{};

    // 0x7c2cb44
    WakeUpSurroundingsOnPushOrDestroy_Prototype() = default;

    // 0x7c2c9e8
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2caa8
    void Materialize(Frame* frame, WakeUpSurroundingsOnPushOrDestroy& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2cab4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WallRunningActor_Prototype final : public TypedComponentPrototype<WallRunningActor> {
  public:
    AssetRefWallRunningSpec Spec{};

    // 0x7c2cd4c
    WallRunningActor_Prototype() = default;

    // 0x7c2cb8c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2ccb0
    void Materialize(Frame* frame, WallRunningActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2ccbc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WarmongerMiniRocketsEnhancement_Prototype final
    : public TypedComponentPrototype<WarmongerMiniRocketsEnhancement> {
  public:
    AssetRefWarmongerMiniRocketsEnhancementSpec Spec{};

    // 0x7c2ceec
    WarmongerMiniRocketsEnhancement_Prototype() = default;

    // 0x7c2cd94
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2ce50
    void Materialize(Frame* frame, WarmongerMiniRocketsEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2ce5c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WaveElement_Prototype final : public TypedComponentPrototype<WaveElement> {
  public:
    // 0x7c2d0dc
    WaveElement_Prototype() = default;

    // 0x7c2cf34
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2d048
    void Materialize(Frame* frame, WaveElement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2d04c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WaveManager_Prototype final : public TypedComponentPrototype<WaveManager> {
  public:
    AssetRefWaveManagerSpec Spec{};
    AssetRefSquadSiegeConfig SquadSiegeConfig{};
    AssetRefWaveReferences referencesAsset{};

    // 0x7c2d2f4
    WaveManager_Prototype() = default;

    // 0x7c2d124
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2d248
    void Materialize(Frame* frame, WaveManager& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2d264
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WaveSpawner_Prototype final : public TypedComponentPrototype<WaveSpawner> {
  public:
    std::uint32_t index{};
    SpawnConfig_Prototype spawnConfig{};
    FP wanderRadius{};
    FP spawnInterval{};
    std::vector<FPVector3> waypoints{};
    FP waypointRadius{};

    // 0x7c2d72c
    WaveSpawner_Prototype() = default;

    // 0x7c2d33c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2d484
    void Materialize(Frame* frame, WaveSpawner& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2d69c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class Weapon_Prototype final : public TypedComponentPrototype<Weapon> {
  public:
    AssetRefWeaponSpec WeaponData{};

    // 0x7c2d984
    Weapon_Prototype() = default;

    // 0x7c2d7ac
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2d8e8
    void Materialize(Frame* frame, Weapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2d8f4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WeaponBehaviour_Charging_Prototype final
    : public TypedComponentPrototype<WeaponBehaviour_Charging> {
  public:
    AssetRefWeaponBehaviour_ChargingSpec Spec{};

    // 0x7c2db78
    WeaponBehaviour_Charging_Prototype() = default;

    // 0x7c2d9cc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2dadc
    void Materialize(Frame* frame, WeaponBehaviour_Charging& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2dae8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WeaponLoadoutActor_Prototype final : public TypedComponentPrototype<WeaponLoadoutActor> {
  public:
    AssetRefWeaponLoadoutSpec spec{};

    // 0x7c2dd94
    WeaponLoadoutActor_Prototype() = default;

    // 0x7c2dbc0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2dcf8
    void Materialize(Frame* frame, WeaponLoadoutActor& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2dd04
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WeaponProjectileSwitcherEnhancement_Prototype final
    : public TypedComponentPrototype<WeaponProjectileSwitcherEnhancement> {
  public:
    // 0x7c2df78
    WeaponProjectileSwitcherEnhancement_Prototype() = default;

    // 0x7c2dddc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2dee4
    void Materialize(Frame* frame, WeaponProjectileSwitcherEnhancement& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2dee8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WeaponShowcaseGameMode_Prototype final
    : public TypedComponentPrototype<WeaponShowcaseGameMode> {
  public:
    // 0x7c2e164
    WeaponShowcaseGameMode_Prototype() = default;

    // 0x7c2dfc0
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2e0d0
    void Materialize(Frame* frame, WeaponShowcaseGameMode& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2e0d4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WeaponUser_Prototype final : public TypedComponentPrototype<WeaponUser> {
  public:
    AssetRefEntityPrototype DefaultEquippedWeapon{};
    FPVector3 weaponPositionOffset{};
    FPQuaternion weaponDefaultRotation{};

    // 0x7c2e3bc
    WeaponUser_Prototype() = default;

    // 0x7c2e1cc
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2e30c
    void Materialize(Frame* frame, WeaponUser& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2e1ac - the component keeps a reference to this prototype.
    void MaterializeUser(Frame* frame, WeaponUser& result,
                         const PrototypeMaterializationContext& context) const;
    // 0x7c2e32c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WrenchThrowProjectile_Prototype final
    : public TypedComponentPrototype<WrenchThrowProjectile> {
  public:
    // 0x7c2e548
    WrenchThrowProjectile_Prototype() = default;

    // 0x7c2e404
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2e4b4
    void Materialize(Frame* frame, WrenchThrowProjectile& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2e4b8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WrenchThrowWeapon_Prototype final : public TypedComponentPrototype<WrenchThrowWeapon> {
  public:
    // 0x7c2e6d4
    WrenchThrowWeapon_Prototype() = default;

    // 0x7c2e590
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2e640
    void Materialize(Frame* frame, WrenchThrowWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2e644
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class WrenchWeapon_Prototype final : public TypedComponentPrototype<WrenchWeapon> {
  public:
    // 0x7c2e8c4
    WrenchWeapon_Prototype() = default;

    // 0x7c2e71c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2e830
    void Materialize(Frame* frame, WrenchWeapon& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2e834
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

class ZoneControlGameMode_Prototype final : public TypedComponentPrototype<ZoneControlGameMode> {
  public:
    AssetRefZoneControlGameModeSpec Spec{};

    // 0x7c2eac8
    ZoneControlGameMode_Prototype() = default;

    // 0x7c2e90c
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7c2ea2c
    void Materialize(Frame* frame, ZoneControlGameMode& result,
                     const PrototypeMaterializationContext& context) const;
    // 0x7c2ea38
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override;
};

} // namespace Prototypes
} // namespace Quantum
