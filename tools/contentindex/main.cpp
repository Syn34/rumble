// contentindex - lists the simulation assets found in the game's bundles (development).
//   contentindex <aa/Android folder> <libunity.so> [--load]
// With --load, every asset is also built through the provider and the number of
// assets served by a real C++ type vs. as raw data is reported, together with the
// component prototypes found for entity prototypes (from their prefabs) and maps (their
// baked entities), counted per prototype type.
#include "content/quantum_code_factories.h"
#include "content/quantum_content_provider.h"
#include "photon_deterministic/log.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/component_prototype.h"
#include "quantum_core/map.h"
#include "math_tables.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: contentindex <bundle folder> <libunity.so> [--load]\n");
        return 2;
    }
    std::ifstream lib(argv[2], std::ios::binary);
    const std::vector<std::uint8_t> libBytes(std::istreambuf_iterator<char>(lib), {});
    auto strings = rr::assets::CommonStrings::FromEngineLibrary(libBytes);
    if (!strings) {
        std::fprintf(stderr, "no field-name table found in %s\n", argv[2]);
        return 1;
    }
    const auto toStderr = [](const char* level) {
        return [level](const std::string& message) { std::fprintf(stderr, "%s: %s\n", level, message.c_str()); };
    };
    Photon::Deterministic::Log::Init(nullptr, toStderr("warning"), toStderr("error"), nullptr);
    rr::assets::ContentLibrary library(argv[1], std::move(*strings));
    rr::content::RegisterQuantumCodeAssetFactories();
    rr::content::QuantumContentProvider provider(library);

    const auto ids = provider.ListAssets();
    std::map<std::string, int> perClass;
    for (const auto& id : ids) ++perClass[provider.ScriptClassOf(id)];
    std::printf("%zu bundles, %zu simulation assets\n", library.Bundles().size(), ids.size());
    for (const auto& [cls, count] : perClass) std::printf("  %5d  %s\n", count, cls.c_str());

    if (argc > 3 && std::string(argv[3]) == "--load") {
        int mapped = 0, unmapped = 0, failed = 0;
        int prototypes = 0, filledPrototypes = 0, maps = 0, mapEntities = 0, filledMapEntities = 0;
        std::map<std::string, int> fromPrefabs, fromMaps;
        std::map<int, int> staticShapes; // Shape3DType -> map static colliders
        int mapsWithMeshes = 0;
        std::size_t meshTriangles = 0;
        int meshKeys = 0, strayKeys = 0;
        std::vector<std::shared_ptr<Quantum::Map>> loadedMaps;
        std::map<std::int64_t, std::shared_ptr<Quantum::BinaryData>> binaryData;
        const auto count = [](const Quantum::EntityPrototypeContainer& container, std::map<std::string, int>& perType) {
            for (const auto& component : container.Components) {
                ++perType[component ? component->PrototypeTypeName() : std::string("(null)")];
            }
        };
        for (const auto& id : ids) {
            std::shared_ptr<Quantum::AssetObject> asset;
            try {
                asset = provider.LoadAsset(id);
            } catch (const std::exception& e) {
                std::fprintf(stderr, "%s: %s\n", id.Path.c_str(), e.what());
            }
            if (!asset) ++failed;
            else if (dynamic_cast<rr::content::UnmappedAsset*>(asset.get())) ++unmapped;
            else ++mapped;
            if (dynamic_cast<Quantum::BinaryData*>(asset.get())) {
                binaryData[id.Guid.Value] = std::static_pointer_cast<Quantum::BinaryData>(asset);
            }
            if (auto* prototype = dynamic_cast<Quantum::EntityPrototype*>(asset.get())) {
                ++prototypes;
                if (prototype->Container && !prototype->Container->Components.empty()) {
                    ++filledPrototypes;
                    count(*prototype->Container, fromPrefabs);
                } else {
                    std::printf("  empty prototype: %s\n", id.Path.c_str());
                }
            }
            if (auto* map = dynamic_cast<Quantum::Map*>(asset.get())) {
                ++maps;
                for (const auto& collider : map->StaticColliders3D) {
                    ++staticShapes[static_cast<int>(collider.ShapeType)];
                }
                if (map->StaticColliders3DTrianglesData.Id.IsValid()) {
                    loadedMaps.push_back(std::static_pointer_cast<Quantum::Map>(asset));
                }
                for (const auto& entity : map->MapEntities) {
                    ++mapEntities;
                    if (entity && !entity->Components.empty()) {
                        ++filledMapEntities;
                        count(*entity, fromMaps);
                    }
                }
            }
        }
        std::printf("loaded: %d with C++ types, %d as raw data, %d failed\n", mapped, unmapped, failed);
        std::printf("entity prototypes: %d, %d with components\n", prototypes, filledPrototypes);
        for (const auto& [type, n] : fromPrefabs) std::printf("  %5d  %s\n", n, type.c_str());
        std::printf("maps: %d with %d entities, %d with components\n", maps, mapEntities, filledMapEntities);
        for (const auto& [type, n] : fromMaps) std::printf("  %5d  %s\n", n, type.c_str());
        // The baked triangles of each map's mesh colliders, read as Map.Loaded does. The
        // triangles' derived data needs sqrt; the tests' stand-in tables are close enough
        // for counting (the real tables are game data with no loader yet).
        rr::test::InstallMathDerivedTables();
        for (const auto& map : loadedMaps) {
            const auto found = binaryData.find(map->StaticColliders3DTrianglesData.Id.Value);
            if (found == binaryData.end()) {
                std::fprintf(stderr, "%s: triangle data %s not found\n", map->Path.c_str(),
                             map->StaticColliders3DTrianglesData.Id.ToString(true).c_str());
                continue;
            }
            try {
                const auto data = found->second->CreateByteStream();
                map->SerializeStaticColliderTriangles(data->Stream, false);
                ++mapsWithMeshes;
                for (const auto& [index, tris] : map->StaticColliders3DTriangles) {
                    meshTriangles += tris.size();
                    const bool isMesh = index >= 0 && static_cast<std::size_t>(index) < map->StaticColliders3D.size() &&
                                        map->StaticColliders3D[static_cast<std::size_t>(index)].ShapeType ==
                                            Quantum::Shape3DType::Mesh;
                    ++(isMesh ? meshKeys : strayKeys);
                    for (const auto& tri : tris) {
                        if (tri.StaticDataIndex != index) ++strayKeys;
                    }
                }
            } catch (const std::exception& e) {
                std::fprintf(stderr, "%s: triangles: %s\n", map->Path.c_str(), e.what());
            }
        }
        std::printf("map static 3D colliders by shape type:");
        for (const auto& [type, n] : staticShapes) std::printf(" %d:%d", type, n);
        std::printf("\nmaps with static mesh data: %d (%zu triangles loaded for %d mesh colliders, %d inconsistent)\n",
                    mapsWithMeshes, meshTriangles, meshKeys, strayKeys);
    }
    return 0;
}
