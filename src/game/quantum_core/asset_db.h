// asset_db.h - Quantum.AssetDB and Quantum.AssetDictionary`2 (quantum.core.dll).
//
// The asset database of one simulation context: a thread-safe cache in front of the
// resource manager. A lookup that misses the cache asks the manager (which loads the asset
// on first use) and remembers the result by GUID and by path. When a dynamic resource
// manager releases an asset it removes it from every database it created.
//
// Assets created at runtime (dynamic GUIDs) are not in here; frames keep those in their
// DynamicAssetDB (dynamic_asset_db.h). FrameBase::FindAsset picks the right one.
#pragma once

#include "quantum_core/asset_guid.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/resource_manager.h"

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace Quantum::Core {
class FrameContext;
}

namespace Quantum {

// A dictionary that many simulation threads read while some add or remove entries.
// Original: Quantum.AssetDictionary`2 - an open hash map whose state is swapped under a
// lock so readers never block. Here readers share a std::shared_mutex instead; lookups,
// insertions and removals behave the same. (Generic: the methods have no own addresses.)
template <class TKey, class TValue, class THash = std::hash<TKey>> class AssetDictionary {
  public:
    AssetDictionary() = default;
    explicit AssetDictionary(int capacity) { _map.reserve(static_cast<std::size_t>(capacity)); }

    bool TryGet(const TKey& key, TValue& value) const {
        std::shared_lock lock(_mutex);
        const auto found = _map.find(key);
        if (found == _map.end()) {
            return false;
        }
        value = found->second;
        return true;
    }

    // False (and nothing changes) when the key is already present.
    bool TryAdd(const TKey& key, TValue value) {
        std::unique_lock lock(_mutex);
        return _map.emplace(key, std::move(value)).second;
    }

    // Adds, or replaces when `overwrite` is set; an existing key without `overwrite` throws
    // std::invalid_argument.
    void Add(const TKey& key, TValue value, bool overwrite) {
        std::unique_lock lock(_mutex);
        const auto [where, added] = _map.emplace(key, value);
        if (!added) {
            if (!overwrite) {
                throw std::invalid_argument("key already present in the asset dictionary");
            }
            where->second = std::move(value);
        }
    }

    bool TryRemove(const TKey& key, TValue& value) {
        std::unique_lock lock(_mutex);
        const auto found = _map.find(key);
        if (found == _map.end()) {
            return false;
        }
        value = std::move(found->second);
        _map.erase(found);
        return true;
    }

    bool TryExists(const TKey& key) const {
        std::shared_lock lock(_mutex);
        return _map.find(key) != _map.end();
    }

    // Grows the table ahead of insertions.
    void Expand(int capacity) {
        std::unique_lock lock(_mutex);
        _map.reserve(static_cast<std::size_t>(capacity));
    }

    int Count() const {
        std::shared_lock lock(_mutex);
        return static_cast<int>(_map.size());
    }

  private:
    mutable std::shared_mutex _mutex;
    std::unordered_map<TKey, TValue, THash> _map;
};

class AssetDB {
  public:
    // 0x7ce60f4 - empty caches in front of `manager` (not owned).
    explicit AssetDB(IResourceManager* manager);

    AssetDB(const AssetDB&) = delete;
    AssetDB& operator=(const AssetDB&) = delete;

    // 0x7ce60ec
    IResourceManager* get_ResourceManager() const { return _resourceManager; }

    // 0x7ce61e4 - detaches from the manager and the context.
    void Dispose();

    // 0x7ce620c - forgets the cached asset (by GUID and path). `mainThread` is unused.
    void DisposeAsset(AssetGuid guid, bool mainThread);

    // 0x7ce62f0 - the cached asset, or the manager's (loading it if needed, then caching
    // it); null for an invalid GUID or an unknown asset.
    AssetObject* FindAsset(AssetGuid guid, bool mainThread);
    // Typed variant (generic, no own address): null when the asset has another type.
    template <class T> T* FindAsset(AssetGuid guid, bool mainThread) {
        return dynamic_cast<T*>(FindAsset(guid, mainThread));
    }

    // 0x7ce64d4 - every asset of the manager, loaded.
    std::vector<std::shared_ptr<AssetObject>> FindAllAssets(bool mainThread);

    // 0x7ce641c - caches `asset` by GUID and, if it has one, by path (a GUID already
    // cached is left alone). Throws std::invalid_argument for null.
    void AddAssetToCollection(std::shared_ptr<AssetObject> asset);

    // Set by the FrameContext that owns this database (Quantum.AssetDB._context).
    Core::FrameContext* _context = nullptr;

  private:
    friend class ResourceManagerBase;
    friend class ResourceManagerDynamic;

    IResourceManager* _resourceManager = nullptr;
    AssetDictionary<AssetGuid, std::shared_ptr<AssetObject>, AssetGuidHash> _byGuid;
    AssetDictionary<std::string, std::shared_ptr<AssetObject>> _byPath;
};

} // namespace Quantum
