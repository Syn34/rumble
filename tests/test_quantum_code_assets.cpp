// Gameplay asset factories: decoded Settings trees (built here, no game data) -> C++ assets.
#include "test_framework.h"

#include "content/quantum_code_factories.h"
#include "content/quantum_content_provider.h"
#include "quantum_code/assets/level_assets.h"
#include "quantum_core/fp_animation_curve.h"
#include "quantum_core/map.h"

#include <initializer_list>
#include <map>
#include <typeinfo>
#include <memory>
#include <string>
#include <utility>

// Like CHECK, but ends the test case on failure (later checks depend on it).
#define REQUIRE(expr)                                                                              \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            ::rr::test::ReportFailure(__FILE__, __LINE__, #expr);                                  \
            return;                                                                                \
        }                                                                                          \
    } while (0)

using rr::assets::Value;
using rr::assets::ValuePtr;
using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;

namespace {

ValuePtr Int(std::int64_t v, const char* type = "int") {
    auto value = std::make_shared<Value>();
    value->type = type;
    value->data = v;
    return value;
}

ValuePtr Str(const std::string& text) {
    auto value = std::make_shared<Value>();
    value->type = "string";
    value->data = text;
    return value;
}

ValuePtr Struct(const char* type, std::initializer_list<std::pair<const char*, ValuePtr>> fields) {
    auto value = std::make_shared<Value>();
    value->type = type;
    Value::Struct members;
    for (const auto& [name, field] : fields) members.push_back({name, field});
    value->data = std::move(members);
    return value;
}

ValuePtr Array(std::initializer_list<ValuePtr> items) {
    auto value = std::make_shared<Value>();
    value->type = "vector";
    value->data = Value::Array(items);
    return value;
}

ValuePtr Fp(std::int64_t raw) { return Struct("FP", {{"RawValue", Int(raw, "SInt64")}}); }
ValuePtr Vec3(std::int64_t x, std::int64_t y, std::int64_t z) {
    return Struct("FPVector3", {{"X", Fp(x)}, {"Y", Fp(y)}, {"Z", Fp(z)}});
}
ValuePtr Ref(std::int64_t guid) {
    return Struct("AssetRef", {{"Id", Struct("AssetGuid", {{"Value", Int(guid, "SInt64")}})}});
}
ValuePtr Identifier(std::int64_t guid) {
    return Struct("AssetObjectIdentifier",
                  {{"Path", Str("Test/Asset")}, {"Guid", Struct("AssetGuid", {{"Value", Int(guid, "SInt64")}})}});
}

template <class T> std::shared_ptr<T> Build(const std::string& scriptClass, const ValuePtr& settings) {
    rr::content::RegisterQuantumCodeAssetFactories();
    return std::dynamic_pointer_cast<T>(rr::content::BuildAsset(scriptClass, *settings));
}

} // namespace

TEST_CASE("Weapon spec factory reads base-class fields, nested lists and references") {
    const auto settings = Struct("WeaponSpecDefault", {
        {"Identifier", Identifier(77)},
        {"DamageType", Int(2)},
        {"ConsumesUltimateCharges", Int(1, "UInt8")},
        {"ConsumptionPerShot", Fp(3 << 15)},
        {"EffectOffset", Vec3(65536, -131072, 0)},
        {"AmmunitionAsset", Ref(1234)},
        {"MultiShotConfigs", Array({Struct("MultiShotConfig", {{"spawnOffsets", Array({Vec3(1, 2, 3), Vec3(4, 5, 6)})}}),
                                    Struct("MultiShotConfig", {{"spawnOffsets", Array({})}})})},
        {"BoltCost", Int(35, "UInt16")},
        {"MaxAmmo", Int(12, "unsigned int")},
        {"enhancementSpecs", Array({Ref(5), Ref(6)})},
    });
    const auto weapon = Build<Quantum::WeaponSpecDefault>("WeaponSpecDefaultAsset", settings);
    REQUIRE(weapon != nullptr);
    CHECK(weapon->DamageType == static_cast<Quantum::DamageType>(2));
    CHECK(weapon->ConsumesUltimateCharges);
    CHECK(weapon->ConsumptionPerShot == FP::FromRaw(3 << 15));
    CHECK(weapon->EffectOffset == FPVector3(FP::FromInt(1), FP::FromInt(-2), FP::FromRaw(0)));
    CHECK(weapon->AmmunitionAsset.Id.Value == 1234);
    REQUIRE(weapon->MultiShotConfigs.size() == 2);
    REQUIRE(weapon->MultiShotConfigs[0].spawnOffsets.size() == 2);
    CHECK(weapon->MultiShotConfigs[0].spawnOffsets[1].Z == FP::FromRaw(6));
    CHECK(weapon->MultiShotConfigs[1].spawnOffsets.empty());
    CHECK(weapon->BoltCost == 35);
    CHECK(weapon->MaxAmmo == 12u);
    REQUIRE(weapon->enhancementSpecs.size() == 2);
    CHECK(weapon->enhancementSpecs[1].Id.Value == 6);
    // Fields absent from the data keep the constructor defaults.
    CHECK(weapon->FireRate == FP::FromInt(1));
    CHECK(weapon->BotsMaxLongPressTime == FP::FromInt(3));
    CHECK(weapon->PickupMaxAmmo == 100u);
}

TEST_CASE("Constructor defaults of the gameplay assets match the original's") {
    const Quantum::WeaponSpecDefault weapon;
    CHECK(weapon.DamageType == static_cast<Quantum::DamageType>(1));
    CHECK(weapon.InputBufferingDuration == FP::FromRaw(16384));
    CHECK(weapon.EffectScale == FPVector3::One());
    CHECK(weapon.BurstInterruptionWindowDuration == FP::FromRaw(655));
    CHECK(weapon.Range == FP::FromInt(20));
    CHECK(weapon.BoltCost == 20);
    CHECK(weapon.MaxAmmo == 100u);
    CHECK(weapon.AmmoBurnPerShot == 1u);
    CHECK(weapon.UseRaycastForLookAtPoint && weapon.UseRangeForDefaultLookPoint);

    const Quantum::HeroSpec hero;
    CHECK(hero.LeveledPower.size() == 10);
    CHECK(hero.HasShield);

    const Quantum::TargetFinderSpec finder;
    CHECK(finder.CamDotValueForOnReticle == FP::FromString("0.999"));
}

TEST_CASE("Map custom data factory reads arrays of nested structs") {
    const auto settings = Struct("MapCustomData", {
        {"Identifier", Identifier(9)},
        {"SpawnPoints", Array({})},
        {"ClosingZones", Array({Struct("ClosingZoneData", {
                              {"startPosition", Vec3(0, 65536, 0)},
                              {"startRadius", Fp(10 << 16)},
                              {"closingPoints", Array({Struct("ClosingPointData", {{"targetPosition", Vec3(1, 1, 1)},
                                                                                  {"targetRadius", Fp(2 << 16)},
                                                                                  {"startTime", Fp(0)},
                                                                                  {"endTime", Fp(30 << 16)}})})},
                          })})},
        {"GoldenBoltSpawnPoints", Array({Vec3(7, 8, 9)})},
    });
    const auto data = Build<Quantum::MapCustomData>("MapCustomDataAsset", settings);
    REQUIRE(data != nullptr);
    REQUIRE(data->ClosingZones.size() == 1);
    CHECK(data->ClosingZones[0].startRadius == FP::FromInt(10));
    REQUIRE(data->ClosingZones[0].closingPoints.size() == 1);
    CHECK(data->ClosingZones[0].closingPoints[0].endTime == FP::FromInt(30));
    REQUIRE(data->GoldenBoltSpawnPoints.size() == 1);
    CHECK(data->GoldenBoltSpawnPoints[0].Y == FP::FromRaw(8));
}

TEST_CASE("Animation curves are read and evaluated between samples") {
    const auto curve = Struct("FPAnimationCurve", {
        {"Samples", Array({Fp(0), Fp(65536), Fp(4 * 65536)})},
        {"PreWrapMode", Int(0)},
        {"PostWrapMode", Int(0)},
        {"StartTime", Fp(0)},
        {"EndTime", Fp(2 * 65536)},
        {"Resolution", Int(2)},
    });
    const auto settings = Struct("SimulationWeaponTable", {
        {"Identifier", Identifier(10)},
        {"botsBackpackToWeaponLevelCurve", curve},
        {"entries", Array({Struct("Entry", {{"uniqueID", Int(3)}, {"entityPrototype", Ref(11)}, {"weaponSpecRef", Ref(12)}})})},
    });
    const auto table = Build<Quantum::SimulationWeaponTable>("SimulationWeaponTableAsset", settings);
    REQUIRE(table != nullptr);
    const Quantum::FPAnimationCurve& c = table->botsBackpackToWeaponLevelCurve;
    REQUIRE(c.Samples.size() == 3);
    CHECK(c.Evaluate(FP::FromRaw(0)) == FP::FromRaw(0));
    CHECK(c.Evaluate(FP::FromRaw(32768)) == FP::FromRaw(32768));   // halfway to the 2nd sample
    CHECK(c.Evaluate(FP::FromInt(1)) == FP::FromInt(1));
    CHECK(c.Evaluate(FP::FromRaw(98304)) == FP::FromRaw(163840));  // halfway between 1 and 4
    CHECK(c.Evaluate(FP::FromInt(5)) == FP::FromInt(4));           // clamped after the end
    CHECK(c.Evaluate(FP::FromInt(-1)) == FP::FromRaw(0));          // clamped before the start
    REQUIRE(table->entries.size() == 1);
    CHECK(table->entries[0].uniqueID == static_cast<Quantum::GENERATED_WEAPON_ID>(3));
    CHECK(table->entries[0].weaponSpecRef.Id.Value == 12);
}

TEST_CASE("Built-in map factory reads static colliders") {
    const auto settings = Struct("Map", {
        {"Identifier", Identifier(20)},
        {"Scene", Str("Level")},
        {"WorldSize", Int(512)},
        {"BucketingAxis", Int(1, "UInt8")},
        {"NavMeshLinks", Array({Ref(21)})},
        {"Regions", Array({Str("A"), Str("B")})},
        {"StaticColliders3D", Array({Struct("MapStaticCollider3D", {
                                   {"Position", Vec3(0, 0, 65536)},
                                   {"ShapeType", Int(2, "UInt8")},
                                   {"BoxExtents", Vec3(65536, 65536, 65536)},
                                   {"StaticData", Struct("StaticColliderData", {{"Name", Str("Wall")}, {"Layer", Int(3)},
                                                                                {"IsTrigger", Int(1, "UInt8")},
                                                                                {"MutableMode", Int(2)}})},
                               })})},
    });
    const auto map = Build<Quantum::Map>("MapAsset", settings);
    REQUIRE(map != nullptr);
    CHECK(map->Scene == "Level");
    CHECK(map->WorldSize == 512);
    CHECK(map->BucketingAxis == Quantum::Map::BucketAxis::Y);
    REQUIRE(map->NavMeshLinks.size() == 1);
    CHECK(map->NavMeshLinks[0].Id.Value == 21);
    CHECK(map->Regions.size() == 2);
    REQUIRE(map->StaticColliders3D.size() == 1);
    const Quantum::MapStaticCollider3D& collider = map->StaticColliders3D[0];
    CHECK(collider.ShapeType == Quantum::Shape3DType::Box);
    CHECK(collider.StaticData.Name == "Wall");
    CHECK(collider.StaticData.IsTrigger);
    CHECK(collider.StaticData.MutableMode == Quantum::StaticColliderMutableMode::ToggleableStartOff);
}

TEST_CASE("Unknown script classes have no factory") {
    rr::content::RegisterQuantumCodeAssetFactories();
    const auto settings = Struct("Nothing", {});
    CHECK(rr::content::BuildAsset("NotAnAssetClass", *settings) == nullptr);
}

namespace {

// A resource manager serving a fixed set of assets by GUID.
class FixedResourceManager final : public Quantum::IResourceManager {
  public:
    std::map<std::int64_t, std::shared_ptr<Quantum::AssetObject>> Assets;

    std::shared_ptr<Quantum::AssetDB> CreateAssetDatabase() override { return nullptr; }
    void DiposeAssetDatabase(Quantum::AssetDB*) override {}
    bool DisposeAsset(Quantum::AssetGuid) override { return false; }
    std::shared_ptr<Quantum::AssetObject> GetAsset(Quantum::AssetGuid guid, bool) override {
        auto it = Assets.find(guid.Value);
        return it != Assets.end() ? it->second : nullptr;
    }
    std::vector<std::shared_ptr<Quantum::AssetObject>> LoadAllAssets(bool) override { return {}; }
    void MainThreadSimulationEnded() override {}
    void MainThreadUpdate(Quantum::Profiling::Profiler*) override {}
    void Dispose() override {}
};

} // namespace

TEST_CASE("Bot SDK assets resolve their references when loaded") {
    FixedResourceManager manager;
    auto idle = std::make_shared<Quantum::HFSMState>();
    auto child = std::make_shared<Quantum::HFSMState>();
    auto decision = std::make_shared<Quantum::TrueDecision>();
    manager.Assets[101] = idle;
    manager.Assets[102] = child;
    manager.Assets[103] = decision;

    child->ParentLink.Id = Quantum::AssetGuid(101);
    Quantum::HFSMTransition transition;
    transition.DecisionLink.Id = Quantum::AssetGuid(103);
    transition.StateLink.Id = Quantum::AssetGuid(101);
    child->Transitions.push_back(transition);
    child->Loaded(&manager, nullptr);
    CHECK(child->Parent == idle.get());
    CHECK(child->IsChildOf(idle.get()));
    CHECK(!idle->IsChildOf(child.get()));
    REQUIRE(child->Transitions.size() == 1);
    CHECK(child->Transitions[0].Decision == decision.get());
    CHECK(child->Transitions[0].State == idle.get());
    CHECK(child->Transitions[0].TransitionSet == nullptr);

    Quantum::HFSMRoot root;
    root.StatesLinks = {Quantum::AssetRefHFSMState{Quantum::AssetGuid(101)}, Quantum::AssetRefHFSMState{Quantum::AssetGuid(102)}};
    root.EventsNames = {"Alert", "Calm"};
    root.Loaded(&manager, nullptr);
    CHECK(root.InitialState().Id.Value == 101);
    REQUIRE(root.States.size() == 2);
    CHECK(root.States[1] == child.get());
    CHECK(root.RegisteredEvents.at("Alert") == 1);
    CHECK(root.RegisteredEvents.at("Calm") == 2);

    // A reference to an asset of another type is an error, as the original's cast.
    Quantum::HFSMState wrong;
    wrong.ParentLink.Id = Quantum::AssetGuid(103);
    bool threw = false;
    try {
        wrong.Loaded(&manager, nullptr);
    } catch (const std::bad_cast&) {
        threw = true;
    }
    CHECK(threw);

    Quantum::AIBlackboard blackboard;
    blackboard.Entries.resize(2);
    blackboard.Entries[0].Key.Key = "target";
    blackboard.Entries[1].Key.Key = "timer";
    blackboard.Loaded(&manager, nullptr);
    CHECK(blackboard.Map.at("timer") == 1);
}

TEST_CASE("Asset helpers: levelled values, tables and results") {
    Quantum::WeaponSpecDefault weapon;
    Quantum::WeaponSpecModifier power;
    power.prop = Quantum::WeaponSpecModifier::Property::Power;
    power.values = {FP::FromInt(100), FP::FromInt(120), FP::FromRaw(140 * 65536 + 40000)};
    weapon.levelUpModifiers.push_back(power);
    CHECK(weapon.GetWeaponPowerAtLevel(1) == 120);
    CHECK(weapon.GetWeaponPowerAtLevel(7) == 140); // past the end: the last value, truncated
    CHECK(Quantum::WeaponSpecDefault{}.GetWeaponPowerAtLevel(0) == -1);
    CHECK(weapon.HasTag(static_cast<Quantum::WeaponTag>(0)) == false);

    Quantum::SimulationGadgetTable gadgets;
    gadgets.definitions.resize(2);
    gadgets.definitions[1].characterId = static_cast<Quantum::GENERATED_CHARACTER_ID>(4);
    gadgets.definitions[1].gadgetSpec.Id = Quantum::AssetGuid(55);
    Quantum::SimulationGadgetTable::GadgetDefinition found;
    CHECK(gadgets.TryGetGadget(static_cast<Quantum::GENERATED_CHARACTER_ID>(4), found));
    CHECK(found.gadgetSpec.Id.Value == 55);
    CHECK(!gadgets.TryGetGadget(static_cast<Quantum::GENERATED_CHARACTER_ID>(9), found));

    CHECK(Quantum::BlastBallGameModeSpec::GetBattleResult(0, 0, 3, 1) == Quantum::BATTLE_RESULT::WIN);
    CHECK(Quantum::BlastBallGameModeSpec::GetBattleResult(0, 0, 2, 2) == Quantum::BATTLE_RESULT::DRAW);
    CHECK(Quantum::BlastBallGameModeSpec::GetBattleResult(0, 0, 0, 2) == Quantum::BATTLE_RESULT::LOSE);
    CHECK(Quantum::FreeForAllGameModeSpec::GetBattleResult(4, 8, 0, 0) == Quantum::BATTLE_RESULT::WIN);
    CHECK(Quantum::FreeForAllGameModeSpec::GetBattleResult(5, 8, 0, 0) == Quantum::BATTLE_RESULT::LOSE);

    Quantum::DifficultyIntervalInt interval;
    interval.atMinDiff = 10;
    interval.atMaxDiff = 20;
    CHECK(interval.Get(FP::FromRaw(32768)) == 15);
    CHECK(interval.Get(FP::FromRaw(40000)) == 17); // 16.1 rounds up

    Quantum::ActorManagerConfig actors;
    actors.PendingRespawnDuration = FP::FromInt(2);
    actors.BotsPendingRespawnDuration = FP::FromInt(5);
    actors.RespawnDuration = FP::FromInt(1);
    actors.BotsUseSpecificPendingRespawnDuration = true;
    CHECK(actors.GetTotalRespawnDuration(false) == FP::FromInt(3));
    CHECK(actors.GetTotalRespawnDuration(true) == FP::FromInt(6));

    Quantum::HeroSpec hero;
    for (int i = 0; i < 10; ++i) hero.LeveledPower[static_cast<std::size_t>(i)] = FP::FromInt(100 + 10 * i);
    CHECK(hero.GetLevelAtPower(nullptr, static_cast<Quantum::GENERATED_CHARACTER_ID>(0), FP::FromInt(133)) == 3);
    CHECK(hero.GetLevelAtPower(nullptr, static_cast<Quantum::GENERATED_CHARACTER_ID>(0), FP::FromInt(135)) == 4);
}
