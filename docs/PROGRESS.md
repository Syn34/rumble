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
| quantum.code.dll | Deterministic gameplay simulation (rules, weapons, AI, game modes) | 7,251 | ~2,860 (data types, Frame, events, signals, system framework, first systems) |
| quantum.code.dll | Deterministic gameplay simulation (rules, weapons, AI, game modes) | 7,251 | 3,147 (data types, Frame, events, signals, asset classes) |
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
| PhotonDeterministic.dll | Deterministic maths, RNG, bit streams, session runtime | 904 | ~790 (maths, RNGSession, ByteUtils, BitStream, offline session runtime) |
| quantum.core.dll, PhotonQuantum.dll | Simulation framework (frame, ECS, assets, prototypes, physics, navigation) | ~6,100 | 1,336 (quantum.core) |
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
| Textures (2D, mips, samplers, ASTC upload where supported) and a lit textured shader, both backends | done (first version) |
| Materials (shader variants), render targets, text | not started |
| Asset loading from the user's game files | simulation assets: all 1,868 built as C++ types from the content bundles (`src/game/content`); meshes, textures, materials, shader identification: readers done (`src/engine/assets`); prefabs/scenes not started |
| Texture decoders | ASTC LDR 4x4..12x12 (linear and sRGB; the only compressed format the game uses) and the uncompressed formats, to RGBA8 |
| Audio | not started |
| Physics / animation | not started |

## Done

- Build skeleton (CMake, C++20, warnings-as-errors clean on GCC and Clang), unit test harness
- PhotonDeterministic maths: FP, FPMath (table-driven trig/sqrt/exp, tables loaded from the
  user's game files), FPLut, FPVector2/3, FPQuaternion, FPMatrix2x2/3x3, FPBounds2, nullable types
- PhotonDeterministic RNGSession (PCG32), ByteUtils, BitStream
- `Quantum.FPMathUtility`: 14 of 16 methods (RandomOnUnitCircle, OrientOnNormal pending)
- quantum.core frame/ECS core: EntityRef, ComponentSet, ComponentTypeId registry, ComponentDataBuffer,
  block/component iterators and filters, FrameBase (entities, components, singletons, deferred
  commits, capacity changes, copy, serialisation), FrameContext (heaps, events, temp memory,
  prediction area), FrameMetaData, FrameSerializer with pointer keys, FrameChecksumerBitStream,
  CRC64. Pending: physics/navigation/prototype parts of the frame, FilterStruct/ComponentGetter,
  DispatcherBase subscriptions
- quantum.core asset system: AssetObject (+ identifier, visitor, config/ref attributes), AssetDB
  (thread-safe cache, AssetDictionary), DynamicAssetDB (shared snapshots, serialisation through
  IAssetSerializer), AssetResource state machine, ResourceManagerBase/Static/Dynamic (worker-thread
  loads routed to the main thread, deferred disposal), typed asset references, built-in assets
  (BinaryData, PhysicsMaterial, CharacterController2D/3D configs, EntityView, TerrainCollider holes,
  PolygonCollider native buffers), Map (world values, navmesh/region resolution), Assert; wired
  into FrameContext (AssetDB, serializer, initial dynamic assets), FrameBase/FrameThreadSafe
  FindAsset/TryFindAsset and the navigation config lookups. The PC pipeline plugs in through
  `IAssetResourceProvider` (asset_resource_provider.h). Pending: Map static colliders and
  triangle data (need MapStaticCollider2D/3D and TriangleCCW.Serialize), TerrainCollider.Bake
- quantum.core prototypes: EntityPrototype asset, EntityPrototypeContainer (lazy thread-safe sort,
  lookup by component type), EntityPrototypeRef, AssetRefEntityPrototype, ComponentPrototype /
  ComponentPrototype<T> (TypedComponentPrototype), ComponentPrototypeVisitorBase,
  PrototypeMaterializationContext, PrototypeValidator, MapEntityId, MapEntityLink and View
  components, Map.MapEntities; FrameBase.Create(EntityPrototype / AssetRefEntityPrototype / map
  entities), MaterializePrototypeInternal, materialised listeners and signal, FindPrototype.
  Prototypes of Transform2D/3D/2DVertical, View and the navmesh steering/avoidance/obstacle
  components materialise; NavMeshSteeringAgent/AvoidanceAgent/AvoidanceObstacle, View and
  MapEntityLink are registered with their C++ types. Pending: the physics, character controller
  and joint prototypes' Materialize/AddToEntity (fields, defaults and dispatch are done; they wait
  for the physics component types) and NavMeshPathfinder_Prototype (needs NavMeshPathfinder.Create
  and SetTarget)
- quantum.core narrow phase: 2D checks for every primitive pair (circle, box, polygon, edge,
  incl. PolygonPolygon clipping, PolygonEdge, oriented BoxBox), 3D sphere-sphere/box/triangle,
  box-box (15-axis SAT, edge/face clipping contacts) and box-triangle (Akenine-Moller SAT, deepest
  corner penetration, clipped contacts), NarrowData2D/3D init from transforms (box edge/plane
  caches), Gjk2D/Gjk3D (distance, simplex solvers, GJK ray-cast shape casts,
  conservative-advancement time of impact with separation functions), Entry2D/3D support mapping
  with CCD motion extrapolation, GJKSettings/CCDSettings, TriangleCCW, line casts against bodies
  (LinecastBody). 3D code works on the real Entry3D layout; 2D still uses a stand-in entry view
- quantum.core physics components: PhysicsBody2D/3D (init, impulses, forces, sleep, world inertia
  tensor, inertia reset from Shape3D inertia factors, Serialize), PhysicsCollider2D/3D,
  PhysicsCallbacks2D/3D collision lists (frame heap), PhysicsJoints/CharacterController layouts,
  all registered with their real types; HitCollection/HitCollection3D buffers; body and
  callbacks prototypes materialise. Pending: collider/callbacks Serialize (Shape Serialize),
  collider Create/OnRemoved (compounds), character controller movement, joints, PhysicsBody2D
  inertia reset
- quantum.core 3D physics scene: Scene3D step run task by task on one thread (entry creation
  with statics first, first/second-half force integration, bucketed sweep-and-prune broad
  phase with the original endpoint order and active/stabbing lists, narrow phase, trigger and
  collision callbacks with the original handler order, post-callback component checks and
  enter/stay/exit lists, CCD pairs sorted by time of impact with their callbacks and
  first-TOI integration, post-callback entry/manifold updates, islands, both contact
  solvers - legacy Gauss-Seidel (accumulated impulses) and hybrid Jacobi/Gauss-Seidel
  (per-body impulse chains) - penetration correction, velocity integration, per-entry and
  per-island sleep detection), UpdateMap (world/bucket sizes, radix-sorted static endpoints,
  static stabbing lists), Entry3D world body settings / integration / AABBs,
  PhysicsSceneSettings, map static colliders (spheres/boxes), CollisionInfo3D /
  TriggerInfo3D / ExitInfo3D, PhysicsEngine3D (owned by FrameContext with UsePhysics3D;
  SetCallbacks; immediate line casts, shape overlaps and shape casts walking the buckets as
  the original, hit order included), CharacterController3D movement (Init, SetConfig, Move,
  ComputeRawMovement, ComputeRawSteer) and its prototype. Not ported: static triangle meshes
  (TriangleMesh, mesh manifolds and their solver/callback paths), broad-phase queries
  resolved in the step (Query3D, Task_QueryResolve, the controller's query hits), compound
  shapes, joints and the joint position solver, the static colliders' mutable state, the
  whole 2D scene/engine and the 2D controller
- `Photon.Deterministic.RNGSession` (13/13; PCG32 seeded by SplitMix64), `BitStream` (107/107,
  incl. generic `SerializeArrayLength`), `ByteUtils` (20/20; the gzip writer emits stored deflate
  blocks, so compressed bytes differ from zlib's)
- PhotonDeterministic session runtime (offline path): DeterministicSession, DeterministicSimulator
  (rollback/prediction), frames and frame input, frame ring buffer, commands and command
  serializer, session config (+ serialisation), local network/input provider, tick inputs, Log,
  Native allocator/utility interfaces. Online-only parts (DeterministicNetwork,
  DeterministicTickInputDecoder, snapshot/checksum-error handling, Protocol wire format) are
  TODO stubs
- `quantum.core` navigation (176 methods): NavMesh (queries, line of sight, raycast, random
  points, position correction, ByteStream data format), navmesh data types and region masks,
  NavMeshAgentConfig, PathFinder (A* over triangles/links with ActiveHeap, funnel string
  pulling), Navigation avoidance maths (HRVO, velocity candidates, agent avoidance),
  navigation components' field layouts/accessors and frame-heap waypoints, ByteStream,
  MailboxSet8, PathUtils. Update loop (Updater/tasks), entity movement and prototypes are TODO
- Inventory tool with method → address mapping
- Desktop platform backend; renderer selection (CLI, settings file, automatic fallback)
- OpenGL and Vulkan backends rendering an identical placeholder scene; frame capture;
  `tools/compare_renderers.py` (0.08% edge-pixel difference between backends on Mesa)

- quantum.code foundation: all 113 enums, 179 typed asset refs, the 222 component types and
  the ~95 value types they use (exact layouts, GetHashCode, Serialize, collection lifetime);
  every hash (505 types, events, Input) checked against the original for random data.
  Component registration in the original order, Frame (globals, input, systems, events,
  copy), FrameEvents (121 event classes), FrameSignals (94 ISignal interfaces), Input,
  InputProvider, RuntimeConfig, SimulationConfig, Constants, SystemBase. Pending: frame
  serialisation/checksum/dump (the global block needs quantum.core's physics state and
  scene settings serialisers), player data, component-specific gameplay methods, systems
- quantum.code assets: all 416 asset classes (weapon/gadget/ultimate specs, enhancements and
  hero improvements, game modes and their configs, actor and gameplay specs, level data, the
  bot SDK's HFSM/BT/GOAP/utility graphs and the game's AI actions, decisions and functions)
  with their exact fields, nested types, base classes and constructor defaults, plus the
  ~80 value types they store; quantum.core's FPAnimationCurve (Evaluate), LayerMask,
  Shape3DConfig, QueryOptions. Implemented: every constructor, the Loaded overrides (graph
  reference resolution, blackboard/event maps, grind rail table), lookup tables, levelled
  values, getters and default virtuals (~570 methods). Pending (TODO with signatures): the
  gameplay methods that drive systems (FireWeapon, game mode Init/DeInit/ComputeBattleResult,
  enhancement/improvement OnInit, actor component Init/Shutdown, power-up effects) and the
  AI actions/decisions/functions, which need the bot SDK's AIContext and the systems' libraries
- Content factories (`src/game/content`): typed readers for decoded `Settings` trees (FP,
  vectors, asset references, enums, lists, nested structs), one reader per asset class and
  value type, and a factory per engine-side script class (`XAsset` -> `X`) for the gameplay
  assets and quantum.core's built-ins (Map with static colliders, NavMesh, BinaryData,
  PhysicsMaterial, character controller configs, EntityView, EntityPrototype, SimulationConfig).
  `contentindex --load` builds all 1,868 simulation assets of the game's bundles with C++
  types. Entity prototypes come with empty containers until the prefab reader exists

- quantum.code systems (`src/game/quantum_code/systems/`): the system framework
  (SystemMainThread, SystemMainThreadFilter with filter structs, groups, signal-only systems,
  generic component-driven enablers/disablers), SystemSetup's full list (186 root systems, six
  groups, 65 grouped systems, in the original order with each system's start state; systems not
  reimplemented yet are TODO shells in `pending_systems.h` so indices and enable bits match), a
  small PC driver (start/update) standing in for the session layer; quantum.core's
  ComponentFilterStruct/ComponentGetter (`quantum_core/component_filter_struct.h`). Reimplemented
  systems: health (initialisation per spec mode, regeneration, overhealth decay, pickups, max-HP
  multipliers), killables (delayed kills, kill/revive with kill-source resolution, extra lives),
  lifetimes, game timer, time scale, kill-count and bolt-count wins, bolts (gathering, losses),
  teams, actor status events and status requests, actor initialisation, combat activity, kill
  history, entity groups, hierarchy, map entities, player connection, physics sleep, KCC surface
  reset, energy cells, game-mode activators, collider scaler and the physics/navigation signal
  forwarders, plus the helper libs they use (KillableLib, HealthLib, GameModeHandlerLib, BoltLib,
  GameTimerLib, TimeScaleLib, PhysicsAbstraction, Ownership, FrameUtility.CreateDynamic, ...).
  The quantum.code asset classes the systems read are declared temporarily in
  `systems/asset_decls.h` until the asset reimplementation lands. Pending: the remaining ~180
  systems (movement, weapons, projectiles, abilities, AI, game modes...), the damage pipeline
  (DamageableSystem, HealthLib.ApplyHit, shields), respawning (ActorManager), and the parts
  that need the 3D scene (death animations, spawn-on-death ray casts, culling, physics step)

- Engine asset readers (`src/engine/assets`): Mesh (vertex streams in every component format,
  multiple streams, streamed `.resS` data, compressed/quantised meshes with packed bit vectors,
  16/32-bit indices, submeshes, bind poses, bone hashes, blend shapes), Texture2D (formats, mip
  chains, stripped mips, sampler settings, streamed data), Material (shader, textures with tiling,
  floats/ints/colours), Shader identification, ResourceStore for `StreamingInfo` ranges. Checked on
  the game's files numerically only (`meshview --check-meshes`): all 226 meshes decode, vertex
  bounds match the stored submesh bounds (worst 0.22% of size, quantisation), indices in range,
  normals unit length within 8-bit quantisation, skin weights sum to 1, 99.7% of triangles wind
  consistently with their normals
- ASTC LDR decoder written from the specification: bit-exact against Mesa's decoder on 242k random
  blocks (all six block sizes, both profiles) and on every mip of the game's 419 ASTC textures
  (`meshview --check-textures`, GPU readback compared with our decode)
- Renderer textures on both backends (RGBA8/sRGB, ASTC where the device has it, mips, samplers,
  four slots per draw, readback for checks), `lit_textured` shader program; the placeholder scene
  adds a lit mipmapped checker cube and an ASTC-textured quad built in code (compare_renderers:
  0.06% edge-pixel difference)
- `tools/meshview`: viewer (orbit camera, either renderer) for a mesh with its material's main
  texture, plus `--list`, `--check-meshes`, `--check-textures`, `--astc-selftest`, `--demo`

## Known issues

- On Mesa's software Vulkan driver (lavapipe), line primitives crossing the near plane
  rasterised incorrectly. The placeholder grid avoids it; needs checking on real GPUs.
- Swapchain resize is implemented but has only been exercised at a fixed size so far.
- Buffer/pipeline/texture destruction on Vulkan stalls the GPU; replace with deferred deletion.
  Texture uploads also wait for the queue (fine at load time, not for streaming).
- Lavapipe has no ASTC, so the native ASTC upload path on Vulkan is untested (it falls back to
  software decoding there); the OpenGL native path is tested on llvmpipe.
- Mesh bounds (`m_LocalAABB`) of some skinned meshes are larger than their vertices; the
  submesh bounds match. Not investigated further.
- The game's meshes are left-handed with clockwise front faces; meshview mirrors X and reverses
  each triangle. The real scene renderer needs the same conversion (and for transforms).

## Next

1. quantum.core: memory/collections (in progress), collision geometry, frame/ECS
2. Concrete Native::Allocator/Utility for the PC platform; hook DeterministicSession to quantum.core's game
3. Renderer: materials (map the game's shaders to PC shader programs), skinning, render targets
4. Asset reader for the game's content files (from the user's own copy)
5. Gameplay systems from quantum.code.dll: damage pipeline and respawning next, then movement,
   weapons and projectiles (need the 3D scene from quantum.core)
4. Asset reader for the game's content files: prefabs (entity prototype components), scenes, textures, meshes
5. Gameplay systems from quantum.code.dll, starting with movement and damage
