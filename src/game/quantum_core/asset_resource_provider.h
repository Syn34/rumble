// asset_resource_provider.h - how the PC build feeds assets into quantum.core (new code; no
// original counterpart).
//
// On Android the Unity side builds a ResourceManagerDynamic from the game's asset resource
// container (one AssetResource per asset: GUID and path) and a Unity-specific
// IResourceLoader that deserialises the asset objects on demand. The PC build replaces
// that loader with this seam:
//
//  1. The asset pipeline (engine side, reading the user's own game files) implements
//     IAssetResourceProvider: ListAssets() returns the asset table, LoadAsset() builds the
//     AssetObject for one entry when the simulation first needs it (its Loaded() hook is
//     run afterwards by the resource manager), ReleaseAsset() is called when the manager
//     disposes it again.
//  2. ProviderResourceLoader adapts a provider to IResourceLoader (synchronously: loads
//     requested from worker threads are still routed through the main thread by
//     ResourceManagerDynamic.MainThreadUpdate, as in the original).
//  3. CreateResourceManager() wires both into a ResourceManagerDynamic. Its
//     CreateAssetDatabase() result goes into FrameContext::Args::AssetDatabase.
//
// InMemoryAssetProvider serves assets that were built in code (tests, and content the PC
// build synthesises itself).
#pragma once

#include "quantum_core/asset_object.h"
#include "quantum_core/resource_manager.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace Quantum {

class IAssetResourceProvider {
  public:
    virtual ~IAssetResourceProvider() = default;

    // Every asset the provider can load: GUID (valid, not dynamic, unique) and path
    // (unique when set).
    virtual std::vector<AssetObjectIdentifier> ListAssets() = 0;
    // The asset for one listed entry, with that entry's GUID and path. Returning null or
    // throwing reports a load failure.
    virtual std::shared_ptr<AssetObject> LoadAsset(const AssetObjectIdentifier& identifier) = 0;
    // The simulation released the asset (the provider may drop cached data).
    virtual void ReleaseAsset(const AssetObjectIdentifier& /*identifier*/) {}
};

class ProviderResourceLoader final : public IResourceLoader {
  public:
    // `provider` is not owned.
    explicit ProviderResourceLoader(IAssetResourceProvider* provider) : _provider(provider) {}

    void AddListener(IResourceLoadListener* listener) override;
    void RemoveListener(IResourceLoadListener* listener) override;
    // Same as LoadResource: the provider has no asynchronous path.
    void LoadResourceAsync(AssetResource& resource) override { LoadResource(resource); }
    void LoadResource(AssetResource& resource) override;
    void DisposeResource(AssetResource& resource) override;

  private:
    std::vector<IResourceLoadListener*> Listeners();

    IAssetResourceProvider* _provider = nullptr;
    std::mutex _mutex;
    std::vector<IResourceLoadListener*> _listeners;
};

// Assets constructed in code, served by GUID.
class InMemoryAssetProvider final : public IAssetResourceProvider {
  public:
    // Registers an asset (by its Guid and Path). Registering a GUID again replaces it.
    void Add(std::shared_ptr<AssetObject> asset);

    std::vector<AssetObjectIdentifier> ListAssets() override;
    std::shared_ptr<AssetObject> LoadAsset(const AssetObjectIdentifier& identifier) override;

  private:
    std::vector<std::shared_ptr<AssetObject>> _assets;
    std::unordered_map<AssetGuid, std::size_t, AssetGuidHash> _indexByGuid;
};

// A resource manager and the loader it pulls from. The loader outlives the manager.
struct ProviderAssetSystem {
    std::unique_ptr<ProviderResourceLoader> Loader;
    std::unique_ptr<ResourceManagerDynamic> ResourceManager;

    ProviderAssetSystem() = default;
    ProviderAssetSystem(ProviderAssetSystem&&) = default;
    ProviderAssetSystem& operator=(ProviderAssetSystem&&) = default;
    ~ProviderAssetSystem() {
        if (ResourceManager) {
            ResourceManager->Dispose();
        }
        ResourceManager.reset();
    }
};

// Builds a ResourceManagerDynamic over everything `provider` lists (not owned; must
// outlive the result). `allocator` is handed to the assets' Loaded/Disposed hooks.
ProviderAssetSystem CreateResourceManager(IAssetResourceProvider& provider,
                                          Photon::Deterministic::Native::Allocator* allocator);

} // namespace Quantum
