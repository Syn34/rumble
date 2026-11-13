// quantum_content_provider.h - serves simulation assets from the game's bundles.
//
// In the original, each simulation asset (weapon specs, game modes, maps...) is
// stored as an engine script object whose `Settings` field holds the asset data,
// including its identifier (path + GUID). This provider indexes those objects in
// the user's bundles and builds the matching C++ asset on demand.
//
// C++ asset types register a factory under the original script class name. Assets
// whose type has no factory yet are served as UnmappedAsset, which keeps the
// decoded data so nothing is lost while the game's asset classes are reimplemented.
#pragma once

#include "quantum_core/asset_resource_provider.h"
#include "rr/assets/content_library.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace rr::content {

// An asset whose C++ type doesn't exist yet: identifier plus the raw decoded data.
class UnmappedAsset final : public Quantum::AssetObject {
public:
    std::string ScriptClass;
    rr::assets::ValuePtr Settings;
};

using AssetFactory =
    std::function<std::shared_ptr<Quantum::AssetObject>(const rr::assets::Value& settings)>;

// Registers the factory for one original script class name (e.g. "WeaponSpecAsset").
void RegisterAssetFactory(const std::string& scriptClass, AssetFactory factory);

// Builds an asset from its decoded `Settings` value with the factory registered for
// `scriptClass`; null if there is none. (The identifier is left to the caller.)
std::shared_ptr<Quantum::AssetObject> BuildAsset(const std::string& scriptClass,
                                                 const rr::assets::Value& settings);

class QuantumContentProvider final : public Quantum::IAssetResourceProvider {
public:
    // `library` must outlive the provider. Only bundles whose file name starts with
    // `bundlePrefix` are scanned for simulation assets.
    explicit QuantumContentProvider(rr::assets::ContentLibrary& library,
                                    std::string bundlePrefix = "quantum_");

    std::vector<Quantum::AssetObjectIdentifier> ListAssets() override;
    std::shared_ptr<Quantum::AssetObject> LoadAsset(const Quantum::AssetObjectIdentifier& id) override;

    // Script class of a listed asset (resolved on first request).
    std::string ScriptClassOf(const Quantum::AssetObjectIdentifier& id);

private:
    struct Entry {
        Quantum::AssetObjectIdentifier id;
        std::string cab;
        std::int64_t pathId = 0;
    };

    const Entry* Find(const Quantum::AssetObjectIdentifier& id) const;

    rr::assets::ContentLibrary& library_;
    std::vector<Entry> entries_;
    std::unordered_map<std::int64_t, std::size_t> byGuid_;
};

} // namespace rr::content
