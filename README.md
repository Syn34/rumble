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

Requirements: CMake 3.21+, a C++20 compiler (MSVC 2022, Clang 15+, or GCC 12+).

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release      # unit tests
./build/src/app/RangerRumble           # (Windows: build\src\app\Release\RangerRumble.exe)
```

## Layout

| Path | What lives there |
|---|---|
| `src/core/` | Engine-independent foundation: deterministic fixed-point maths, utilities. |
| `src/platform/` | OS abstraction (window, input, time, files) with swappable backends. |
| `src/game/` | The reimplemented game, grouped by the original assembly it replaces. |
| `src/app/` | The PC executable's entry point and main loop. |
| `tests/` | Unit tests (no external dependencies). |
| `tools/` | Scripts run on *your* copy of the game to build work lists and study behaviour. |
| `docs/` | Architecture notes and progress tracking. |

## How the original is put together (and what that means here)

The Android game is a Unity (6000.3) title compiled with IL2CPP. Its code falls into
three groups, and each is handled differently:

1. **Game code** (gameplay simulation, game client, the developer's framework).
   This is what gets reimplemented function by function in `src/game/`.
2. **Third-party libraries** (JSON, serialisation, tweening, input, networking SDKs,
   the .NET base library). These are not decompiled; game code uses standard C++ or
   small purpose-written replacements instead.
3. **The engine runtime** (rendering, physics, animation, audio middleware, asset
   loading). Replaced by our own engine layer behind clean interfaces, starting with
   `src/platform/`.

See [CONTRIBUTING.md](CONTRIBUTING.md) for the rules every change follows.
