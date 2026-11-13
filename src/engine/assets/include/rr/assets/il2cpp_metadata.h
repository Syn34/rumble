// il2cpp_metadata.h - a small reader for the game's global-metadata.dat (engine layer, new
// code).
//
// The original compiled some constant tables into its code as array initialisers; IL2CPP
// keeps their bytes in global-metadata.dat as the default values of fields of the
// assembly's <PrivateImplementationDetails> class. The PC build reads those bytes from the
// user's copy at runtime (FPMath's small lookup tables, for example).
//
// Supports metadata version 39 (Unity 6000.3), the version the game ships with.
#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace rr::assets {

class Il2CppMetadata {
public:
    // Throws ReadError for files that aren't v39 metadata.
    explicit Il2CppMetadata(std::vector<std::uint8_t> bytes);
    static std::optional<Il2CppMetadata> Load(const std::filesystem::path& file);

    struct StaticData {
        std::string fieldName;
        std::uint32_t dataOffset = 0;            // into the default-value data blob
        std::span<const std::uint8_t> available; // from the data to the end of the blob
    };

    // The array-initialiser data of every <PrivateImplementationDetails> field of the image
    // (e.g. "PhotonDeterministic.dll"), in data order. The blob doesn't store sizes, so
    // `available` runs to the end of the blob; callers take what they need and check it.
    std::vector<StaticData> PrivateImplementationData(const std::string& imageName) const;

private:
    struct Section {
        std::uint32_t offset = 0, size = 0, count = 0;
        std::uint32_t Stride() const { return count ? size / count : 0; }
    };
    std::string String(std::uint32_t index) const;
    std::uint32_t Index(std::size_t pos, int width) const;

    std::vector<std::uint8_t> bytes_;
    Section strings_, fieldDefaultValues_, defaultValueData_, fields_, typeDefinitions_, images_;
    int typeIndexWidth_ = 4, typeDefIndexWidth_ = 4, genericContainerWidth_ = 4;
};

} // namespace rr::assets
