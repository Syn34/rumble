// Methods of the bot SDK's assets (quantum.code.dll): resolving the graphs' stored
// references when the assets are loaded.
#include "quantum_code/assets/bot_sdk_assets.h"

#include "quantum_code/assets/asset_linking.h"

#include <stdexcept>

namespace Quantum {

namespace {

// Dictionary.Add: a key may be registered once.
template <class Map, class Key, class Value> void AddUnique(Map& map, const Key& key, Value value, const char* what) {
    if (!map.emplace(key, value).second) throw std::invalid_argument(std::string("duplicate ") + what + ": " + key);
}

} // namespace

void HFSMTransition::Setup(IResourceManager* resourceManager) {
    Decision = LinkAsset<HFSMDecision>(resourceManager, DecisionLink.Id);
    State = LinkAsset<HFSMState>(resourceManager, StateLink.Id);
    TransitionSet = LinkAsset<HFSMTransitionSet>(resourceManager, TransitionSetLink.Id);
}

void AIBlackboard::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    Map.clear();
    for (std::size_t i = 0; i < Entries.size(); ++i)
        AddUnique(Map, Entries[i].Key.Key, static_cast<std::int32_t>(i), "blackboard key");
}

void BTComposite::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    _childInstances.assign(Children.size(), nullptr);
    for (std::size_t i = 0; i < Children.size(); ++i) {
        BTNode* child = LinkAsset<BTNode>(resourceManager, Children[i].Id);
        // A missing child throws in the original too (null dereference).
        if (child == nullptr) throw std::invalid_argument("behaviour tree child not found: " + Children[i].Id.ToString(true));
        _childInstances[i] = child;
        child->Parent = this;
        child->ParentIndex = static_cast<std::int32_t>(i);
    }
    _serviceInstances = LinkAssets<BTService>(resourceManager, Services);
    if (TopmostDecorator.Id.IsValid()) _topmostDecoratorInstance = LinkAsset<BTNode>(resourceManager, TopmostDecorator.Id);
}

void BTDecorator::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    if (!Child.Id.IsValid()) return;
    _childInstance = LinkAsset<BTNode>(resourceManager, Child.Id);
    if (_childInstance) _childInstance->Parent = this;
}

void BTLeaf::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    _serviceInstances = LinkAssets<BTService>(resourceManager, Services);
}

void Consideration::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    Rank = LinkAsset<AIFunctionT<std::int32_t>>(resourceManager, RankRef.Id);
    for (ResponseCurvePack& pack : ResponseCurvePacks)
        pack.ResponseCurve = LinkAsset<::Quantum::ResponseCurve>(resourceManager, pack.ResponseCurveRef.Id);
    OnEnterActions = LinkAssets<AIAction>(resourceManager, OnEnterActionsRefs);
    OnUpdateActions = LinkAssets<AIAction>(resourceManager, OnUpdateActionsRefs);
    OnExitActions = LinkAssets<AIAction>(resourceManager, OnExitActionsRefs);
    Commitment = LinkAsset<AIFunctionT<bool>>(resourceManager, CommitmentRef.Id);
    NextConsiderations = LinkAssets<Consideration>(resourceManager, NextConsiderationsRefs);
}

void GOAPDefaultAction::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    PlanStateValidation = LinkAsset<GOAPBackValidation>(resourceManager, PlanStateValidationLink.Id);
    OnActivate = LinkAssets<AIAction>(resourceManager, OnActivateLinks);
    OnUpdate = LinkAssets<AIAction>(resourceManager, OnUpdateLinks);
    OnDeactivate = LinkAssets<AIAction>(resourceManager, OnDeactivateLinks);
}

void GOAPDefaultGoal::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    OnInitPlanning = LinkAssets<AIAction>(resourceManager, OnInitPlanningLinks);
    OnActivate = LinkAssets<AIAction>(resourceManager, OnActivateLinks);
    OnDeactivate = LinkAssets<AIAction>(resourceManager, OnDeactivateLinks);
}

void GOAPRoot::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    Goals = LinkAssets<GOAPGoal>(resourceManager, GoalRefs);
    Actions = LinkAssets<GOAPAction>(resourceManager, ActionRefs);
}

void HFSMLogicalDecision::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    _decisions = LinkAssets<HFSMDecision>(resourceManager, Decisions);
}

AssetRefHFSMState HFSMRoot::InitialState() const {
    // The original indexes element 0 of a non-null array (an empty one throws).
    if (StatesLinks.empty()) return AssetRefHFSMState{};
    return StatesLinks.front();
}

void HFSMRoot::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    RegisteredEvents.clear();
    for (std::size_t i = 0; i < EventsNames.size(); ++i)
        AddUnique(RegisteredEvents, EventsNames[i], static_cast<std::int32_t>(i + 1), "HFSM event");
    States = LinkAssets<HFSMState>(resourceManager, StatesLinks);
}

void HFSMState::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    OnUpdate = LinkAssets<AIAction>(resourceManager, OnUpdateLinks);
    OnEnter = LinkAssets<AIAction>(resourceManager, OnEnterLinks);
    OnExit = LinkAssets<AIAction>(resourceManager, OnExitLinks);
    Children = LinkAssets<HFSMState>(resourceManager, ChildrenLinks);
    Parent = LinkAsset<HFSMState>(resourceManager, ParentLink.Id);
    for (HFSMTransition& transition : Transitions) transition.Setup(resourceManager);
}

bool HFSMState::IsChildOf(const HFSMState* state) const {
    for (const HFSMState* ancestor = Parent; ancestor != nullptr; ancestor = ancestor->Parent)
        if (ancestor == state) return true;
    return false;
}

void HFSMTransitionSet::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    Prerequisite = LinkAsset<HFSMDecision>(resourceManager, PrerequisiteLink.Id);
    for (HFSMTransition& transition : Transitions) transition.Setup(resourceManager);
}

const AIConfig::KeyValuePair* AIConfig::Get(const std::string& key) const {
    for (const KeyValuePair& pair : KeyValuePairs)
        if (pair.Key == key) return &pair;
    return nullptr;
}

AssetRefConsideration Consideration::GetConsideration(Collections::QList<AssetRefConsideration> sourceList,
                                                     std::int32_t id) const {
    (void)sourceList;
    const Consideration* next = NextConsiderations.at(static_cast<std::size_t>(id));
    return AssetRefConsideration{next ? next->Guid : AssetGuid{}};
}

std::int32_t Consideration::Count(Collections::QList<AssetRefConsideration> sourceList) const {
    (void)sourceList;
    return static_cast<std::int32_t>(NextConsiderations.size());
}

void UTRoot::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    (void)allocator;
    Considerations = LinkAssets<Consideration>(resourceManager, ConsiderationsRefs);
}

} // namespace Quantum
