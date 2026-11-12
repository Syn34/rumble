// assetinfo - prints what a game content file contains (for development).
//   assetinfo <file>   e.g. gamedata/apk/assets/bin/Data/globalgamemanagers
#include "rr/assets/serialized_file.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: assetinfo <serialized file>\n");
        return 2;
    }
    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }
    std::vector<std::uint8_t> bytes(std::istreambuf_iterator<char>(in), {});
    try {
        rr::assets::SerializedFile file(std::move(bytes));
        std::printf("format %u, engine %s, platform %d, type trees: %s\n", file.FormatVersion(),
                    file.EngineVersion().c_str(), file.TargetPlatform(),
                    file.HasTypeTrees() ? "yes" : "no");
        std::printf("%zu types, %zu objects, %zu externals\n", file.Types().size(),
                    file.Objects().size(), file.Externals().size());
        std::map<std::int32_t, std::size_t> perClass;
        for (const auto& o : file.Objects()) ++perClass[o.classId];
        for (const auto& [id, count] : perClass) {
            std::printf("  %6zu x class %d (%s)\n", count, id, rr::assets::ClassName(id));
        }
        for (const auto& e : file.Externals()) std::printf("  external: %s\n", e.path.c_str());
    } catch (const rr::assets::ReadError& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
    return 0;
}
