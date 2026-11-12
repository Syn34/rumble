# Progress

Target: Android v1.10.4 (arm64), Unity 6000.3.10f1, IL2CPP metadata v39.

## Decisions

- **Offline only.** Local matches against bots; no online services are contacted.
- **Two renderers.** OpenGL 4.1 and Vulkan 1.0, chosen by the user per run or in settings.
- **Everything managed is decompiled**, including third-party libraries. Only the native
  engine runtime is replaced by our own engine layer.

## Scope

Method counts are methods with bodies, from `tools/il2cpp_inventory.py`
(171,382 across 168 assemblies in total).

### Game code

| Original assembly | Role | Methods | Done |
|---|---|---:|---:|
| quantum.code.dll | Deterministic gameplay simulation (rules, weapons, AI, game modes) | 7,251 | 14 |
| GameClasses.dll | Shared game types | 1,162 | 0 |
| Assembly-CSharp.dll | Game client (UI, presentation, flow) | 20,495 | 0 |
| Fidelio-Framework.dll, FidelioFramework-Core.dll | Developer's client framework | 16,438 | 0 |
| OhbibiCore.dll, AFuncs.dll, OhBibi.* | Developer's core/services libraries | ~3,500 | 0 |

### Third-party libraries (largest first)

| Original assembly | Role | Methods | Done |
|---|---|---:|---:|
| mscorlib.dll, System*.dll | .NET base library | ~30,000 | 0 |
| UnityEngine.*.dll, Unity.*.dll | Engine-facing managed API (UI Toolkit, core, render pipelines, TextMeshPro…) | ~40,000 | 0 |
| Rewired_Core.dll | Input mapping | 7,149 | 0 |
| Newtonsoft.Json.dll | JSON | 4,096 | 0 |
| PhotonQuantum.dll, quantum.core.dll, PhotonDeterministic.dll | Deterministic simulation framework and maths | 7,000 | partial (`FP` type) |
| AK.Wwise.*.dll | Audio middleware bindings | ~2,500 | 0 |
| MessagePack.dll, protobuf-net.dll | Binary serialisation | ~2,750 | 0 |
| DOTween*.dll, Cinemachine.dll, NGUI.dll | Tweening, cameras, UI | ~4,400 | 0 |

The remaining assemblies (analytics, ads, store, social-login, QR, Zendesk, Firebase
bindings and similar) mostly wrap online services; under the offline decision they become
local no-ops with documented behaviour.

### Engine layer (new code, replaces the native runtime)

| Module | Status |
|---|---|
| Platform: headless + SDL2 desktop (window, keyboard, mouse, gamepad, timing, files) | done (first version) |
| Renderer interface + OpenGL 4.1 backend | done (first version) |
| Renderer interface + Vulkan 1.0 backend | done (first version) |
| Shader pipeline (one GLSL source → GL text + SPIR-V, embedded) | done |
| Textures, materials, render targets, text | not started |
| Asset loading from the user's game files | not started |
| Audio | not started |
| Physics / animation | not started |

## Done

- Build skeleton (CMake, C++20, warnings-as-errors clean on GCC and Clang), unit test harness
- `rr::det::FP` fixed-point type matching the simulation's arithmetic rules
- `Quantum.FPMathUtility`: 14 of 16 methods (RandomOnUnitCircle, OrientOnNormal pending)
- `Photon.Deterministic.RNGSession` (13/13; PCG32 seeded by SplitMix64), `BitStream` (107/107,
  incl. generic `SerializeArrayLength`), `ByteUtils` (20/20; the gzip writer emits stored deflate
  blocks, so compressed bytes differ from zlib's)
- Inventory tool with method → address mapping
- Desktop platform backend; renderer selection (CLI, settings file, automatic fallback)
- OpenGL and Vulkan backends rendering an identical placeholder scene; frame capture;
  `tools/compare_renderers.py` (0.08% edge-pixel difference between backends on Mesa)

## Known issues

- On Mesa's software Vulkan driver (lavapipe), line primitives crossing the near plane
  rasterised incorrectly. The placeholder grid avoids it; needs checking on real GPUs.
- Swapchain resize is implemented but has only been exercised at a fixed size so far.
- Buffer/pipeline destruction on Vulkan stalls the GPU; replace with deferred deletion.

## Next

1. Deterministic maths library (PhotonDeterministic): FP trig/sqrt tables, FPQuaternion,
   FPMatrix, RNGSession — decompiled with addresses
2. Simulation framework types (quantum.core): entities, components, frame, systems
3. Renderer: textures, materials, render targets
4. Asset reader for the game's content files (from the user's own copy)
5. Gameplay systems from quantum.code.dll, starting with movement and damage
