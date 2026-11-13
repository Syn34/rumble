// Original: Quantum.DynamicAssetDB, Quantum.IAssetSerializerExtensions (quantum.core.dll).
#include "dynamic_asset_db.h"

#include "quantum_core/assert.h"

#include <cstring>
#include <stdexcept>

namespace Quantum {

namespace {

constexpr int HeaderSize = 12;

void WriteLittleEndian(std::vector<std::uint8_t>& out, std::size_t offset, std::uint64_t value,
                       int byteCount) {
    for (int i = 0; i < byteCount; ++i) {
        out[offset + static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(value >> (8 * i));
    }
}

std::uint64_t ReadLittleEndian(std::span<const std::uint8_t> in, std::size_t offset,
                               int byteCount) {
    if (in.size() < offset + static_cast<std::size_t>(byteCount)) {
        throw std::out_of_range("serialised dynamic asset database is truncated");
    }
    std::uint64_t value = 0;
    for (int i = 0; i < byteCount; ++i) {
        value |= std::uint64_t{in[offset + static_cast<std::size_t>(i)]} << (8 * i);
    }
    return value;
}

[[noreturn]] void ThrowNotImplemented() {
    throw std::logic_error("not implemented by DynamicAssetDB");
}

} // namespace

std::vector<std::shared_ptr<AssetObject>> DeserializeAssets(IAssetSerializer& serializer,
                                                            std::span<const std::uint8_t> data) {
    return serializer.DeserializeAssets(data, 0, static_cast<int>(data.size()));
}

DynamicAssetDB::DynamicAssetDB() : _head(std::make_shared<const Head>()) {}

std::shared_ptr<const DynamicAssetDB::Head> DynamicAssetDB::CurrentHead() const {
    std::lock_guard lock(_mutex);
    return _head;
}

bool DynamicAssetDB::IsEmpty() const { return CurrentHead()->ByGuid.empty(); }

AssetGuid DynamicAssetDB::NextGuid() const {
    std::lock_guard lock(_mutex);
    return AssetGuid(static_cast<std::int64_t>(static_cast<std::uint64_t>(_lastGuid + 1) |
                                               AssetGuid::DynamicBit));
}

void DynamicAssetDB::AddAssetWithValidDynamicGuid(std::shared_ptr<AssetObject> asset) {
    if (asset == nullptr) {
        throw std::invalid_argument("asset");
    }
    if (!asset->Guid.IsDynamic()) {
        throw std::invalid_argument("asset GUID is not dynamic");
    }
    std::lock_guard lock(_mutex);
    if (_lastGuid < (asset->Guid.Value & ~static_cast<std::int64_t>(AssetGuid::DynamicBit))) {
        throw std::invalid_argument("dynamic asset GUID was never handed out");
    }
    auto next = std::make_shared<Head>(*_head);
    if (!next->ByGuid.emplace(asset->Guid, asset).second) {
        throw std::invalid_argument("dynamic asset GUID already present");
    }
    if (asset->HasPath() && !next->ByPath.emplace(asset->Path, asset).second) {
        throw std::invalid_argument("dynamic asset path already present");
    }
    _head = std::move(next);
}

void DynamicAssetDB::CopyFrom(const DynamicAssetDB& other) {
    if (&other == this) {
        return;
    }
    std::scoped_lock lock(_mutex, other._mutex);
    _head = other._head;
    _lastGuid = other._lastGuid;
}

std::vector<std::shared_ptr<AssetObject>>
DynamicAssetDB::Deserialize(std::span<const std::uint8_t> bytes, IAssetSerializer& assetSerializer) {
    const auto version = static_cast<std::int32_t>(ReadLittleEndian(bytes, 0, 4));
    if (version >= 2) {
        throw std::invalid_argument("unsupported dynamic asset database format " +
                                    std::to_string(version));
    }
    {
        std::lock_guard lock(_mutex);
        _lastGuid = static_cast<std::int64_t>(ReadLittleEndian(bytes, 4, 8));
        _head = std::make_shared<const Head>();
    }
    auto assets = assetSerializer.DeserializeAssets(bytes, HeaderSize,
                                                    static_cast<int>(bytes.size()) - HeaderSize);
    for (const auto& asset : assets) {
        AddAssetWithValidDynamicGuid(asset);
    }
    return assets;
}

std::vector<std::uint8_t> DynamicAssetDB::Serialize(IAssetSerializer& serializer) const {
    std::vector<std::uint8_t> header;
    std::vector<std::uint8_t> assets;
    Serialize(serializer, header, assets);
    header.insert(header.end(), assets.begin(), assets.end());
    return header;
}

void DynamicAssetDB::Serialize(IAssetSerializer& serializer, std::vector<std::uint8_t>& header,
                               std::vector<std::uint8_t>& assets) const {
    header.assign(HeaderSize, 0);
    std::int64_t lastGuid = 0;
    {
        std::lock_guard lock(_mutex);
        lastGuid = _lastGuid;
    }
    WriteLittleEndian(header, 0, static_cast<std::uint32_t>(FormatVersion), 4);
    WriteLittleEndian(header, 4, static_cast<std::uint64_t>(lastGuid), 8);
    assets = serializer.SerializeAssets(Assets());
}

bool DynamicAssetDB::DisposeAsset(AssetGuid guid) {
    if (!guid.IsDynamic()) {
        return _staticResourceManager != nullptr && _staticResourceManager->DisposeAsset(guid);
    }
    std::lock_guard lock(_mutex);
    const auto found = _head->ByGuid.find(guid);
    if (found == _head->ByGuid.end()) {
        return false;
    }
    auto next = std::make_shared<Head>(*_head);
    const std::shared_ptr<AssetObject> removed = found->second;
    next->ByGuid.erase(guid);
    if (removed->HasPath()) {
        const auto byPath = next->ByPath.find(removed->Path);
        Assert::Always(byPath != next->ByPath.end());
        Assert::Always(byPath->second == removed);
        next->ByPath.erase(byPath);
    }
    _head = std::move(next);
    return true;
}

std::vector<std::shared_ptr<AssetObject>> DynamicAssetDB::Assets() const {
    const auto head = CurrentHead();
    std::vector<std::shared_ptr<AssetObject>> assets;
    assets.reserve(head->ByGuid.size());
    for (const auto& entry : head->ByGuid) {
        assets.push_back(entry.second);
    }
    return assets;
}

AssetObject* DynamicAssetDB::FindAsset(AssetGuid guid) const {
    const auto head = CurrentHead();
    const auto found = head->ByGuid.find(guid);
    // The snapshot may be replaced after this returns; the asset stays alive as long as
    // any snapshot (this frame's or a copy's) still holds it.
    return found != head->ByGuid.end() ? found->second.get() : nullptr;
}

std::int64_t DynamicAssetDB::StripDynamicFlag(AssetGuid guid) {
    if (!guid.IsDynamic()) {
        throw std::invalid_argument("asset GUID is not dynamic");
    }
    return guid.Value & ~static_cast<std::int64_t>(AssetGuid::DynamicBit);
}

std::shared_ptr<AssetDB> DynamicAssetDB::CreateAssetDatabase() { ThrowNotImplemented(); }

void DynamicAssetDB::DiposeAssetDatabase(AssetDB* /*db*/) { ThrowNotImplemented(); }

std::shared_ptr<AssetObject> DynamicAssetDB::GetAsset(AssetGuid guid, bool mainThread) {
    if (guid.IsDynamic()) {
        const auto head = CurrentHead();
        const auto found = head->ByGuid.find(guid);
        return found != head->ByGuid.end() ? found->second : nullptr;
    }
    return _staticResourceManager != nullptr ? _staticResourceManager->GetAsset(guid, mainThread)
                                             : nullptr;
}

std::vector<std::shared_ptr<AssetObject>> DynamicAssetDB::LoadAllAssets(bool /*mainThread*/) {
    ThrowNotImplemented();
}

void DynamicAssetDB::MainThreadSimulationEnded() { ThrowNotImplemented(); }

void DynamicAssetDB::MainThreadUpdate(Profiling::Profiler* /*profiler*/) { ThrowNotImplemented(); }

} // namespace Quantum
