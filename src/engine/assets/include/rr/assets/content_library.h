// content_library.h - on-demand access to the game's content bundles.
//
// Bundles refer to each other through the internal names of the serialized files
// they contain ("CAB-<hash>"). The library scans a folder of bundles once (reading
// only their directories), then opens and decompresses a bundle the first time one
// of its files is needed, and resolves object pointers across files.
#pragma once

#include "rr/assets/bundle_file.h"
#include "rr/assets/common_strings.h"
#include "rr/assets/serialized_file.h"
#include "rr/assets/type_tree_value.h"

#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace rr::assets {

struct ObjectRef {
    const SerializedFile* file = nullptr;
    const ObjectInfo* object = nullptr;
    std::string cab; // name of the serialized file holding the object
};

struct ScriptName {
    std::string assembly;
    std::string ns;
    std::string className;

    std::string FullName() const { return ns.empty() ? className : ns + "." + className; }
};

class ContentLibrary {
public:
    // `bundleDir` holds the *.bundle files (aa/Android in the asset pack).
    ContentLibrary(std::filesystem::path bundleDir, CommonStrings strings);

    const std::vector<std::filesystem::path>& Bundles() const { return bundles_; }
    // Serialized files inside one bundle (from the directory scan).
    std::vector<std::string> FilesInBundle(const std::filesystem::path& bundle) const;

    // Opens (and caches) the serialized file with this internal name.
    const SerializedFile* FindFile(const std::string& cab);

    // Follows a pointer stored in an object of file `fromCab`.
    std::optional<ObjectRef> Resolve(const std::string& fromCab, ObjectPointer pointer);

    // Decodes an object through its embedded type description.
    ValuePtr Decode(const ObjectRef& ref) const;

    // Class of the script attached to a script-backed object (via its MonoScript).
    std::optional<ScriptName> ScriptOf(const ObjectRef& ref);

private:
    std::filesystem::path dir_;
    CommonStrings strings_;
    std::vector<std::filesystem::path> bundles_;
    std::map<std::string, std::filesystem::path> cabToBundle_;
    std::map<std::filesystem::path, std::vector<std::string>> bundleFiles_;

    std::mutex mutex_;
    std::map<std::string, std::unique_ptr<SerializedFile>> openFiles_;
    std::map<std::pair<std::string, std::int64_t>, std::optional<ScriptName>> scriptCache_;
};

// Reads an object pointer ({m_FileID, m_PathID}) out of a decoded value.
std::optional<ObjectPointer> AsObjectPointer(const Value* value);

} // namespace rr::assets
