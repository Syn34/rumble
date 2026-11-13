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

#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>
#include <string>

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
    }
    return 0;
}
