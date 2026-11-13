// asset_refs.h - typed asset references of the gameplay simulation (quantum.code.dll).
//
// Each AssetRefXxx is an AssetGuid that is known to name an asset of class Xxx. They
// all behave alike: compared and serialised as the GUID, hashed as
// seed * 31 + GUID hash with a per-type seed (which the original's code generator chose;
// it is kept because hashes feed checksums and hash-keyed collections).
#pragma once

#include "quantum_core/asset_guid.h"

#include <cstdint>
#include <string>

namespace Photon::Deterministic {
class IDeterministicFrameSerializer;
}

namespace Quantum {

namespace detail {
constexpr std::int32_t TypedAssetRefHash(std::uint32_t seed, AssetGuid id) {
    const auto bits = static_cast<std::uint64_t>(id.Value);
    const auto guidHash = static_cast<std::uint32_t>(bits) ^ static_cast<std::uint32_t>(bits >> 32);
    return static_cast<std::int32_t>(seed * 31u + guidHash);
}
} // namespace detail

struct AssetRefAIAction {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8bec0 (Equals), 0x7b8bf40 (Equals)
    friend constexpr bool operator==(AssetRefAIAction a, AssetRefAIAction b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAIAction other) const { return Id == other.Id; }
    // 0x7b8bf48
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(89u, Id); }
    // 0x7b8beb4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAIAction*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAIAction) == AssetRefAIAction::SIZE);

struct AssetRefAIBlackboard {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8bff4 (==), 0x7b8bf6c (Equals), 0x7b8bfec (Equals)
    friend constexpr bool operator==(AssetRefAIBlackboard a, AssetRefAIBlackboard b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAIBlackboard other) const { return Id == other.Id; }
    // 0x7b88f74
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(97u, Id); }
    // 0x7b8bf60
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b890bc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAIBlackboard*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8783c, Quantum.AssetRefAIBlackboard.op_Implicit): AIBlackboard asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefAIBlackboard) == AssetRefAIBlackboard::SIZE);

struct AssetRefAIBlackboardInitializer {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c008 (Equals), 0x7b8c088 (Equals)
    friend constexpr bool operator==(AssetRefAIBlackboardInitializer a, AssetRefAIBlackboardInitializer b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAIBlackboardInitializer other) const { return Id == other.Id; }
    // 0x7b8c090
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(101u, Id); }
    // 0x7b8bffc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8c0a8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAIBlackboardInitializer*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAIBlackboardInitializer) == AssetRefAIBlackboardInitializer::SIZE);

struct AssetRefAIBotSDKConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c0c8 (Equals), 0x7b8c148 (Equals)
    friend constexpr bool operator==(AssetRefAIBotSDKConfig a, AssetRefAIBotSDKConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAIBotSDKConfig other) const { return Id == other.Id; }
    // 0x7b8c150
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(103u, Id); }
    // 0x7b8c0bc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAIBotSDKConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAIBotSDKConfig) == AssetRefAIBotSDKConfig::SIZE);

struct AssetRefAIConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c174 (Equals), 0x7b8c1f4 (Equals)
    friend constexpr bool operator==(AssetRefAIConfig a, AssetRefAIConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAIConfig other) const { return Id == other.Id; }
    // 0x7b7d668
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(107u, Id); }
    // 0x7b8c168
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b7da24
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAIConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAIConfig) == AssetRefAIConfig::SIZE);

struct AssetRefAIFunction {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c208 (Equals), 0x7b8c288 (Equals)
    friend constexpr bool operator==(AssetRefAIFunction a, AssetRefAIFunction b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAIFunction other) const { return Id == other.Id; }
    // 0x7b8c290
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(109u, Id); }
    // 0x7b8c1fc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAIFunction*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAIFunction) == AssetRefAIFunction::SIZE);

struct AssetRefAIMoveForwardBehaviourSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c2b4 (Equals), 0x7b8c334 (Equals)
    friend constexpr bool operator==(AssetRefAIMoveForwardBehaviourSpec a, AssetRefAIMoveForwardBehaviourSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAIMoveForwardBehaviourSpec other) const { return Id == other.Id; }
    // 0x7b8c33c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(113u, Id); }
    // 0x7b8c2a8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8c354
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAIMoveForwardBehaviourSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAIMoveForwardBehaviourSpec) == AssetRefAIMoveForwardBehaviourSpec::SIZE);

struct AssetRefAINavigationSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c374 (Equals), 0x7b8c3f4 (Equals)
    friend constexpr bool operator==(AssetRefAINavigationSpec a, AssetRefAINavigationSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAINavigationSpec other) const { return Id == other.Id; }
    // 0x7b8c3fc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(127u, Id); }
    // 0x7b8c368
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8c414
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAINavigationSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAINavigationSpec) == AssetRefAINavigationSpec::SIZE);

struct AssetRefAISpawnerConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c434 (Equals), 0x7b8c4b4 (Equals)
    friend constexpr bool operator==(AssetRefAISpawnerConfig a, AssetRefAISpawnerConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAISpawnerConfig other) const { return Id == other.Id; }
    // 0x7b8c4bc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(131u, Id); }
    // 0x7b8c428
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAISpawnerConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAISpawnerConfig) == AssetRefAISpawnerConfig::SIZE);

struct AssetRefActorComponentSpecBase {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c4e0 (Equals), 0x7b8c560 (Equals)
    friend constexpr bool operator==(AssetRefActorComponentSpecBase a, AssetRefActorComponentSpecBase b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefActorComponentSpecBase other) const { return Id == other.Id; }
    // 0x7b8c568
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(137u, Id); }
    // 0x7b8c4d4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefActorComponentSpecBase*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefActorComponentSpecBase) == AssetRefActorComponentSpecBase::SIZE);

struct AssetRefActorIcedSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c590 (Equals), 0x7b8c610 (Equals)
    friend constexpr bool operator==(AssetRefActorIcedSpec a, AssetRefActorIcedSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefActorIcedSpec other) const { return Id == other.Id; }
    // 0x7b8c618
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(139u, Id); }
    // 0x7b8c584
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefActorIcedSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefActorIcedSpec) == AssetRefActorIcedSpec::SIZE);

struct AssetRefActorManagerConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c640 (Equals), 0x7b8c6c0 (Equals)
    friend constexpr bool operator==(AssetRefActorManagerConfig a, AssetRefActorManagerConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefActorManagerConfig other) const { return Id == other.Id; }
    // 0x7b8c6c8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(149u, Id); }
    // 0x7b8c634
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8c6e4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefActorManagerConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefActorManagerConfig) == AssetRefActorManagerConfig::SIZE);

struct AssetRefActorPhysicsSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c704 (Equals), 0x7b8c784 (Equals)
    friend constexpr bool operator==(AssetRefActorPhysicsSpec a, AssetRefActorPhysicsSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefActorPhysicsSpec other) const { return Id == other.Id; }
    // 0x7b8c78c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(151u, Id); }
    // 0x7b8c6f8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefActorPhysicsSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefActorPhysicsSpec) == AssetRefActorPhysicsSpec::SIZE);

struct AssetRefActorSheepedSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c7b4 (Equals), 0x7b8c834 (Equals)
    friend constexpr bool operator==(AssetRefActorSheepedSpec a, AssetRefActorSheepedSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefActorSheepedSpec other) const { return Id == other.Id; }
    // 0x7b8c83c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(157u, Id); }
    // 0x7b8c7a8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefActorSheepedSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefActorSheepedSpec) == AssetRefActorSheepedSpec::SIZE);

struct AssetRefActorStatusModifierSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c864 (Equals), 0x7b8c8e4 (Equals)
    friend constexpr bool operator==(AssetRefActorStatusModifierSpec a, AssetRefActorStatusModifierSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefActorStatusModifierSpec other) const { return Id == other.Id; }
    // 0x7b8c8ec
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(163u, Id); }
    // 0x7b8c858
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8c908
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefActorStatusModifierSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefActorStatusModifierSpec) == AssetRefActorStatusModifierSpec::SIZE);

struct AssetRefActorStatusSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c928 (Equals), 0x7b8c9a8 (Equals)
    friend constexpr bool operator==(AssetRefActorStatusSpec a, AssetRefActorStatusSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefActorStatusSpec other) const { return Id == other.Id; }
    // 0x7b8c9b0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(167u, Id); }
    // 0x7b8c91c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8c9cc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefActorStatusSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefActorStatusSpec) == AssetRefActorStatusSpec::SIZE);

struct AssetRefAddDamageIncreaseOnTargetedEffectSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8c9ec (Equals), 0x7b8ca6c (Equals)
    friend constexpr bool operator==(AssetRefAddDamageIncreaseOnTargetedEffectSpec a, AssetRefAddDamageIncreaseOnTargetedEffectSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAddDamageIncreaseOnTargetedEffectSpec other) const { return Id == other.Id; }
    // 0x7b8ca74
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(173u, Id); }
    // 0x7b8c9e0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAddDamageIncreaseOnTargetedEffectSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAddDamageIncreaseOnTargetedEffectSpec) == AssetRefAddDamageIncreaseOnTargetedEffectSpec::SIZE);

struct AssetRefAddEntitiesToSpawnOnProjectileDeathSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8ca9c (Equals), 0x7b8cb1c (Equals)
    friend constexpr bool operator==(AssetRefAddEntitiesToSpawnOnProjectileDeathSpec a, AssetRefAddEntitiesToSpawnOnProjectileDeathSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAddEntitiesToSpawnOnProjectileDeathSpec other) const { return Id == other.Id; }
    // 0x7b8cb24
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(179u, Id); }
    // 0x7b8ca90
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8cb40
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAddEntitiesToSpawnOnProjectileDeathSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b7cfe4, Quantum.AssetRefAddEntitiesToSpawnOnProjectileDeathSpec.op_Implicit): AddEntitiesToSpawnOnProjectileDeathSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefAddEntitiesToSpawnOnProjectileDeathSpec) == AssetRefAddEntitiesToSpawnOnProjectileDeathSpec::SIZE);

struct AssetRefAmmunitionDamageSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8cb60 (Equals), 0x7b8cbe0 (Equals)
    friend constexpr bool operator==(AssetRefAmmunitionDamageSpec a, AssetRefAmmunitionDamageSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAmmunitionDamageSpec other) const { return Id == other.Id; }
    // 0x7b8cbe8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(181u, Id); }
    // 0x7b8cb54
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8cc04
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAmmunitionDamageSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAmmunitionDamageSpec) == AssetRefAmmunitionDamageSpec::SIZE);

struct AssetRefAmoeboidLauncherEnhancementPuddleSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8cc34 (Equals), 0x7b8ccb4 (Equals)
    friend constexpr bool operator==(AssetRefAmoeboidLauncherEnhancementPuddleSpec a, AssetRefAmoeboidLauncherEnhancementPuddleSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAmoeboidLauncherEnhancementPuddleSpec other) const { return Id == other.Id; }
    // 0x7b8ccbc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(191u, Id); }
    // 0x7b8cc18
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8ccd8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAmoeboidLauncherEnhancementPuddleSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8cc24, Quantum.AssetRefAmoeboidLauncherEnhancementPuddleSpec.op_Implicit): AmoeboidLauncherEnhancementPuddleSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefAmoeboidLauncherEnhancementPuddleSpec) == AssetRefAmoeboidLauncherEnhancementPuddleSpec::SIZE);

struct AssetRefAmoeboidLevelUpActorSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8ccf8 (Equals), 0x7b8cd78 (Equals)
    friend constexpr bool operator==(AssetRefAmoeboidLevelUpActorSpec a, AssetRefAmoeboidLevelUpActorSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAmoeboidLevelUpActorSpec other) const { return Id == other.Id; }
    // 0x7b8cd80
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(193u, Id); }
    // 0x7b8ccec
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAmoeboidLevelUpActorSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAmoeboidLevelUpActorSpec) == AssetRefAmoeboidLevelUpActorSpec::SIZE);

struct AssetRefAoeTrailSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8cda8 (Equals), 0x7b8ce28 (Equals)
    friend constexpr bool operator==(AssetRefAoeTrailSpec a, AssetRefAoeTrailSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAoeTrailSpec other) const { return Id == other.Id; }
    // 0x7b8ce30
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(197u, Id); }
    // 0x7b8cd9c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8ce4c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAoeTrailSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAoeTrailSpec) == AssetRefAoeTrailSpec::SIZE);

struct AssetRefAreaOfEffectIgnitionSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8ce6c (Equals), 0x7b8ceec (Equals)
    friend constexpr bool operator==(AssetRefAreaOfEffectIgnitionSpec a, AssetRefAreaOfEffectIgnitionSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAreaOfEffectIgnitionSpec other) const { return Id == other.Id; }
    // 0x7b8cef4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(199u, Id); }
    // 0x7b8ce60
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAreaOfEffectIgnitionSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAreaOfEffectIgnitionSpec) == AssetRefAreaOfEffectIgnitionSpec::SIZE);

struct AssetRefAreaOfEffectLevelUpSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8cf1c (Equals), 0x7b8cf9c (Equals)
    friend constexpr bool operator==(AssetRefAreaOfEffectLevelUpSpec a, AssetRefAreaOfEffectLevelUpSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAreaOfEffectLevelUpSpec other) const { return Id == other.Id; }
    // 0x7b8cfa4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(211u, Id); }
    // 0x7b8cf10
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8cfc0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAreaOfEffectLevelUpSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAreaOfEffectLevelUpSpec) == AssetRefAreaOfEffectLevelUpSpec::SIZE);

struct AssetRefAreaOfEffectSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8cfe0 (Equals), 0x7b8d060 (Equals)
    friend constexpr bool operator==(AssetRefAreaOfEffectSpec a, AssetRefAreaOfEffectSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefAreaOfEffectSpec other) const { return Id == other.Id; }
    // 0x7b8d068
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(223u, Id); }
    // 0x7b8cfd4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d084
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefAreaOfEffectSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefAreaOfEffectSpec) == AssetRefAreaOfEffectSpec::SIZE);

struct AssetRefBTComposite {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d0a4 (Equals), 0x7b8d124 (Equals)
    friend constexpr bool operator==(AssetRefBTComposite a, AssetRefBTComposite b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBTComposite other) const { return Id == other.Id; }
    // 0x7b8d12c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(227u, Id); }
    // 0x7b8d098
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d148
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBTComposite*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBTComposite) == AssetRefBTComposite::SIZE);

struct AssetRefBTDecorator {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d168 (Equals), 0x7b8d1e8 (Equals)
    friend constexpr bool operator==(AssetRefBTDecorator a, AssetRefBTDecorator b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBTDecorator other) const { return Id == other.Id; }
    // 0x7b8d1f0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(229u, Id); }
    // 0x7b8d15c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d20c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBTDecorator*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBTDecorator) == AssetRefBTDecorator::SIZE);

struct AssetRefBTNode {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b7e15c (!=), 0x7b8d22c (Equals), 0x7b8d2ac (Equals)
    friend constexpr bool operator==(AssetRefBTNode a, AssetRefBTNode b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBTNode other) const { return Id == other.Id; }
    // 0x7b7d680
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(233u, Id); }
    // 0x7b8d220
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b7da38
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBTNode*>(ptr)->Id, serializer);
    }
    // TODO(0x7b7e14c, Quantum.AssetRefBTNode.op_Implicit): BTNode asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefBTNode) == AssetRefBTNode::SIZE);

struct AssetRefBTRoot {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d2c0 (Equals), 0x7b8d340 (Equals)
    friend constexpr bool operator==(AssetRefBTRoot a, AssetRefBTRoot b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBTRoot other) const { return Id == other.Id; }
    // 0x7b7d69c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(239u, Id); }
    // 0x7b8d2b4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b7da4c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBTRoot*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBTRoot) == AssetRefBTRoot::SIZE);

struct AssetRefBTService {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d354 (Equals), 0x7b8d3d4 (Equals)
    friend constexpr bool operator==(AssetRefBTService a, AssetRefBTService b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBTService other) const { return Id == other.Id; }
    // 0x7b8d3dc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(241u, Id); }
    // 0x7b8d348
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d3f8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBTService*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBTService) == AssetRefBTService::SIZE);

struct AssetRefBigBoomMineSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d418 (Equals), 0x7b8d498 (Equals)
    friend constexpr bool operator==(AssetRefBigBoomMineSpec a, AssetRefBigBoomMineSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBigBoomMineSpec other) const { return Id == other.Id; }
    // 0x7b8d4a0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(251u, Id); }
    // 0x7b8d40c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d4bc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBigBoomMineSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBigBoomMineSpec) == AssetRefBigBoomMineSpec::SIZE);

struct AssetRefBlastBallSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d4dc (Equals), 0x7b8d55c (Equals)
    friend constexpr bool operator==(AssetRefBlastBallSpec a, AssetRefBlastBallSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBlastBallSpec other) const { return Id == other.Id; }
    // 0x7b8d564
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(257u, Id); }
    // 0x7b8d4d0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d580
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBlastBallSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBlastBallSpec) == AssetRefBlastBallSpec::SIZE);

struct AssetRefBlasterMoveSpeedEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d5b0 (Equals), 0x7b8d630 (Equals)
    friend constexpr bool operator==(AssetRefBlasterMoveSpeedEnhancementSpec a, AssetRefBlasterMoveSpeedEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBlasterMoveSpeedEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b8d638
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(263u, Id); }
    // 0x7b8d594
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d654
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBlasterMoveSpeedEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8d5a0, Quantum.AssetRefBlasterMoveSpeedEnhancementSpec.op_Implicit): BlasterMoveSpeedEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefBlasterMoveSpeedEnhancementSpec) == AssetRefBlasterMoveSpeedEnhancementSpec::SIZE);

struct AssetRefBlasterSpecialShotEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d684 (Equals), 0x7b8d704 (Equals)
    friend constexpr bool operator==(AssetRefBlasterSpecialShotEnhancementSpec a, AssetRefBlasterSpecialShotEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBlasterSpecialShotEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b8d70c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(269u, Id); }
    // 0x7b8d668
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d728
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBlasterSpecialShotEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8d674, Quantum.AssetRefBlasterSpecialShotEnhancementSpec.op_Implicit): BlasterSpecialShotEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefBlasterSpecialShotEnhancementSpec) == AssetRefBlasterSpecialShotEnhancementSpec::SIZE);

struct AssetRefBoltDropperSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d7e0 (==), 0x7b8d758 (Equals), 0x7b8d7d8 (Equals)
    friend constexpr bool operator==(AssetRefBoltDropperSpec a, AssetRefBoltDropperSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBoltDropperSpec other) const { return Id == other.Id; }
    // 0x7b8d7e8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(271u, Id); }
    // 0x7b8d73c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBoltDropperSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8d748, Quantum.AssetRefBoltDropperSpec.op_Implicit): BoltDropperSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefBoltDropperSpec) == AssetRefBoltDropperSpec::SIZE);

struct AssetRefBoltGathererActorSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d8a8 (==), 0x7b8d820 (Equals), 0x7b8d8a0 (Equals)
    friend constexpr bool operator==(AssetRefBoltGathererActorSpec a, AssetRefBoltGathererActorSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBoltGathererActorSpec other) const { return Id == other.Id; }
    // 0x7b8d8b0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(277u, Id); }
    // 0x7b8d804
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d8cc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBoltGathererActorSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8d810, Quantum.AssetRefBoltGathererActorSpec.op_Implicit): BoltGathererActorSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefBoltGathererActorSpec) == AssetRefBoltGathererActorSpec::SIZE);

struct AssetRefBoltPowerActorSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d984 (==), 0x7b8d8fc (Equals), 0x7b8d97c (Equals)
    friend constexpr bool operator==(AssetRefBoltPowerActorSpec a, AssetRefBoltPowerActorSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBoltPowerActorSpec other) const { return Id == other.Id; }
    // 0x7b8d98c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(281u, Id); }
    // 0x7b8d8e0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8d9a8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBoltPowerActorSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8d8ec, Quantum.AssetRefBoltPowerActorSpec.op_Implicit): BoltPowerActorSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefBoltPowerActorSpec) == AssetRefBoltPowerActorSpec::SIZE);

struct AssetRefBombardierDroneSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8d9c8 (Equals), 0x7b8da48 (Equals)
    friend constexpr bool operator==(AssetRefBombardierDroneSpec a, AssetRefBombardierDroneSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBombardierDroneSpec other) const { return Id == other.Id; }
    // 0x7b8da50
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(283u, Id); }
    // 0x7b8d9bc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8da6c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBombardierDroneSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBombardierDroneSpec) == AssetRefBombardierDroneSpec::SIZE);

struct AssetRefBombardierWeaponAutoTargetSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8da8c (Equals), 0x7b8db0c (Equals)
    friend constexpr bool operator==(AssetRefBombardierWeaponAutoTargetSpec a, AssetRefBombardierWeaponAutoTargetSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBombardierWeaponAutoTargetSpec other) const { return Id == other.Id; }
    // 0x7b8db14
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(293u, Id); }
    // 0x7b8da80
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8db30
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBombardierWeaponAutoTargetSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBombardierWeaponAutoTargetSpec) == AssetRefBombardierWeaponAutoTargetSpec::SIZE);

struct AssetRefBumperBarConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8db50 (Equals), 0x7b8dbd0 (Equals)
    friend constexpr bool operator==(AssetRefBumperBarConfig a, AssetRefBumperBarConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBumperBarConfig other) const { return Id == other.Id; }
    // 0x7b8dbd8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(307u, Id); }
    // 0x7b8db44
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8dbf4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBumperBarConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBumperBarConfig) == AssetRefBumperBarConfig::SIZE);

struct AssetRefBumperConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8dc14 (Equals), 0x7b8dc94 (Equals)
    friend constexpr bool operator==(AssetRefBumperConfig a, AssetRefBumperConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBumperConfig other) const { return Id == other.Id; }
    // 0x7b8dc9c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(311u, Id); }
    // 0x7b8dc08
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8dcb8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBumperConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBumperConfig) == AssetRefBumperConfig::SIZE);

struct AssetRefBuzzbladesRicochetSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8dcd8 (Equals), 0x7b8dd58 (Equals)
    friend constexpr bool operator==(AssetRefBuzzbladesRicochetSpec a, AssetRefBuzzbladesRicochetSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefBuzzbladesRicochetSpec other) const { return Id == other.Id; }
    // 0x7b8dd60
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(313u, Id); }
    // 0x7b8dccc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8dd7c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefBuzzbladesRicochetSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefBuzzbladesRicochetSpec) == AssetRefBuzzbladesRicochetSpec::SIZE);

struct AssetRefCallbackAction {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8de34 (==), 0x7b8ddac (Equals), 0x7b8de2c (Equals)
    friend constexpr bool operator==(AssetRefCallbackAction a, AssetRefCallbackAction b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefCallbackAction other) const { return Id == other.Id; }
    // 0x7b8de3c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(317u, Id); }
    // 0x7b8dd90
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8de58
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefCallbackAction*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8dd9c, Quantum.AssetRefCallbackAction.op_Implicit): CallbackAction asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefCallbackAction) == AssetRefCallbackAction::SIZE);

struct AssetRefChargeAttackSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8de78 (Equals), 0x7b8def8 (Equals)
    friend constexpr bool operator==(AssetRefChargeAttackSpec a, AssetRefChargeAttackSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefChargeAttackSpec other) const { return Id == other.Id; }
    // 0x7b8df00
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(331u, Id); }
    // 0x7b8de6c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8df1c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefChargeAttackSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefChargeAttackSpec) == AssetRefChargeAttackSpec::SIZE);

struct AssetRefColliderResizeSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8df3c (Equals), 0x7b8dfbc (Equals)
    friend constexpr bool operator==(AssetRefColliderResizeSpec a, AssetRefColliderResizeSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefColliderResizeSpec other) const { return Id == other.Id; }
    // 0x7b8dfc4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(337u, Id); }
    // 0x7b8df30
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8dfe0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefColliderResizeSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefColliderResizeSpec) == AssetRefColliderResizeSpec::SIZE);

struct AssetRefCollisionForceSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e000 (Equals), 0x7b8e080 (Equals)
    friend constexpr bool operator==(AssetRefCollisionForceSpec a, AssetRefCollisionForceSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefCollisionForceSpec other) const { return Id == other.Id; }
    // 0x7b8e088
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(347u, Id); }
    // 0x7b8dff4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e0a4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefCollisionForceSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefCollisionForceSpec) == AssetRefCollisionForceSpec::SIZE);

struct AssetRefCompoundHFSMSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e0c4 (Equals), 0x7b8e144 (Equals)
    friend constexpr bool operator==(AssetRefCompoundHFSMSpec a, AssetRefCompoundHFSMSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefCompoundHFSMSpec other) const { return Id == other.Id; }
    // 0x7b8448c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(349u, Id); }
    // 0x7b8e0b8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b846a0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefCompoundHFSMSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefCompoundHFSMSpec) == AssetRefCompoundHFSMSpec::SIZE);

struct AssetRefConsideration {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b867f8 (==), 0x7b86e1c (!=), 0x7b8e158 (Equals), 0x7b8e1d8 (Equals)
    friend constexpr bool operator==(AssetRefConsideration a, AssetRefConsideration b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefConsideration other) const { return Id == other.Id; }
    // 0x7b8e1e0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(353u, Id); }
    // 0x7b8e14c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e1fc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefConsideration*>(ptr)->Id, serializer);
    }
    // TODO(0x7b849a8, Quantum.AssetRefConsideration.op_Implicit): Consideration asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefConsideration) == AssetRefConsideration::SIZE);

struct AssetRefCryosliderSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e21c (Equals), 0x7b8e29c (Equals)
    friend constexpr bool operator==(AssetRefCryosliderSpec a, AssetRefCryosliderSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefCryosliderSpec other) const { return Id == other.Id; }
    // 0x7b8e2a4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(359u, Id); }
    // 0x7b8e210
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e2c0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefCryosliderSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefCryosliderSpec) == AssetRefCryosliderSpec::SIZE);

struct AssetRefCryosliderWeaponAutoTargetSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e2e0 (Equals), 0x7b8e360 (Equals)
    friend constexpr bool operator==(AssetRefCryosliderWeaponAutoTargetSpec a, AssetRefCryosliderWeaponAutoTargetSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefCryosliderWeaponAutoTargetSpec other) const { return Id == other.Id; }
    // 0x7b8e368
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(367u, Id); }
    // 0x7b8e2d4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e384
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefCryosliderWeaponAutoTargetSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefCryosliderWeaponAutoTargetSpec) == AssetRefCryosliderWeaponAutoTargetSpec::SIZE);

struct AssetRefDamageChargeableSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e3a4 (Equals), 0x7b8e424 (Equals)
    friend constexpr bool operator==(AssetRefDamageChargeableSpec a, AssetRefDamageChargeableSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDamageChargeableSpec other) const { return Id == other.Id; }
    // 0x7b8e42c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(373u, Id); }
    // 0x7b8e398
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e448
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDamageChargeableSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefDamageChargeableSpec) == AssetRefDamageChargeableSpec::SIZE);

struct AssetRefDamageIncreasedOnTargetsEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e478 (Equals), 0x7b8e4f8 (Equals)
    friend constexpr bool operator==(AssetRefDamageIncreasedOnTargetsEnhancementSpec a, AssetRefDamageIncreasedOnTargetsEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDamageIncreasedOnTargetsEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b8e500
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(379u, Id); }
    // 0x7b8e45c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e51c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDamageIncreasedOnTargetsEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8e468, Quantum.AssetRefDamageIncreasedOnTargetsEnhancementSpec.op_Implicit): DamageIncreasedOnTargetsEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefDamageIncreasedOnTargetsEnhancementSpec) == AssetRefDamageIncreasedOnTargetsEnhancementSpec::SIZE);

struct AssetRefDamageMultiplierHeroImprovementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e53c (Equals), 0x7b8e5bc (Equals)
    friend constexpr bool operator==(AssetRefDamageMultiplierHeroImprovementSpec a, AssetRefDamageMultiplierHeroImprovementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDamageMultiplierHeroImprovementSpec other) const { return Id == other.Id; }
    // 0x7b8e5c4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(383u, Id); }
    // 0x7b8e530
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDamageMultiplierHeroImprovementSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefDamageMultiplierHeroImprovementSpec) == AssetRefDamageMultiplierHeroImprovementSpec::SIZE);

struct AssetRefDamageMultiplierSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e5ec (Equals), 0x7b8e66c (Equals)
    friend constexpr bool operator==(AssetRefDamageMultiplierSpec a, AssetRefDamageMultiplierSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDamageMultiplierSpec other) const { return Id == other.Id; }
    // 0x7b8e674
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(389u, Id); }
    // 0x7b8e5e0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e690
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDamageMultiplierSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefDamageMultiplierSpec) == AssetRefDamageMultiplierSpec::SIZE);

struct AssetRefDamageSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e6b0 (Equals), 0x7b8e730 (Equals)
    friend constexpr bool operator==(AssetRefDamageSpec a, AssetRefDamageSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDamageSpec other) const { return Id == other.Id; }
    // 0x7b8e738
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(397u, Id); }
    // 0x7b8e6a4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e754
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDamageSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefDamageSpec) == AssetRefDamageSpec::SIZE);

struct AssetRefDamageToImpulseConverterSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e774 (Equals), 0x7b8e7f4 (Equals)
    friend constexpr bool operator==(AssetRefDamageToImpulseConverterSpec a, AssetRefDamageToImpulseConverterSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDamageToImpulseConverterSpec other) const { return Id == other.Id; }
    // 0x7b8e7fc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(401u, Id); }
    // 0x7b8e768
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e818
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDamageToImpulseConverterSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefDamageToImpulseConverterSpec) == AssetRefDamageToImpulseConverterSpec::SIZE);

struct AssetRefDamageableSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e838 (Equals), 0x7b8e8b8 (Equals)
    friend constexpr bool operator==(AssetRefDamageableSpec a, AssetRefDamageableSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDamageableSpec other) const { return Id == other.Id; }
    // 0x7b8e8c0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(409u, Id); }
    // 0x7b8e82c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e8dc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDamageableSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefDamageableSpec) == AssetRefDamageableSpec::SIZE);

struct AssetRefDashActorConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e994 (==), 0x7b8e90c (Equals), 0x7b8e98c (Equals)
    friend constexpr bool operator==(AssetRefDashActorConfig a, AssetRefDashActorConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDashActorConfig other) const { return Id == other.Id; }
    // 0x7b8e99c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(419u, Id); }
    // 0x7b8e8f0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8e9b8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDashActorConfig*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8e8fc, Quantum.AssetRefDashActorConfig.op_Implicit): DashActorConfig asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefDashActorConfig) == AssetRefDashActorConfig::SIZE);

struct AssetRefDeathAnimationSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8e9d8 (Equals), 0x7b8ea58 (Equals)
    friend constexpr bool operator==(AssetRefDeathAnimationSpec a, AssetRefDeathAnimationSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDeathAnimationSpec other) const { return Id == other.Id; }
    // 0x7b8ea60
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(421u, Id); }
    // 0x7b8e9cc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8ea7c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDeathAnimationSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefDeathAnimationSpec) == AssetRefDeathAnimationSpec::SIZE);

struct AssetRefDroppableCollectorSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8eb34 (==), 0x7b8eaac (Equals), 0x7b8eb2c (Equals)
    friend constexpr bool operator==(AssetRefDroppableCollectorSpec a, AssetRefDroppableCollectorSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDroppableCollectorSpec other) const { return Id == other.Id; }
    // 0x7b8eb3c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(431u, Id); }
    // 0x7b8ea90
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8eb58
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDroppableCollectorSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8ea9c, Quantum.AssetRefDroppableCollectorSpec.op_Implicit): DroppableCollectorSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefDroppableCollectorSpec) == AssetRefDroppableCollectorSpec::SIZE);

struct AssetRefDroppableManagerSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8eb78 (Equals), 0x7b8ebf8 (Equals)
    friend constexpr bool operator==(AssetRefDroppableManagerSpec a, AssetRefDroppableManagerSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefDroppableManagerSpec other) const { return Id == other.Id; }
    // 0x7b8ec00
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(433u, Id); }
    // 0x7b8eb6c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8ec1c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefDroppableManagerSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefDroppableManagerSpec) == AssetRefDroppableManagerSpec::SIZE);

struct AssetRefElementalDamageTable {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8ec3c (Equals), 0x7b8ecbc (Equals)
    friend constexpr bool operator==(AssetRefElementalDamageTable a, AssetRefElementalDamageTable b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefElementalDamageTable other) const { return Id == other.Id; }
    // 0x7b8ecc4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(439u, Id); }
    // 0x7b8ec30
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefElementalDamageTable*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefElementalDamageTable) == AssetRefElementalDamageTable::SIZE);

struct AssetRefEliminationGameModeSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8ecec (Equals), 0x7b8ed6c (Equals)
    friend constexpr bool operator==(AssetRefEliminationGameModeSpec a, AssetRefEliminationGameModeSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefEliminationGameModeSpec other) const { return Id == other.Id; }
    // 0x7b8ed74
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(443u, Id); }
    // 0x7b8ece0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8ed90
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEliminationGameModeSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefEliminationGameModeSpec) == AssetRefEliminationGameModeSpec::SIZE);

struct AssetRefEnergyCellDropperSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8edb0 (Equals), 0x7b8ee30 (Equals)
    friend constexpr bool operator==(AssetRefEnergyCellDropperSpec a, AssetRefEnergyCellDropperSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefEnergyCellDropperSpec other) const { return Id == other.Id; }
    // 0x7b8ee38
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(449u, Id); }
    // 0x7b8eda4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEnergyCellDropperSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefEnergyCellDropperSpec) == AssetRefEnergyCellDropperSpec::SIZE);

struct AssetRefEnergyCellManagerSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8ee60 (Equals), 0x7b8eee0 (Equals)
    friend constexpr bool operator==(AssetRefEnergyCellManagerSpec a, AssetRefEnergyCellManagerSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefEnergyCellManagerSpec other) const { return Id == other.Id; }
    // 0x7b8eee8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(457u, Id); }
    // 0x7b8ee54
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8ef04
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEnergyCellManagerSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefEnergyCellManagerSpec) == AssetRefEnergyCellManagerSpec::SIZE);

struct AssetRefEnergyCellPowerUpSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8ef24 (Equals), 0x7b8efa4 (Equals)
    friend constexpr bool operator==(AssetRefEnergyCellPowerUpSpec a, AssetRefEnergyCellPowerUpSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefEnergyCellPowerUpSpec other) const { return Id == other.Id; }
    // 0x7b8efac
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(461u, Id); }
    // 0x7b8ef18
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEnergyCellPowerUpSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefEnergyCellPowerUpSpec) == AssetRefEnergyCellPowerUpSpec::SIZE);

struct AssetRefEnforcerFirstShotBonusEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8efe4 (Equals), 0x7b8f064 (Equals)
    friend constexpr bool operator==(AssetRefEnforcerFirstShotBonusEnhancementSpec a, AssetRefEnforcerFirstShotBonusEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefEnforcerFirstShotBonusEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b8f06c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(463u, Id); }
    // 0x7b8efc8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f088
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEnforcerFirstShotBonusEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8efd4, Quantum.AssetRefEnforcerFirstShotBonusEnhancementSpec.op_Implicit): EnforcerFirstShotBonusEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefEnforcerFirstShotBonusEnhancementSpec) == AssetRefEnforcerFirstShotBonusEnhancementSpec::SIZE);

struct AssetRefEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f0a8 (Equals), 0x7b8f128 (Equals)
    friend constexpr bool operator==(AssetRefEnhancementSpec a, AssetRefEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b8f130
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(467u, Id); }
    // 0x7b8f09c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEnhancementSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefEnhancementSpec) == AssetRefEnhancementSpec::SIZE);

struct AssetRefEntityDropperSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f1f0 (==), 0x7b8f168 (Equals), 0x7b8f1e8 (Equals)
    friend constexpr bool operator==(AssetRefEntityDropperSpec a, AssetRefEntityDropperSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefEntityDropperSpec other) const { return Id == other.Id; }
    // 0x7b8f1f8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(479u, Id); }
    // 0x7b8f14c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f214
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEntityDropperSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8f158, Quantum.AssetRefEntityDropperSpec.op_Implicit): EntityDropperSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefEntityDropperSpec) == AssetRefEntityDropperSpec::SIZE);

struct AssetRefEntityGroupSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f234 (Equals), 0x7b8f2b4 (Equals)
    friend constexpr bool operator==(AssetRefEntityGroupSpec a, AssetRefEntityGroupSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefEntityGroupSpec other) const { return Id == other.Id; }
    // 0x7b8f2bc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(487u, Id); }
    // 0x7b8f228
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f2d8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEntityGroupSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefEntityGroupSpec) == AssetRefEntityGroupSpec::SIZE);

struct AssetRefFireReactorSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f2f8 (Equals), 0x7b8f378 (Equals)
    friend constexpr bool operator==(AssetRefFireReactorSpec a, AssetRefFireReactorSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefFireReactorSpec other) const { return Id == other.Id; }
    // 0x7b8f380
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(491u, Id); }
    // 0x7b8f2ec
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f39c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefFireReactorSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefFireReactorSpec) == AssetRefFireReactorSpec::SIZE);

struct AssetRefGOAPAction {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f3bc (Equals), 0x7b8f43c (Equals)
    friend constexpr bool operator==(AssetRefGOAPAction a, AssetRefGOAPAction b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGOAPAction other) const { return Id == other.Id; }
    // 0x7b8f444
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(499u, Id); }
    // 0x7b8f3b0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f460
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGOAPAction*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGOAPAction) == AssetRefGOAPAction::SIZE);

struct AssetRefGOAPBackValidation {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f480 (Equals), 0x7b8f500 (Equals)
    friend constexpr bool operator==(AssetRefGOAPBackValidation a, AssetRefGOAPBackValidation b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGOAPBackValidation other) const { return Id == other.Id; }
    // 0x7b8f508
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(503u, Id); }
    // 0x7b8f474
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGOAPBackValidation*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGOAPBackValidation) == AssetRefGOAPBackValidation::SIZE);

struct AssetRefGOAPGoal {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f530 (Equals), 0x7b8f5b0 (Equals)
    friend constexpr bool operator==(AssetRefGOAPGoal a, AssetRefGOAPGoal b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGOAPGoal other) const { return Id == other.Id; }
    // 0x7b80558
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(509u, Id); }
    // 0x7b8f524
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b80930
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGOAPGoal*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGOAPGoal) == AssetRefGOAPGoal::SIZE);

struct AssetRefGOAPHeuristic {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f5c4 (Equals), 0x7b8f644 (Equals)
    friend constexpr bool operator==(AssetRefGOAPHeuristic a, AssetRefGOAPHeuristic b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGOAPHeuristic other) const { return Id == other.Id; }
    // 0x7b8f64c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(521u, Id); }
    // 0x7b8f5b8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f668
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGOAPHeuristic*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGOAPHeuristic) == AssetRefGOAPHeuristic::SIZE);

struct AssetRefGOAPRoot {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f688 (Equals), 0x7b8f708 (Equals)
    friend constexpr bool operator==(AssetRefGOAPRoot a, AssetRefGOAPRoot b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGOAPRoot other) const { return Id == other.Id; }
    // 0x7b8058c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(523u, Id); }
    // 0x7b8f67c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b80944
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGOAPRoot*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGOAPRoot) == AssetRefGOAPRoot::SIZE);

struct AssetRefGadgetSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f7b4 (==), 0x7b8f72c (Equals), 0x7b8f7ac (Equals)
    friend constexpr bool operator==(AssetRefGadgetSpec a, AssetRefGadgetSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGadgetSpec other) const { return Id == other.Id; }
    // 0x7b8f7bc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(541u, Id); }
    // 0x7b8f710
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f7d8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGadgetSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b8f71c, Quantum.AssetRefGadgetSpec.op_Implicit): GadgetSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefGadgetSpec) == AssetRefGadgetSpec::SIZE);

struct AssetRefGameManagerConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f7f8 (Equals), 0x7b8f878 (Equals)
    friend constexpr bool operator==(AssetRefGameManagerConfig a, AssetRefGameManagerConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGameManagerConfig other) const { return Id == other.Id; }
    // 0x7b8f880
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(547u, Id); }
    // 0x7b8f7ec
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f89c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGameManagerConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGameManagerConfig) == AssetRefGameManagerConfig::SIZE);

struct AssetRefGameModeHandlerConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f8bc (Equals), 0x7b8f93c (Equals)
    friend constexpr bool operator==(AssetRefGameModeHandlerConfig a, AssetRefGameModeHandlerConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGameModeHandlerConfig other) const { return Id == other.Id; }
    // 0x7b8f944
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(557u, Id); }
    // 0x7b8f8b0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8f960
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGameModeHandlerConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGameModeHandlerConfig) == AssetRefGameModeHandlerConfig::SIZE);

struct AssetRefGameModeHandlerLevelConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8f980 (Equals), 0x7b8fa00 (Equals)
    friend constexpr bool operator==(AssetRefGameModeHandlerLevelConfig a, AssetRefGameModeHandlerLevelConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGameModeHandlerLevelConfig other) const { return Id == other.Id; }
    // 0x7b8fa08
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(563u, Id); }
    // 0x7b8f974
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8fa24
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGameModeHandlerLevelConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGameModeHandlerLevelConfig) == AssetRefGameModeHandlerLevelConfig::SIZE);

struct AssetRefGameModeSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8fa44 (Equals), 0x7b8fac4 (Equals)
    friend constexpr bool operator==(AssetRefGameModeSpec a, AssetRefGameModeSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGameModeSpec other) const { return Id == other.Id; }
    // 0x7b8facc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(569u, Id); }
    // 0x7b8fa38
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8fae8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGameModeSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGameModeSpec) == AssetRefGameModeSpec::SIZE);

struct AssetRefGameModesTable {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8fb08 (Equals), 0x7b8fb88 (Equals)
    friend constexpr bool operator==(AssetRefGameModesTable a, AssetRefGameModesTable b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGameModesTable other) const { return Id == other.Id; }
    // 0x7b8fb90
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(571u, Id); }
    // 0x7b8fafc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8fbac
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGameModesTable*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGameModesTable) == AssetRefGameModesTable::SIZE);

struct AssetRefGameModifiersSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b8fbcc (Equals), 0x7b8fc4c (Equals)
    friend constexpr bool operator==(AssetRefGameModifiersSpec a, AssetRefGameModifiersSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGameModifiersSpec other) const { return Id == other.Id; }
    // 0x7b8fc54
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(577u, Id); }
    // 0x7b8fbc0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b8fc70
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGameModifiersSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGameModifiersSpec) == AssetRefGameModifiersSpec::SIZE);

struct AssetRefGatlingEnhancementResetAmmoOnGadgetUseSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90064 (Equals), 0x7b900e4 (Equals)
    friend constexpr bool operator==(AssetRefGatlingEnhancementResetAmmoOnGadgetUseSpec a, AssetRefGatlingEnhancementResetAmmoOnGadgetUseSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGatlingEnhancementResetAmmoOnGadgetUseSpec other) const { return Id == other.Id; }
    // 0x7b900ec
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(587u, Id); }
    // 0x7b90058
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGatlingEnhancementResetAmmoOnGadgetUseSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGatlingEnhancementResetAmmoOnGadgetUseSpec) == AssetRefGatlingEnhancementResetAmmoOnGadgetUseSpec::SIZE);

struct AssetRefGatlingEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90124 (Equals), 0x7b901a4 (Equals)
    friend constexpr bool operator==(AssetRefGatlingEnhancementSpec a, AssetRefGatlingEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGatlingEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b901ac
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(593u, Id); }
    // 0x7b90108
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b901c8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGatlingEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b90114, Quantum.AssetRefGatlingEnhancementSpec.op_Implicit): GatlingEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefGatlingEnhancementSpec) == AssetRefGatlingEnhancementSpec::SIZE);

struct AssetRefGoldenBoltConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b901e8 (Equals), 0x7b90268 (Equals)
    friend constexpr bool operator==(AssetRefGoldenBoltConfig a, AssetRefGoldenBoltConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGoldenBoltConfig other) const { return Id == other.Id; }
    // 0x7b90270
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(599u, Id); }
    // 0x7b901dc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGoldenBoltConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGoldenBoltConfig) == AssetRefGoldenBoltConfig::SIZE);

struct AssetRefGrenadeLauncherAftershockEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b902a8 (Equals), 0x7b90328 (Equals)
    friend constexpr bool operator==(AssetRefGrenadeLauncherAftershockEnhancementSpec a, AssetRefGrenadeLauncherAftershockEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGrenadeLauncherAftershockEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b90330
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(601u, Id); }
    // 0x7b9028c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9034c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGrenadeLauncherAftershockEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b90298, Quantum.AssetRefGrenadeLauncherAftershockEnhancementSpec.op_Implicit): GrenadeLauncherAftershockEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefGrenadeLauncherAftershockEnhancementSpec) == AssetRefGrenadeLauncherAftershockEnhancementSpec::SIZE);

struct AssetRefGrenadeLauncherStunGrenadeEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9037c (Equals), 0x7b903fc (Equals)
    friend constexpr bool operator==(AssetRefGrenadeLauncherStunGrenadeEnhancementSpec a, AssetRefGrenadeLauncherStunGrenadeEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGrenadeLauncherStunGrenadeEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b90404
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(607u, Id); }
    // 0x7b90360
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b90420
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGrenadeLauncherStunGrenadeEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b9036c, Quantum.AssetRefGrenadeLauncherStunGrenadeEnhancementSpec.op_Implicit): GrenadeLauncherStunGrenadeEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefGrenadeLauncherStunGrenadeEnhancementSpec) == AssetRefGrenadeLauncherStunGrenadeEnhancementSpec::SIZE);

struct AssetRefGrinderActorConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90440 (Equals), 0x7b904c0 (Equals)
    friend constexpr bool operator==(AssetRefGrinderActorConfig a, AssetRefGrinderActorConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGrinderActorConfig other) const { return Id == other.Id; }
    // 0x7b904c8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(613u, Id); }
    // 0x7b90434
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b904e4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGrinderActorConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGrinderActorConfig) == AssetRefGrinderActorConfig::SIZE);

struct AssetRefGroundDashActorConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90504 (Equals), 0x7b90584 (Equals)
    friend constexpr bool operator==(AssetRefGroundDashActorConfig a, AssetRefGroundDashActorConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefGroundDashActorConfig other) const { return Id == other.Id; }
    // 0x7b9058c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(617u, Id); }
    // 0x7b904f8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b905a8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefGroundDashActorConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefGroundDashActorConfig) == AssetRefGroundDashActorConfig::SIZE);

struct AssetRefHFSMDecision {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b905c8 (Equals), 0x7b90648 (Equals)
    friend constexpr bool operator==(AssetRefHFSMDecision a, AssetRefHFSMDecision b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHFSMDecision other) const { return Id == other.Id; }
    // 0x7b90650
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(619u, Id); }
    // 0x7b905bc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHFSMDecision*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefHFSMDecision) == AssetRefHFSMDecision::SIZE);

struct AssetRefHFSMRoot {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90688 (Equals), 0x7b90708 (Equals)
    friend constexpr bool operator==(AssetRefHFSMRoot a, AssetRefHFSMRoot b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHFSMRoot other) const { return Id == other.Id; }
    // 0x7b90710
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(631u, Id); }
    // 0x7b9066c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9072c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHFSMRoot*>(ptr)->Id, serializer);
    }
    // TODO(0x7b90678, Quantum.AssetRefHFSMRoot.op_Implicit): HFSMRoot asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefHFSMRoot) == AssetRefHFSMRoot::SIZE);

struct AssetRefHFSMState {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9075c (Equals), 0x7b907dc (Equals)
    friend constexpr bool operator==(AssetRefHFSMState a, AssetRefHFSMState b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHFSMState other) const { return Id == other.Id; }
    // 0x7b907e4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(641u, Id); }
    // 0x7b90740
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b90800
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHFSMState*>(ptr)->Id, serializer);
    }
    // TODO(0x7b9074c, Quantum.AssetRefHFSMState.op_Implicit): HFSMState asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefHFSMState) == AssetRefHFSMState::SIZE);

struct AssetRefHFSMTransitionSet {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90820 (Equals), 0x7b908a0 (Equals)
    friend constexpr bool operator==(AssetRefHFSMTransitionSet a, AssetRefHFSMTransitionSet b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHFSMTransitionSet other) const { return Id == other.Id; }
    // 0x7b908a8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(643u, Id); }
    // 0x7b90814
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHFSMTransitionSet*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefHFSMTransitionSet) == AssetRefHFSMTransitionSet::SIZE);

struct AssetRefHeadHunterWallHackEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b908e0 (Equals), 0x7b90960 (Equals)
    friend constexpr bool operator==(AssetRefHeadHunterWallHackEnhancementSpec a, AssetRefHeadHunterWallHackEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHeadHunterWallHackEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b90968
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(647u, Id); }
    // 0x7b908c4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b90984
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHeadHunterWallHackEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b908d0, Quantum.AssetRefHeadHunterWallHackEnhancementSpec.op_Implicit): HeadHunterWallHackEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefHeadHunterWallHackEnhancementSpec) == AssetRefHeadHunterWallHackEnhancementSpec::SIZE);

struct AssetRefHealSourceSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b909a4 (Equals), 0x7b90a24 (Equals)
    friend constexpr bool operator==(AssetRefHealSourceSpec a, AssetRefHealSourceSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHealSourceSpec other) const { return Id == other.Id; }
    // 0x7b90a2c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(653u, Id); }
    // 0x7b90998
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b90a48
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHealSourceSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefHealSourceSpec) == AssetRefHealSourceSpec::SIZE);

struct AssetRefHealthSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90b00 (==), 0x7b90a78 (Equals), 0x7b90af8 (Equals)
    friend constexpr bool operator==(AssetRefHealthSpec a, AssetRefHealthSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHealthSpec other) const { return Id == other.Id; }
    // 0x7b90b08
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(659u, Id); }
    // 0x7b90a5c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b90b24
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHealthSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b90a68, Quantum.AssetRefHealthSpec.op_Implicit): HealthSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefHealthSpec) == AssetRefHealthSpec::SIZE);

struct AssetRefHelipackActorSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90b44 (Equals), 0x7b90bc4 (Equals)
    friend constexpr bool operator==(AssetRefHelipackActorSpec a, AssetRefHelipackActorSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHelipackActorSpec other) const { return Id == other.Id; }
    // 0x7b90bcc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(661u, Id); }
    // 0x7b90b38
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b90be8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHelipackActorSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefHelipackActorSpec) == AssetRefHelipackActorSpec::SIZE);

struct AssetRefHeroSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90c08 (Equals), 0x7b90c88 (Equals)
    friend constexpr bool operator==(AssetRefHeroSpec a, AssetRefHeroSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHeroSpec other) const { return Id == other.Id; }
    // 0x7b90c90
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(673u, Id); }
    // 0x7b90bfc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHeroSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefHeroSpec) == AssetRefHeroSpec::SIZE);

struct AssetRefHoloshieldGloveHealZoneEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90cc8 (Equals), 0x7b90d48 (Equals)
    friend constexpr bool operator==(AssetRefHoloshieldGloveHealZoneEnhancementSpec a, AssetRefHoloshieldGloveHealZoneEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHoloshieldGloveHealZoneEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b90d50
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(677u, Id); }
    // 0x7b90cac
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b90d6c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHoloshieldGloveHealZoneEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b90cb8, Quantum.AssetRefHoloshieldGloveHealZoneEnhancementSpec.op_Implicit): HoloshieldGloveHealZoneEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefHoloshieldGloveHealZoneEnhancementSpec) == AssetRefHoloshieldGloveHealZoneEnhancementSpec::SIZE);

struct AssetRefHoverBootsEnhancementCollisionSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90d8c (Equals), 0x7b90e0c (Equals)
    friend constexpr bool operator==(AssetRefHoverBootsEnhancementCollisionSpec a, AssetRefHoverBootsEnhancementCollisionSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefHoverBootsEnhancementCollisionSpec other) const { return Id == other.Id; }
    // 0x7b90e14
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(683u, Id); }
    // 0x7b90d80
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefHoverBootsEnhancementCollisionSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefHoverBootsEnhancementCollisionSpec) == AssetRefHoverBootsEnhancementCollisionSpec::SIZE);

struct AssetRefImprovementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90e3c (Equals), 0x7b90ebc (Equals)
    friend constexpr bool operator==(AssetRefImprovementSpec a, AssetRefImprovementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefImprovementSpec other) const { return Id == other.Id; }
    // 0x7b90ec4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(691u, Id); }
    // 0x7b90e30
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefImprovementSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefImprovementSpec) == AssetRefImprovementSpec::SIZE);

struct AssetRefItemSpawnerConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90eec (Equals), 0x7b90f6c (Equals)
    friend constexpr bool operator==(AssetRefItemSpawnerConfig a, AssetRefItemSpawnerConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefItemSpawnerConfig other) const { return Id == other.Id; }
    // 0x7b90f74
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(701u, Id); }
    // 0x7b90ee0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b90f90
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefItemSpawnerConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefItemSpawnerConfig) == AssetRefItemSpawnerConfig::SIZE);

struct AssetRefJumpAttackSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b90fb0 (Equals), 0x7b91030 (Equals)
    friend constexpr bool operator==(AssetRefJumpAttackSpec a, AssetRefJumpAttackSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefJumpAttackSpec other) const { return Id == other.Id; }
    // 0x7b91038
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(709u, Id); }
    // 0x7b90fa4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91054
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefJumpAttackSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefJumpAttackSpec) == AssetRefJumpAttackSpec::SIZE);

struct AssetRefKccExtensionConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91074 (Equals), 0x7b910f4 (Equals)
    friend constexpr bool operator==(AssetRefKccExtensionConfig a, AssetRefKccExtensionConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefKccExtensionConfig other) const { return Id == other.Id; }
    // 0x7b910fc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(719u, Id); }
    // 0x7b91068
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91118
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefKccExtensionConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefKccExtensionConfig) == AssetRefKccExtensionConfig::SIZE);

struct AssetRefKccModifierSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91138 (Equals), 0x7b911b8 (Equals)
    friend constexpr bool operator==(AssetRefKccModifierSpec a, AssetRefKccModifierSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefKccModifierSpec other) const { return Id == other.Id; }
    // 0x7b911c0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(727u, Id); }
    // 0x7b9112c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b911dc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefKccModifierSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefKccModifierSpec) == AssetRefKccModifierSpec::SIZE);

struct AssetRefLedgeGrabbingSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b911fc (Equals), 0x7b9127c (Equals)
    friend constexpr bool operator==(AssetRefLedgeGrabbingSpec a, AssetRefLedgeGrabbingSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefLedgeGrabbingSpec other) const { return Id == other.Id; }
    // 0x7b91284
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(733u, Id); }
    // 0x7b911f0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b912a0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefLedgeGrabbingSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefLedgeGrabbingSpec) == AssetRefLedgeGrabbingSpec::SIZE);

struct AssetRefLevelButtonConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b912c0 (Equals), 0x7b91340 (Equals)
    friend constexpr bool operator==(AssetRefLevelButtonConfig a, AssetRefLevelButtonConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefLevelButtonConfig other) const { return Id == other.Id; }
    // 0x7b91348
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(739u, Id); }
    // 0x7b912b4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91364
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefLevelButtonConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefLevelButtonConfig) == AssetRefLevelButtonConfig::SIZE);

struct AssetRefLevelButtonGameplayElementConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91384 (Equals), 0x7b91404 (Equals)
    friend constexpr bool operator==(AssetRefLevelButtonGameplayElementConfig a, AssetRefLevelButtonGameplayElementConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefLevelButtonGameplayElementConfig other) const { return Id == other.Id; }
    // 0x7b9140c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(743u, Id); }
    // 0x7b91378
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91428
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefLevelButtonGameplayElementConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefLevelButtonGameplayElementConfig) == AssetRefLevelButtonGameplayElementConfig::SIZE);

struct AssetRefLevelUpActorSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91448 (Equals), 0x7b914c8 (Equals)
    friend constexpr bool operator==(AssetRefLevelUpActorSpec a, AssetRefLevelUpActorSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefLevelUpActorSpec other) const { return Id == other.Id; }
    // 0x7b914d0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(751u, Id); }
    // 0x7b9143c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b914ec
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefLevelUpActorSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefLevelUpActorSpec) == AssetRefLevelUpActorSpec::SIZE);

struct AssetRefLifetimeSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9150c (Equals), 0x7b9158c (Equals)
    friend constexpr bool operator==(AssetRefLifetimeSpec a, AssetRefLifetimeSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefLifetimeSpec other) const { return Id == other.Id; }
    // 0x7b91594
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(757u, Id); }
    // 0x7b91500
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b915b0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefLifetimeSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefLifetimeSpec) == AssetRefLifetimeSpec::SIZE);

struct AssetRefMapCustomData {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b915d0 (Equals), 0x7b91650 (Equals)
    friend constexpr bool operator==(AssetRefMapCustomData a, AssetRefMapCustomData b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMapCustomData other) const { return Id == other.Id; }
    // 0x7b91658
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(761u, Id); }
    // 0x7b915c4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMapCustomData*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefMapCustomData) == AssetRefMapCustomData::SIZE);

struct AssetRefMeleeAttackSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91680 (Equals), 0x7b91700 (Equals)
    friend constexpr bool operator==(AssetRefMeleeAttackSpec a, AssetRefMeleeAttackSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMeleeAttackSpec other) const { return Id == other.Id; }
    // 0x7b91708
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(769u, Id); }
    // 0x7b91674
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91724
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMeleeAttackSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefMeleeAttackSpec) == AssetRefMeleeAttackSpec::SIZE);

struct AssetRefMeleeJumpAttackSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91754 (Equals), 0x7b917d4 (Equals)
    friend constexpr bool operator==(AssetRefMeleeJumpAttackSpec a, AssetRefMeleeJumpAttackSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMeleeJumpAttackSpec other) const { return Id == other.Id; }
    // 0x7b917dc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(773u, Id); }
    // 0x7b91738
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b917f8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMeleeJumpAttackSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b91744, Quantum.AssetRefMeleeJumpAttackSpec.op_Implicit): MeleeJumpAttackSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefMeleeJumpAttackSpec) == AssetRefMeleeJumpAttackSpec::SIZE);

struct AssetRefMineLauncherEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91818 (Equals), 0x7b91898 (Equals)
    friend constexpr bool operator==(AssetRefMineLauncherEnhancementSpec a, AssetRefMineLauncherEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMineLauncherEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b918a0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(787u, Id); }
    // 0x7b9180c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMineLauncherEnhancementSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefMineLauncherEnhancementSpec) == AssetRefMineLauncherEnhancementSpec::SIZE);

struct AssetRefMineLauncherProjectileOverrideEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b918c8 (Equals), 0x7b91948 (Equals)
    friend constexpr bool operator==(AssetRefMineLauncherProjectileOverrideEnhancementSpec a, AssetRefMineLauncherProjectileOverrideEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMineLauncherProjectileOverrideEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b91950
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(797u, Id); }
    // 0x7b918bc
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMineLauncherProjectileOverrideEnhancementSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefMineLauncherProjectileOverrideEnhancementSpec) == AssetRefMineLauncherProjectileOverrideEnhancementSpec::SIZE);

struct AssetRefMobGroupConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91978 (Equals), 0x7b919f8 (Equals)
    friend constexpr bool operator==(AssetRefMobGroupConfig a, AssetRefMobGroupConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMobGroupConfig other) const { return Id == other.Id; }
    // 0x7b91a00
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(809u, Id); }
    // 0x7b9196c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91a1c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMobGroupConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefMobGroupConfig) == AssetRefMobGroupConfig::SIZE);

struct AssetRefMobInfoSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91a3c (Equals), 0x7b91abc (Equals)
    friend constexpr bool operator==(AssetRefMobInfoSpec a, AssetRefMobInfoSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMobInfoSpec other) const { return Id == other.Id; }
    // 0x7b91ac4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(811u, Id); }
    // 0x7b91a30
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91ae0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMobInfoSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefMobInfoSpec) == AssetRefMobInfoSpec::SIZE);

struct AssetRefMoveToSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91b00 (Equals), 0x7b91b80 (Equals)
    friend constexpr bool operator==(AssetRefMoveToSpec a, AssetRefMoveToSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMoveToSpec other) const { return Id == other.Id; }
    // 0x7b91b88
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(821u, Id); }
    // 0x7b91af4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91ba4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMoveToSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefMoveToSpec) == AssetRefMoveToSpec::SIZE);

struct AssetRefMovingWallConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91bc4 (Equals), 0x7b91c44 (Equals)
    friend constexpr bool operator==(AssetRefMovingWallConfig a, AssetRefMovingWallConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefMovingWallConfig other) const { return Id == other.Id; }
    // 0x7b91c4c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(823u, Id); }
    // 0x7b91bb8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91c68
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefMovingWallConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefMovingWallConfig) == AssetRefMovingWallConfig::SIZE);

struct AssetRefOverhealthPowerUpSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91c88 (Equals), 0x7b91d08 (Equals)
    friend constexpr bool operator==(AssetRefOverhealthPowerUpSpec a, AssetRefOverhealthPowerUpSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefOverhealthPowerUpSpec other) const { return Id == other.Id; }
    // 0x7b91d10
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(827u, Id); }
    // 0x7b91c7c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefOverhealthPowerUpSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefOverhealthPowerUpSpec) == AssetRefOverhealthPowerUpSpec::SIZE);

struct AssetRefParabolaFollowerSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91d38 (Equals), 0x7b91db8 (Equals)
    friend constexpr bool operator==(AssetRefParabolaFollowerSpec a, AssetRefParabolaFollowerSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefParabolaFollowerSpec other) const { return Id == other.Id; }
    // 0x7b91dc0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(829u, Id); }
    // 0x7b91d2c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91ddc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefParabolaFollowerSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefParabolaFollowerSpec) == AssetRefParabolaFollowerSpec::SIZE);

struct AssetRefPowerUpSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91e0c (Equals), 0x7b91e8c (Equals)
    friend constexpr bool operator==(AssetRefPowerUpSpec a, AssetRefPowerUpSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefPowerUpSpec other) const { return Id == other.Id; }
    // 0x7b91e94
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(839u, Id); }
    // 0x7b91df0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91eb0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefPowerUpSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b91dfc, Quantum.AssetRefPowerUpSpec.op_Implicit): PowerUpSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefPowerUpSpec) == AssetRefPowerUpSpec::SIZE);

struct AssetRefPreciseHitReceiverSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91ed0 (Equals), 0x7b91f50 (Equals)
    friend constexpr bool operator==(AssetRefPreciseHitReceiverSpec a, AssetRefPreciseHitReceiverSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefPreciseHitReceiverSpec other) const { return Id == other.Id; }
    // 0x7b91f58
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(853u, Id); }
    // 0x7b91ec4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b91f74
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefPreciseHitReceiverSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefPreciseHitReceiverSpec) == AssetRefPreciseHitReceiverSpec::SIZE);

struct AssetRefPrioritisedTargetHandlerSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b91f94 (Equals), 0x7b92014 (Equals)
    friend constexpr bool operator==(AssetRefPrioritisedTargetHandlerSpec a, AssetRefPrioritisedTargetHandlerSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefPrioritisedTargetHandlerSpec other) const { return Id == other.Id; }
    // 0x7b9201c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(857u, Id); }
    // 0x7b91f88
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b92038
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefPrioritisedTargetHandlerSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefPrioritisedTargetHandlerSpec) == AssetRefPrioritisedTargetHandlerSpec::SIZE);

struct AssetRefProjectileSpawnerSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92058 (Equals), 0x7b920d8 (Equals)
    friend constexpr bool operator==(AssetRefProjectileSpawnerSpec a, AssetRefProjectileSpawnerSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefProjectileSpawnerSpec other) const { return Id == other.Id; }
    // 0x7b920e0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(859u, Id); }
    // 0x7b9204c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b920fc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefProjectileSpawnerSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefProjectileSpawnerSpec) == AssetRefProjectileSpawnerSpec::SIZE);

struct AssetRefProjectileSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9211c (Equals), 0x7b9219c (Equals)
    friend constexpr bool operator==(AssetRefProjectileSpec a, AssetRefProjectileSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefProjectileSpec other) const { return Id == other.Id; }
    // 0x7b921a4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(863u, Id); }
    // 0x7b92110
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b921c0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefProjectileSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefProjectileSpec) == AssetRefProjectileSpec::SIZE);

struct AssetRefPushableSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b921e0 (Equals), 0x7b92260 (Equals)
    friend constexpr bool operator==(AssetRefPushableSpec a, AssetRefPushableSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefPushableSpec other) const { return Id == other.Id; }
    // 0x7b92268
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(877u, Id); }
    // 0x7b921d4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b92284
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefPushableSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefPushableSpec) == AssetRefPushableSpec::SIZE);

struct AssetRefPusherSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b922a4 (Equals), 0x7b92324 (Equals)
    friend constexpr bool operator==(AssetRefPusherSpec a, AssetRefPusherSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefPusherSpec other) const { return Id == other.Id; }
    // 0x7b9232c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(881u, Id); }
    // 0x7b92298
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b92348
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefPusherSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefPusherSpec) == AssetRefPusherSpec::SIZE);

struct AssetRefRecoverAmmoOnDashEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92378 (Equals), 0x7b923f8 (Equals)
    friend constexpr bool operator==(AssetRefRecoverAmmoOnDashEnhancementSpec a, AssetRefRecoverAmmoOnDashEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefRecoverAmmoOnDashEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b92400
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(883u, Id); }
    // 0x7b9235c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9241c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefRecoverAmmoOnDashEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b92368, Quantum.AssetRefRecoverAmmoOnDashEnhancementSpec.op_Implicit): RecoverAmmoOnDashEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefRecoverAmmoOnDashEnhancementSpec) == AssetRefRecoverAmmoOnDashEnhancementSpec::SIZE);

struct AssetRefReplaceMultiShotConfigsEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9244c (Equals), 0x7b924cc (Equals)
    friend constexpr bool operator==(AssetRefReplaceMultiShotConfigsEnhancementSpec a, AssetRefReplaceMultiShotConfigsEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefReplaceMultiShotConfigsEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b924d4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(887u, Id); }
    // 0x7b92430
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b924f0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefReplaceMultiShotConfigsEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b9243c, Quantum.AssetRefReplaceMultiShotConfigsEnhancementSpec.op_Implicit): ReplaceMultiShotConfigsEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefReplaceMultiShotConfigsEnhancementSpec) == AssetRefReplaceMultiShotConfigsEnhancementSpec::SIZE);

struct AssetRefRewardReceiverSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92510 (Equals), 0x7b92590 (Equals)
    friend constexpr bool operator==(AssetRefRewardReceiverSpec a, AssetRefRewardReceiverSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefRewardReceiverSpec other) const { return Id == other.Id; }
    // 0x7b92598
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(907u, Id); }
    // 0x7b92504
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b925b4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefRewardReceiverSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefRewardReceiverSpec) == AssetRefRewardReceiverSpec::SIZE);

struct AssetRefRewardSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b925d4 (Equals), 0x7b92654 (Equals)
    friend constexpr bool operator==(AssetRefRewardSpec a, AssetRefRewardSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefRewardSpec other) const { return Id == other.Id; }
    // 0x7b9265c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(911u, Id); }
    // 0x7b925c8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefRewardSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefRewardSpec) == AssetRefRewardSpec::SIZE);

struct AssetRefRollSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92684 (Equals), 0x7b92704 (Equals)
    friend constexpr bool operator==(AssetRefRollSpec a, AssetRefRollSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefRollSpec other) const { return Id == other.Id; }
    // 0x7b9270c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(919u, Id); }
    // 0x7b92678
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b92728
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefRollSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefRollSpec) == AssetRefRollSpec::SIZE);

struct AssetRefSeekNewTargetPriorityList {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92748 (Equals), 0x7b927c8 (Equals)
    friend constexpr bool operator==(AssetRefSeekNewTargetPriorityList a, AssetRefSeekNewTargetPriorityList b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSeekNewTargetPriorityList other) const { return Id == other.Id; }
    // 0x7b927d0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(929u, Id); }
    // 0x7b9273c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSeekNewTargetPriorityList*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSeekNewTargetPriorityList) == AssetRefSeekNewTargetPriorityList::SIZE);

struct AssetRefShapeOverlapperSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b927f8 (Equals), 0x7b92878 (Equals)
    friend constexpr bool operator==(AssetRefShapeOverlapperSpec a, AssetRefShapeOverlapperSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefShapeOverlapperSpec other) const { return Id == other.Id; }
    // 0x7b92880
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(937u, Id); }
    // 0x7b927ec
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9289c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefShapeOverlapperSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefShapeOverlapperSpec) == AssetRefShapeOverlapperSpec::SIZE);

struct AssetRefShieldedActorSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b928bc (Equals), 0x7b9293c (Equals)
    friend constexpr bool operator==(AssetRefShieldedActorSpec a, AssetRefShieldedActorSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefShieldedActorSpec other) const { return Id == other.Id; }
    // 0x7b92944
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(941u, Id); }
    // 0x7b928b0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b92960
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefShieldedActorSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefShieldedActorSpec) == AssetRefShieldedActorSpec::SIZE);

struct AssetRefShotgunBerserkerEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92990 (Equals), 0x7b92a10 (Equals)
    friend constexpr bool operator==(AssetRefShotgunBerserkerEnhancementSpec a, AssetRefShotgunBerserkerEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefShotgunBerserkerEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b92a18
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(947u, Id); }
    // 0x7b92974
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b92a34
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefShotgunBerserkerEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b92980, Quantum.AssetRefShotgunBerserkerEnhancementSpec.op_Implicit): ShotgunBerserkerEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefShotgunBerserkerEnhancementSpec) == AssetRefShotgunBerserkerEnhancementSpec::SIZE);

struct AssetRefShotgunEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92a64 (Equals), 0x7b92ae4 (Equals)
    friend constexpr bool operator==(AssetRefShotgunEnhancementSpec a, AssetRefShotgunEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefShotgunEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b92aec
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(953u, Id); }
    // 0x7b92a48
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b92b08
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefShotgunEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b92a54, Quantum.AssetRefShotgunEnhancementSpec.op_Implicit): ShotgunEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefShotgunEnhancementSpec) == AssetRefShotgunEnhancementSpec::SIZE);

struct AssetRefShrinkingZoneSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92b28 (Equals), 0x7b92ba8 (Equals)
    friend constexpr bool operator==(AssetRefShrinkingZoneSpec a, AssetRefShrinkingZoneSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefShrinkingZoneSpec other) const { return Id == other.Id; }
    // 0x7b92bb0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(967u, Id); }
    // 0x7b92b1c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b92bcc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefShrinkingZoneSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefShrinkingZoneSpec) == AssetRefShrinkingZoneSpec::SIZE);

struct AssetRefSimulationConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7ca3d0c (Equals), 0x7ca3d14 (Equals)
    friend constexpr bool operator==(AssetRefSimulationConfig a, AssetRefSimulationConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSimulationConfig other) const { return Id == other.Id; }
    // 0x7ca3d94
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(0u, Id); }
    // 0x7ca3d00
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSimulationConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSimulationConfig) == AssetRefSimulationConfig::SIZE);

struct AssetRefSimulationGadgetTable {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92bec (Equals), 0x7b92c6c (Equals)
    friend constexpr bool operator==(AssetRefSimulationGadgetTable a, AssetRefSimulationGadgetTable b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSimulationGadgetTable other) const { return Id == other.Id; }
    // 0x7b92c74
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(971u, Id); }
    // 0x7b92be0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSimulationGadgetTable*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSimulationGadgetTable) == AssetRefSimulationGadgetTable::SIZE);

struct AssetRefSimulationHeroesTable {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92c9c (Equals), 0x7b92d1c (Equals)
    friend constexpr bool operator==(AssetRefSimulationHeroesTable a, AssetRefSimulationHeroesTable b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSimulationHeroesTable other) const { return Id == other.Id; }
    // 0x7b92d24
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(977u, Id); }
    // 0x7b92c90
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSimulationHeroesTable*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSimulationHeroesTable) == AssetRefSimulationHeroesTable::SIZE);

struct AssetRefSimulationMeleeWeaponsTable {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92d4c (Equals), 0x7b92dcc (Equals)
    friend constexpr bool operator==(AssetRefSimulationMeleeWeaponsTable a, AssetRefSimulationMeleeWeaponsTable b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSimulationMeleeWeaponsTable other) const { return Id == other.Id; }
    // 0x7b92dd4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(983u, Id); }
    // 0x7b92d40
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSimulationMeleeWeaponsTable*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSimulationMeleeWeaponsTable) == AssetRefSimulationMeleeWeaponsTable::SIZE);

struct AssetRefSimulationUltimateTable {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92dfc (Equals), 0x7b92e7c (Equals)
    friend constexpr bool operator==(AssetRefSimulationUltimateTable a, AssetRefSimulationUltimateTable b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSimulationUltimateTable other) const { return Id == other.Id; }
    // 0x7b92e84
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(991u, Id); }
    // 0x7b92df0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSimulationUltimateTable*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSimulationUltimateTable) == AssetRefSimulationUltimateTable::SIZE);

struct AssetRefSimulationWeaponTable {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92eac (Equals), 0x7b92f2c (Equals)
    friend constexpr bool operator==(AssetRefSimulationWeaponTable a, AssetRefSimulationWeaponTable b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSimulationWeaponTable other) const { return Id == other.Id; }
    // 0x7b92f34
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(997u, Id); }
    // 0x7b92ea0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSimulationWeaponTable*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSimulationWeaponTable) == AssetRefSimulationWeaponTable::SIZE);

struct AssetRefSpeedBoostPowerUpSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b92f5c (Equals), 0x7b92fdc (Equals)
    friend constexpr bool operator==(AssetRefSpeedBoostPowerUpSpec a, AssetRefSpeedBoostPowerUpSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSpeedBoostPowerUpSpec other) const { return Id == other.Id; }
    // 0x7b92fe4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1009u, Id); }
    // 0x7b92f50
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSpeedBoostPowerUpSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSpeedBoostPowerUpSpec) == AssetRefSpeedBoostPowerUpSpec::SIZE);

struct AssetRefSpeederSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9300c (Equals), 0x7b9308c (Equals)
    friend constexpr bool operator==(AssetRefSpeederSpec a, AssetRefSpeederSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSpeederSpec other) const { return Id == other.Id; }
    // 0x7b93094
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1013u, Id); }
    // 0x7b93000
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b930b0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSpeederSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSpeederSpec) == AssetRefSpeederSpec::SIZE);

struct AssetRefSquadSiegeConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b930d0 (Equals), 0x7b93150 (Equals)
    friend constexpr bool operator==(AssetRefSquadSiegeConfig a, AssetRefSquadSiegeConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSquadSiegeConfig other) const { return Id == other.Id; }
    // 0x7b93158
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1019u, Id); }
    // 0x7b930c4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93174
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSquadSiegeConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSquadSiegeConfig) == AssetRefSquadSiegeConfig::SIZE);

struct AssetRefSquadSiegeLevelConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93194 (Equals), 0x7b93214 (Equals)
    friend constexpr bool operator==(AssetRefSquadSiegeLevelConfig a, AssetRefSquadSiegeLevelConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSquadSiegeLevelConfig other) const { return Id == other.Id; }
    // 0x7b9321c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1021u, Id); }
    // 0x7b93188
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSquadSiegeLevelConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSquadSiegeLevelConfig) == AssetRefSquadSiegeLevelConfig::SIZE);

struct AssetRefStaticColliderConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93244 (Equals), 0x7b932c4 (Equals)
    friend constexpr bool operator==(AssetRefStaticColliderConfig a, AssetRefStaticColliderConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefStaticColliderConfig other) const { return Id == other.Id; }
    // 0x7b932cc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1031u, Id); }
    // 0x7b93238
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefStaticColliderConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefStaticColliderConfig) == AssetRefStaticColliderConfig::SIZE);

struct AssetRefStaticTriggerConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b932f4 (Equals), 0x7b93374 (Equals)
    friend constexpr bool operator==(AssetRefStaticTriggerConfig a, AssetRefStaticTriggerConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefStaticTriggerConfig other) const { return Id == other.Id; }
    // 0x7b9337c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1033u, Id); }
    // 0x7b932e8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefStaticTriggerConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefStaticTriggerConfig) == AssetRefStaticTriggerConfig::SIZE);

struct AssetRefStreamEffectSpawnerSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b933a4 (Equals), 0x7b93424 (Equals)
    friend constexpr bool operator==(AssetRefStreamEffectSpawnerSpec a, AssetRefStreamEffectSpawnerSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefStreamEffectSpawnerSpec other) const { return Id == other.Id; }
    // 0x7b9342c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1039u, Id); }
    // 0x7b93398
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93448
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefStreamEffectSpawnerSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefStreamEffectSpawnerSpec) == AssetRefStreamEffectSpawnerSpec::SIZE);

struct AssetRefStreamEffectSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93468 (Equals), 0x7b934e8 (Equals)
    friend constexpr bool operator==(AssetRefStreamEffectSpec a, AssetRefStreamEffectSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefStreamEffectSpec other) const { return Id == other.Id; }
    // 0x7b934f0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1049u, Id); }
    // 0x7b9345c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9350c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefStreamEffectSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefStreamEffectSpec) == AssetRefStreamEffectSpec::SIZE);

struct AssetRefSweepAttackSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9352c (Equals), 0x7b935ac (Equals)
    friend constexpr bool operator==(AssetRefSweepAttackSpec a, AssetRefSweepAttackSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefSweepAttackSpec other) const { return Id == other.Id; }
    // 0x7b935b4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1051u, Id); }
    // 0x7b93520
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b935d0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefSweepAttackSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefSweepAttackSpec) == AssetRefSweepAttackSpec::SIZE);

struct AssetRefTargetFinderSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b935f0 (Equals), 0x7b93670 (Equals)
    friend constexpr bool operator==(AssetRefTargetFinderSpec a, AssetRefTargetFinderSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefTargetFinderSpec other) const { return Id == other.Id; }
    // 0x7b93678
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1061u, Id); }
    // 0x7b935e4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93694
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefTargetFinderSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefTargetFinderSpec) == AssetRefTargetFinderSpec::SIZE);

struct AssetRefTargetedEffectSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b936b4 (Equals), 0x7b93734 (Equals)
    friend constexpr bool operator==(AssetRefTargetedEffectSpec a, AssetRefTargetedEffectSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefTargetedEffectSpec other) const { return Id == other.Id; }
    // 0x7b9373c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1063u, Id); }
    // 0x7b936a8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93758
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefTargetedEffectSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefTargetedEffectSpec) == AssetRefTargetedEffectSpec::SIZE);

struct AssetRefTeslaClawMoreBeamEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93788 (Equals), 0x7b93808 (Equals)
    friend constexpr bool operator==(AssetRefTeslaClawMoreBeamEnhancementSpec a, AssetRefTeslaClawMoreBeamEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefTeslaClawMoreBeamEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b93810
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1069u, Id); }
    // 0x7b9376c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9382c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefTeslaClawMoreBeamEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b93778, Quantum.AssetRefTeslaClawMoreBeamEnhancementSpec.op_Implicit): TeslaClawMoreBeamEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefTeslaClawMoreBeamEnhancementSpec) == AssetRefTeslaClawMoreBeamEnhancementSpec::SIZE);

struct AssetRefTrailSpawnerSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9384c (Equals), 0x7b938cc (Equals)
    friend constexpr bool operator==(AssetRefTrailSpawnerSpec a, AssetRefTrailSpawnerSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefTrailSpawnerSpec other) const { return Id == other.Id; }
    // 0x7b938d4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1087u, Id); }
    // 0x7b93840
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b938f0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefTrailSpawnerSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefTrailSpawnerSpec) == AssetRefTrailSpawnerSpec::SIZE);

struct AssetRefTravolatorStaticTriggerConfig {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93910 (Equals), 0x7b93990 (Equals)
    friend constexpr bool operator==(AssetRefTravolatorStaticTriggerConfig a, AssetRefTravolatorStaticTriggerConfig b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefTravolatorStaticTriggerConfig other) const { return Id == other.Id; }
    // 0x7b93998
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1091u, Id); }
    // 0x7b93904
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b939b4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefTravolatorStaticTriggerConfig*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefTravolatorStaticTriggerConfig) == AssetRefTravolatorStaticTriggerConfig::SIZE);

struct AssetRefTutorialGameModeSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b939d4 (Equals), 0x7b93a54 (Equals)
    friend constexpr bool operator==(AssetRefTutorialGameModeSpec a, AssetRefTutorialGameModeSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefTutorialGameModeSpec other) const { return Id == other.Id; }
    // 0x7b93a5c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1093u, Id); }
    // 0x7b939c8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93a78
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefTutorialGameModeSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefTutorialGameModeSpec) == AssetRefTutorialGameModeSpec::SIZE);

struct AssetRefUTRoot {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93b20 (==), 0x7b93a98 (Equals), 0x7b93b18 (Equals)
    friend constexpr bool operator==(AssetRefUTRoot a, AssetRefUTRoot b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefUTRoot other) const { return Id == other.Id; }
    // 0x7b93b28
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1097u, Id); }
    // 0x7b93a8c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93b44
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefUTRoot*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefUTRoot) == AssetRefUTRoot::SIZE);

struct AssetRefUltimateAbilitySpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93b64 (Equals), 0x7b93be4 (Equals)
    friend constexpr bool operator==(AssetRefUltimateAbilitySpec a, AssetRefUltimateAbilitySpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefUltimateAbilitySpec other) const { return Id == other.Id; }
    // 0x7b93bec
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1103u, Id); }
    // 0x7b93b58
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93c08
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefUltimateAbilitySpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefUltimateAbilitySpec) == AssetRefUltimateAbilitySpec::SIZE);

struct AssetRefUltimateAbilityUserSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93c28 (Equals), 0x7b93ca8 (Equals)
    friend constexpr bool operator==(AssetRefUltimateAbilityUserSpec a, AssetRefUltimateAbilityUserSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefUltimateAbilityUserSpec other) const { return Id == other.Id; }
    // 0x7b93cb0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1109u, Id); }
    // 0x7b93c1c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93ccc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefUltimateAbilityUserSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefUltimateAbilityUserSpec) == AssetRefUltimateAbilityUserSpec::SIZE);

struct AssetRefVoidRepulserBulldozerEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93cfc (Equals), 0x7b93d7c (Equals)
    friend constexpr bool operator==(AssetRefVoidRepulserBulldozerEnhancementSpec a, AssetRefVoidRepulserBulldozerEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefVoidRepulserBulldozerEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b93d84
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1117u, Id); }
    // 0x7b93ce0
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93da0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefVoidRepulserBulldozerEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b93cec, Quantum.AssetRefVoidRepulserBulldozerEnhancementSpec.op_Implicit): VoidRepulserBulldozerEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefVoidRepulserBulldozerEnhancementSpec) == AssetRefVoidRepulserBulldozerEnhancementSpec::SIZE);

struct AssetRefVoidRepulserSpeedBoostEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93dd0 (Equals), 0x7b93e50 (Equals)
    friend constexpr bool operator==(AssetRefVoidRepulserSpeedBoostEnhancementSpec a, AssetRefVoidRepulserSpeedBoostEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefVoidRepulserSpeedBoostEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b93e58
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1123u, Id); }
    // 0x7b93db4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93e74
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefVoidRepulserSpeedBoostEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b93dc0, Quantum.AssetRefVoidRepulserSpeedBoostEnhancementSpec.op_Implicit): VoidRepulserSpeedBoostEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefVoidRepulserSpeedBoostEnhancementSpec) == AssetRefVoidRepulserSpeedBoostEnhancementSpec::SIZE);

struct AssetRefVoidRepulserVoidWaveEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93ea4 (Equals), 0x7b93f24 (Equals)
    friend constexpr bool operator==(AssetRefVoidRepulserVoidWaveEnhancementSpec a, AssetRefVoidRepulserVoidWaveEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefVoidRepulserVoidWaveEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b93f2c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1129u, Id); }
    // 0x7b93e88
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b93f48
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefVoidRepulserVoidWaveEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b93e94, Quantum.AssetRefVoidRepulserVoidWaveEnhancementSpec.op_Implicit): VoidRepulserVoidWaveEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefVoidRepulserVoidWaveEnhancementSpec) == AssetRefVoidRepulserVoidWaveEnhancementSpec::SIZE);

struct AssetRefWallRunningSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b93f68 (Equals), 0x7b93fe8 (Equals)
    friend constexpr bool operator==(AssetRefWallRunningSpec a, AssetRefWallRunningSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWallRunningSpec other) const { return Id == other.Id; }
    // 0x7b93ff0
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1151u, Id); }
    // 0x7b93f5c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9400c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWallRunningSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefWallRunningSpec) == AssetRefWallRunningSpec::SIZE);

struct AssetRefWarmongerMiniRocketsEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9403c (Equals), 0x7b940bc (Equals)
    friend constexpr bool operator==(AssetRefWarmongerMiniRocketsEnhancementSpec a, AssetRefWarmongerMiniRocketsEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWarmongerMiniRocketsEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b940c4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1153u, Id); }
    // 0x7b94020
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b940e0
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWarmongerMiniRocketsEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b9402c, Quantum.AssetRefWarmongerMiniRocketsEnhancementSpec.op_Implicit): WarmongerMiniRocketsEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefWarmongerMiniRocketsEnhancementSpec) == AssetRefWarmongerMiniRocketsEnhancementSpec::SIZE);

struct AssetRefWaveManagerSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b94100 (Equals), 0x7b94180 (Equals)
    friend constexpr bool operator==(AssetRefWaveManagerSpec a, AssetRefWaveManagerSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWaveManagerSpec other) const { return Id == other.Id; }
    // 0x7b94188
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1163u, Id); }
    // 0x7b940f4
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b941a4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWaveManagerSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefWaveManagerSpec) == AssetRefWaveManagerSpec::SIZE);

struct AssetRefWaveReferences {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b941c4 (Equals), 0x7b94244 (Equals)
    friend constexpr bool operator==(AssetRefWaveReferences a, AssetRefWaveReferences b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWaveReferences other) const { return Id == other.Id; }
    // 0x7b9424c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1171u, Id); }
    // 0x7b941b8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b94268
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWaveReferences*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefWaveReferences) == AssetRefWaveReferences::SIZE);

struct AssetRefWeaponBehaviour_ChargingSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b94288 (Equals), 0x7b94308 (Equals)
    friend constexpr bool operator==(AssetRefWeaponBehaviour_ChargingSpec a, AssetRefWeaponBehaviour_ChargingSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWeaponBehaviour_ChargingSpec other) const { return Id == other.Id; }
    // 0x7b94310
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1181u, Id); }
    // 0x7b9427c
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9432c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWeaponBehaviour_ChargingSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefWeaponBehaviour_ChargingSpec) == AssetRefWeaponBehaviour_ChargingSpec::SIZE);

struct AssetRefWeaponEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b9435c (Equals), 0x7b943dc (Equals)
    friend constexpr bool operator==(AssetRefWeaponEnhancementSpec a, AssetRefWeaponEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWeaponEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b943e4
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1187u, Id); }
    // 0x7b94340
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b94400
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWeaponEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b9434c, Quantum.AssetRefWeaponEnhancementSpec.op_Implicit): WeaponEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefWeaponEnhancementSpec) == AssetRefWeaponEnhancementSpec::SIZE);

struct AssetRefWeaponLoadoutSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b94420 (Equals), 0x7b944a0 (Equals)
    friend constexpr bool operator==(AssetRefWeaponLoadoutSpec a, AssetRefWeaponLoadoutSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWeaponLoadoutSpec other) const { return Id == other.Id; }
    // 0x7b944a8
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1193u, Id); }
    // 0x7b94414
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b944c4
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWeaponLoadoutSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefWeaponLoadoutSpec) == AssetRefWeaponLoadoutSpec::SIZE);

struct AssetRefWeaponProjectileSwitcherEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b944f4 (Equals), 0x7b94574 (Equals)
    friend constexpr bool operator==(AssetRefWeaponProjectileSwitcherEnhancementSpec a, AssetRefWeaponProjectileSwitcherEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWeaponProjectileSwitcherEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b9457c
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1201u, Id); }
    // 0x7b944d8
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b94598
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWeaponProjectileSwitcherEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b944e4, Quantum.AssetRefWeaponProjectileSwitcherEnhancementSpec.op_Implicit): WeaponProjectileSwitcherEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefWeaponProjectileSwitcherEnhancementSpec) == AssetRefWeaponProjectileSwitcherEnhancementSpec::SIZE);

struct AssetRefWeaponSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b94650 (==), 0x7b94658 (!=), 0x7b945c8 (Equals), 0x7b94648 (Equals)
    friend constexpr bool operator==(AssetRefWeaponSpec a, AssetRefWeaponSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWeaponSpec other) const { return Id == other.Id; }
    // 0x7b94660
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1213u, Id); }
    // 0x7b945ac
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b9467c
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWeaponSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b945b8, Quantum.AssetRefWeaponSpec.op_Implicit): WeaponSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefWeaponSpec) == AssetRefWeaponSpec::SIZE);

struct AssetRefWrenchGroundSlamEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b946ac (Equals), 0x7b9472c (Equals)
    friend constexpr bool operator==(AssetRefWrenchGroundSlamEnhancementSpec a, AssetRefWrenchGroundSlamEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWrenchGroundSlamEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b94734
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1217u, Id); }
    // 0x7b94690
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b94750
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWrenchGroundSlamEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b9469c, Quantum.AssetRefWrenchGroundSlamEnhancementSpec.op_Implicit): WrenchGroundSlamEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefWrenchGroundSlamEnhancementSpec) == AssetRefWrenchGroundSlamEnhancementSpec::SIZE);

struct AssetRefWrenchThrowEnhancementSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b94780 (Equals), 0x7b94800 (Equals)
    friend constexpr bool operator==(AssetRefWrenchThrowEnhancementSpec a, AssetRefWrenchThrowEnhancementSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefWrenchThrowEnhancementSpec other) const { return Id == other.Id; }
    // 0x7b94808
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1223u, Id); }
    // 0x7b94764
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b94824
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefWrenchThrowEnhancementSpec*>(ptr)->Id, serializer);
    }
    // TODO(0x7b94770, Quantum.AssetRefWrenchThrowEnhancementSpec.op_Implicit): WrenchThrowEnhancementSpec asset -> reference (its GUID,
    // empty for null); needs the asset classes.
};
static_assert(sizeof(AssetRefWrenchThrowEnhancementSpec) == AssetRefWrenchThrowEnhancementSpec::SIZE);

struct AssetRefZoneControlGameModeSpec {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 8;

    AssetGuid Id;

    // 0x7b94844 (Equals), 0x7b948c4 (Equals)
    friend constexpr bool operator==(AssetRefZoneControlGameModeSpec a, AssetRefZoneControlGameModeSpec b) { return a.Id == b.Id; }
    constexpr bool Equals(AssetRefZoneControlGameModeSpec other) const { return Id == other.Id; }
    // 0x7b948cc
    constexpr std::int32_t GetHashCode() const { return detail::TypedAssetRefHash(1229u, Id); }
    // 0x7b94838
    std::string ToString() const { return AssetRef{Id}.ToString(); }
    // 0x7b948e8
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefZoneControlGameModeSpec*>(ptr)->Id, serializer);
    }
};
static_assert(sizeof(AssetRefZoneControlGameModeSpec) == AssetRefZoneControlGameModeSpec::SIZE);

} // namespace Quantum
