// prototypes_values.cpp - prototypes of the value types stored in components (quantum.code.dll).
#include "quantum_code/prototypes/prototypes.h"

#include "quantum_code/prototypes/component_prototype_visitor.h"
#include "quantum_code/prototypes/prototype_support.h"

namespace Quantum::Prototypes {

void UTMomentumData_Prototype::Materialize(Frame*, UTMomentumData& result,
                                           const PrototypeMaterializationContext&) const {
    result.DecayAmount = DecayAmount;
    result.Value = Value;
}

void UTMomentumPack_Prototype::Materialize(Frame* frame, UTMomentumPack& result,
                                           const PrototypeMaterializationContext& context) const {
    result.ConsiderationRef = ConsiderationRef;
    MomentumData.Materialize(frame, result.MomentumData, context);
}

void UtilityReasoner_Prototype::Materialize(Frame* frame, UtilityReasoner& result,
                                            const PrototypeMaterializationContext& context) const {
    result.UTRoot = UTRoot;
    result.TimeToTick = TimeToTick;
    if (Considerations.empty()) {
        result.set_Considerations({});
    } else {
        auto ptr = result.get_Considerations();
        auto list = frame->AllocateList(ptr, static_cast<int>(Considerations.size()));
        for (const auto& item : Considerations) {
            list.Add(item);
        }
        result.set_Considerations(ptr);
    }
    if (CooldownsDict.empty()) {
        result.set_CooldownsDict({});
    } else {
        auto ptr = result.get_CooldownsDict();
        auto dictionary = frame->AllocateDictionary(ptr, static_cast<int>(CooldownsDict.size()));
        for (const auto& entry : CooldownsDict) {
            PrototypeValidator::AddToDictionary(dictionary, entry.Key, entry.Value, context);
        }
        result.set_CooldownsDict(ptr);
    }
    if (HighRankConsiderations.empty()) {
        result.set_HighRankConsiderations({});
    } else {
        auto ptr = result.get_HighRankConsiderations();
        auto list = frame->AllocateList(ptr, static_cast<int>(HighRankConsiderations.size()));
        for (const auto& item : HighRankConsiderations) {
            list.Add(item);
        }
        result.set_HighRankConsiderations(ptr);
    }
    if (MomentumList.empty()) {
        result.set_MomentumList({});
    } else {
        auto ptr = result.get_MomentumList();
        auto list = frame->AllocateList(ptr, static_cast<int>(MomentumList.size()));
        for (const auto& item : MomentumList) {
            UTMomentumPack value{};
            item.Materialize(frame, value, context);
            list.Add(value);
        }
        result.set_MomentumList(ptr);
    }
    if (PreviousExecution.empty()) {
        result.set_PreviousExecution({});
    } else {
        auto ptr = result.get_PreviousExecution();
        auto list = frame->AllocateList(ptr, static_cast<int>(PreviousExecution.size()));
        for (const auto& item : PreviousExecution) {
            list.Add(item);
        }
        result.set_PreviousExecution(ptr);
    }
}

void SpawnConfig_Prototype::Materialize(Frame*, SpawnConfig& result,
                                        const PrototypeMaterializationContext&) const {
    result.findPositionAttempts = findPositionAttempts;
    result.useRandomSpawnTransform = useRandomSpawnTransform;
    result.fallbackYOffset = fallbackYOffset;
    result.freePositionRadius = freePositionRadius;
    result.spawnPositionOffsetDistance = spawnPositionOffsetDistance;
}

void KillZoneArea_Prototype::Materialize(Frame*, KillZoneArea& result,
                                         const PrototypeMaterializationContext&) const {
    result.surfaceType = surfaceType;
    result.size = size;
    result.corner = corner;
}

void HFSMData_Prototype::Materialize(Frame*, HFSMData& result,
                                     const PrototypeMaterializationContext& context) const {
    result.StatesHistory = StatesHistory;
    result.UsesStateHistory = UsesStateHistory;
    result.Root = Root;
    result.CurrentState = CurrentState;
    result.Time = Time;
    {
        const int count =
            PrototypeValidator::CheckLength(static_cast<int>(Times.size()), 8, context);
        auto target = result.get_Times();
        for (int i = 0; i < count; ++i)
            target[static_cast<std::size_t>(i)] = Times[static_cast<std::size_t>(i)];
    }
}

void BTDataValue_Prototype::Materialize(Frame*, BTDataValue& result,
                                        const PrototypeMaterializationContext& context) const {
    if (_field_used_ == "FPVALUE") {
        *result.get_FPValue() = FPValue;
    } else if (_field_used_ == "INTVALUE") {
        *result.get_IntValue() = IntValue;
    } else if (!_field_used_.empty()) {
        PrototypeValidator::UnknownUnionField(_field_used_, context);
    }
}

void BlackboardValue_Prototype::Materialize(Frame*, BlackboardValue& result,
                                            const PrototypeMaterializationContext& context) const {
    if (_field_used_ == "BOOLEANVALUE") {
        *result.get_BooleanValue() = BooleanValue;
    } else if (_field_used_ == "BYTEVALUE") {
        *result.get_ByteValue() = ByteValue;
    } else if (_field_used_ == "INTEGERVALUE") {
        *result.get_IntegerValue() = IntegerValue;
    } else if (_field_used_ == "FPVALUE") {
        *result.get_FPValue() = FPValue;
    } else if (_field_used_ == "FPVECTOR2VALUE") {
        *result.get_FPVector2Value() = FPVector2Value;
    } else if (_field_used_ == "FPVECTOR3VALUE") {
        *result.get_FPVector3Value() = FPVector3Value;
    } else if (_field_used_ == "ENTITYREFVALUE") {
        PrototypeValidator::FindMapEntity(EntityRefValue, context, *result.get_EntityRefValue());
    } else if (_field_used_ == "ASSETREFVALUE") {
        *result.get_AssetRefValue() = AssetRefValue;
    } else if (!_field_used_.empty()) {
        PrototypeValidator::UnknownUnionField(_field_used_, context);
    }
}

void BlackboardEntry_Prototype::Materialize(Frame* frame, BlackboardEntry& result,
                                            const PrototypeMaterializationContext& context) const {
    Value.Materialize(frame, result.Value, context);
    if (ReactiveDecorators.empty()) {
        result.set_ReactiveDecorators({});
    } else {
        auto ptr = result.get_ReactiveDecorators();
        auto list = frame->AllocateList(ptr, static_cast<int>(ReactiveDecorators.size()));
        for (const auto& item : ReactiveDecorators) {
            list.Add(item);
        }
        result.set_ReactiveDecorators(ptr);
    }
}

} // namespace Quantum::Prototypes
