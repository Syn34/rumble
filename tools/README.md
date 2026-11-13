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

## il2cpp_lookup.py

Helpers while studying one function: `disasm <addr> [count]` and `usage <slot>` (resolves
the metadata-usage slot a function loads before a class-init call to the string literal
or method it refers to).

```sh
python3 -I tools/il2cpp_lookup.py --binary <libil2cpp.so> --metadata <global-metadata.dat> disasm 0x7ce0508 60
```

## progress.py

Updates the completion percentage in README.md and `docs/progress.json` (counts only).

```sh
python3 tools/il2cpp_inventory.py <global-metadata.dat> --binary <libil2cpp.so> --out build/inventory --assemblies all
python3 tools/progress.py --inventory build/inventory
```

## assetinfo

Lists what a content file contains and can decode objects through their embedded type
descriptions. Works on serialized files and on the content bundles in
`split_UnityDataAssetPack/assets/aa/Android/` (extract that APK split first).

```sh
build/tools/assetinfo <file.bundle> --engine-lib <libunity.so from the arm64 split> --dump all
```

`--engine-lib` supplies the engine's shared field-name table, which isn't stored in the
content files. Output is derived from your game files: don't commit it.

## contentindex

Lists the simulation assets (weapon specs, heroes, maps...) found in the extracted asset
pack, grouped by original script class, and with `--load` builds each one through the
asset provider the game uses.

```sh
build/tools/contentindex <split_UnityDataAssetPack/assets/aa/Android> <libunity.so> --load
```

## meshview

Views a mesh from a content bundle with its material's main texture (OpenGL or Vulkan,
drag to orbit, wheel to zoom), and runs numeric checks of the mesh and texture readers.

```sh
build/tools/meshview <bundle> --engine-lib <libunity.so> --list
build/tools/meshview <bundle> --engine-lib <libunity.so> --mesh <name> [--renderer opengl]
build/tools/meshview <bundle-dir> --engine-lib <libunity.so> --check-meshes
build/tools/meshview <bundle-dir> --engine-lib <libunity.so> --check-textures --renderer opengl
build/tools/meshview --astc-selftest 20000 --renderer opengl   # random blocks, no game files
build/tools/meshview --demo                                     # generated mesh, no game files
```

`--check-textures` decodes every mip in software and, where the renderer samples a format
natively (ASTC on Mesa's OpenGL driver, for example), compares the GPU's decode with ours.
`--frames N --screenshot out.ppm` saves the last frame. Everything it shows or saves comes from
your game files: keep it local.
