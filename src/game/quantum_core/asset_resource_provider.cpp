// The PC asset-loading seam (new code, see the header).
#include "asset_resource_provider.h"

#include <algorithm>
#include <exception>
#include <stdexcept>

namespace Quantum {

void ProviderResourceLoader::AddListener(IResourceLoadListener* listener) {
    std::lock_guard lock(_mutex);
    if (std::find(_listeners.begin(), _listeners.end(), listener) == _listeners.end()) {
        _listeners.push_back(listener);
    }
}

void ProviderResourceLoader::RemoveListener(IResourceLoadListener* listener) {
    std::lock_guard lock(_mutex);
    _listeners.erase(std::remove(_listeners.begin(), _listeners.end(), listener),
                     _listeners.end());
}

std::vector<IResourceLoadListener*> ProviderResourceLoader::Listeners() {
    std::lock_guard lock(_mutex);
    return _listeners;
}

void ProviderResourceLoader::LoadResource(AssetResource& resource) {
    std::shared_ptr<AssetObject> asset;
    std::string error;
    try {
        asset = _provider->LoadAsset(resource.get_Identifier());
        if (asset == nullptr) {
            error = "the asset provider has no asset for " + resource.get_Identifier().ToString();
        }
    } catch (const std::exception& exception) {
        error = exception.what();
    }
    for (IResourceLoadListener* listener : Listeners()) {
        if (asset != nullptr) {
            listener->AssetLoaded(*this, resource, asset);
        } else {
            listener->AssetLoadFailed(*this, resource, error);
        }
    }
}

void ProviderResourceLoader::DisposeResource(AssetResource& resource) {
    _provider->ReleaseAsset(resource.get_Identifier());
}

void InMemoryAssetProvider::Add(std::shared_ptr<AssetObject> asset) {
    if (asset == nullptr) {
        throw std::invalid_argument("asset");
    }
    if (const auto found = _indexByGuid.find(asset->Guid); found != _indexByGuid.end()) {
        _assets[found->second] = std::move(asset);
        return;
    }
    _indexByGuid.emplace(asset->Guid, _assets.size());
    _assets.push_back(std::move(asset));
}

std::vector<AssetObjectIdentifier> InMemoryAssetProvider::ListAssets() {
    std::vector<AssetObjectIdentifier> identifiers;
    identifiers.reserve(_assets.size());
    for (const auto& asset : _assets) {
        identifiers.push_back(asset->Identifier());
    }
    return identifiers;
}

std::shared_ptr<AssetObject>
InMemoryAssetProvider::LoadAsset(const AssetObjectIdentifier& identifier) {
    const auto found = _indexByGuid.find(identifier.Guid);
    return found != _indexByGuid.end() ? _assets[found->second] : nullptr;
}

ProviderAssetSystem CreateResourceManager(IAssetResourceProvider& provider,
                                          Photon::Deterministic::Native::Allocator* allocator) {
    std::vector<std::unique_ptr<AssetResource>> resources;
    for (const AssetObjectIdentifier& identifier : provider.ListAssets()) {
        resources.push_back(std::make_unique<AssetResource>(identifier.Guid, identifier.Path));
    }
    ProviderAssetSystem system;
    system.Loader = std::make_unique<ProviderResourceLoader>(&provider);
    system.ResourceManager = std::make_unique<ResourceManagerDynamic>();
    system.ResourceManager->Init(std::move(resources), system.Loader.get(), allocator);
    return system;
}

} // namespace Quantum
