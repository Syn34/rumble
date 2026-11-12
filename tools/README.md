# Tools

Everything here runs locally against **your own** copy of the game. Outputs go to
`build/` (or another folder you choose) and are gitignored — never commit them.

## Getting the inputs

From the APK set (base + arm64 split):

```sh
mkdir -p gamedata/apk && cd gamedata/apk
unzip -o ../../path/to/base.apk 'assets/bin/Data/*'
unzip -o ../../path/to/split_config.arm64_v8a.apk 'lib/arm64-v8a/libil2cpp.so'
```

## il2cpp_inventory.py

Builds the work list: every assembly, type and method in the game, with the address
of each compiled method body so it can be found in a disassembler while studying it.

```sh
pip install pyelftools
python3 tools/il2cpp_inventory.py \
    gamedata/apk/assets/bin/Data/Managed/Metadata/global-metadata.dat \
    --binary gamedata/apk/lib/arm64-v8a/libil2cpp.so \
    --assemblies quantum.code.dll,GameClasses.dll,Assembly-CSharp.dll
```

With `--fields <assemblies>` it also writes `<assembly>.fields.txt`: every type's fields with
their types and byte offsets (used to reconstruct structs and to decode game data).

Writes `build/inventory/summary.json` (method counts per assembly) and a
`<assembly>.methods.tsv` per requested assembly (token, address, namespace, type,
method, parameter count, status). Supports IL2CPP metadata v39 (Unity 6000.3).
