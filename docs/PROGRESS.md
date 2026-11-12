# Progress

Target: Android v1.10.4 (arm64), Unity 6000.3.10f1, IL2CPP metadata v39.

## Scope

Method counts below are methods with bodies, from `tools/il2cpp_inventory.py`.

### Game code — reimplemented function by function

| Original assembly | Role | Methods | Done |
|---|---|---:|---:|
| quantum.code.dll | Deterministic gameplay simulation (rules, weapons, AI, game modes) | 7,251 | 14 |
| GameClasses.dll | Shared game types | 1,162 | 0 |
| Assembly-CSharp.dll | Game client (UI, presentation, flow) | 20,495 | 0 |
| Fidelio-Framework.dll, FidelioFramework-Core.dll | Developer's client framework | 16,438 | 0 |
| OhbibiCore.dll, AFuncs.dll, OhBibi.* | Developer's core/services libraries | ~3,500 | 0 |

### Replaced, not decompiled

- Deterministic maths library → `src/core` (`FP`, `FPVector2/3`, `FPMath`; in progress)
- .NET base library, JSON, MessagePack/protobuf, DOTween, Rewired → standard C++ / small replacements as needed
- Engine runtime (rendering, physics, animation, UI, asset loading) → own engine layer
- Audio middleware → own audio layer (format support TBD)
- Online services (matchmaking, accounts, analytics, purchases) → TBD, see open questions

## Done

- Build skeleton (CMake, C++20, warnings-as-errors clean on GCC and Clang), unit test harness
- Platform interface with a headless backend; fixed-step main loop
- `rr::det::FP` fixed-point type matching the simulation's arithmetic rules
- `Quantum.FPMathUtility`: 14 of 16 methods (RandomOnUnitCircle, OrientOnNormal pending)
- Inventory tool with method → address mapping

## Next

1. Deterministic maths: FPQuaternion, FPMath trig (sin/cos/atan2/sqrt), RNG session
2. Simulation core types the game code is built on: entities, components, frame, systems
3. Desktop platform backend (window, input, audio device)
4. Asset reader for the game's content files (from the user's own copy)
5. Gameplay systems from quantum.code.dll, starting with movement and damage
