// Tests for quantum.core's asset system, with assets built in code.
#include "test_framework.h"

#include "photon_deterministic/system_allocator.h"
#include "quantum_core/assert.h"
#include "quantum_core/asset_db.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/asset_resource_provider.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/byte_stream.h"
#include "quantum_core/dynamic_asset_db.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/frame_thread_safe.h"
#include "quantum_core/map.h"
#include "quantum_core/navigation/navigation.h"
#include "quantum_core/navigation/navmesh.h"
#include "quantum_core/navigation/navmesh_agent_config.h"
#include "quantum_core/polygon_collider.h"
#include "quantum_core/resource_manager.h"

#include <atomic>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace Quantum;
using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;
using Photon::Deterministic::SystemAllocator;
using Quantum::Core::FrameBase;
using Quantum::Core::FrameContext;

namespace {

template <class E, class F> bool Throws(F&& f) {
    try {
        f();
    } catch (const E&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

// An asset that counts its hooks.
class CountingAsset final : public AssetObject {
  public:
    int LoadedCalls = 0;
    int DisposedCalls = 0;
    IResourceManager* LastManager = nullptr;
    int Payload = 0;

    void Loaded(IResourceManager* resourceManager, Allocator* /*allocator*/) override {
        ++LoadedCalls;
        LastManager = resourceManager;
    }
    void Disposed(IResourceManager* /*resourceManager*/, Allocator* /*allocator*/) override {
        ++DisposedCalls;
    }
};

std::shared_ptr<CountingAsset> MakeCounting(std::int64_t guid, std::string path = {},
                                            int payload = 0) {
    auto asset = std::make_shared<CountingAsset>();
    asset->Guid = AssetGuid(guid);
    asset->Path = std::move(path);
    asset->Payload = payload;
    return asset;
}

AssetGuid Dynamic(std::int64_t number) {
    return AssetGuid(static_cast<std::int64_t>(static_cast<std::uint64_t>(number) |
                                               AssetGuid::DynamicBit));
}

// A toy serializer: per asset its GUID (8 bytes), payload (4) and path length + bytes.
class TestSerializer final : public IAssetSerializer {
  public:
    std::vector<std::uint8_t>
    SerializeAssets(const std::vector<std::shared_ptr<AssetObject>>& assets) override {
        std::vector<std::uint8_t> out;
        for (const auto& asset : assets) {
            const auto* counting = dynamic_cast<const CountingAsset*>(asset.get());
            Append(out, static_cast<std::uint64_t>(asset->Guid.Value), 8);
            Append(out, static_cast<std::uint32_t>(counting ? counting->Payload : 0), 4);
            Append(out, asset->Path.size(), 4);
            out.insert(out.end(), asset->Path.begin(), asset->Path.end());
        }
        return out;
    }
    std::string PrintAsset(const AssetObject& asset) override { return asset.Identifier().ToString(); }
    std::vector<std::shared_ptr<AssetObject>>
    DeserializeAssets(std::span<const std::uint8_t> data, int index, int count) override {
        std::vector<std::shared_ptr<AssetObject>> assets;
        std::size_t at = static_cast<std::size_t>(index);
        const std::size_t end = at + static_cast<std::size_t>(count);
        while (at < end) {
            const auto guid = static_cast<std::int64_t>(Read(data, at, 8));
            const auto payload = static_cast<int>(Read(data, at, 4));
            const auto length = static_cast<std::size_t>(Read(data, at, 4));
            std::string path(reinterpret_cast<const char*>(data.data() + at), length);
            at += length;
            assets.push_back(MakeCounting(guid, path, payload));
        }
        return assets;
    }

  private:
    static void Append(std::vector<std::uint8_t>& out, std::uint64_t value, int bytes) {
        for (int i = 0; i < bytes; ++i) {
            out.push_back(static_cast<std::uint8_t>(value >> (8 * i)));
        }
    }
    static std::uint64_t Read(std::span<const std::uint8_t> data, std::size_t& at, int bytes) {
        std::uint64_t value = 0;
        for (int i = 0; i < bytes; ++i) {
            value |= std::uint64_t{data[at++]} << (8 * i);
        }
        return value;
    }
};

// Builds a serialised dynamic database holding `assets` with `lastGuid` handed out.
std::vector<std::uint8_t> SerializedDynamicDB(const std::vector<std::shared_ptr<AssetObject>>& assets,
                                              std::int64_t lastGuid) {
    TestSerializer serializer;
    std::vector<std::uint8_t> bytes = {1, 0, 0, 0};
    for (int i = 0; i < 8; ++i) {
        bytes.push_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(lastGuid) >> (8 * i)));
    }
    const auto body = serializer.SerializeAssets(assets);
    bytes.insert(bytes.end(), body.begin(), body.end());
    return bytes;
}

class CountingVisitor final : public IAssetObjectVisitor {
  public:
    std::string Last;
    void Visit(BinaryData&) override { Last = "BinaryData"; }
    void Visit(CharacterController2DConfig&) override { Last = "CC2D"; }
    void Visit(CharacterController3DConfig&) override { Last = "CC3D"; }
    void Visit(EntityPrototype&) override { Last = "EntityPrototype"; }
    void Visit(EntityView&) override { Last = "EntityView"; }
    void Visit(Map&) override { Last = "Map"; }
    void Visit(NavMesh&) override { Last = "NavMesh"; }
    void Visit(NavMeshAgentConfig&) override { Last = "NavMeshAgentConfig"; }
    void Visit(PhysicsMaterial&) override { Last = "PhysicsMaterial"; }
    void Visit(PolygonCollider&) override { Last = "PolygonCollider"; }
    void Visit(TerrainCollider&) override { Last = "TerrainCollider"; }
};

class AssetTestFrame final : public FrameBase {
  public:
    explicit AssetTestFrame(FrameContext* context) : FrameBase(context) {}
    FrameMetaData* get_FrameMetaData() override { return &_meta; }
    CommitCommandsModes get_CommitCommandsMode() const override {
        return CommitCommandsModes::OnFrameSimulationBegin;
    }
    FP get_DeltaTime() const override { return FP::_1(); }
    void set_DeltaTime(FP) override {}
    Map* get_Map() const override { return MapAsset; }
    std::string DumpFrame(int) override { return {}; }
    std::uint64_t CalculateChecksum() override { return 0; }
    std::vector<std::uint8_t> Serialize(Photon::Deterministic::DeterministicFrameSerializeMode) override {
        return {};
    }
    void Deserialize(std::span<const std::uint8_t>) override {}
    void Copy(const Photon::Deterministic::DeterministicFrame&) override {}

    Map* MapAsset = nullptr;

  private:
    FrameMetaData _meta{};
};

} // namespace

TEST_CASE("Asset identifiers, references and type names") {
    CHECK(AssetRef{AssetGuid(0x2A)}.ToString() == "Id: [000000000000002A]");
    const AssetObjectIdentifier identifier{"Folder/Thing", AssetGuid(7)};
    CHECK(identifier.ToString() == "Folder/Thing [0000000000000007]");

    auto config = std::make_shared<NavMeshAgentConfig>();
    config->Guid = AssetGuid(99);
    CHECK(AssetRef::FromAsset(config.get()).Id == AssetGuid(99));
    CHECK(!AssetRef::FromAsset(nullptr).Id.IsValid());
    const AssetRefNavMeshAgentConfig typed = AssetRefNavMeshAgentConfig::FromAsset(config.get());
    CHECK(typed.Id == AssetGuid(99));
    CHECK(typed == AssetRefNavMeshAgentConfig{AssetGuid(99)});
    CHECK(typed.ToString() == "Id: [0000000000000063]");
    CHECK(static_cast<AssetRef>(typed).Id == AssetGuid(99));
    CHECK(sizeof(AssetRefMap) == 8);

    CHECK(AssetTypeName<NavMeshAgentConfig>() == "NavMeshAgentConfig");
    const AssetObject& base = *config;
    CHECK(AssetTypeName(typeid(base)) == "NavMeshAgentConfig");
    CHECK(config->HasPath() == false);

    AssetObjectConfigAttribute attribute;
    CHECK(attribute._generateLinkingScripts && attribute._generateAsMainScript);
    CHECK(attribute._customCreateAssetMenuOrder == -1);
    attribute.SetGenerateAssetCreateMenu(false);
    CHECK(!attribute._generateAssetCreateMenu);

    CHECK(Throws<AssertException>([] { Assert::Always(false); }));
    Assert::Always(true);
}

TEST_CASE("Built-in assets dispatch to their visitor method") {
    CountingVisitor visitor;
    BinaryData data;
    data.Dispatch(visitor);
    CHECK(visitor.Last == "BinaryData");
    Map map;
    map.Dispatch(visitor);
    CHECK(visitor.Last == "Map");
    NavMesh navmesh;
    navmesh.Dispatch(visitor);
    CHECK(visitor.Last == "NavMesh");
    TerrainCollider terrain;
    terrain.Dispatch(visitor);
    CHECK(visitor.Last == "TerrainCollider");
    PolygonCollider polygon;
    static_cast<IBuiltInAssetObject&>(polygon).Dispatch(visitor);
    CHECK(visitor.Last == "PolygonCollider");
}

TEST_CASE("AssetResource state transitions") {
    AssetResource resource(AssetGuid(5), "path");
    CHECK(resource.HasPath());
    CHECK(resource.get_Identifier().Guid == AssetGuid(5));
    CHECK(!resource.TryTransition(AssetResource::Unloaded, false)); // only from Disposing
    CHECK(!resource.TryTransition(AssetResource::Disposing, false)); // only from Loaded
    CHECK(resource.TryTransition(AssetResource::Loading, false));
    CHECK(!resource.TryTransition(AssetResource::Loading, false));
    CHECK(resource.TryTransition(AssetResource::Loading, true)); // re-entry allowed
    CHECK(!resource.TryTransition(AssetResource::LoadingAsync, true)); // never backwards
    resource.State = AssetResource::Loaded;
    CHECK(resource.IsLoaded());
    CHECK(resource.TryTransition(AssetResource::Disposing, false));
    // Asking for the asset while it waits for disposal cancels the disposal.
    CHECK(!resource.TryTransition(AssetResource::Loading, false));
    CHECK(resource.State == AssetResource::Loaded);
    CHECK(resource.TryTransition(AssetResource::Disposing, false));
    CHECK(resource.TryTransition(AssetResource::Unloaded, false));
    CHECK(resource.State == AssetResource::Unloaded);
}

TEST_CASE("ResourceManagerStatic loads once and validates its resources") {
    SystemAllocator allocator;
    auto a = MakeCounting(10, "a");
    auto b = MakeCounting(11);
    ResourceManagerStatic manager({a, b}, &allocator);
    CHECK(manager.Count() == 2);
    CHECK(manager.GetAssetGuid("a") == AssetGuid(10));
    CHECK(!manager.GetAssetGuid("missing").IsValid());

    CHECK(manager.GetAsset(AssetGuid(10), true) == a);
    CHECK(manager.GetAsset(AssetGuid(10), false) == a);
    CHECK(a->LoadedCalls == 1);
    CHECK(a->LastManager == &manager);
    CHECK(manager.GetAsset(AssetGuid(12), true) == nullptr);
    CHECK(Throws<std::invalid_argument>([&] { manager.GetAsset(Dynamic(1), true); }));
    CHECK(manager.DisposeAsset(AssetGuid(10))); // static assets stay loaded
    CHECK(!manager.DisposeAsset(AssetGuid(12)));

    const auto all = manager.LoadAllAssets(true);
    CHECK(all.size() == 2 && all[0] == a && all[1] == b);
    CHECK(b->LoadedCalls == 1);

    manager.Dispose();
    CHECK(a->DisposedCalls == 1 && b->DisposedCalls == 1);

    CHECK(Throws<std::invalid_argument>([&] {
        ResourceManagerStatic duplicate({MakeCounting(1), MakeCounting(1)}, nullptr);
    }));
    CHECK(Throws<std::invalid_argument>([&] {
        ResourceManagerStatic duplicate({MakeCounting(1, "p"), MakeCounting(2, "p")}, nullptr);
    }));
    CHECK(Throws<std::invalid_argument>([&] { ResourceManagerStatic bad({MakeCounting(0)}, nullptr); }));
    CHECK(Throws<std::invalid_argument>([&] {
        ResourceManagerStatic bad({MakeCounting(Dynamic(3).Value)}, nullptr);
    }));
}

TEST_CASE("AssetDB caches lookups by GUID and path") {
    auto a = MakeCounting(20, "x/a");
    auto config = std::make_shared<NavMeshAgentConfig>();
    config->Guid = AssetGuid(21);
    ResourceManagerStatic manager({a, config}, nullptr);
    const auto db = manager.CreateAssetDatabase();
    CHECK(db->get_ResourceManager() == &manager);
    CHECK(db->FindAsset(AssetGuid(), true) == nullptr);
    CHECK(db->FindAsset(AssetGuid(20), true) == a.get());
    CHECK(db->FindAsset<NavMeshAgentConfig>(AssetGuid(21), true) == config.get());
    CHECK(db->FindAsset<NavMeshAgentConfig>(AssetGuid(20), true) == nullptr);
    CHECK(db->FindAsset(AssetGuid(99), true) == nullptr);
    CHECK(db->FindAllAssets(true).size() == 2);

    db->DisposeAsset(AssetGuid(20), true);
    // Found again through the manager.
    CHECK(db->FindAsset(AssetGuid(20), true) == a.get());
    CHECK(Throws<std::invalid_argument>([&] { db->AddAssetToCollection(nullptr); }));

    manager.DiposeAssetDatabase(db.get());
    CHECK(db->get_ResourceManager() == nullptr);
}

TEST_CASE("ResourceManagerDynamic loads through the provider and defers disposal") {
    SystemAllocator allocator;
    InMemoryAssetProvider provider;
    auto a = MakeCounting(30, "dyn/a");
    auto polygon = std::make_shared<PolygonCollider>();
    polygon->Guid = AssetGuid(31);
    polygon->Vertices = {FPVector2(1, 1), FPVector2(-1, 1), FPVector2(-1, -1), FPVector2(1, -1)};
    provider.Add(a);
    provider.Add(polygon);

    ProviderAssetSystem system = CreateResourceManager(provider, &allocator);
    ResourceManagerDynamic& manager = *system.ResourceManager;
    CHECK(manager.Count() == 2);
    const auto db = manager.CreateAssetDatabase();

    CHECK(db->FindAsset(AssetGuid(30), true) == a.get());
    CHECK(a->LoadedCalls == 1);
    CHECK(db->FindAsset<PolygonCollider>(AssetGuid(31), true) == polygon.get());
    CHECK(polygon->VerticesBuffer != nullptr && polygon->NormalsBuffer != nullptr);
    CHECK(polygon->Normals.size() == 4);

    // Disposal waits for the end of the simulation step, then reaches the database too.
    CHECK(manager.DisposeAsset(AssetGuid(30)));
    CHECK(a->DisposedCalls == 0);
    manager.MainThreadSimulationEnded();
    CHECK(a->DisposedCalls == 1);
    AssetResource* resource = nullptr;
    CHECK(manager.TryGetAssetResource(AssetGuid(30), resource));
    CHECK(resource->Asset == nullptr && resource->State == AssetResource::Unloaded);
    // Loaded again on the next lookup.
    CHECK(db->FindAsset(AssetGuid(30), true) == a.get());
    CHECK(a->LoadedCalls == 2);

    // A load requested from a worker thread runs in the main thread's update.
    CHECK(manager.DisposeAsset(AssetGuid(30)));
    manager.MainThreadSimulationEnded();
    std::atomic<bool> done{false};
    std::shared_ptr<AssetObject> fromWorker;
    std::thread worker([&] {
        fromWorker = manager.GetAsset(AssetGuid(30), false);
        done = true;
    });
    while (!done) {
        manager.MainThreadUpdate(nullptr);
        std::this_thread::yield();
    }
    worker.join();
    CHECK(fromWorker == a);
    CHECK(a->LoadedCalls == 3);

    manager.DiposeAssetDatabase(db.get());
    manager.Dispose();
    CHECK(polygon->VerticesBuffer == nullptr);
    CHECK(a->DisposedCalls == 3);
}

TEST_CASE("ResourceManagerDynamic reports a failed load") {
    class FailingProvider final : public IAssetResourceProvider {
      public:
        std::vector<AssetObjectIdentifier> ListAssets() override {
            return {AssetObjectIdentifier{"broken", AssetGuid(40)}};
        }
        std::shared_ptr<AssetObject> LoadAsset(const AssetObjectIdentifier&) override {
            throw std::runtime_error("unreadable");
        }
    };
    FailingProvider provider;
    ProviderAssetSystem system = CreateResourceManager(provider, nullptr);
    CHECK(system.ResourceManager->GetAsset(AssetGuid(40), true) == nullptr);
    CHECK(system.ResourceManager->GetAssetGuid("broken") == AssetGuid(40));
}

TEST_CASE("DynamicAssetDB serialises, copies and looks up dynamic assets") {
    TestSerializer serializer;
    DynamicAssetDB db;
    CHECK(db.IsEmpty());
    CHECK(db.NextGuid() == Dynamic(1));

    const auto bytes = SerializedDynamicDB({MakeCounting(Dynamic(1).Value, "one", 11),
                                            MakeCounting(Dynamic(2).Value, {}, 22)},
                                           2);
    const auto loaded = db.Deserialize(bytes, serializer);
    CHECK(loaded.size() == 2);
    CHECK(!db.IsEmpty());
    CHECK(db.NextGuid() == Dynamic(3));
    auto* one = db.FindAsset<CountingAsset>(Dynamic(1));
    CHECK(one != nullptr && one->Payload == 11 && one->Path == "one");
    CHECK(db.FindAsset(Dynamic(5)) == nullptr);

    // Round trip.
    const auto again = db.Serialize(serializer);
    CHECK(again == bytes);
    std::vector<std::uint8_t> header;
    std::vector<std::uint8_t> body;
    db.Serialize(serializer, header, body);
    CHECK(header.size() == 12 && header[0] == 1);

    // Copies are independent.
    DynamicAssetDB copy;
    copy.CopyFrom(db);
    CHECK(copy.DisposeAsset(Dynamic(1)));
    CHECK(!copy.DisposeAsset(Dynamic(1)));
    CHECK(copy.FindAsset(Dynamic(1)) == nullptr);
    CHECK(db.FindAsset(Dynamic(1)) == one);
    CHECK(copy.Assets().size() == 1 && db.Assets().size() == 2);

    // Only GUIDs up to the last one handed out, only dynamic ones, no duplicates.
    CHECK(Throws<std::invalid_argument>(
        [&] { db.AddAssetWithValidDynamicGuid(MakeCounting(Dynamic(3).Value)); }));
    CHECK(Throws<std::invalid_argument>([&] { db.AddAssetWithValidDynamicGuid(MakeCounting(7)); }));
    CHECK(Throws<std::invalid_argument>(
        [&] { db.AddAssetWithValidDynamicGuid(MakeCounting(Dynamic(2).Value)); }));
    CHECK(Throws<std::invalid_argument>([&] { db.AddAssetWithValidDynamicGuid(nullptr); }));
    CHECK(DynamicAssetDB::StripDynamicFlag(Dynamic(9)) == 9);
    CHECK(Throws<std::invalid_argument>([] { DynamicAssetDB::StripDynamicFlag(AssetGuid(9)); }));

    // Version check.
    auto future = bytes;
    future[0] = 2;
    CHECK(Throws<std::invalid_argument>([&] { db.Deserialize(future, serializer); }));

    // Static GUIDs go to the static manager.
    auto staticAsset = MakeCounting(50);
    ResourceManagerStatic manager({staticAsset}, nullptr);
    CHECK(db.GetAsset(AssetGuid(50), true) == nullptr);
    db.set_StaticResourceManager(&manager);
    CHECK(db.GetAsset(AssetGuid(50), true) == staticAsset);
    CHECK(db.GetAsset(Dynamic(2), true) != nullptr);
    CHECK(Throws<std::logic_error>([&] { db.LoadAllAssets(true); }));
}

TEST_CASE("Built-in asset hooks") {
    PhysicsMaterial material;
    CHECK(material.FrictionStatic.RawValue() == 26214);
    material.FrictionStatic = FP::FromInt(3);
    material.FrictionDynamic = FP::FromInt(-1);
    material.Loaded(nullptr, nullptr);
    CHECK(material.FrictionStatic == FP::_1() && material.FrictionDynamic == FP::_0());

    CharacterController2DConfig cc2d;
    cc2d.Loaded(nullptr, nullptr);
    CHECK(cc2d.GravityStrength() == FP::FromInt(10));
    CHECK(cc2d.GravityNormalized() == FPVector2(0, -1));
    CHECK(cc2d.MaxContacts == 4 && cc2d.AirControl);

    CharacterController3DConfig cc3d;
    cc3d.Gravity = FPVector3(FP::FromInt(3), FP::FromInt(0), FP::FromInt(4));
    cc3d.Loaded(nullptr, nullptr);
    CHECK(cc3d.GravityStrength() == FP::FromInt(5));
    CHECK(cc3d.Offset.Y == FP::_0_50());

    TerrainCollider terrain;
    terrain.Resolution = 10;
    CHECK(!terrain.IsHole(1, 2)); // no mask
    CHECK(Throws<std::out_of_range>([&] { terrain.SetHole(1, 2); }));
    terrain.HoleMask.assign(2, 0);
    terrain.SetHole(1, 2);
    CHECK(terrain.IsHole(1, 2));
    CHECK(!terrain.IsHole(2, 1));
    CHECK(terrain.HoleMask[0] == (std::uint64_t{1} << 12));
    CHECK(!terrain.IsHole(-5, 0));
    CHECK(Throws<std::out_of_range>([&] { terrain.SetHole(20, 0); }));
    CHECK(terrain.Triangles().empty());

    BinaryData data;
    data.Data = {1, 0, 0, 0};
    const auto stream = data.CreateByteStream();
    CHECK(stream->Stream.ReadInt() == 1);
}

TEST_CASE("Map values, navmeshes and regions") {
    Map map;
    CHECK(map.WorldSize == 256 && map.BucketsCount == 16 && map.TriangleMeshCellSize == 4);
    map.GridSizeX = 10;
    map.GridSizeY = 4;
    map.GridNodeSize = 3;
    CHECK(map.WorldExtentX() == 15 && map.WorldExtentY() == 6);
    CHECK(map.WorldOffset() == FPVector2(-15, -6));

    map.WorldSize = 0;
    map.EnsurePhysicsWorldSizeInValidRange();
    CHECK(map.WorldSize == 1);
    map.WorldSize = 20000; // bucketing X, sorting Y
    map.EnsurePhysicsWorldSizeInValidRange();
    CHECK(map.WorldSize == 20000);
    map.WorldSize = 40000;
    map.EnsurePhysicsWorldSizeInValidRange();
    CHECK(map.WorldSize == 0x7FFF);
    map.SortingAxis = Map::SortAxis::X;
    map.EnsurePhysicsWorldSizeInValidRange();
    CHECK(map.WorldSize == 0x3FFF);

    auto navmesh = std::make_shared<NavMesh>();
    navmesh->Guid = AssetGuid(60);
    navmesh->Name = "Ground";
    auto triangles = std::make_shared<BinaryData>();
    triangles->Guid = AssetGuid(61);
    auto mapAsset = std::make_shared<Map>();
    mapAsset->Guid = AssetGuid(62);
    mapAsset->BucketsCount = 0;
    mapAsset->NavMeshLinks = {AssetRefNavMesh{AssetGuid(60)}};
    mapAsset->Regions = {"Red", "Blue"};
    mapAsset->StaticColliders3DTrianglesData = AssetRefBinaryData{AssetGuid(61)};

    ResourceManagerStatic manager({navmesh, triangles, mapAsset}, nullptr);
    CHECK(manager.GetAsset(AssetGuid(62), true) == mapAsset);
    CHECK(mapAsset->BucketsCount == 1);
    CHECK(mapAsset->GetNavMesh("Ground") == navmesh.get());
    CHECK(mapAsset->GetNavMesh("Air") == nullptr);
    CHECK(mapAsset->RegionMap.at("Blue") == 1);

    mapAsset->Regions = {"Twice", "Twice"};
    CHECK(Throws<std::invalid_argument>([&] { mapAsset->InitNavMeshes(&manager); }));
    mapAsset->Regions.clear();
    mapAsset->NavMeshLinks = {AssetRefNavMesh{AssetGuid(70)}};
    CHECK(Throws<std::invalid_argument>([&] { mapAsset->InitNavMeshes(&manager); }));
}

TEST_CASE("NavMesh reads its data asset through the resource manager") {
    NavMesh source;
    source.Name = "Source";
    std::vector<std::uint8_t> buffer(1024);
    ByteStream writer(&buffer);
    source.Serialize(writer, true);
    buffer.resize(static_cast<std::size_t>(writer.Position()));

    auto data = std::make_shared<BinaryData>();
    data->Guid = AssetGuid(80);
    data->Data = buffer;
    auto navmesh = std::make_shared<NavMesh>();
    navmesh->Guid = AssetGuid(81);
    navmesh->DataAsset = AssetGuid(80);
    InMemoryAssetProvider provider;
    provider.Add(data);
    provider.Add(navmesh);
    ProviderAssetSystem system = CreateResourceManager(provider, nullptr);
    CHECK(system.ResourceManager->GetAsset(AssetGuid(81), true) == navmesh);
    // The blob was handed back after reading: its disposal is pending.
    AssetResource* blob = nullptr;
    CHECK(system.ResourceManager->TryGetAssetResource(AssetGuid(80), blob));
    CHECK(blob->State == AssetResource::Disposing);
    system.ResourceManager->MainThreadSimulationEnded();
    CHECK(blob->State == AssetResource::Unloaded && blob->Asset == nullptr);
}

TEST_CASE("Frames find static and dynamic assets") {
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    platform.Allocator = &allocator;

    auto config = std::make_shared<NavMeshAgentConfig>();
    config->Guid = AssetGuid(NavMeshAgentConfig::DEFAULT_ID);
    config->Speed = FP::FromInt(9);
    config->Acceleration = FP::FromInt(4);
    auto view = std::make_shared<EntityView>();
    view->Guid = AssetGuid(90);
    ResourceManagerStatic manager({config, view}, nullptr);

    TestSerializer serializer;
    auto initial = std::make_shared<DynamicAssetDB>();
    initial->Deserialize(SerializedDynamicDB({MakeCounting(Dynamic(1).Value, {}, 5)}, 1),
                         serializer);

    FrameContext::Args args;
    args.PlatformInfo = &platform;
    args.HeapConfig = Quantum::Allocator::Heap::Config(12, 256, 4);
    args.AssetDatabase = manager.CreateAssetDatabase();
    args.AssetSerializer = &serializer;
    args.InitialDynamicAssets = initial;
    auto context = std::make_unique<FrameContext>(args);
    CHECK(context->get_AssetDB() == args.AssetDatabase.get());
    CHECK(args.AssetDatabase->_context == context.get());
    CHECK(context->get_AssetSerializer() == &serializer);

    {
        AssetTestFrame frame(context.get());
        CHECK(frame.get_DynamicAssetDB()->get_StaticResourceManager() == &manager);
        CHECK(frame.FindAsset<NavMeshAgentConfig>(AssetGuid(NavMeshAgentConfig::DEFAULT_ID)) ==
              config.get());
        CHECK(frame.FindAsset<EntityView>(AssetRef{AssetGuid(90)}) == view.get());
        CHECK(frame.FindAsset<NavMeshAgentConfig>(AssetGuid(90)) == nullptr); // type mismatch
        CHECK(frame.FindAsset<EntityView>(AssetGuid(91)) == nullptr);         // missing
        CHECK(frame.FindAsset<EntityView>(AssetGuid()) == nullptr);
        auto* dynamic = frame.FindAsset<CountingAsset>(Dynamic(1));
        CHECK(dynamic != nullptr && dynamic->Payload == 5);

        EntityView* found = nullptr;
        CHECK(frame.TryFindAsset(AssetGuid(90), found) && found == view.get());
        CHECK(!frame.TryFindAsset(AssetGuid(91), found) && found == nullptr);
        CHECK(frame.TryFindAsset(AssetRefEntityView{AssetGuid(90)}, found));

        // Typed references and the thread-safe frame.
        CHECK(AssetRefEntityView{AssetGuid(90)}.FindAsset(&frame) == view.get());
        FrameThreadSafe threadSafe(&frame, 2);
        CHECK(threadSafe.FindAsset<EntityView>(AssetGuid(90)) == view.get());
        CHECK(AssetRefNavMeshAgentConfig{AssetGuid(NavMeshAgentConfig::DEFAULT_ID)}.FindAsset(
                  &threadSafe) == config.get());
        CHECK(threadSafe.get_Map() == nullptr);

        // The steering agent falls back on the default agent config.
        NavMeshSteeringAgent steering{};
        steering.SetConfig(&frame, nullptr, false);
        CHECK(steering.MaxSpeed() == FP::FromInt(9));

        // A navigation entry finds its pathfinder's config.
        Navigation::Entry entry{};
        CHECK(entry.TryGetConfig(&frame) == nullptr);
        frame.Free();
    }

    context->Dispose();
    CHECK(args.AssetDatabase->get_ResourceManager() == nullptr);
    CHECK(args.AssetDatabase->_context == nullptr);
}
