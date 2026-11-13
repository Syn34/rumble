#include "quantum_content_provider.h"

#include <cstdio>

namespace rr::content {

namespace {

std::mutex& FactoryMutex() {
    static std::mutex m;
    return m;
}

std::map<std::string, AssetFactory>& Factories() {
    static std::map<std::string, AssetFactory> factories;
    return factories;
}

std::map<std::string, SourceAssetFactory>& SourceFactories() {
    static std::map<std::string, SourceAssetFactory> factories;
    return factories;
}

// Settings.Identifier.{Path, Guid.Value}, if this object is a simulation asset.
std::optional<Quantum::AssetObjectIdentifier> IdentifierOf(const rr::assets::Value& object) {
    const auto* settings = object.Get("Settings");
    const auto* identifier = settings ? settings->Get("Identifier") : nullptr;
    const auto* guid = identifier ? identifier->Get("Guid") : nullptr;
    const auto* value = guid ? guid->Get("Value") : nullptr;
    if (!value) return std::nullopt;
    Quantum::AssetObjectIdentifier id;
    id.Guid = Quantum::AssetGuid(value->AsInt());
    if (const auto* path = identifier->Get("Path"); path && path->AsString()) id.Path = *path->AsString();
    if (!id.Guid.IsValid()) return std::nullopt;
    return id;
}

} // namespace

void RegisterAssetFactory(const std::string& scriptClass, AssetFactory factory) {
    std::lock_guard lock(FactoryMutex());
    Factories()[scriptClass] = std::move(factory);
}

void RegisterSourceAssetFactory(const std::string& scriptClass, SourceAssetFactory factory) {
    std::lock_guard lock(FactoryMutex());
    SourceFactories()[scriptClass] = std::move(factory);
}

std::shared_ptr<Quantum::AssetObject> BuildAsset(const std::string& scriptClass,
                                                 const rr::assets::Value& settings) {
    AssetFactory factory;
    {
        std::lock_guard lock(FactoryMutex());
        if (auto it = Factories().find(scriptClass); it != Factories().end()) factory = it->second;
    }
    return factory ? factory(settings) : nullptr;
}

QuantumContentProvider::QuantumContentProvider(rr::assets::ContentLibrary& library,
                                               std::string bundlePrefix)
    : library_(library) {
    constexpr std::int32_t kScriptObjectClass = static_cast<std::int32_t>(rr::assets::ClassId::MonoBehaviour);
    for (const auto& bundle : library_.Bundles()) {
        if (bundle.filename().string().rfind(bundlePrefix, 0) != 0) continue;
        for (const std::string& cab : library_.FilesInBundle(bundle)) {
            const rr::assets::SerializedFile* file = library_.FindFile(cab);
            if (!file || !file->HasTypeTrees()) continue;
            for (const auto& object : file->Objects()) {
                if (object.classId != kScriptObjectClass) continue;
                try {
                    const auto value = library_.Decode({file, &object, cab});
                    if (auto id = IdentifierOf(*value)) {
                        if (byGuid_.count(id->Guid.Value)) continue; // first occurrence wins
                        byGuid_[id->Guid.Value] = entries_.size();
                        byPath_.emplace(id->Path, entries_.size());
                        entries_.push_back({*id, cab, object.pathId});
                    }
                } catch (const rr::assets::ReadError& e) {
                    std::fprintf(stderr, "content: cannot decode object %lld in %s: %s\n",
                                 static_cast<long long>(object.pathId), cab.c_str(), e.what());
                }
            }
        }
    }
}

std::vector<Quantum::AssetObjectIdentifier> QuantumContentProvider::ListAssets() {
    std::vector<Quantum::AssetObjectIdentifier> ids;
    ids.reserve(entries_.size());
    for (const Entry& e : entries_) ids.push_back(e.id);
    return ids;
}

const QuantumContentProvider::Entry* QuantumContentProvider::Find(
    const Quantum::AssetObjectIdentifier& id) const {
    auto it = byGuid_.find(id.Guid.Value);
    return it != byGuid_.end() ? &entries_[it->second] : nullptr;
}

Quantum::AssetGuid QuantumContentProvider::GuidOfPath(const std::string& path) const {
    auto it = byPath_.find(path);
    return it != byPath_.end() ? entries_[it->second].id.Guid : Quantum::AssetGuid{};
}

std::optional<rr::assets::ObjectRef> QuantumContentProvider::SourceOf(Quantum::AssetGuid guid) {
    auto it = byGuid_.find(guid.Value);
    if (it == byGuid_.end()) return std::nullopt;
    const Entry& entry = entries_[it->second];
    const auto* file = library_.FindFile(entry.cab);
    const auto* object = file ? file->FindObject(entry.pathId) : nullptr;
    if (!object) return std::nullopt;
    return rr::assets::ObjectRef{file, object, entry.cab};
}

std::string QuantumContentProvider::ScriptClassOf(const Quantum::AssetObjectIdentifier& id) {
    const Entry* entry = Find(id);
    if (!entry) return {};
    const auto* file = library_.FindFile(entry->cab);
    const auto* object = file ? file->FindObject(entry->pathId) : nullptr;
    if (!object) return {};
    const auto script = library_.ScriptOf({file, object, entry->cab});
    return script ? script->className : std::string{};
}

std::shared_ptr<Quantum::AssetObject> QuantumContentProvider::LoadAsset(
    const Quantum::AssetObjectIdentifier& id) {
    const Entry* entry = Find(id);
    if (!entry) return nullptr;
    const auto* file = library_.FindFile(entry->cab);
    const auto* object = file ? file->FindObject(entry->pathId) : nullptr;
    if (!object) return nullptr;

    const auto value = library_.Decode({file, object, entry->cab});
    const auto* settings = value->Get("Settings");
    const std::string scriptClass = ScriptClassOf(id);

    AssetFactory factory;
    SourceAssetFactory sourceFactory;
    {
        std::lock_guard lock(FactoryMutex());
        if (auto it = Factories().find(scriptClass); it != Factories().end()) factory = it->second;
        if (auto it = SourceFactories().find(scriptClass); it != SourceFactories().end()) {
            sourceFactory = it->second;
        }
    }
    std::shared_ptr<Quantum::AssetObject> asset;
    if (sourceFactory) {
        asset = sourceFactory(AssetSource{library_, {file, object, entry->cab}, *value, settings, *this});
    } else if (factory && settings) {
        asset = factory(*settings);
    } else {
        auto unmapped = std::make_shared<UnmappedAsset>();
        unmapped->ScriptClass = scriptClass;
        unmapped->Settings = value;
        asset = std::move(unmapped);
    }
    if (asset) asset->SetIdentifier(entry->id);
    return asset;
}

} // namespace rr::content
