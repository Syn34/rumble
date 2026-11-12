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

Early foundation. See [docs/PROGRESS.md](docs/PROGRESS.md) for what exists and what's next.

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
frame), `--headless` (no window). Set `RR_VULKAN_VALIDATION=1` to enable the Vulkan
validation layer if it is installed.

## Layout

| Path | What lives there |
|---|---|
| `src/core/` | Engine-independent foundation: deterministic fixed-point maths, utilities. |
| `src/platform/` | OS abstraction (window, input, time, files): headless and SDL2 desktop backends. |
| `src/render/` | Graphics-API-neutral renderer interface with OpenGL and Vulkan backends; shaders. |
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
