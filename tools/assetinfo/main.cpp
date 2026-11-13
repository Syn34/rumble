// assetinfo - prints what a game content file contains (for development).
//   assetinfo <file> [--engine-lib libunity.so] [--dump <pathId>|all]
// <file> is a serialized file (e.g. globalgamemanagers) or a content bundle. With
// --dump, objects are decoded through their embedded type descriptions; the engine
// library from your copy of the game supplies the shared field-name table.
#include "rr/assets/bundle_file.h"
#include "rr/assets/serialized_file.h"
#include "rr/assets/type_tree_value.h"

#include <optional>
#include <string>

#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>

namespace {

struct DumpRequest {
    std::optional<rr::assets::CommonStrings> strings;
    std::string what; // "" = no dump, "all", or a path ID
};
DumpRequest g_dump;

std::vector<std::uint8_t> ReadFile(const char* path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}

void DumpObjects(const rr::assets::SerializedFile& file) {
    if (g_dump.what.empty()) return;
    if (!file.HasTypeTrees()) {
        std::printf("  (no type descriptions in this file; cannot decode objects)\n");
        return;
    }
    const rr::assets::CommonStrings empty;
    const auto& strings = g_dump.strings ? *g_dump.strings : empty;
    for (const auto& object : file.Objects()) {
        if (g_dump.what != "all" && std::to_string(object.pathId) != g_dump.what) continue;
        std::printf("  object %lld (class %d):\n", static_cast<long long>(object.pathId), object.classId);
        try {
            std::printf("    %s\n", rr::assets::ReadTypeTreeObject(file, object, strings)->Dump(2).c_str());
        } catch (const rr::assets::ReadError& e) {
            std::printf("    decode error: %s\n", e.what());
        }
    }
}

void PrintSerializedFile(const rr::assets::SerializedFile& file) {
    std::printf("  format %u, engine %s, platform %d, type trees: %s\n", file.FormatVersion(),
                file.EngineVersion().c_str(), file.TargetPlatform(),
                file.HasTypeTrees() ? "yes" : "no");
    std::printf("  %zu types, %zu objects, %zu externals\n", file.Types().size(),
                file.Objects().size(), file.Externals().size());
    std::map<std::int32_t, std::size_t> perClass;
    for (const auto& o : file.Objects()) ++perClass[o.classId];
    for (const auto& [id, count] : perClass) {
        std::printf("    %6zu x class %d (%s)\n", count, id, rr::assets::ClassName(id));
    }
    for (const auto& e : file.Externals()) std::printf("    external: %s\n", e.path.c_str());
    DumpObjects(file);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: assetinfo <file> [--engine-lib libunity.so] [--dump <pathId>|all]\n");
        return 2;
    }
    for (int i = 2; i + 1 < argc; i += 2) {
        const std::string option = argv[i];
        if (option == "--engine-lib") {
            g_dump.strings = rr::assets::CommonStrings::FromEngineLibrary(ReadFile(argv[i + 1]));
            if (!g_dump.strings) std::fprintf(stderr, "warning: no name table found in %s\n", argv[i + 1]);
        } else if (option == "--dump") {
            g_dump.what = argv[i + 1];
        }
    }
    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }
    std::vector<std::uint8_t> bytes(std::istreambuf_iterator<char>(in), {});
    try {
        if (rr::assets::BundleFile::LooksLikeBundle(bytes)) {
            rr::assets::BundleFile bundle(bytes);
            std::printf("bundle format %u, engine %s, %zu entries\n", bundle.FormatVersion(),
                        bundle.EngineVersion().c_str(), bundle.Entries().size());
            for (const auto& entry : bundle.Entries()) {
                std::printf("entry %s (%llu bytes)%s\n", entry.path.c_str(),
                            static_cast<unsigned long long>(entry.size),
                            entry.IsSerializedFile() ? " [serialized file]" : "");
                if (entry.IsSerializedFile()) {
                    PrintSerializedFile(rr::assets::SerializedFile(bundle.CopyEntry(entry)));
                }
            }
        } else {
            PrintSerializedFile(rr::assets::SerializedFile(std::move(bytes)));
        }
    } catch (const rr::assets::ReadError& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
    return 0;
}
