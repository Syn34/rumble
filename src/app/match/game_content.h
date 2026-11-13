// game_content.h - opens the user's game files for a match (PC glue): the content bundles
// (ContentLibrary with the engine's field-name table from libunity.so), the simulation
// assets (QuantumContentProvider and the asset factories) behind a resource manager, the
// player's Resources and FPMath's lookup tables.
#pragma once

#include "math_tables.h"

#include "content/quantum_content_provider.h"
#include "quantum_core/asset_resource_provider.h"
#include "rr/assets/content_library.h"
#include "rr/assets/player_resources.h"

#include "photon_deterministic/system_allocator.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rr::app {

struct ContentOptions {
    std::filesystem::path contentDir;            // asset pack aa/Android folder (*.bundle)
    std::filesystem::path engineLib;             // libunity.so
    std::vector<std::filesystem::path> dataDirs; // player data folders (globalgamemanagers...)
    std::filesystem::path metadata;              // global-metadata.dat (default: <data>/Managed/Metadata/)
    std::filesystem::path lutDir;                // optional exported LUT/*.bytes
    bool allowApproximateMath = false;
};

class GameContent {
public:
    // Null (and `error` set) when the bundles or the engine library can't be opened.
    static std::unique_ptr<GameContent> Open(const ContentOptions& options, std::string& error);
    ~GameContent();

    assets::ContentLibrary& Library() { return *library_; }
    content::QuantumContentProvider& Provider() { return *provider_; }
    Quantum::IResourceManager& ResourceManager() { return *assets_.ResourceManager; }
    assets::PlayerResources* Resources() { return resources_.get(); }
    const MathTablesReport& MathTables() const { return mathTables_; }
    // Assets the PC build had to make up because the staged files don't have them.
    const std::vector<std::string>& Notes() const { return notes_; }

    // Every listed asset of one original script class (e.g. "MapAsset").
    std::vector<Quantum::AssetObjectIdentifier> AssetsOfClass(const std::string& scriptClass);
    // An asset of `scriptClass` by GUID (decimal) or by path / file name (case-insensitive;
    // a unique substring of the last path component is enough).
    std::optional<Quantum::AssetObjectIdentifier> Find(const std::string& scriptClass, const std::string& nameOrGuid);
    // Loads an asset through the resource manager (null if it isn't there).
    std::shared_ptr<Quantum::AssetObject> Load(Quantum::AssetGuid guid);

private:
    GameContent() = default;

    std::unique_ptr<assets::ContentLibrary> library_;
    std::unique_ptr<content::QuantumContentProvider> provider_;
    Quantum::InMemoryAssetProvider synthesized_;
    std::unique_ptr<Quantum::IAssetResourceProvider> combined_;
    Photon::Deterministic::SystemAllocator assetAllocator_;
    Quantum::ProviderAssetSystem assets_;
    std::unique_ptr<assets::PlayerResources> resources_;
    MathTablesReport mathTables_;
    std::vector<std::string> notes_;
    std::vector<Quantum::AssetObjectIdentifier> listed_;
    std::map<std::string, std::string> classOf_; // guid -> script class (cache)
};

} // namespace rr::app
