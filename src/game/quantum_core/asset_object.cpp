// Original: Quantum.AssetObject, Quantum.AssetObjectIdentifier (quantum.core.dll).
#include "asset_object.h"

#include <memory>

#if defined(__GNUG__)
#include <cxxabi.h>
#include <cstdlib>
#endif

namespace Quantum {

std::string AssetObjectIdentifier::ToString() const { return Path + " " + Guid.ToString(true); }

void AssetObject::Loaded(IResourceManager* /*resourceManager*/, Allocator* /*allocator*/) {}

void AssetObject::Disposed(IResourceManager* /*resourceManager*/, Allocator* /*allocator*/) {}

AssetRef AssetRef::FromAsset(const AssetObject* asset) {
    return AssetRef{asset != nullptr ? asset->Guid : AssetGuid{}};
}

std::string AssetTypeName(const std::type_info& type) {
    std::string name = type.name();
#if defined(__GNUG__)
    int status = 0;
    std::unique_ptr<char, void (*)(void*)> demangled(
        abi::__cxa_demangle(type.name(), nullptr, nullptr, &status), std::free);
    if (status == 0 && demangled) {
        name = demangled.get();
    }
#endif
    // Drop "class "/"struct " (MSVC) and namespaces: the original prints the bare name.
    for (const char* prefix : {"class ", "struct "}) {
        if (name.rfind(prefix, 0) == 0) {
            name.erase(0, std::char_traits<char>::length(prefix));
        }
    }
    if (const auto colon = name.rfind("::"); colon != std::string::npos) {
        name.erase(0, colon + 2);
    }
    return name;
}

} // namespace Quantum
