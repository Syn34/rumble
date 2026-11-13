// bot_sdk_assets.h - the bot SDK's assets: HFSM, behaviour tree, GOAP and utility theory graphs, blackboards and AI configs (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/assets/asset_value_types.h"

namespace Quantum {

class AIAction : public AssetObject {
  public:
    std::string Label;

    // 0x7b7e644
    AIAction() = default;

    // TODO(0x7b7e59c, Quantum.AIAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
    // TODO(0x7b7e5a0, Quantum.AIAction.Update): void Update(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
};

class AIBlackboard : public AssetObject {
  public:
    std::vector<AIBlackboardEntry> Entries;
    std::unordered_map<std::string, std::int32_t> Map; // not stored; filled by Loaded
    AssetRefAIBlackboardInitializer InitializerRef;

    // 0x7b87624
    AIBlackboard() = default;

    // 0x7b87514 - Map from each entry's key to its index (a duplicate key throws).
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class AIBlackboardInitializer : public AssetObject {
  public:
    struct AIBlackboardInitialValue {
        bool AsBoolean{};
        std::uint8_t AsByte{};
        std::int32_t AsInteger{};
        FP AsFP{};
        FPVector2 AsFPVector2;
        FPVector3 AsFPVector3;
        EntityRef AsEntityRef;
        AssetRef AsAssetRef;
    };

    struct AIBlackboardInitialValueEntry {
        std::string Key;
        AIBlackboardInitializer::AIBlackboardInitialValue Value;
    };

    bool ReportMissingEntries = true;
    AssetRefAIBlackboard AIBlackboard;
    std::vector<AIBlackboardInitializer::AIBlackboardInitialValueEntry> InitialValues;

    // 0x7b89524
    AIBlackboardInitializer() = default;

    // TODO(0x7b890d0, Quantum.AIBlackboardInitializer.InitializeBlackboard): void
    //   InitializeBlackboard(Frame frame, AIBlackboardComponent* blackboard,
    //   AIBlackboardInitializer blackboardInitializer, AIBlackboardInitialValueEntry[]
    //   blackboardOverrides)
    // TODO(0x7b8919c, Quantum.AIBlackboardInitializer.ApplyEntries): void ApplyEntries(Frame frame,
    //   AIBlackboardComponent* blackboard, AIBlackboardInitializer blackboardInitializer,
    //   AIBlackboardInitialValueEntry[] values)
};

class AIConfig : public AssetObject {
  public:
    enum class EValueType : std::int32_t {
        None = 0,
        Int = 1,
        Bool = 2,
        Byte = 3,
        FP = 4,
        FPVector2 = 5,
        FPVector3 = 6,
        String = 7,
        EntityRef = 8,
        AssetRef = 9,
    };

    struct Value {
        std::int32_t Integer{};
        bool Boolean{};
        std::uint8_t Byte{};
        ::Quantum::FP FP{};
        ::Quantum::FPVector2 FPVector2;
        ::Quantum::FPVector3 FPVector3;
        std::string String;
        ::Quantum::EntityRef EntityRef;
        ::Quantum::AssetRef AssetRef;
    };

    struct KeyValuePair {
        std::string Key;
        AIConfig::EValueType Type{};
        AIConfig::Value Value;

        // 0x7b7f22c
        KeyValuePair() = default;
    };

    AssetRefAIConfig DefaultConfig;
    std::vector<AIConfig::KeyValuePair> KeyValuePairs;

    // 0x7b7f1a0
    AIConfig() = default;

    // TODO(0x7b7f0d8, Quantum.AIConfig.Get): KeyValuePair Get(string key)
};

class AIFunction : public AssetObject {
  public:

    // 0x7b7e918
    AIFunction() = default;
};

// Quantum.AIFunction<T>: an AIFunction computing a T. Execute(frame, entity, aiContext) is
// each subclass's own; it needs the bot SDK's AIContext, which isn't reimplemented yet.
template <class T> class AIFunctionT : public AIFunction {
  public:
};

// Quantum.EnumEqualsFunction<T>: true when an enum value read through Value equals the
// subclass's reference value (GetEnumValue). Generic code without addresses.
template <class T> class EnumEqualsFunction : public AIFunctionT<bool> {
  public:
    EnumAIParam<T> Value;
};

class BTNode : public AssetObject {
  public:
    std::string Label;
    std::int32_t Id{};
    BTNode* Parent = nullptr; // not stored
    std::int32_t ParentIndex{}; // not stored

    // 0x7b7e164
    BTNode() = default;
};

class BTService : public AssetObject {
  public:
    FP IntervalInSec{};
    bool RunOnEnter{};
    std::int32_t Id{};

    // 0x7b7e554
    BTService() = default;
};

class CompoundHFSMSpec : public AssetObject {
  public:
    HFSMSetupData navSetupData;
    HFSMSetupData attackSetupData;

    // 0x7c8e258
    CompoundHFSMSpec() = default;
};

class Consideration : public AssetObject {
  public:
    std::string Label;
    AssetRefAIFunction RankRef;
    AssetRefAIFunction CommitmentRef;
    std::vector<AssetRefConsideration> NextConsiderationsRefs;
    std::vector<AssetRefAIAction> OnEnterActionsRefs;
    std::vector<AssetRefAIAction> OnUpdateActionsRefs;
    std::vector<AssetRefAIAction> OnExitActionsRefs;
    AIFunctionT<std::int32_t>* Rank = nullptr; // not stored; filled by Loaded
    AIFunctionT<bool>* Commitment = nullptr; // not stored; filled by Loaded
    std::vector<Consideration*> NextConsiderations; // not stored; filled by Loaded
    std::vector<AIAction*> OnEnterActions; // not stored; filled by Loaded
    std::vector<AIAction*> OnUpdateActions; // not stored; filled by Loaded
    std::vector<AIAction*> OnExitActions; // not stored; filled by Loaded
    std::vector<ResponseCurvePack> ResponseCurvePacks;
    FP BaseScore{};
    UTMomentumData MomentumData;
    bool UseNestedMomentum{};
    FP Cooldown{};
    bool CooldownCancelsMomentum = true;
    std::uint8_t Depth{};

    // 0x7b856fc
    Consideration() = default;

    // TODO(0x7b84968, Quantum.Consideration.GetConsideration): AssetRefConsideration
    //   GetConsideration(AssetRefConsideration> sourceList, int id)
    // TODO(0x7b849b8, Quantum.Consideration.Count): int Count(AssetRefConsideration> sourceList)
    // TODO(0x7b849d0, Quantum.Consideration.GetRank): int GetRank(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
    // TODO(0x7b849e8, Quantum.Consideration.Score): FP Score(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
    // TODO(0x7b84aec, Quantum.Consideration.OnEnter): void OnEnter(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
    // TODO(0x7b84b80, Quantum.Consideration.OnExit): void OnExit(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
    // TODO(0x7b84c14, Quantum.Consideration.OnUpdate): void OnUpdate(FrameThreadSafe frame,
    //   UtilityReasoner* reasoner, EntityRef entity, AIContext aiContext)
    // 0x7b84d60 - resolves the rank and commitment functions, the response curves, the enter/update/exit actions and the next considerations.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class GOAPAction : public AssetObject {
  public:
    std::string Label;
    GOAPState Conditions;
    GOAPState Effects;
    bool Interruptible{};

    // 0x7b80a7c
    GOAPAction() = default;
};

class GOAPBackValidation : public AssetObject {
  public:

    // 0x7b80a84
    GOAPBackValidation() = default;
};

class GOAPGoal : public AssetObject {
  public:
    enum class EInterruptionBehavior : std::int32_t {
        Never = 0,
        Always = 1,
        BasedOnActions = 2,
    };

    std::string Label;
    GOAPState StartState;
    GOAPState TargetState;
    GOAPGoal::EInterruptionBehavior InterruptionBehavior{};

    // 0x7b815f8
    GOAPGoal() = default;
};

class GOAPHeuristic : public AssetObject {
  public:

    // 0x7b81600
    GOAPHeuristic() = default;
};

class GOAPRoot : public AssetObject {
  public:
    std::string Label;
    FP InterruptionCheckInterval = FP::FromRaw(32768); // 0.5
    std::vector<AssetRefGOAPGoal> GoalRefs;
    std::vector<AssetRefGOAPAction> ActionRefs;
    std::vector<GOAPGoal*> Goals; // not stored; filled by Loaded
    std::vector<GOAPAction*> Actions; // not stored; filled by Loaded

    // 0x7b819bc
    GOAPRoot() = default;

    // 0x7b81608 - resolves the goals and actions.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class HFSMDecision : public AssetObject {
  public:
    std::string Label;

    // 0x7b81a78
    HFSMDecision() = default;

    // TODO(0x7b819cc, Quantum.HFSMDecision.Decide): bool Decide(Frame frame, EntityRef entity,
    //   AIContext aiContext)
    // TODO(0x7b819d4, Quantum.HFSMDecision.DecideThreadSafe): bool DecideThreadSafe(FrameThreadSafe
    //   frame, EntityRef entity, AIContext aiContext)
};

class HFSMRoot : public AssetObject {
  public:
    std::string Label;
    std::vector<AssetRefHFSMState> StatesLinks;
    std::vector<HFSMState*> States; // not stored; filled by Loaded
    std::vector<std::string> EventsNames;
    std::unordered_map<std::string, std::int32_t> RegisteredEvents; // not stored; filled by Loaded

    // 0x7b83078
    HFSMRoot() = default;

    // 0x7b825fc - the first state (none without states).
    AssetRefHFSMState InitialState() const;
    // 0x7b82dd8 - registers the event names (ids from 1, in order; a duplicate throws) and resolves the states.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class HFSMState : public AssetObject {
  public:
    std::string Label;
    std::uint8_t Index{};
    std::vector<AssetRefAIAction> OnUpdateLinks;
    std::vector<AssetRefAIAction> OnEnterLinks;
    std::vector<AssetRefAIAction> OnExitLinks;
    std::vector<HFSMTransition> Transitions;
    std::vector<AssetRefHFSMState> ChildrenLinks;
    AssetRefHFSMState ParentLink;
    std::int32_t Level{};
    std::vector<AIAction*> OnUpdate; // not stored; filled by Loaded
    std::vector<AIAction*> OnEnter; // not stored; filled by Loaded
    std::vector<AIAction*> OnExit; // not stored; filled by Loaded
    std::vector<HFSMState*> Children; // not stored; filled by Loaded
    HFSMState* Parent = nullptr; // not stored; filled by Loaded

    // 0x7b84028
    HFSMState() = default;

    // 0x7b83100 - resolves the update/enter/exit actions, the children and the parent, and sets up the transitions.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
    // TODO(0x7b827b0, Quantum.HFSMState.UpdateState): bool UpdateState(FrameThreadSafe frame, FP
    //   deltaTime, HFSMData* hfsmData, EntityRef entity, AIContext aiContext)
    // TODO(0x7b829a0, Quantum.HFSMState.Event): bool Event(FrameThreadSafe frame, HFSMData*
    //   hfsmData, EntityRef entity, int eventInt, AIContext aiContext)
    // TODO(0x7b82bf8, Quantum.HFSMState.EnterState): void EnterState(FrameThreadSafe frame,
    //   HFSMData* hfsmData, EntityRef entity, AIContext aiContext)
    // TODO(0x7b83e94, Quantum.HFSMState.FindChildInHistory): HFSMState
    //   FindChildInHistory(FrameThreadSafe frame, HFSMData* hfsmData)
    // TODO(0x7b82ab4, Quantum.HFSMState.ExitState): void ExitState(HFSMState nextState,
    //   FrameThreadSafe frame, HFSMData* hfsmData, EntityRef entity, AIContext aiContext)
    // 0x7b83f00 - whether `state` is an ancestor of this state.
    bool IsChildOf(const HFSMState* state) const;
    // TODO(0x7b83f1c, Quantum.HFSMState.IsSiblingOf): bool IsSiblingOf(FrameThreadSafe frame,
    //   HFSMData* hfsmData, HFSMState state)
    // TODO(0x7b83bc4, Quantum.HFSMState.UpdateActions): void UpdateActions(FrameThreadSafe frame,
    //   EntityRef entity, HFSMData* hfsmData, AIContext aiContext, AIAction[] actions)
    // TODO(0x7b83ccc, Quantum.HFSMState.CheckTransitions): bool CheckTransitions(FrameThreadSafe
    //   frame, HFSMTransition[] transitions, HFSMData* hfsmData, EntityRef entity, int eventKey,
    //   AIContext aiContext, int depth)
};

class HFSMTransitionSet : public AssetObject {
  public:
    std::string Label;
    AssetRefHFSMDecision PrerequisiteLink;
    std::vector<HFSMTransition> Transitions;
    HFSMDecision* Prerequisite = nullptr; // not stored; filled by Loaded

    // 0x7b841dc
    HFSMTransitionSet() = default;

    // 0x7b84038 - resolves the prerequisite decision and sets up the transitions.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class UTRoot : public AssetObject {
  public:
    std::vector<AssetRefConsideration> ConsiderationsRefs;
    std::vector<Consideration*> Considerations; // not stored; filled by Loaded

    // 0x7b8750c
    UTRoot() = default;

    // 0x7b87308 - resolves the considerations (unlike the other graph assets it skips the base call, which does nothing).
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class BTComposite : public BTNode {
  public:
    std::vector<AssetRefBTNode> Children;
    std::vector<AssetRefBTService> Services;
    AssetRefBTNode TopmostDecorator;
    BTDataIndex CurrentChildIndex;
    bool IsDynamic{};
    std::vector<BTNode*> _childInstances; // not stored; filled by Loaded
    std::vector<BTService*> _serviceInstances; // not stored; filled by Loaded
    BTNode* _topmostDecoratorInstance = nullptr; // not stored; filled by Loaded

    // 0x7b7dc18
    BTComposite() = default;

    // 0x7b7dc30 - resolves the children (each gets this node as Parent and its index as ParentIndex), the services and the topmost decorator.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class BTDecorator : public BTNode {
  public:
    AssetRefBTNode Child;
    BTAbort AbortType{};
    BTNode* _childInstance = nullptr; // not stored; filled by Loaded

    // 0x7b7e320
    BTDecorator() = default;

    // 0x7b7e188 - resolves the child, if set, and makes this node its Parent.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class BTLeaf : public BTNode {
  public:
    std::vector<AssetRefBTService> Services;
    std::vector<BTService*> _serviceInstances; // not stored; filled by Loaded

    // 0x7b7e544
    BTLeaf() = default;

    // 0x7b7e328 - resolves the services.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class CheckBlackboardInt : public HFSMDecision {
  public:
    AIBlackboardValueKey Key;
    EValueComparison Comparison = EValueComparison::MoreThan;
    AIParamInt DesiredValue = AIParamInt(1);

    // 0x7b8a160
    CheckBlackboardInt() = default;

    // TODO(0x7b8a030, Quantum.CheckBlackboardInt.Decide): bool Decide(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class DebugAction : public AIAction {
  public:
    std::string Message;

    // 0x7b89e7c
    DebugAction() = default;

    // TODO(0x7b89d54, Quantum.DebugAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class DebugService : public BTService {
  public:
    std::string Message;

    // 0x7b7e594
    DebugService() = default;
};

class GOAPDefaultAction : public GOAPAction {
  public:
    AIParamBool Validation = AIParamBool(true);
    AIParamFP Cost = AIParamFP(FP::FromInt(1));
    AssetRefGOAPBackValidation PlanStateValidationLink;
    std::vector<AssetRefAIAction> OnActivateLinks;
    std::vector<AssetRefAIAction> OnUpdateLinks;
    std::vector<AssetRefAIAction> OnDeactivateLinks;
    AIParamBool IsDone;
    AIParamBool IsFailed;
    GOAPBackValidation* PlanStateValidation = nullptr; // not stored; filled by Loaded
    std::vector<AIAction*> OnActivate; // not stored; filled by Loaded
    std::vector<AIAction*> OnUpdate; // not stored; filled by Loaded
    std::vector<AIAction*> OnDeactivate; // not stored; filled by Loaded

    // 0x7b81070
    GOAPDefaultAction() = default;

    // 0x7b80a8c - resolves the plan-state validation and the activate/update/deactivate actions.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class GOAPDefaultGoal : public GOAPGoal {
  public:
    AIParamBool Validation = AIParamBool(true);
    AIParamFP Relevancy = AIParamFP(FP::FromInt(1));
    AIParamFP DisableTime;
    std::vector<AssetRefAIAction> OnInitPlanningLinks;
    std::vector<AssetRefAIAction> OnActivateLinks;
    std::vector<AssetRefAIAction> OnDeactivateLinks;
    AIParamBool IsFinished;
    std::vector<AIAction*> OnInitPlanning; // not stored; filled by Loaded
    std::vector<AIAction*> OnActivate; // not stored; filled by Loaded
    std::vector<AIAction*> OnDeactivate; // not stored; filled by Loaded

    // 0x7b815b0
    GOAPDefaultGoal() = default;

    // 0x7b810b8 - resolves the init-planning/activate/deactivate actions.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class GOAPSingleBackValidation : public GOAPBackValidation {
  public:
};

class HFSMLogicalDecision : public HFSMDecision {
  public:
    std::vector<AssetRefHFSMDecision> Decisions;
    std::vector<HFSMDecision*> _decisions; // not stored; filled by Loaded

    // 0x7b81c9c
    HFSMLogicalDecision() = default;

    // 0x7b81a80 - resolves the combined decisions.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class IncreaseBlackboardInt : public AIAction {
  public:
    AIBlackboardValueKey Key;
    AIParamInt IncrementAmount;

    // 0x7b89f90
    IncreaseBlackboardInt() = default;

    // TODO(0x7b89e90, Quantum.IncreaseBlackboardInt.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class IsTrue : public HFSMDecision {
  public:
    AIParamBool Value;

    // 0x7ccb030
    IsTrue() = default;

    // TODO(0x7ccafc0, Quantum.IsTrue.Decide): bool Decide(Frame frame, EntityRef entity, AIContext
    //   aiContext)
};

class SetBlackboardBool : public AIAction {
  public:
    AIBlackboardValueKey Key;
    bool value{};

    // 0x7cba3f0
    SetBlackboardBool() = default;

    // TODO(0x7cba3ac, Quantum.SetBlackboardBool.Update): void Update(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class SetBlackboardEntityRef : public AIAction {
  public:
    AIBlackboardValueKey Key;
    AIParamEntityRef Value;

    // 0x7cba3a4
    SetBlackboardEntityRef() = default;

    // TODO(0x7cba318, Quantum.SetBlackboardEntityRef.Update): void Update(Frame f, EntityRef
    //   entity, AIContext aiContext)
};

class SetBlackboardFP : public AIAction {
  public:
    AIBlackboardValueKey Key;
    AIParamFP Value;

    // 0x7cba240
    SetBlackboardFP() = default;

    // TODO(0x7cba1b4, Quantum.SetBlackboardFP.Update): void Update(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class SetBlackboardFPVector3 : public AIAction {
  public:
    AIBlackboardValueKey Key;
    AIParamFPVector3 Value;

    // 0x7cba310
    SetBlackboardFPVector3() = default;

    // TODO(0x7cba248, Quantum.SetBlackboardFPVector3.Update): void Update(Frame f, EntityRef
    //   entity, AIContext aiContext)
};

class SetBlackboardInt : public AIAction {
  public:
    AIBlackboardValueKey Key;
    AIParamInt Value;

    // 0x7b8a028
    SetBlackboardInt() = default;

    // TODO(0x7b89f98, Quantum.SetBlackboardInt.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class TrueDecision : public HFSMDecision {
  public:

    // 0x7b8a25c
    TrueDecision() = default;

    // TODO(0x7b8a254, Quantum.TrueDecision.Decide): bool Decide(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class BTBlackboardCompare : public BTDecorator {
  public:
    AIBlackboardValueKey BlackboardKeyA;
    AIBlackboardValueKey BlackboardKeyB;

    // 0x7b7e55c
    BTBlackboardCompare() = default;
};

class BTCooldown : public BTDecorator {
  public:
    FP CooldownTime{};
    BTDataIndex StartTimeIndex;

    // 0x7b7e564
    BTCooldown() = default;
};

class BTForceResult : public BTDecorator {
  public:
    BTStatus Result{};

    // 0x7b7e56c
    BTForceResult() = default;
};

class BTLoop : public BTDecorator {
  public:
    std::int32_t LoopIterations{};
    bool LoopForever{};
    FP LoopTimeout = FP::FromInt(-1);
    BTDataIndex StartTimeIndex;
    BTDataIndex IterationCountIndex;

    // 0x7b7e574
    BTLoop() = default;
};

class BTRoot : public BTDecorator {
  public:
    std::int32_t NodesAmount{};

    // 0x7b7e54c
    BTRoot() = default;
};

class BTSelector : public BTComposite {
  public:

    // 0x7b7dc10
    BTSelector() = default;
};

class BTSelectorRandom : public BTComposite {
  public:

    // 0x7b7dc20
    BTSelectorRandom() = default;
};

class BTSequence : public BTComposite {
  public:

    // 0x7b7dc28
    BTSequence() = default;
};

class DebugLeaf : public BTLeaf {
  public:
    std::string Message;

    // 0x7b7e584
    DebugLeaf() = default;
};

class HFSMAndDecision : public HFSMLogicalDecision {
  public:

    // 0x7b81e10
    HFSMAndDecision() = default;

    // TODO(0x7b81d60, Quantum.HFSMAndDecision.DecideThreadSafe): bool
    //   DecideThreadSafe(FrameThreadSafe frame, EntityRef entity, AIContext aiContext)
    // TODO(0x7b81d64, Quantum.HFSMAndDecision.CheckDecisions): bool CheckDecisions(FrameThreadSafe
    //   frame, EntityRef entity, AIContext aiContext)
};

class HFSMNotDecision : public HFSMLogicalDecision {
  public:

    // 0x7b81e5c
    HFSMNotDecision() = default;

    // TODO(0x7b81e18, Quantum.HFSMNotDecision.DecideThreadSafe): bool
    //   DecideThreadSafe(FrameThreadSafe frame, EntityRef entity, AIContext aiContext)
    // TODO(0x7b81e1c, Quantum.HFSMNotDecision.CheckDecisions): bool CheckDecisions(FrameThreadSafe
    //   frame, EntityRef entity, AIContext aiContext)
};

class HFSMOrDecision : public HFSMLogicalDecision {
  public:

    // 0x7b81d58
    HFSMOrDecision() = default;

    // TODO(0x7b81ca4, Quantum.HFSMOrDecision.DecideThreadSafe): bool
    //   DecideThreadSafe(FrameThreadSafe frame, EntityRef entity, AIContext aiContext)
    // TODO(0x7b81ca8, Quantum.HFSMOrDecision.CheckDecisions): bool CheckDecisions(FrameThreadSafe
    //   frame, EntityRef entity, AIContext aiContext)
};

class ResponseCurve : public AIFunctionT<FP> {
  public:
    AIParamFP Input;
    FPAnimationCurve Curve;
    bool Clamp01 = true;
    FP MultiplyFactor = FP::FromInt(1);

    // 0x7b80240
    ResponseCurve() = default;

    // TODO(0x7b80088, Quantum.ResponseCurve.Execute): FP Execute(Frame frame, EntityRef entity,
    //   AIContext aiContext)
    // TODO(0x7b800d8, Quantum.ResponseCurve.Execute): FP Execute(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
};

class WaitLeaf : public BTLeaf {
  public:
    FP Duration{};
    BTDataIndex EndTimeIndex;

    // 0x7b7e58c
    WaitLeaf() = default;
};

} // namespace Quantum
