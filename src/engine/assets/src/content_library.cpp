#include "rr/assets/content_library.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>

namespace rr::assets {

namespace {

std::vector<std::uint8_t> ReadWholeFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw ReadError("cannot open " + path.string());
    return {std::istreambuf_iterator<char>(in), {}};
}

// "archive:/CAB-x/CAB-x" -> "CAB-x"
std::string CabFromExternalPath(const std::string& path) {
    const auto slash = path.find_last_of('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

} // namespace

std::optional<ObjectPointer> AsObjectPointer(const Value* value) {
    if (!value) return std::nullopt;
    const Value* fileId = value->Get("m_FileID");
    const Value* pathId = value->Get("m_PathID");
    if (!fileId || !pathId) return std::nullopt;
    return ObjectPointer{static_cast<std::int32_t>(fileId->AsInt()), pathId->AsInt()};
}

ContentLibrary::ContentLibrary(std::filesystem::path bundleDir, CommonStrings strings)
    : dir_(std::move(bundleDir)), strings_(std::move(strings)) {
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(dir_, ec)) {
        if (entry.path().extension() == ".bundle") bundles_.push_back(entry.path());
    }
    std::sort(bundles_.begin(), bundles_.end());
    for (const auto& bundle : bundles_) {
        try {
            for (const BundleEntry& e : BundleFile::ReadEntries(bundle.string())) {
                if (!e.IsSerializedFile()) continue;
                cabToBundle_[e.path] = bundle;
                bundleFiles_[bundle].push_back(e.path);
            }
        } catch (const ReadError& e) {
            std::fprintf(stderr, "content: skipping %s (%s)\n", bundle.filename().string().c_str(), e.what());
        }
    }
}

std::vector<std::string> ContentLibrary::FilesInBundle(const std::filesystem::path& bundle) const {
    auto it = bundleFiles_.find(bundle);
    return it != bundleFiles_.end() ? it->second : std::vector<std::string>{};
}

const SerializedFile* ContentLibrary::FindFile(const std::string& cab) {
    std::lock_guard lock(mutex_);
    if (auto it = openFiles_.find(cab); it != openFiles_.end()) return it->second.get();
    auto bundleIt = cabToBundle_.find(cab);
    if (bundleIt == cabToBundle_.end()) return nullptr;

    // Opening a bundle makes all of its serialized files available at once.
    const auto bytes = ReadWholeFile(bundleIt->second);
    BundleFile bundle(bytes);
    for (const BundleEntry& e : bundle.Entries()) {
        if (e.IsSerializedFile() && !openFiles_.count(e.path)) {
            openFiles_[e.path] = std::make_unique<SerializedFile>(bundle.CopyEntry(e));
        }
    }
    auto it = openFiles_.find(cab);
    return it != openFiles_.end() ? it->second.get() : nullptr;
}

std::optional<ObjectRef> ContentLibrary::Resolve(const std::string& fromCab, ObjectPointer pointer) {
    if (pointer.pathId == 0) return std::nullopt;
    const SerializedFile* from = FindFile(fromCab);
    if (!from) return std::nullopt;
    std::string cab = fromCab;
    if (pointer.fileId != 0) {
        const auto index = static_cast<std::size_t>(pointer.fileId - 1);
        if (index >= from->Externals().size()) return std::nullopt;
        cab = CabFromExternalPath(from->Externals()[index].path);
    }
    const SerializedFile* file = FindFile(cab);
    if (!file) return std::nullopt;
    const ObjectInfo* object = file->FindObject(pointer.pathId);
    if (!object) return std::nullopt;
    return ObjectRef{file, object, cab};
}

ValuePtr ContentLibrary::Decode(const ObjectRef& ref) const {
    return ReadTypeTreeObject(*ref.file, *ref.object, strings_);
}

std::optional<ScriptName> ContentLibrary::ScriptOf(const ObjectRef& ref) {
    const ValuePtr value = Decode(ref);
    const auto scriptPointer = AsObjectPointer(value->Get("m_Script"));
    if (!scriptPointer) return std::nullopt;
    const auto script = Resolve(ref.cab, *scriptPointer);
    if (!script) return std::nullopt;

    const auto key = std::make_pair(script->cab, script->object->pathId);
    {
        std::lock_guard lock(mutex_);
        if (auto it = scriptCache_.find(key); it != scriptCache_.end()) return it->second;
    }
    std::optional<ScriptName> name;
    const ValuePtr scriptValue = Decode(*script);
    if (const Value* cls = scriptValue->Get("m_ClassName"); cls && cls->AsString()) {
        name = ScriptName{};
        name->className = *cls->AsString();
        if (const Value* ns = scriptValue->Get("m_Namespace"); ns && ns->AsString()) name->ns = *ns->AsString();
        if (const Value* asmName = scriptValue->Get("m_AssemblyName"); asmName && asmName->AsString()) {
            name->assembly = *asmName->AsString();
        }
    }
    std::lock_guard lock(mutex_);
    scriptCache_[key] = name;
    return name;
}

} // namespace rr::assets
