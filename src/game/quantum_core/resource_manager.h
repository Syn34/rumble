// resource_manager.h - where simulation assets come from (quantum.core.dll):
// Quantum.AssetResource, Quantum.IResourceManager, Quantum.ResourceManagerBase,
// Quantum.ResourceManagerStatic, Quantum.IResourceLoader and Quantum.ResourceManagerDynamic.
//
// A resource manager knows every static asset of the game by GUID (and path) through an
// AssetResource record each, and loads the asset behind a record on first use:
//
//  * ResourceManagerStatic is built from asset objects that already exist in memory;
//    "loading" only runs the asset's Loaded() hook.
//  * ResourceManagerDynamic gets bare records (GUID + path) and an IResourceLoader that
//    produces the asset objects. Loads requested on the main thread run at once; loads
//    requested from a simulation worker thread are queued for the main thread
//    (MainThreadUpdate) while the worker waits. Disposal is deferred the same way: assets
//    are released in MainThreadSimulationEnded, and every asset database created by the
//    manager drops its cached reference.
//
// Each simulation context owns an AssetDB (asset_db.h) created by its resource manager;
// assets created at runtime live in the frame's DynamicAssetDB instead.
//
// The PC asset pipeline plugs in through IResourceLoader; see asset_resource_provider.h.
#pragma once

#include "quantum_core/asset_guid.h"
#include "quantum_core/asset_object.h"

#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


namespace Quantum::Profiling {
class Profiler;
}

namespace Quantum {

class AssetDB;

// Hash of a GUID for unordered containers (same value as AssetGuid.GetHashCode).
struct AssetGuidHash {
    std::size_t operator()(AssetGuid guid) const {
        return static_cast<std::size_t>(static_cast<std::uint32_t>(guid.GetHashCode()));
    }
};

// One static asset known to a resource manager: its identity, the loaded object (null
// while unloaded) and the loading state.
class AssetResource {
  public:
    // Values of State. The original uses the bare numbers; names describe their use.
    static constexpr int Unloaded = 0;
    // Not set by quantum.core itself; counted as "has work pending" by
    // ResourceManagerDynamic.Dispose. (Name is a guess.)
    static constexpr int Queued = 1;
    static constexpr int LoadingAsync = 2;
    static constexpr int Loading = 3;
    // The loader delivered the object; its Loaded() hook is running.
    static constexpr int LoadedCallback = 4;
    static constexpr int Loaded = 5;
    // Disposal requested; the asset is released on the main thread.
    static constexpr int Disposing = 6;

    AssetGuid Guid;
    std::string Path;
    std::shared_ptr<AssetObject> Asset;
    std::atomic<int> State{Unloaded};

    // 0x7ce66d0
    AssetResource() = default;
    AssetResource(AssetGuid guid, std::string path, std::shared_ptr<AssetObject> asset = nullptr)
        : Guid(guid), Path(std::move(path)), Asset(std::move(asset)) {}
    AssetResource(const AssetResource&) = delete;
    AssetResource& operator=(const AssetResource&) = delete;

    // 0x7ce6580
    AssetObjectIdentifier get_Identifier() const { return AssetObjectIdentifier{Path, Guid}; }
    // 0x7ce65a8
    bool IsLoaded() const { return State.load() == Loaded; }
    // 0x7ce65c8
    bool HasPath() const { return !Path.empty(); }

    // 0x7ce65e8 - moves State forward to `newLoadingState`:
    //  * Unloaded only from Disposing, Disposing only from Loaded;
    //  * any other state only upwards; staying in the same state succeeds only with
    //    `allowReentry`;
    //  * a resource that is being disposed is put back to Loaded (the pending disposal is
    //    cancelled) and the transition fails.
    bool TryTransition(int newLoadingState, bool allowReentry);
};

// Original: Quantum.IResourceManager (+ System.IDisposable).
class IResourceManager {
  public:
    virtual ~IResourceManager() = default;

    virtual std::shared_ptr<AssetDB> CreateAssetDatabase() = 0;
    // (Spelling as in the original.)
    virtual void DiposeAssetDatabase(AssetDB* db) = 0;
    virtual bool DisposeAsset(AssetGuid guid) = 0;
    virtual std::shared_ptr<AssetObject> GetAsset(AssetGuid guid, bool mainThread) = 0;
    // The original returns a lazy sequence; here every asset is loaded up front, in the
    // order the sequence would have produced them.
    virtual std::vector<std::shared_ptr<AssetObject>> LoadAllAssets(bool mainThread) = 0;
    virtual void MainThreadSimulationEnded() = 0;
    virtual void MainThreadUpdate(Profiling::Profiler* profiler) = 0;
    // IDisposable.Dispose
    virtual void Dispose() = 0;
};

// Lookup tables and the shared IResourceManager behaviour of the static and dynamic
// managers.
class ResourceManagerBase : public IResourceManager {
  public:
    // 0x7ce81a4 - empty tables.
    ResourceManagerBase() = default;
    ~ResourceManagerBase() override = default;
    ResourceManagerBase(const ResourceManagerBase&) = delete;
    ResourceManagerBase& operator=(const ResourceManagerBase&) = delete;

    // 0x7ce76d8 - a database that loads through this manager.
    std::shared_ptr<AssetDB> CreateAssetDatabase() override;
    // 0x7ce7730 - detaches the database from its manager and context.
    void DiposeAssetDatabase(AssetDB* db) override;
    // 0x7ce7764 - releases the asset behind `guid` (DisposeAsset(AssetResource)); false
    // for an unknown GUID.
    bool DisposeAsset(AssetGuid guid) override;
    // 0x7ce77f4
    bool TryGetAssetResource(AssetGuid guid, AssetResource*& resource) const;
    // 0x7ce785c
    bool TryGetAssetResource(const std::string& path, AssetResource*& resource) const;
    // 0x7ce78c4 - loads (LoadResource) and returns the asset; null for an unknown GUID.
    // Throws std::invalid_argument for a dynamic GUID.
    std::shared_ptr<AssetObject> GetAsset(AssetGuid guid, bool mainThread) override;
    // 0x7ce79a4 (and its iterator, <LoadAllAssets>d__11.MoveNext 0x7ce829c) - every
    // resource in registration order, loaded.
    std::vector<std::shared_ptr<AssetObject>> LoadAllAssets(bool mainThread) override;
    // 0x7ce7a6c - GUID registered for `path`, or an invalid GUID.
    AssetGuid GetAssetGuid(const std::string& path) const;
    // 0x7ce7a9c
    void MainThreadSimulationEnded() override {}
    // 0x7ce7aa0
    void MainThreadUpdate(Profiling::Profiler* /*profiler*/) override {}

    // Number of registered resources.
    int Count() const { return static_cast<int>(_resources.size()); }

    // Release / load the asset behind one record (the manager-specific part).
    virtual void DisposeAsset(AssetResource& resource) = 0;
    virtual std::shared_ptr<AssetObject> LoadResource(AssetResource& resource, bool mainThread) = 0;

  protected:
    // 0x7ce7aa4 - replaces the tables with `resources`. Throws std::invalid_argument for a
    // null record, an invalid or dynamic GUID, or a GUID or path registered twice.
    void Init(std::vector<std::unique_ptr<AssetResource>> resources);

    // Registration order (the order the original's dictionary enumerates in).
    const std::vector<std::unique_ptr<AssetResource>>& Resources() const { return _resources; }

  private:
    std::vector<std::unique_ptr<AssetResource>> _resources;
    std::unordered_map<AssetGuid, AssetResource*, AssetGuidHash> _byGuid;
    std::unordered_map<std::string, AssetResource*> _byPath;
};

// Assets that are already in memory. Original: Quantum.ResourceManagerStatic.
class ResourceManagerStatic final : public ResourceManagerBase {
  public:
    using Allocator = Photon::Deterministic::Native::Allocator;

    // 0x7ce85dc - one resource per asset (its GUID and path; <>c.<.ctor>b__1_0 0x7ce89b4).
    // `allocator` is handed to the assets' Loaded/Disposed hooks; not owned, but
    // Dispose() disposes it as the original does.
    ResourceManagerStatic(const std::vector<std::shared_ptr<AssetObject>>& assets,
                          Allocator* allocator);

    // 0x7ce8708 - runs Disposed() on every loaded asset, then disposes the allocator.
    void Dispose() override;

    using ResourceManagerBase::DisposeAsset;
    // 0x7ce88b4 - static assets stay loaded.
    void DisposeAsset(AssetResource& resource) override;
    // 0x7ce88b8 - the first caller marks the resource loaded and runs Loaded(); others wait
    // for it.
    std::shared_ptr<AssetObject> LoadResource(AssetResource& resource, bool mainThread) override;

  private:
    Allocator* _allocator = nullptr;
};

class IResourceLoader;

// Receiver of IResourceLoader's completion events: the original's ResourceLoaded
// (0x7ce8a40 ctor, 0x7ce8b4c Invoke) and ResourceLoadFailed (0x7ce8b60 ctor, 0x7ce8c6c
// Invoke) delegates.
class IResourceLoadListener {
  public:
    virtual ~IResourceLoadListener() = default;
    virtual void AssetLoaded(IResourceLoader& loader, AssetResource& resource,
                             std::shared_ptr<AssetObject> asset) = 0;
    virtual void AssetLoadFailed(IResourceLoader& loader, AssetResource& resource,
                                 const std::string& error) = 0;
};

// Produces the asset object for a resource record. Original: Quantum.IResourceLoader.
// A loader reports every load - synchronous or not - through its listeners' AssetLoaded
// or AssetLoadFailed.
class IResourceLoader {
  public:
    virtual ~IResourceLoader() = default;

    // add_LoadCompleted + add_LoadFailed / remove_LoadCompleted + remove_LoadFailed.
    virtual void AddListener(IResourceLoadListener* listener) = 0;
    virtual void RemoveListener(IResourceLoadListener* listener) = 0;

    virtual void LoadResourceAsync(AssetResource& resource) = 0;
    virtual void LoadResource(AssetResource& resource) = 0;
    virtual void DisposeResource(AssetResource& resource) = 0;
};

// Assets loaded on demand by an IResourceLoader. Original: Quantum.ResourceManagerDynamic.
class ResourceManagerDynamic final : public ResourceManagerBase, public IResourceLoadListener {
  public:
    using Allocator = Photon::Deterministic::Native::Allocator;

    // 0x7ce8c80 - remembers the creating thread.
    ResourceManagerDynamic();
    ~ResourceManagerDynamic() override;

    // 0x7ce8e04 - registers the resources, subscribes to the loader (not owned) and logs
    // how many resources there are. `allocator` as for ResourceManagerStatic.
    void Init(std::vector<std::unique_ptr<AssetResource>> resources, IResourceLoader* loader,
              Allocator* allocator);

    // 0x7ce9150 - also remembers the database so asset disposal can reach it.
    std::shared_ptr<AssetDB> CreateAssetDatabase() override;
    // 0x7ce91d4
    void DiposeAssetDatabase(AssetDB* db) override;
    // 0x7ce9250 - unsubscribes from the loader, releases every asset that is loaded or
    // being loaded, then disposes the allocator.
    void Dispose() override;
    // 0x7ce96b4 - runs the loads queued by other threads (in request order).
    void MainThreadUpdate(Profiling::Profiler* profiler) override;
    // 0x7ce98f0 - releases the assets whose disposal was requested.
    void MainThreadSimulationEnded() override;

    // 0x7ce9c3c - stores the object, runs its Loaded() hook (errors are logged) and marks
    // the resource loaded.
    void AssetLoaded(IResourceLoader& loader, AssetResource& resource,
                     std::shared_ptr<AssetObject> asset) override;
    // 0x7ce9ddc - logs the failure. The resource keeps its state (as in the original, a
    // thread waiting for it keeps waiting).
    void AssetLoadFailed(IResourceLoader& loader, AssetResource& resource,
                         const std::string& error) override;

    using ResourceManagerBase::DisposeAsset;
    // 0x7ce999c - a loaded asset is marked for disposal and queued for
    // MainThreadSimulationEnded.
    void DisposeAsset(AssetResource& resource) override;
    // 0x7ce9a20 - on the main thread: loads now through the loader. Elsewhere: queues the
    // load for MainThreadUpdate and waits until the asset is there.
    std::shared_ptr<AssetObject> LoadResource(AssetResource& resource, bool mainThread) override;

  private:
    // A load requested by a worker thread. Original: ResourceLoadingRequest (0x7ce9be4).
    struct ResourceLoadingRequest {
        bool Async = false;
        AssetResource* Resource = nullptr;
    };

    // Requests pushed by any thread, taken by the main thread.
    // Original: ResourceLoadingQueue (0x7ce8dfc ctor).
    class ResourceLoadingQueue {
      public:
        // 0x7ce9bec
        void Push(ResourceLoadingRequest request);
        // 0x7ce9864 - takes every pending request, oldest first.
        std::vector<ResourceLoadingRequest> Pop();
        bool Empty() const;

      private:
        mutable std::mutex _mutex;
        std::vector<ResourceLoadingRequest> _requests;
    };

    // 0x7ce9548 - Disposed() hook, forget the object, tell the loader, drop it from every
    // asset database.
    void DisposeAssetNoChecks(AssetResource& resource);

    std::thread::id _creatorThread;
    IResourceLoader* _loader = nullptr;
    // Quantum.SynchonizedSet<AssetDB>
    std::mutex _assetDBsMutex;
    std::vector<AssetDB*> _assetDBs;
    ResourceLoadingQueue _resourcesToLoad;
    // Quantum.SynchronizedQueue<AssetResource>
    std::mutex _resourcesToDisposeMutex;
    std::deque<AssetResource*> _resourcesToDispose;
    Allocator* _allocator = nullptr;
};

} // namespace Quantum
