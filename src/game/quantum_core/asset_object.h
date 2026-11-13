// asset_object.h - Quantum.AssetObject, its identifier, the built-in asset visitor and the
// asset config attribute (quantum.core.dll).
//
// Every simulation asset (maps, navmeshes, physics materials, the game's weapon and mode
// specs...) derives from AssetObject. An asset is identified by a 64-bit GUID (see
// asset_guid.h) and optionally by a path; the asset database looks assets up by either.
//
// Assets are immutable while a simulation runs. Loaded() is called once after an asset was
// loaded (to build caches or native buffers), Disposed() before it is released.
//
// Ownership: assets are shared - resource managers, the per-frame dynamic database and the
// per-context lookup cache all hold std::shared_ptr<AssetObject>; lookups hand out plain
// pointers that stay valid while the asset is loaded.
#pragma once

#include "photon_deterministic/native.h"
#include "quantum_core/asset_guid.h"

#include <string>
#include <typeinfo>


namespace Quantum {

class IResourceManager;

class BinaryData;
class CharacterController2DConfig;
class CharacterController3DConfig;
class EntityPrototype;
class EntityView;
class Map;
class NavMesh;
class NavMeshAgentConfig;
class PhysicsMaterial;
struct PolygonCollider;
class TerrainCollider;

// GUID and path of an asset. Original: Quantum.AssetObjectIdentifier.
struct AssetObjectIdentifier {
    std::string Path;
    AssetGuid Guid;

    // 0x7ce0900 - the path, a space and the bracketed GUID.
    std::string ToString() const;
};

// Double dispatch over the asset types quantum.core itself defines.
// Original: Quantum.IAssetObjectVisitor (methods in the original slot order).
class IAssetObjectVisitor {
  public:
    virtual ~IAssetObjectVisitor() = default;
    virtual void Visit(BinaryData& asset) = 0;
    virtual void Visit(CharacterController2DConfig& asset) = 0;
    virtual void Visit(CharacterController3DConfig& asset) = 0;
    virtual void Visit(EntityPrototype& asset) = 0;
    virtual void Visit(EntityView& asset) = 0;
    virtual void Visit(Map& asset) = 0;
    virtual void Visit(NavMesh& asset) = 0;
    virtual void Visit(NavMeshAgentConfig& asset) = 0;
    virtual void Visit(PhysicsMaterial& asset) = 0;
    virtual void Visit(PolygonCollider& asset) = 0;
    virtual void Visit(TerrainCollider& asset) = 0;
};

// Implemented by the built-in asset types. Original: Quantum.IBuiltInAssetObject.
class IBuiltInAssetObject {
  public:
    virtual ~IBuiltInAssetObject() = default;
    virtual void Dispatch(IAssetObjectVisitor& visitor) = 0;
};

class AssetObject {
  public:
    using Allocator = Photon::Deterministic::Native::Allocator;

    // Quantum.AssetObject.Identifier, kept as two fields: get_Guid (0x7ce0c40) /
    // set_Guid (0x7ce9f08) and get_Path (0x7ce62e8) / set_Path (0x7ce9f10) are plain
    // accesses to them.
    AssetGuid Guid;
    std::string Path;

    // 0x7ce0c2c
    AssetObject() = default;
    virtual ~AssetObject() = default;
    AssetObject(const AssetObject&) = default;
    AssetObject& operator=(const AssetObject&) = default;
    AssetObject(AssetObject&&) = default;
    AssetObject& operator=(AssetObject&&) = default;

    AssetObjectIdentifier Identifier() const { return AssetObjectIdentifier{Path, Guid}; }
    void SetIdentifier(const AssetObjectIdentifier& identifier) {
        Path = identifier.Path;
        Guid = identifier.Guid;
    }

    // 0x7ce9f20
    bool HasPath() const { return !Path.empty(); }

    // 0x7ce9f18 - after loading; the base does nothing. `resourceManager` is the manager
    // that loaded the asset (null when the asset is used outside one); `allocator` is the
    // manager's native allocator.
    virtual void Loaded(IResourceManager* resourceManager, Allocator* allocator);
    // 0x7ce9f1c - before release; the base does nothing.
    virtual void Disposed(IResourceManager* resourceManager, Allocator* allocator);
};

// Readable type name for log messages (the original prints Type.Name).
std::string AssetTypeName(const std::type_info& type);
template <class T> std::string AssetTypeName() { return AssetTypeName(typeid(T)); }

// Options of the editor-side code generator for an asset class; no effect at runtime.
// Original: Quantum.AssetObjectConfigAttribute.
struct AssetObjectConfigAttribute {
    bool _generateLinkingScripts = true;
    bool _generateAssetCreateMenu = true;
    bool _generateAssetResetMethod = true;
    bool _generateAsMainScript = true;
    std::string _customCreateAssetMenuName;
    int _customCreateAssetMenuOrder = -1;

    // 0x7d361b4 - every generator option on, no custom menu.
    AssetObjectConfigAttribute() = default;

    // 0x7d3618c
    void SetGenerateLinkingScripts(bool value) { _generateLinkingScripts = value; }
    // 0x7d36194
    void SetGenerateAssetCreateMenu(bool value) { _generateAssetCreateMenu = value; }
    // 0x7d3619c
    void SetGenerateAssetResetMethod(bool value) { _generateAssetResetMethod = value; }
    // 0x7d361a4
    void SetGenerateAsMainScript(bool value) { _generateAsMainScript = value; }
    // 0x7d361ac
    void SetCustomCreateAssetMenuName(std::string value) {
        _customCreateAssetMenuName = std::move(value);
    }
};

} // namespace Quantum
