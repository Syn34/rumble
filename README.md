# Ranger Rumble — native PC reimplementation

A from-scratch, human-readable C++ reimplementation of *Ratchet & Clank: Ranger Rumble*
(Android, v1.10.4) that builds with a standard PC toolchain and runs as a native application.

This is a **decompilation-by-rewriting** project. Each original function is studied to
understand what it does, then written again as clean, idiomatic C++ that serves the same
purpose. Nothing here translates, lifts or emulates the original machine code, and the
program never loads or calls into the original binaries.

> This repository contains **no game assets or original binaries**. You need your own
> legally obtained copy of the game; tools in `tools/` read it locally.

## Status

<!-- progress:start -->
**Overall: 4.06%** — 6,951 of 171,382 original methods reimplemented

`█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░`

| Part | Done | Methods | Progress |
|---|---:|---:|---:|
| Game code | 4,762 | 50,426 | 9.44% |
| Third-party libraries | 2,189 | 120,956 | 1.81% |

<details><summary>Per assembly (started ones first, then largest)</summary>

| Assembly | Done | Methods | Progress |
|---|---:|---:|---:|
| PhotonDeterministic.dll | 686 | 904 | 75.9% |
| quantum.code.dll | 4,762 | 7,251 | 65.7% |
| quantum.core.dll | 1,479 | 2,608 | 56.7% |
| PhotonQuantum.dll | 24 | 3,488 | 0.7% |
| Assembly-CSharp.dll | 0 | 20,495 | 0.0% |
| Fidelio-Framework.dll | 0 | 15,081 | 0.0% |
| mscorlib.dll | 0 | 13,074 | 0.0% |
| UnityEngine.UIElementsModule.dll | 0 | 11,118 | 0.0% |
| UnityEngine.CoreModule.dll | 0 | 8,518 | 0.0% |
| Rewired_Core.dll | 0 | 7,149 | 0.0% |
| System.Xml.dll | 0 | 6,468 | 0.0% |
| System.dll | 0 | 5,442 | 0.0% |
| Unity.RenderPipelines.Core.Runtime.dll | 0 | 4,981 | 0.0% |
| Newtonsoft.Json.dll | 0 | 4,096 | 0.0% |
| System.Core.dll | 0 | 3,461 | 0.0% |
| Unity.RenderPipelines.Universal.Runtime.dll | 0 | 3,340 | 0.0% |
| System.Data.dll | 0 | 3,279 | 0.0% |
| Unity.Zendesk.dll | 0 | 2,473 | 0.0% |
| AK.Wwise.Unity.API.dll | 0 | 2,129 | 0.0% |
| UnityEngine.UI.dll | 0 | 1,903 | 0.0% |
| NGUI.dll | 0 | 1,849 | 0.0% |
| Unity.TextMeshPro.dll | 0 | 1,814 | 0.0% |
| zxing.unity.dll | 0 | 1,802 | 0.0% |
| MessagePack.dll | 0 | 1,780 | 0.0% |
| OhbibiCore.dll | 0 | 1,647 | 0.0% |
| AFuncs.dll | 0 | 1,382 | 0.0% |
| FidelioFramework-Core.dll | 0 | 1,357 | 0.0% |
| Cinemachine.dll | 0 | 1,350 | 0.0% |
| DOTween.dll | 0 | 1,163 | 0.0% |
| GameClasses.dll | 0 | 1,162 | 0.0% |
| UnityEngine.AnimationModule.dll | 0 | 986 | 0.0% |
| protobuf-net.dll | 0 | 968 | 0.0% |
| System.Collections.Immutable.dll | 0 | 907 | 0.0% |
| Unity.Zendesk.MessagingCore.dll | 0 | 890 | 0.0% |
| UnityEngine.IMGUIModule.dll | 0 | 878 | 0.0% |
| Unity.ResourceManager.dll | 0 | 861 | 0.0% |
| UnityEngine.TextCoreTextEngineModule.dll | 0 | 848 | 0.0% |
| Unity.RenderPipelines.GPUDriven.Runtime.dll | 0 | 815 | 0.0% |
| UnityEngine.PropertiesModule.dll | 0 | 782 | 0.0% |
| Photon3Unity3D.dll | 0 | 762 | 0.0% |
| *123 more* | 0 | 20,121 | |

</details>

<sub>Counted by `tools/progress.py` on 2026-11-13: a method is done when its original address is cited above a reimplementation under `src/game/`. The engine layer (platform, renderers, asset readers) is new code and isn't counted.</sub>
<!-- progress:end -->

See [docs/PROGRESS.md](docs/PROGRESS.md) for details and what's next.

## Building

Requirements:

- CMake 3.21+ and a C++20 compiler (MSVC 2022, Clang 15+, or GCC 12+)
- SDL2 — found on the system (package manager, vcpkg, `SDL2_DIR`), otherwise
  downloaded automatically (`RR_FETCH_DEPENDENCIES=ON`, the default)
- For the Vulkan renderer: the [Vulkan SDK](https://vulkan.lunarg.com) (headers, loader,
  `glslangValidator`). Without it the build continues with OpenGL only.

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release      # unit tests
./build/src/app/RangerRumble           # (Windows: build\src\app\Release\RangerRumble.exe)
```

Linux packages (Ubuntu/Debian): `libsdl2-dev libvulkan-dev glslang-tools`.

Build options: `RR_ENABLE_OPENGL`, `RR_ENABLE_VULKAN`, `RR_ENABLE_DESKTOP` (OFF gives a
headless build), `RR_FETCH_DEPENDENCIES`, `RR_WARNINGS_AS_ERRORS`.

## Choosing a renderer

Both an **OpenGL 4.1** and a **Vulkan 1.0** renderer are built in; pick one per run:

```sh
RangerRumble --renderer vulkan
RangerRumble --renderer opengl --save-renderer   # also remember it for next time
RangerRumble --list-renderers
```

Without `--renderer`, the `renderer=` line in `userdata/settings.ini` is used, and failing
that Vulkan, then OpenGL. If the chosen renderer can't start (old driver, missing Vulkan
runtime) the other one is tried automatically.

Other options: `--frames N` (quit after N frames), `--screenshot out.ppm` (save the last
frame), `--headless` (no window), `--scene-demo` (draw a synthetic scene through the engine's
scene renderer). Set `RR_VULKAN_VALIDATION=1` to enable the Vulkan
validation layer if it is installed.

## Running a match

An offline match runs the reimplemented simulation on your own copy of the game. Point the
program at the extracted files once (they are remembered with `--save-content`):

```sh
RangerRumble --match q_ffa_megalopolis_01b \
    --content  <asset pack>/assets/aa/Android \
    --engine-lib <arm64 split>/lib/arm64-v8a/libunity.so \
    --data <base apk>/assets/bin/Data --data <asset pack>/assets/bin/Data \
    --save-content
RangerRumble --match q_ffa_megalopolis_01b                       # in a window, either renderer
RangerRumble --match q_ffa_megalopolis_01b --headless --ticks 600 # numbers only
```

`--match` takes a map name (or a unique part of it) or its GUID; `--game-mode` a mode name
(`FREE_FOR_ALL`, `BLAST_BALL`...), number or game mode GUID (default: from the map name);
`--hero`, `--players`, `--seed` set up the rest. In a window, W/A/S/D or the left stick move,
the right mouse button or right stick turn the camera, Space jump, left mouse fire, Shift dash,
F melee, Q gadget, E ultimate, R reload (fixed default bindings until the game's input-mapping
library is reimplemented). `--headless` prints entities per component, the systems that ran
or failed and a checksum every 60 ticks (`--check-views` also instantiates every entity view
prefab and prints counts); `python3 tools/match_determinism.py` runs a match twice
and compares them.

FPMath's lookup tables are game data: the small ones come from `global-metadata.dat`, the large
ones from the player's Resources files in `bin/Data` (or exported `LUT/*.bytes` in `--lut-dir`).
If those files are missing, `--approx-math` computes stand-ins so the match still runs, but not
bit-exactly like the original. Much of the gameplay is still being reimplemented; see
[docs/PROGRESS.md](docs/PROGRESS.md) for what a match does today.

## Layout

| Path | What lives there |
|---|---|
| `src/platform/` | OS abstraction (window, input, time, files): headless and SDL2 desktop backends. |
| `src/render/` | Graphics-API-neutral renderer interface with OpenGL and Vulkan backends; shaders. |
| `src/engine/` | Engine layer on top: readers for the game's content files (`assets/`) and scenes with the scene renderer (`scene/`). |
| `src/game/` | The reimplemented game, grouped by the original assembly it replaces. |
| `src/app/` | The PC executable's entry point and main loop. |
| `tests/` | Unit tests (no external dependencies). |
| `tools/` | Scripts run on *your* copy of the game to build work lists and study behaviour. |
| `docs/` | Architecture notes and progress tracking. |

## How the original is put together (and what that means here)

The Android game is a Unity (6000.3) title compiled with IL2CPP. All of its managed
code — the game itself, the developer's framework, **and every third-party library it
ships** (deterministic maths and simulation framework, JSON and binary serialisation,
tweening, input mapping, UI, the .NET base library and so on) — is reimplemented
function by function in `src/game/`, grouped by the original assembly.

The native engine runtime underneath (rendering, physics, animation, audio, asset
loading) is replaced by this project's own engine layer: `src/platform/`, `src/render/`
and the modules that will follow.

The PC version is **offline only**: matches are played locally (against bots). Online
services the original talked to (matchmaking, accounts, purchases, analytics, push
messages) are not contacted; code that depended on them is reimplemented against local
equivalents or stubbed with a clear note.

See [CONTRIBUTING.md](CONTRIBUTING.md) for the rules every change follows.
