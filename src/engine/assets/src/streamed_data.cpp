#include "rr/assets/streamed_data.h"

#include <algorithm>
#include <fstream>
#include <iterator>

namespace rr::assets {

std::optional<StreamingInfo> ReadStreamingInfo(const Value* value) {
    if (!value || !value->AsStruct()) return std::nullopt;
    StreamingInfo info;
    if (const Value* v = value->Get("offset")) info.offset = static_cast<std::uint64_t>(v->AsInt());
    if (const Value* v = value->Get("size")) info.size = static_cast<std::uint64_t>(v->AsInt());
    if (const Value* v = value->Get("path"); v && v->AsString()) info.path = *v->AsString();
    return info;
}

std::vector<std::uint8_t> ValueBytes(const Value* value) {
    if (!value) return {};
    if (const auto* bytes = std::get_if<Value::Bytes>(&value->data)) return *bytes;
    std::vector<std::uint8_t> out;
    if (const auto* array = value->AsArray()) {
        out.reserve(array->size());
        for (const ValuePtr& item : *array) out.push_back(static_cast<std::uint8_t>(item->AsInt()));
    }
    return out;
}

namespace {

std::string EntryNameFromPath(const std::string& path) {
    const auto slash = path.find_last_of('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

} // namespace

ResourceStore::ResourceStore(const ContentLibrary& library, std::size_t cachedBundles)
    : cacheLimit_(std::max<std::size_t>(1, cachedBundles)) {
    for (const auto& bundle : library.Bundles()) {
        try {
            for (const BundleEntry& entry : BundleFile::ReadEntries(bundle.string())) {
                if (!entry.IsSerializedFile()) entryToBundle_[entry.path] = bundle;
            }
        } catch (const ReadError&) {
            // Unreadable bundles were already reported by the library's own scan.
        }
    }
}

const BundleFile& ResourceStore::OpenBundle(const std::filesystem::path& path) {
    for (auto it = cache_.begin(); it != cache_.end(); ++it) {
        if (it->first == path) {
            std::rotate(it, it + 1, cache_.end()); // mark as most recently used
            return *cache_.back().second;
        }
    }
    std::ifstream in(path, std::ios::binary);
    if (!in) throw ReadError("cannot open " + path.string());
    const std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(in), {}};
    auto bundle = std::make_unique<BundleFile>(bytes);
    if (cache_.size() >= cacheLimit_) cache_.erase(cache_.begin());
    cache_.emplace_back(path, std::move(bundle));
    return *cache_.back().second;
}

std::vector<std::uint8_t> ResourceStore::Fetch(const StreamingInfo& info) {
    const std::string name = EntryNameFromPath(info.path);
    std::lock_guard lock(mutex_);
    const auto it = entryToBundle_.find(name);
    if (it == entryToBundle_.end()) throw ReadError("resource " + name + " not found in any bundle");
    const BundleFile& bundle = OpenBundle(it->second);
    const BundleEntry* entry = bundle.FindEntry(name);
    if (!entry) throw ReadError("resource " + name + " missing from its bundle");
    const auto data = bundle.EntryData(*entry);
    if (info.offset > data.size() || info.size > data.size() - info.offset) {
        throw ReadError("resource range outside " + name);
    }
    const auto begin = data.begin() + static_cast<std::ptrdiff_t>(info.offset);
    return {begin, begin + static_cast<std::ptrdiff_t>(info.size)};
}

std::string ReadObjectName(const SerializedFile& file, const ObjectInfo& object) {
    try {
        BinaryReader reader = file.OpenObject(object);
        const std::int32_t length = reader.I32();
        if (length < 0 || static_cast<std::size_t>(length) > reader.Remaining()) return {};
        const auto bytes = reader.Bytes(static_cast<std::size_t>(length));
        return {bytes.begin(), bytes.end()};
    } catch (const ReadError&) {
        return {};
    }
}

} // namespace rr::assets
