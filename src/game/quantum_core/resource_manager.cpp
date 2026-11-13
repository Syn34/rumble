// Original: Quantum.AssetResource, Quantum.ResourceManagerBase, Quantum.ResourceManagerStatic,
// Quantum.ResourceManagerDynamic (quantum.core.dll).
#include "resource_manager.h"

#include "photon_deterministic/log.h"
#include "photon_deterministic/native.h"
#include "quantum_core/assert.h"
#include "quantum_core/asset_db.h"

#include <algorithm>
#include <exception>
#include <stdexcept>

namespace Quantum {

using Photon::Deterministic::Log;

// --- AssetResource ---------------------------------------------------------------------

bool AssetResource::TryTransition(int newLoadingState, bool allowReentry) {
    if (newLoadingState == Unloaded) {
        int expected = Disposing;
        return State.compare_exchange_strong(expected, Unloaded);
    }
    if (newLoadingState == Disposing) {
        int expected = Loaded;
        return State.compare_exchange_strong(expected, Disposing);
    }
    for (;;) {
        int current = State.load();
        if (current == Disposing) {
            // Someone wants the asset while it is about to be released: keep it.
            int expected = Disposing;
            State.compare_exchange_strong(expected, Loaded);
            Assert::Always(expected == Loaded || expected == Disposing);
            return false;
        }
        if (current >= newLoadingState) {
            return current == newLoadingState && allowReentry;
        }
        if (State.compare_exchange_strong(current, newLoadingState)) {
            return true;
        }
    }
}

// --- ResourceManagerBase ---------------------------------------------------------------

std::shared_ptr<AssetDB> ResourceManagerBase::CreateAssetDatabase() {
    return std::make_shared<AssetDB>(this);
}

void ResourceManagerBase::DiposeAssetDatabase(AssetDB* db) {
    if (db == nullptr) {
        throw std::invalid_argument("db");
    }
    db->_resourceManager = nullptr;
    db->_context = nullptr;
}

bool ResourceManagerBase::DisposeAsset(AssetGuid guid) {
    AssetResource* resource = nullptr;
    if (!TryGetAssetResource(guid, resource)) {
        return false;
    }
    DisposeAsset(*resource);
    return true;
}

bool ResourceManagerBase::TryGetAssetResource(AssetGuid guid, AssetResource*& resource) const {
    const auto found = _byGuid.find(guid);
    resource = found != _byGuid.end() ? found->second : nullptr;
    return resource != nullptr;
}

bool ResourceManagerBase::TryGetAssetResource(const std::string& path,
                                              AssetResource*& resource) const {
    const auto found = _byPath.find(path);
    resource = found != _byPath.end() ? found->second : nullptr;
    return resource != nullptr;
}

std::shared_ptr<AssetObject> ResourceManagerBase::GetAsset(AssetGuid guid, bool mainThread) {
    if (guid.IsDynamic()) {
        throw std::invalid_argument("dynamic asset " + guid.ToString(true) +
                                    " requested from a static resource manager");
    }
    AssetResource* resource = nullptr;
    if (!TryGetAssetResource(guid, resource)) {
        return nullptr;
    }
    return LoadResource(*resource, mainThread);
}

std::vector<std::shared_ptr<AssetObject>> ResourceManagerBase::LoadAllAssets(bool mainThread) {
    std::vector<std::shared_ptr<AssetObject>> assets;
    assets.reserve(_resources.size());
    for (const auto& resource : _resources) {
        assets.push_back(LoadResource(*resource, mainThread));
    }
    return assets;
}

AssetGuid ResourceManagerBase::GetAssetGuid(const std::string& path) const {
    AssetResource* resource = nullptr;
    return TryGetAssetResource(path, resource) ? resource->Guid : AssetGuid{};
}

void ResourceManagerBase::Init(std::vector<std::unique_ptr<AssetResource>> resources) {
    _resources.clear();
    _byGuid.clear();
    _byPath.clear();
    for (auto& resource : resources) {
        if (resource == nullptr) {
            throw std::invalid_argument("resource list contains a null entry");
        }
        const AssetObjectIdentifier identifier = resource->get_Identifier();
        if (!resource->Guid.IsValid()) {
            throw std::invalid_argument("resource without a valid GUID: " + identifier.ToString());
        }
        if (resource->Guid.IsDynamic()) {
            throw std::invalid_argument("resource with a dynamic GUID: " + identifier.ToString());
        }
        AssetResource* entry = resource.get();
        if (const auto [where, added] = _byGuid.emplace(entry->Guid, entry); !added) {
            throw std::invalid_argument("GUID registered twice: " + identifier.ToString() +
                                        ", first by " + where->second->get_Identifier().ToString());
        }
        if (entry->HasPath()) {
            if (const auto [where, added] = _byPath.emplace(entry->Path, entry); !added) {
                // The GUID entry made above is taken back before reporting.
                Assert::Always(_byGuid.erase(entry->Guid) == 1);
                throw std::invalid_argument("path registered twice: " + identifier.ToString() +
                                            ", first by " +
                                            where->second->get_Identifier().ToString());
            }
        }
        _resources.push_back(std::move(resource));
    }
}

// --- ResourceManagerStatic -------------------------------------------------------------

ResourceManagerStatic::ResourceManagerStatic(
    const std::vector<std::shared_ptr<AssetObject>>& assets, Allocator* allocator)
    : _allocator(allocator) {
    std::vector<std::unique_ptr<AssetResource>> resources;
    resources.reserve(assets.size());
    for (const auto& asset : assets) {
        if (asset == nullptr) {
            throw std::invalid_argument("asset list contains a null entry");
        }
        resources.push_back(std::make_unique<AssetResource>(asset->Guid, asset->Path, asset));
    }
    Init(std::move(resources));
}

void ResourceManagerStatic::Dispose() {
    for (const auto& resource : Resources()) {
        if (resource->State.load() != AssetResource::Loaded) {
            continue;
        }
        resource->Asset->Disposed(this, _allocator);
        resource->State.exchange(AssetResource::Unloaded);
    }
    if (_allocator != nullptr) {
        _allocator->Dispose();
    }
}

void ResourceManagerStatic::DisposeAsset(AssetResource& /*resource*/) {}

std::shared_ptr<AssetObject> ResourceManagerStatic::LoadResource(AssetResource& resource,
                                                                 bool /*mainThread*/) {
    while (resource.State.load() != AssetResource::Loaded) {
        if (resource.TryTransition(AssetResource::Loading, false)) {
            // As in the original the resource is flagged loaded before the hook runs.
            resource.State.exchange(AssetResource::Loaded);
            resource.Asset->Loaded(this, _allocator);
        } else {
            std::this_thread::yield();
        }
    }
    return resource.Asset;
}

// --- ResourceManagerDynamic ------------------------------------------------------------

ResourceManagerDynamic::ResourceManagerDynamic() : _creatorThread(std::this_thread::get_id()) {}

ResourceManagerDynamic::~ResourceManagerDynamic() {
    if (_loader != nullptr) {
        _loader->RemoveListener(this);
    }
}

void ResourceManagerDynamic::Init(std::vector<std::unique_ptr<AssetResource>> resources,
                                  IResourceLoader* loader, Allocator* allocator) {
    (void)_resourcesToLoad.Pop();
    {
        std::lock_guard lock(_resourcesToDisposeMutex);
        _resourcesToDispose.clear();
    }
    _allocator = allocator;
    _loader = loader;
    if (_loader != nullptr) {
        _loader->AddListener(this);
    }
    ResourceManagerBase::Init(std::move(resources));
    Log::Info("Resource manager ready, {0} resources registered", Count());
}

std::shared_ptr<AssetDB> ResourceManagerDynamic::CreateAssetDatabase() {
    auto db = std::make_shared<AssetDB>(this);
    std::lock_guard lock(_assetDBsMutex);
    if (std::find(_assetDBs.begin(), _assetDBs.end(), db.get()) == _assetDBs.end()) {
        _assetDBs.push_back(db.get());
    }
    return db;
}

void ResourceManagerDynamic::DiposeAssetDatabase(AssetDB* db) {
    if (db == nullptr) {
        throw std::invalid_argument("db");
    }
    db->_resourceManager = nullptr;
    db->_context = nullptr;
    std::lock_guard lock(_assetDBsMutex);
    _assetDBs.erase(std::remove(_assetDBs.begin(), _assetDBs.end(), db), _assetDBs.end());
}

void ResourceManagerDynamic::Dispose() {
    if (_loader != nullptr) {
        _loader->RemoveListener(this);
    }
    for (const auto& resource : Resources()) {
        if (resource->Asset != nullptr || resource->State.load() >= AssetResource::Queued) {
            DisposeAssetNoChecks(*resource);
        }
    }
    if (_allocator != nullptr) {
        _allocator->Dispose();
    }
}

void ResourceManagerDynamic::MainThreadUpdate(Profiling::Profiler* /*profiler*/) {
    if (_resourcesToLoad.Empty()) {
        return;
    }
    for (const ResourceLoadingRequest& request : _resourcesToLoad.Pop()) {
        AssetResource& resource = *request.Resource;
        if (request.Async) {
            if (resource.TryTransition(AssetResource::LoadingAsync, false)) {
                _loader->LoadResourceAsync(resource);
            }
        } else if (resource.TryTransition(AssetResource::Loading, true)) {
            _loader->LoadResource(resource);
        }
    }
}

void ResourceManagerDynamic::MainThreadSimulationEnded() {
    for (;;) {
        AssetResource* resource = nullptr;
        {
            std::lock_guard lock(_resourcesToDisposeMutex);
            if (_resourcesToDispose.empty()) {
                return;
            }
            resource = _resourcesToDispose.front();
            _resourcesToDispose.pop_front();
        }
        // Only if nobody asked for the asset again in the meantime (see TryTransition).
        int expected = AssetResource::Disposing;
        if (resource->State.compare_exchange_strong(expected, AssetResource::Unloaded)) {
            DisposeAssetNoChecks(*resource);
        }
    }
}

void ResourceManagerDynamic::AssetLoaded(IResourceLoader& /*loader*/, AssetResource& resource,
                                         std::shared_ptr<AssetObject> asset) {
    Assert::Always(asset != nullptr);
    resource.State.exchange(AssetResource::LoadedCallback);
    resource.Asset = asset;
    try {
        asset->Loaded(this, _allocator);
    } catch (const std::exception& error) {
        Log::Error("Loaded() of asset {0} threw: {1}", asset->Identifier().ToString(),
                   error.what());
    }
    resource.State.exchange(AssetResource::Loaded);
}

void ResourceManagerDynamic::AssetLoadFailed(IResourceLoader& /*loader*/, AssetResource& resource,
                                             const std::string& error) {
    Log::Error("Asset {0} could not be loaded: {1}", resource.Guid.ToString(true), error);
}

void ResourceManagerDynamic::DisposeAsset(AssetResource& resource) {
    int expected = AssetResource::Loaded;
    if (resource.State.compare_exchange_strong(expected, AssetResource::Disposing)) {
        std::lock_guard lock(_resourcesToDisposeMutex);
        _resourcesToDispose.push_back(&resource);
    }
}

std::shared_ptr<AssetObject> ResourceManagerDynamic::LoadResource(AssetResource& resource,
                                                                  bool mainThread) {
    if (mainThread) {
        if (resource.TryTransition(AssetResource::Loading, true)) {
            _loader->LoadResource(resource);
        }
        return resource.Asset;
    }
    if (resource.TryTransition(AssetResource::Loading, false)) {
        _resourcesToLoad.Push(ResourceLoadingRequest{false, &resource});
    } else if (resource.State.load() > AssetResource::Loaded) {
        return resource.Asset;
    }
    // (The original brackets the wait with a HostProfiler debug sample.)
    while (resource.State.load() < AssetResource::Loaded) {
        std::this_thread::yield();
    }
    return resource.Asset;
}

void ResourceManagerDynamic::DisposeAssetNoChecks(AssetResource& resource) {
    if (resource.Asset != nullptr) {
        resource.Asset->Disposed(this, _allocator);
    }
    resource.Asset = nullptr;
    resource.State.exchange(AssetResource::Unloaded);
    _loader->DisposeResource(resource);

    std::vector<AssetDB*> databases;
    {
        std::lock_guard lock(_assetDBsMutex);
        databases = _assetDBs;
    }
    for (AssetDB* db : databases) {
        db->DisposeAsset(resource.Guid, false);
    }
}

void ResourceManagerDynamic::ResourceLoadingQueue::Push(ResourceLoadingRequest request) {
    std::lock_guard lock(_mutex);
    _requests.push_back(request);
}

std::vector<ResourceManagerDynamic::ResourceLoadingRequest>
ResourceManagerDynamic::ResourceLoadingQueue::Pop() {
    std::lock_guard lock(_mutex);
    std::vector<ResourceLoadingRequest> taken;
    taken.swap(_requests);
    return taken;
}

bool ResourceManagerDynamic::ResourceLoadingQueue::Empty() const {
    std::lock_guard lock(_mutex);
    return _requests.empty();
}

} // namespace Quantum
