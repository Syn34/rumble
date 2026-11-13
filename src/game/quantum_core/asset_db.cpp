// Original: Quantum.AssetDB (quantum.core.dll).
#include "asset_db.h"

#include "quantum_core/assert.h"

namespace Quantum {

AssetDB::AssetDB(IResourceManager* manager) : _resourceManager(manager) {}

void AssetDB::Dispose() {
    _resourceManager = nullptr;
    _context = nullptr;
}

void AssetDB::DisposeAsset(AssetGuid guid, bool /*mainThread*/) {
    std::shared_ptr<AssetObject> removed;
    if (!_byGuid.TryRemove(guid, removed) || !removed->HasPath()) {
        return;
    }
    std::shared_ptr<AssetObject> removedByPath;
    Assert::Always(_byPath.TryRemove(removed->Path, removedByPath));
    Assert::Always(removedByPath == removed);
}

AssetObject* AssetDB::FindAsset(AssetGuid guid, bool mainThread) {
    if (!guid.IsValid()) {
        return nullptr;
    }
    std::shared_ptr<AssetObject> asset;
    if (_byGuid.TryGet(guid, asset)) {
        return asset.get();
    }
    asset = _resourceManager->GetAsset(guid, mainThread);
    if (asset != nullptr) {
        Assert::Always(asset->Guid == guid);
        AddAssetToCollection(asset);
    }
    return asset.get();
}

std::vector<std::shared_ptr<AssetObject>> AssetDB::FindAllAssets(bool mainThread) {
    return _resourceManager->LoadAllAssets(mainThread);
}

void AssetDB::AddAssetToCollection(std::shared_ptr<AssetObject> asset) {
    if (asset == nullptr) {
        throw std::invalid_argument("asset");
    }
    if (!_byGuid.TryAdd(asset->Guid, asset) || !asset->HasPath()) {
        return;
    }
    const std::string path = asset->Path;
    Assert::Always(_byPath.TryAdd(path, std::move(asset)));
}

} // namespace Quantum
