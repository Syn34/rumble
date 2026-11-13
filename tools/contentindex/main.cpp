// contentindex - lists the simulation assets found in the game's bundles (development).
//   contentindex <aa/Android folder> <libunity.so> [--load]
// With --load, every asset is also built through the provider and the number of
// assets served by a real C++ type vs. as raw data is reported.
#include "content/quantum_content_provider.h"

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
    rr::assets::ContentLibrary library(argv[1], std::move(*strings));
    rr::content::QuantumContentProvider provider(library);

    const auto ids = provider.ListAssets();
    std::map<std::string, int> perClass;
    for (const auto& id : ids) ++perClass[provider.ScriptClassOf(id)];
    std::printf("%zu bundles, %zu simulation assets\n", library.Bundles().size(), ids.size());
    for (const auto& [cls, count] : perClass) std::printf("  %5d  %s\n", count, cls.c_str());

    if (argc > 3 && std::string(argv[3]) == "--load") {
        int mapped = 0, unmapped = 0, failed = 0;
        for (const auto& id : ids) {
            auto asset = provider.LoadAsset(id);
            if (!asset) ++failed;
            else if (dynamic_cast<rr::content::UnmappedAsset*>(asset.get())) ++unmapped;
            else ++mapped;
        }
        std::printf("loaded: %d with C++ types, %d as raw data, %d failed\n", mapped, unmapped, failed);
    }
    return 0;
}
