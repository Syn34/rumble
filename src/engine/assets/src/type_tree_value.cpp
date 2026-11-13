#include "rr/assets/type_tree_value.h"

#include <cstring>
#include <sstream>

namespace rr::assets {

namespace {

constexpr std::uint32_t kAlignAfter = 0x4000;

class TreeReader {
public:
    TreeReader(const std::vector<TypeTreeNode>& nodes, BinaryReader& r, const CommonStrings* strings)
        : nodes_(nodes), r_(r), strings_(strings) {}

    // Reads the node at `index`; returns the index just past its subtree.
    std::size_t Read(std::size_t index, ValuePtr& out) {
        const TypeTreeNode& node = nodes_[index];
        const std::string type = Name(node.type);
        const std::size_t end = SubtreeEnd(index);
        out = std::make_shared<Value>();
        out->type = type;

        bool align = (node.metaFlags & kAlignAfter) != 0;
        if (end == index + 1) {
            ReadPrimitive(type, node.byteSize, *out);
        } else if (type == "string") {
            // string → Array → (int size, char data); the string itself carries the alignment.
            const auto length = CheckedCount(r_.I32());
            const auto bytes = r_.Bytes(length);
            out->data = std::string(bytes.begin(), bytes.end());
            align = align || (nodes_[index + 1].metaFlags & kAlignAfter) != 0;
        } else if (type == "TypelessData") {
            const auto length = CheckedCount(r_.I32());
            const auto bytes = r_.Bytes(length);
            out->data = Value::Bytes(bytes.begin(), bytes.end());
        } else if (Name(nodes_[index + 1].type) == "Array") {
            // A wrapper (vector, map, staticvector...) around one Array child.
            Value::Array items;
            ReadArray(index + 1, items, align);
            out->data = std::move(items);
        } else if (type == "Array") {
            Value::Array items;
            ReadArray(index, items, align);
            out->data = std::move(items);
            align = false; // handled inside ReadArray
        } else {
            Value::Struct fields;
            std::size_t child = index + 1;
            while (child < end) {
                ValuePtr value;
                const std::string name = Name(nodes_[child].name);
                child = Read(child, value);
                fields.push_back({name, std::move(value)});
            }
            out->data = std::move(fields);
        }
        if (align) r_.Align(4);
        return end;
    }

private:
    std::string Name(const std::string& raw) const { return strings_ ? strings_->Resolve(raw) : raw; }

    std::size_t SubtreeEnd(std::size_t index) const {
        std::size_t next = index + 1;
        while (next < nodes_.size() && nodes_[next].level > nodes_[index].level) ++next;
        return next;
    }

    std::size_t CheckedCount(std::int32_t count) const {
        if (count < 0 || static_cast<std::size_t>(count) > r_.Remaining()) {
            throw ReadError("type tree: element count larger than the remaining data");
        }
        return static_cast<std::size_t>(count);
    }

    // Array node: children are "size" (int) and "data" (element description).
    void ReadArray(std::size_t arrayIndex, Value::Array& items, bool& align) {
        const std::size_t dataIndex = arrayIndex + 2;
        const auto count = CheckedCount(r_.I32());
        if (dataIndex >= nodes_.size()) throw ReadError("type tree: malformed array");
        items.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            ValuePtr item;
            Read(dataIndex, item);
            items.push_back(std::move(item));
        }
        if (nodes_[arrayIndex].metaFlags & kAlignAfter) align = true;
    }

    void ReadPrimitive(const std::string& type, std::int32_t size, Value& out) {
        if (type == "bool") {
            out.data = r_.Bool();
        } else if (type == "float") {
            out.data = static_cast<double>(r_.F32());
        } else if (type == "double") {
            std::uint64_t bits = r_.U64();
            double d;
            std::memcpy(&d, &bits, sizeof(d));
            out.data = d;
        } else if (type == "UInt8" || type == "char" || type == "unsigned char") {
            out.data = std::uint64_t{r_.U8()};
        } else if (type == "SInt8") {
            out.data = std::int64_t{static_cast<std::int8_t>(r_.U8())};
        } else if (type == "UInt16" || type == "unsigned short") {
            out.data = std::uint64_t{r_.U16()};
        } else if (type == "SInt16" || type == "short") {
            out.data = std::int64_t{r_.I16()};
        } else if (type == "UInt32" || type == "unsigned int" || type == "Type*") {
            out.data = std::uint64_t{r_.U32()};
        } else if (type == "SInt32" || type == "int") {
            out.data = std::int64_t{r_.I32()};
        } else if (type == "UInt64" || type == "unsigned long long" || type == "FileSize") {
            out.data = r_.U64();
        } else if (type == "SInt64" || type == "long long") {
            out.data = r_.I64();
        } else if (size > 0) {
            // Unknown leaf: keep its raw bytes rather than failing.
            const auto bytes = r_.Bytes(static_cast<std::size_t>(size));
            out.data = Value::Bytes(bytes.begin(), bytes.end());
        }
    }

    const std::vector<TypeTreeNode>& nodes_;
    BinaryReader& r_;
    const CommonStrings* strings_;
};

} // namespace

const Value* Value::Get(const std::string& field) const {
    if (const auto* fields = std::get_if<Struct>(&data)) {
        for (const auto& f : *fields) {
            if (f.name == field) return f.value.get();
        }
    }
    return nullptr;
}

std::int64_t Value::AsInt(std::int64_t fallback) const {
    if (const auto* v = std::get_if<std::int64_t>(&data)) return *v;
    if (const auto* v = std::get_if<std::uint64_t>(&data)) return static_cast<std::int64_t>(*v);
    if (const auto* v = std::get_if<bool>(&data)) return *v ? 1 : 0;
    return fallback;
}

double Value::AsDouble(double fallback) const {
    if (const auto* v = std::get_if<double>(&data)) return *v;
    if (std::holds_alternative<std::int64_t>(data) || std::holds_alternative<std::uint64_t>(data)) {
        return static_cast<double>(AsInt());
    }
    return fallback;
}

const std::string* Value::AsString() const { return std::get_if<std::string>(&data); }
const Value::Array* Value::AsArray() const { return std::get_if<Array>(&data); }
const Value::Struct* Value::AsStruct() const { return std::get_if<Struct>(&data); }

std::string Value::Dump(int indent) const {
    std::ostringstream out;
    const std::string pad(static_cast<std::size_t>(indent) * 2, ' ');
    std::visit(
        [&](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                out << "<empty>";
            } else if constexpr (std::is_same_v<T, bool>) {
                out << (v ? "true" : "false");
            } else if constexpr (std::is_same_v<T, std::string>) {
                out << '"' << v << '"';
            } else if constexpr (std::is_same_v<T, Bytes>) {
                out << "<" << v.size() << " bytes>";
            } else if constexpr (std::is_same_v<T, Array>) {
                out << "[" << v.size() << "]";
                for (std::size_t i = 0; i < v.size() && i < 64; ++i) {
                    out << "\n" << pad << "  [" << i << "] " << v[i]->Dump(indent + 1);
                }
                if (v.size() > 64) out << "\n" << pad << "  ...";
            } else if constexpr (std::is_same_v<T, Struct>) {
                out << type;
                for (const auto& f : v) out << "\n" << pad << "  " << f.name << ": " << f.value->Dump(indent + 1);
            } else {
                out << v;
            }
        },
        data);
    return out.str();
}

ValuePtr ReadTypeTreeValue(const std::vector<TypeTreeNode>& nodes, BinaryReader& reader,
                           const CommonStrings* strings) {
    if (nodes.empty()) throw ReadError("type tree: no type description");
    ValuePtr root;
    TreeReader(nodes, reader, strings).Read(0, root);
    return root;
}

ValuePtr ReadTypeTreeObject(const SerializedFile& file, const ObjectInfo& object,
                            const CommonStrings& strings) {
    const auto& type = file.Types().at(static_cast<std::size_t>(object.typeIndex));
    BinaryReader reader = file.OpenObject(object);
    return ReadTypeTreeValue(type.typeTree, reader, &strings);
}

} // namespace rr::assets
