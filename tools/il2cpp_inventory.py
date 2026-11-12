#!/usr/bin/env python3
"""
il2cpp_inventory.py - build a local function inventory from the user's own
copy of the game's IL2CPP metadata (global-metadata.dat).

The output is a *work list* for the reimplementation effort: which assemblies,
types and methods exist, so we can track what has been rewritten in C++.
It is written under build/inventory/ (gitignored) and must not be committed,
because it is derived from the original game files.

Supports metadata version 39 (Unity 6000.3). Older/newer layouts are rejected
with a clear error rather than guessed at.

If --binary points at libil2cpp.so, each listed method also gets the address
of its compiled body. Addresses are for traceability comments and for looking
the function up in a disassembler while studying its behaviour; they are never
used by the C++ code. (Requires: pip install pyelftools)

Usage:
    python3 tools/il2cpp_inventory.py <global-metadata.dat> [--binary libil2cpp.so]
                                      [--out DIR] [--assemblies a.dll,b.dll]
"""
import argparse
import json
import os
import struct
import sys
from collections import defaultdict

METADATA_MAGIC = 0xFAB11BAF

# Section order of the v38/v39 header. Every section is (offset, size, count).
SECTION_NAMES = [
    "stringLiteral", "stringLiteralData", "string", "events", "properties",
    "methods", "parameterDefaultValues", "fieldDefaultValues",
    "fieldAndParameterDefaultValueData", "fieldMarshaledSizes", "parameters",
    "fields", "genericParameters", "genericParameterConstraints",
    "genericContainers", "nestedTypes", "interfaces", "vtableMethods",
    "interfaceOffsets", "typeDefinitions", "images", "assemblies", "fieldRefs",
    "referencedAssemblies", "attributeData", "attributeDataRange",
    "unresolvedVirtualCallParameterTypes", "unresolvedVirtualCallParameterRanges",
    "windowsRuntimeTypeNames", "windowsRuntimeStrings", "exportedTypeDefinitions",
]


def index_width(count):
    """IL2CPP v38+ stores some indices in the smallest width that fits."""
    if count <= 0xFF:
        return 1
    if count <= 0xFFFF:
        return 2
    return 4


class Reader:
    def __init__(self, data, pos=0):
        self.data = data
        self.pos = pos

    def u8(self):
        v = self.data[self.pos]
        self.pos += 1
        return v

    def u16(self):
        v = struct.unpack_from("<H", self.data, self.pos)[0]
        self.pos += 2
        return v

    def i32(self):
        v = struct.unpack_from("<i", self.data, self.pos)[0]
        self.pos += 4
        return v

    def u32(self):
        v = struct.unpack_from("<I", self.data, self.pos)[0]
        self.pos += 4
        return v

    def u64(self):
        v = struct.unpack_from("<Q", self.data, self.pos)[0]
        self.pos += 8
        return v

    def index(self, width):
        """Read a variable-width index; the all-ones value means 'none' (-1)."""
        if width == 1:
            v = self.u8()
            return -1 if v == 0xFF else v
        if width == 2:
            v = self.u16()
            return -1 if v == 0xFFFF else v
        return self.i32()


class Metadata:
    def __init__(self, data):
        self.data = data
        magic, self.version = struct.unpack_from("<Ii", data, 0)
        if magic != METADATA_MAGIC:
            raise ValueError("not an IL2CPP global-metadata.dat (bad magic)")
        if self.version != 39:
            raise ValueError(f"metadata v{self.version} is not supported yet (only v39)")

        self.sections = {}
        r = Reader(data, 8)
        for name in SECTION_NAMES:
            self.sections[name] = (r.i32(), r.i32(), r.i32())

        # Widths that are variable in v39.
        self.w_typedef = index_width(self.count("typeDefinitions"))
        self.w_gencontainer = index_width(self.count("genericContainers"))
        self.w_param = index_width(self.count("parameters"))
        # The Il2CppType table lives in the binary, so derive its index width
        # from the interface-offset pair size (type index + int32 offset).
        off, size, cnt = self.sections["interfaceOffsets"]
        self.w_type = size // cnt - 4 if cnt else 4

        self.images = self._read_images()
        self.assemblies = self._read_assemblies()
        self.types = self._read_types()
        self.methods = self._read_methods()

    def count(self, name):
        return self.sections[name][2]

    def string(self, index):
        base = self.sections["string"][0]
        start = base + index
        end = self.data.index(b"\0", start)
        return self.data[start:end].decode("utf-8", errors="replace")

    def _records(self, name):
        off, size, cnt = self.sections[name]
        stride = size // cnt if cnt else 0
        for i in range(cnt):
            yield i, Reader(self.data, off + i * stride)

    def _read_images(self):
        images = []
        for _, r in self._records("images"):
            name = r.i32()
            assembly = r.i32()
            first_type = r.index(self.w_typedef)
            type_count = r.u32()
            images.append({
                "name": self.string(name),
                "assembly": assembly,
                "first_type": first_type,
                "type_count": type_count,
            })
        return images

    def _read_assemblies(self):
        assemblies = []
        for _, r in self._records("assemblies"):
            image = r.i32()
            r.u32()  # token
            r.u32()  # module token
            r.i32()  # referenced assembly start
            r.i32()  # referenced assembly count
            name = r.i32()
            r.i32()  # culture
            r.i32()  # public key
            r.u32()  # hash alg
            r.i32()  # hash len
            r.u32()  # flags
            ver = (r.i32(), r.i32(), r.i32(), r.i32())
            assemblies.append({"image": image, "name": self.string(name),
                               "version": ".".join(map(str, ver))})
        return assemblies

    def _read_types(self):
        types = []
        for _, r in self._records("typeDefinitions"):
            name = r.i32()
            namespace = r.i32()
            r.index(self.w_type)          # byval type
            declaring = r.index(self.w_type)
            r.index(self.w_type)          # parent
            r.index(self.w_gencontainer)  # generic container
            flags = r.u32()
            r.i32()                       # first field
            first_method = r.i32()
            r.i32(); r.i32(); r.i32(); r.i32()  # events, properties, nested, interfaces
            r.i32()                       # vtable start
            r.i32()                       # interface offsets start
            method_count = r.u16()
            types.append({
                "name": self.string(name),
                "namespace": self.string(namespace),
                "flags": flags,
                "first_method": first_method,
                "method_count": method_count,
                "nested": declaring != -1,
            })
        return types

    def _read_methods(self):
        methods = []
        for _, r in self._records("methods"):
            name = r.i32()
            declaring = r.index(self.w_typedef)
            r.index(self.w_type)          # return type
            r.u32()                       # return parameter token
            r.index(self.w_param)         # parameter start
            r.index(self.w_gencontainer)  # generic container
            token = r.u32()
            flags = r.u16()
            r.u16()                       # iflags
            r.u16()                       # slot
            param_count = r.u16()
            methods.append({
                "name": self.string(name),
                "type": declaring,
                "token": token,
                "flags": flags,
                "params": param_count,
            })
        return methods


class Binary:
    """Locates per-assembly method pointer tables in an Android arm64 libil2cpp.so.

    Each Il2CppCodeGenModule starts with { const char* moduleName;
    uint32_t methodPointerCount; const Il2CppMethodPointer* methodPointers; ... }.
    In a PIC shared object these pointers are stored as R_AARCH64_RELATIVE
    relocations, so we resolve them through the relocation table.
    """
    R_AARCH64_RELATIVE = 1027

    def __init__(self, path):
        from elftools.elf.elffile import ELFFile
        with open(path, "rb") as f:
            self.data = f.read()
        elf = ELFFile(open(path, "rb"))
        self.loads = [(seg["p_vaddr"], seg["p_offset"], seg["p_filesz"])
                      for seg in elf.iter_segments() if seg["p_type"] == "PT_LOAD"]
        self.relative = {}
        for sec in elf.iter_sections():
            if sec["sh_type"] != "SHT_RELA":
                continue
            raw = self.data[sec["sh_offset"]: sec["sh_offset"] + sec["sh_size"]]
            for off, info, addend in struct.iter_unpack("<QQq", raw):
                if info & 0xFFFFFFFF == self.R_AARCH64_RELATIVE:
                    self.relative[off] = addend
        self.pointers_to = defaultdict(list)
        for where, target in self.relative.items():
            self.pointers_to[target].append(where)

    def file_offset(self, vaddr):
        for va, off, size in self.loads:
            if va <= vaddr < va + size:
                return off + (vaddr - va)
        raise ValueError(f"address {vaddr:#x} is not file-backed")

    def method_pointers(self, module_name):
        """Return the list of method body addresses for one assembly (index = RID-1)."""
        needle = b"\0" + module_name.encode() + b"\0"
        pos = self.data.find(needle)
        while pos != -1:
            string_vaddr = pos + 1  # rodata is identity-mapped in these builds
            for module in self.pointers_to.get(string_vaddr, []):
                count = struct.unpack_from("<I", self.data, self.file_offset(module + 8))[0]
                table = self.relative.get(module + 16)
                if table:
                    return [self.relative.get(table + 8 * i, 0) for i in range(count)]
            pos = self.data.find(needle, pos + 1)
        return None


METHOD_ATTR_ABSTRACT = 0x0400
METHOD_ATTR_PINVOKE = 0x2000


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("metadata")
    ap.add_argument("--binary", help="libil2cpp.so, to add method body addresses")
    ap.add_argument("--out", default=os.path.join("build", "inventory"))
    ap.add_argument("--assemblies", default="",
                    help="comma-separated list; write per-method lists only for these")
    args = ap.parse_args()

    with open(args.metadata, "rb") as f:
        md = Metadata(f.read())

    os.makedirs(args.out, exist_ok=True)
    wanted = {a.strip() for a in args.assemblies.split(",") if a.strip()}
    binary = Binary(args.binary) if args.binary else None

    summary = []
    for img in md.images:
        types = md.types[img["first_type"]: img["first_type"] + img["type_count"]]
        n_methods = sum(t["method_count"] for t in types)
        n_bodies = 0
        for t in types:
            for m in md.methods[t["first_method"]: t["first_method"] + t["method_count"]]:
                if not (m["flags"] & (METHOD_ATTR_ABSTRACT | METHOD_ATTR_PINVOKE)):
                    n_bodies += 1
        summary.append({"image": img["name"], "types": len(types),
                        "methods": n_methods, "methods_with_bodies": n_bodies})

        if img["name"] in wanted:
            pointers = binary.method_pointers(img["name"]) if binary else None
            path = os.path.join(args.out, img["name"] + ".methods.tsv")
            with open(path, "w", encoding="utf-8") as out:
                out.write("token\taddress\tnamespace\ttype\tmethod\tparams\tstatus\n")
                for t in types:
                    first, cnt = t["first_method"], t["method_count"]
                    for m in md.methods[first:first + cnt]:
                        if m["flags"] & (METHOD_ATTR_ABSTRACT | METHOD_ATTR_PINVOKE):
                            continue
                        rid = m["token"] & 0x00FFFFFF
                        addr = pointers[rid - 1] if pointers and rid <= len(pointers) else 0
                        out.write(f"0x{m['token']:08X}\t{addr:#x}\t{t['namespace']}\t{t['name']}\t"
                                  f"{m['name']}\t{m['params']}\tTODO\n")

    summary.sort(key=lambda s: -s["methods_with_bodies"])
    with open(os.path.join(args.out, "summary.json"), "w") as out:
        json.dump({"metadata_version": md.version, "images": summary}, out, indent=1)

    total = sum(s["methods_with_bodies"] for s in summary)
    print(f"metadata v{md.version}: {len(md.images)} images, {len(md.types)} types, "
          f"{len(md.methods)} methods ({total} with bodies)")
    for s in summary[:40]:
        print(f"{s['methods_with_bodies']:>8}  {s['types']:>6}  {s['image']}")


if __name__ == "__main__":
    sys.exit(main())
