// dynamic_asset_db.h - Quantum.DynamicAssetDB and Quantum.IAssetSerializer (quantum.core.dll).
//
// Assets created while a simulation runs carry dynamic GUIDs (top bit set, a running
// number below). Each frame owns a DynamicAssetDB with them. A frame copy must be cheap and
// independent, so the tables are immutable snapshots shared between copies: CopyFrom only
// shares the current snapshot, and every change builds a new one (the original uses
// persistent maps and a compare-and-swap; here a mutex guards the swap and the snapshot is
// rebuilt). Lookups of static GUIDs are forwarded to the static resource manager.
//
// Serialised form (Serialize/Deserialize): a 12-byte header - format version (int32, 1)
// and the last GUID number handed out (int64), little-endian - followed by whatever the
// IAssetSerializer makes of the assets.
#pragma once

#include "quantum_core/asset_guid.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/resource_manager.h"

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>

namespace Quantum {

// Turns assets into bytes and back (the game supplies the implementation).
// Original: Quantum.IAssetSerializer.
class IAssetSerializer {
  public:
    virtual ~IAssetSerializer() = default;
    virtual std::vector<std::uint8_t>
    SerializeAssets(const std::vector<std::shared_ptr<AssetObject>>& assets) = 0;
    virtual std::string PrintAsset(const AssetObject& asset) = 0;
    // Reads `count` bytes of `data` starting at `index`.
    virtual std::vector<std::shared_ptr<AssetObject>>
    DeserializeAssets(std::span<const std::uint8_t> data, int index, int count) = 0;
};

// 0x7cea010 (IAssetSerializerExtensions.DeserializeAssets) - the whole buffer.
std::vector<std::shared_ptr<AssetObject>> DeserializeAssets(IAssetSerializer& serializer,
                                                            std::span<const std::uint8_t> data);

class DynamicAssetDB final : public IResourceManager {
  public:
    static constexpr int FormatVersion = 1;

    // 0x7ce66e8 - empty, last GUID number 0.
    DynamicAssetDB();

    // 0x7ce66e0 / 0x7ce66d8 - the manager that resolves static GUIDs (not owned).
    IResourceManager* get_StaticResourceManager() const { return _staticResourceManager; }
    void set_StaticResourceManager(IResourceManager* value) { _staticResourceManager = value; }

    // 0x7ce6818
    bool IsEmpty() const;
    // 0x7ce6870 - the GUID the next dynamic asset gets.
    AssetGuid NextGuid() const;

    // 0x7ce6880 - registers an asset whose dynamic GUID was handed out already. Throws
    // std::invalid_argument for null, a static GUID or a GUID number above the last one
    // handed out. A GUID or path already present is an error of the snapshot maps
    // (std::invalid_argument as well).
    void AddAssetWithValidDynamicGuid(std::shared_ptr<AssetObject> asset);

    // 0x7ce6ad0 - shares `other`'s current snapshot and GUID counter.
    void CopyFrom(const DynamicAssetDB& other);

    // 0x7ce6aec - replaces the contents with a serialised database (see the file comment)
    // and returns the deserialised assets. Throws std::invalid_argument for a format
    // version this code does not know.
    std::vector<std::shared_ptr<AssetObject>> Deserialize(std::span<const std::uint8_t> bytes,
                                                          IAssetSerializer& assetSerializer);
    // 0x7ce6f04 - header and assets in one buffer.
    std::vector<std::uint8_t> Serialize(IAssetSerializer& serializer) const;
    // 0x7ce6fe4 - header and assets separately.
    void Serialize(IAssetSerializer& serializer, std::vector<std::uint8_t>& header,
                   std::vector<std::uint8_t>& assets) const;

    // 0x7ce7170 - removes a dynamic asset (true if it was there); a static GUID goes to the
    // static resource manager.
    bool DisposeAsset(AssetGuid guid) override;

    // 0x7ce7118 - every dynamic asset, in GUID order.
    std::vector<std::shared_ptr<AssetObject>> Assets() const;

    // 0x7ce740c - the dynamic asset with this GUID, or null.
    AssetObject* FindAsset(AssetGuid guid) const;
    // Typed lookup (generic, no own address).
    template <class T> T* FindAsset(AssetGuid guid) const {
        return dynamic_cast<T*>(FindAsset(guid));
    }

    // 0x7ce7464 - the GUID number; throws std::invalid_argument for a static GUID.
    static std::int64_t StripDynamicFlag(AssetGuid guid);

    // IResourceManager. Only GetAsset and DisposeAsset are meaningful; the others throw
    // std::logic_error ("not implemented") as in the original.
    // 0x7ce74d4
    std::shared_ptr<AssetDB> CreateAssetDatabase() override;
    // 0x7ce750c
    void DiposeAssetDatabase(AssetDB* db) override;
    // 0x7ce7544 - dynamic GUIDs from this database, static ones from the static manager
    // (null without one).
    std::shared_ptr<AssetObject> GetAsset(AssetGuid guid, bool mainThread) override;
    // 0x7ce762c
    std::vector<std::shared_ptr<AssetObject>> LoadAllAssets(bool mainThread) override;
    // 0x7ce7664
    void MainThreadSimulationEnded() override;
    // 0x7ce769c
    void MainThreadUpdate(Profiling::Profiler* profiler) override;
    // 0x7ce76d4 - nothing to release.
    void Dispose() override {}

  private:
    // One immutable snapshot of the tables. Original: DynamicAssetDB.Head (0x7ce6758 empty,
    // 0x7ce6a8c from two maps).
    struct Head {
        std::map<AssetGuid, std::shared_ptr<AssetObject>> ByGuid;
        std::map<std::string, std::shared_ptr<AssetObject>> ByPath;
    };

    std::shared_ptr<const Head> CurrentHead() const;

    IResourceManager* _staticResourceManager = nullptr;
    mutable std::mutex _mutex;
    std::shared_ptr<const Head> _head;
    std::int64_t _lastGuid = 0;
};

} // namespace Quantum
