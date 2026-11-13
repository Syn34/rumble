#include "game_content.h"

#include "content/quantum_code_factories.h"
#include "quantum_code/simulation_config.h"
#include "rr/assets/common_strings.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>

namespace rr::app {

namespace {

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Serves the bundles' assets and, after them, the ones the PC build made up.
class CombinedProvider final : public Quantum::IAssetResourceProvider {
public:
    CombinedProvider(Quantum::IAssetResourceProvider& first, Quantum::IAssetResourceProvider& second)
        : first_(first), second_(second) {}
    std::vector<Quantum::AssetObjectIdentifier> ListAssets() override {
        auto all = first_.ListAssets();
        for (const auto& id : second_.ListAssets()) {
            if (std::none_of(all.begin(), all.end(), [&](const auto& a) { return a.Guid == id.Guid; })) all.push_back(id);
        }
        firstGuids_.clear();
        for (const auto& id : first_.ListAssets()) firstGuids_.push_back(id.Guid.Value);
        std::sort(firstGuids_.begin(), firstGuids_.end());
        return all;
    }
    std::shared_ptr<Quantum::AssetObject> LoadAsset(const Quantum::AssetObjectIdentifier& id) override {
        if (std::binary_search(firstGuids_.begin(), firstGuids_.end(), id.Guid.Value)) return first_.LoadAsset(id);
        return second_.LoadAsset(id);
    }

private:
    Quantum::IAssetResourceProvider& first_;
    Quantum::IAssetResourceProvider& second_;
    std::vector<std::int64_t> firstGuids_;
};

} // namespace

// Members go in reverse order: the resource manager (which pulls from the providers) first.
GameContent::~GameContent() = default;

std::unique_ptr<GameContent> GameContent::Open(const ContentOptions& options, std::string& error) {
    std::unique_ptr<GameContent> content(new GameContent());
    std::error_code ec;
    if (options.contentDir.empty() || !std::filesystem::is_directory(options.contentDir, ec)) {
        error = "content folder not found (--content <asset pack aa/Android folder>): " + options.contentDir.string();
        return nullptr;
    }
    std::ifstream lib(options.engineLib, std::ios::binary);
    if (!lib) {
        error = "engine library not found (--engine-lib <libunity.so>): " + options.engineLib.string();
        return nullptr;
    }
    const std::vector<std::uint8_t> libBytes((std::istreambuf_iterator<char>(lib)), std::istreambuf_iterator<char>());
    auto strings = assets::CommonStrings::FromEngineLibrary(libBytes);
    if (!strings) {
        error = "no field-name table found in " + options.engineLib.string();
        return nullptr;
    }
    content->library_ = std::make_unique<assets::ContentLibrary>(options.contentDir, std::move(*strings));
    content::RegisterQuantumCodeAssetFactories();
    content->provider_ = std::make_unique<content::QuantumContentProvider>(*content->library_);
    content->listed_ = content->provider_->ListAssets();

    // The simulation config lives in the player's Resources (db/configs), which the PC
    // build can't decode yet: without one, use the class defaults.
    if (content->AssetsOfClass("SimulationConfigAsset").empty()) {
        auto config = std::make_shared<Quantum::SimulationConfig>();
        Quantum::AssetObjectIdentifier id;
        id.Guid = Quantum::AssetGuid(Quantum::SimulationConfig::DEFAULT_ID);
        id.Path = "PC/DefaultSimulationConfig";
        config->SetIdentifier(id);
        content->synthesized_.Add(config);
        content->notes_.push_back("SimulationConfig: not in the bundles (Resources db/configs/simulationconfig); "
                                  "using the class defaults");
    }

    content->combined_ = std::make_unique<CombinedProvider>(*content->provider_, content->synthesized_);
    content->assets_ = Quantum::CreateResourceManager(*content->combined_, &content->assetAllocator_);

    // Player data folders: Resources and the metadata.
    std::vector<std::filesystem::path> dataDirs = options.dataDirs;
    content->resources_ = assets::PlayerResources::Open(dataDirs);
    std::filesystem::path metadata = options.metadata;
    if (metadata.empty()) {
        for (const auto& dir : dataDirs) {
            const auto candidate = dir / "Managed" / "Metadata" / "global-metadata.dat";
            if (std::filesystem::is_regular_file(candidate, ec)) {
                metadata = candidate;
                break;
            }
        }
    }
    content->mathTables_ = LoadMathTables(content->resources_.get(), metadata, options.lutDir, options.allowApproximateMath);
    return content;
}

std::vector<Quantum::AssetObjectIdentifier> GameContent::AssetsOfClass(const std::string& scriptClass) {
    std::vector<Quantum::AssetObjectIdentifier> out;
    for (const auto& id : listed_) {
        const std::string key = std::to_string(id.Guid.Value);
        auto it = classOf_.find(key);
        if (it == classOf_.end()) it = classOf_.emplace(key, provider_->ScriptClassOf(id)).first;
        if (it->second == scriptClass) out.push_back(id);
    }
    return out;
}

std::optional<Quantum::AssetObjectIdentifier> GameContent::Find(const std::string& scriptClass,
                                                                const std::string& nameOrGuid) {
    const auto candidates = AssetsOfClass(scriptClass);
    const bool numeric = !nameOrGuid.empty() &&
                         std::all_of(nameOrGuid.begin() + (nameOrGuid[0] == '-' ? 1 : 0), nameOrGuid.end(),
                                     [](unsigned char c) { return std::isdigit(c) != 0; });
    if (numeric) {
        const long long guid = std::stoll(nameOrGuid);
        for (const auto& id : candidates) {
            if (id.Guid.Value == guid) return id;
        }
        return std::nullopt;
    }
    const std::string wanted = Lower(nameOrGuid);
    std::optional<Quantum::AssetObjectIdentifier> partial;
    int partialMatches = 0;
    for (const auto& id : candidates) {
        const std::string path = Lower(id.Path);
        const auto slash = path.find_last_of('/');
        const std::string leaf = slash == std::string::npos ? path : path.substr(slash + 1);
        if (path == wanted || leaf == wanted) return id;
        if (leaf.find(wanted) != std::string::npos) {
            partial = id;
            ++partialMatches;
        }
    }
    return partialMatches == 1 ? partial : std::nullopt;
}

std::shared_ptr<Quantum::AssetObject> GameContent::Load(Quantum::AssetGuid guid) {
    return assets_.ResourceManager->GetAsset(guid, true);
}

} // namespace rr::app
