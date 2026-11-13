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
| Textures, materials, render targets, text | not started |
| Asset loading from the user's game files | not started (quantum.core side ready: `IAssetResourceProvider` seam in `src/game/quantum_core/asset_resource_provider.h`) |
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
  collision callbacks with enter/stay/exit lists, islands, legacy Gauss-Seidel contact solver,
  penetration correction, velocity integration, sleeping), Entry3D world body settings /
  integration / AABBs, PhysicsSceneSettings, map static colliders (spheres/boxes),
  PhysicsEngine3D (owned by FrameContext with UsePhysics3D; SetCallbacks; immediate line casts
  and shape overlaps). Partly verified against the original and marked TODO: callback
  handlers, islands, the velocity solver loop, sleep detection, UpdateMap, immediate queries.
  Not ported: static meshes (TriangleMesh), broad-phase queries resolved in the step, shape
  casts, compounds, CCD integration, joints, the hybrid Jacobi/Gauss-Seidel solver, the
  whole 2D scene/engine
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

## Known issues

- On Mesa's software Vulkan driver (lavapipe), line primitives crossing the near plane
  rasterised incorrectly. The placeholder grid avoids it; needs checking on real GPUs.
- Swapchain resize is implemented but has only been exercised at a fixed size so far.
- Buffer/pipeline destruction on Vulkan stalls the GPU; replace with deferred deletion.

## Next

1. quantum.core: memory/collections (in progress), collision geometry, frame/ECS
2. Concrete Native::Allocator/Utility for the PC platform; hook DeterministicSession to quantum.core's game
3. Renderer: textures, materials, render targets
4. Asset reader for the game's content files (from the user's own copy)
5. Gameplay systems from quantum.code.dll, starting with movement and damage
