#!/usr/bin/env python3
"""
il2cpp_lookup.py - small helpers for studying a function in the original binary.

    disasm  <addr> [count]   disassemble `count` instructions (default 40)
    usage   <slot-addr>      resolve a metadata-usage slot (the address loaded
                             before a class-init call) to the string literal,
                             type or method it refers to

Works on your local copy only; nothing it prints may be committed.
Needs: pip install pyelftools capstone

Usage:
    python3 -I tools/il2cpp_lookup.py --binary libil2cpp.so --metadata global-metadata.dat disasm 0x7ce0508 60
"""
import argparse
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from il2cpp_inventory import Binary, Metadata  # noqa: E402

USAGE_KINDS = {1: "type-info", 2: "type", 3: "method", 4: "field", 5: "string", 6: "method-ref"}


def string_literal(md, index):
    table_off, _, count = md.sections["stringLiteral"]
    data_off, data_size, _ = md.sections["stringLiteralData"]
    start = struct.unpack_from("<i", md.data, table_off + 4 * index)[0]
    end = (struct.unpack_from("<i", md.data, table_off + 4 * (index + 1))[0]
           if index + 1 < count else data_size)
    return md.data[data_off + start: data_off + end].decode("utf-8", errors="replace")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--binary", required=True)
    ap.add_argument("--metadata", required=True)
    ap.add_argument("command", choices=["disasm", "usage"])
    ap.add_argument("address")
    ap.add_argument("count", nargs="?", type=int, default=40)
    args = ap.parse_args()

    binary = Binary(args.binary)
    address = int(args.address, 16)

    if args.command == "disasm":
        from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs
        cs = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
        start = binary.file_offset(address)
        for insn in cs.disasm(binary.data[start: start + 4 * args.count], address):
            print(f"  {insn.address:#x}  {insn.mnemonic:8} {insn.op_str}")
        return 0

    with open(args.metadata, "rb") as f:
        md = Metadata(f.read())
    # The slot is relocated to a word holding the encoded usage: kind in bits
    # 29-31, index in bits 1-28.
    target = binary.read_pointer(address)
    encoded = struct.unpack_from("<Q", binary.data, binary.file_offset(target))[0]
    kind = (encoded >> 29) & 7
    index = (encoded & 0x1FFFFFFF) >> 1
    print(f"{USAGE_KINDS.get(kind, kind)} #{index}")
    if kind == 5:
        print(repr(string_literal(md, index)))
    elif kind == 3 and index < len(md.methods):
        m = md.methods[index]
        print(f"{md.type_full_name(m['type'])}.{m['name']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
