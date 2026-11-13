// type_tree_value.h - decodes an object's bytes using its embedded type description.
//
// Files built with type descriptions (the game's content bundles are) can be read
// without knowing each class in advance: the description lists every field with
// its type, size and alignment. This turns an object into a tree of Values that
// the game's asset loaders then map onto their own C++ types.
#pragma once

#include "rr/assets/common_strings.h"
#include "rr/assets/serialized_file.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace rr::assets {

struct Value;
using ValuePtr = std::shared_ptr<Value>;

struct Value {
    struct Field {
        std::string name;
        ValuePtr value;
    };
    using Struct = std::vector<Field>;     // in declaration order
    using Array = std::vector<ValuePtr>;
    using Bytes = std::vector<std::uint8_t>;

    std::string type; // type name from the description, e.g. "int", "string", "PPtr<GameObject>"
    std::variant<std::monostate, bool, std::int64_t, std::uint64_t, double, std::string, Bytes,
                 Array, Struct>
        data;

    // Convenience accessors; they return null / defaults when the shape differs.
    const Value* Get(const std::string& field) const;
    std::int64_t AsInt(std::int64_t fallback = 0) const;
    double AsDouble(double fallback = 0) const;
    const std::string* AsString() const;
    const Array* AsArray() const;
    const Struct* AsStruct() const;

    // Readable multi-line dump (for tools and debugging).
    std::string Dump(int indent = 0) const;
};

// Reads one object. Throws ReadError if the description doesn't match the bytes.
ValuePtr ReadTypeTreeObject(const SerializedFile& file, const ObjectInfo& object,
                            const CommonStrings& strings);

// Same, for a type description supplied directly (e.g. for tests).
ValuePtr ReadTypeTreeValue(const std::vector<TypeTreeNode>& nodes, BinaryReader& reader,
                           const CommonStrings* strings);

} // namespace rr::assets
