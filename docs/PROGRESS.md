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
| quantum.code.dll | Deterministic gameplay simulation (rules, weapons, AI, game modes) | 7,251 | 4,730 (data types, Frame, events, signals, asset classes, component prototypes, system framework, damage/death/respawn loop, shields, KCC modifiers) |
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
| quantum.core.dll, PhotonQuantum.dll | Simulation framework (frame, ECS, assets, prototypes, physics, navigation) | ~6,100 | 1,479 (quantum.core), 24 (PhotonQuantum: entity prototype loading) |
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
| Materials: the game's shaders mapped onto our scene programs (unlit / lit / lit + normal map; opaque, alpha-tested, alpha-blended, additive, premultiplied; cull/depth state from the shader's forward pass), both backends | done (first version) |
| Scenes: prefab/scene hierarchies instantiated into engine scenes (mesh + skinned renderers, LOD groups, static batching data), GPU skinning, frustum culling, queue/depth sorting, orbit/fly camera (`src/engine/scene`, `tools/sceneview`) | done (first version) |
| Render targets, shadows, fog, post effects, particles, text | not started |
| Asset loading from the user's game files | simulation assets: all 1,868 built as C++ types from the content bundles (`src/game/content`); meshes, textures, materials, shader identification: readers done (`src/engine/assets`); prefab and scene hierarchies instantiate into engine scenes (`src/engine/scene`) |
| Texture decoders | ASTC LDR 4x4..12x12 (linear and sRGB; the only compressed format the game uses) and the uncompressed formats, to RGBA8 |
| Audio | not started |
| Physics / animation | not started (skinning follows node transforms; no animation clips or blend shapes yet) |

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
  `IAssetResourceProvider` (asset_resource_provider.h). Map static mesh triangles are read in
  Map.Loaded (TriangleCCW.Serialize, Map.SerializeStaticColliderTriangles): all 52 maps of the
  game have them (182,256 triangles for 5,799 mesh colliders, next to 11,943 boxes;
  `contentindex --load` reports them). Pending: MapStaticCollider2D (the game has none),
  TerrainCollider.Bake
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
  for the physics component types; PhysicsBody3D, PhysicsCollider3D, PhysicsCallbacks3D and
  CharacterController3D prototypes are done) and NavMeshPathfinder_Prototype (needs
  NavMeshPathfinder.Create and SetTarget)
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
  all registered with their real types; HitCollection/HitCollection3D buffers; body, collider
  (Shape3DConfig.CreateShape, PhysicsCollider3D.Create) and callbacks prototypes materialise;
  PhysicsCollider3D/PhysicsCallbacks3D/CharacterController3D Serialize (Shape3D.Serialize),
  collider OnRemoved. Pending: compound shapes (CompoundShape3D buffers, CopyFrom,
  FreePersistent, Serialize), the 2D colliders/callbacks Serialize, joints, PhysicsBody2D
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
  ComputeRawMovement, ComputeRawSteer) and its prototype. Static triangle meshes:
  TriangleMesh (hashed sparse cell grid, insertion, map initialisation with immutable and
  toggleable-on colliders, predicted-frame guard) owned by the engine; the mesh broad phase
  (EntryCheckTriangles with per-thread triangle mailboxes, per-bucket mesh manifolds and
  chained triangle manifolds), mesh triggers, triangle narrow phase and pruning
  (NarrowPhaseMeshes), mesh collision callbacks and infos (average triangle normal),
  CCD sorting/callbacks, islands, both solvers (legacy chained impulses, hybrid
  AggregateMesh) and per-triangle penetration correction; queries traverse the mesh
  (3D DDA for line casts, a swept cell cube for shape casts, covered cells for overlaps)
  with the original's per-triangle tests. Broad-phase queries (AddLinecastQuery,
  AddOverlapShapeQuery, per-step hit collections, GetQueryHits/TryGetQueryHits, the
  controller's query hits) are resolved in the step, but through the immediate query walk
  instead of the original's query endpoints (same pairs and filters; hit order within a
  bucket may differ). Hit3D static collider lookups. Not ported: the broad phase's query
  endpoints, compound shapes, joints and the joint position solver, the static colliders'
  mutable state (StaticCollidersState), TriangleMesh.Serialize (snapshots), the whole 2D
  scene/engine and the 2D controller
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
  types.
- Component prototypes: engine-side prefab reader (`rr/assets/prefab.h`: game objects with
  components, transforms, hierarchies and world transforms, script data via type trees,
  pointers across bundles); all 185 quantum.code component prototypes and the 15 value-type
  prototypes (`quantum_code/prototypes/`) with Materialize/AddToEntity/Dispatch (map entity
  links, heap lists and dictionaries in the original allocation order, fixed buffers, unions,
  prototype references for the user-extended ones), Quantum.ComponentPrototypeVisitor,
  FlatEntityPrototypeContainer (Collect/Store) and its StoreVisitor; readers for every game and
  framework prototype; PhotonQuantum's entity prototype side (`content/unity_entity_prototype.h`:
  the EntityPrototype script's PreSerialize / SerializeImplicitComponents, source-collider
  shapes, body config upgrade, EntityPrototypeConverter, the editor-side twins of linking
  prototypes, EntityPrototypeAsset.PrepareAsset and MapAsset.PrepareAsset, FPMathUtils
  conversions). `contentindex --load` now fills all 251 entity prototypes from their prefabs and
  all 997 baked map entities of the 52 maps, and reports prototypes per type. Pending: 2D
  source colliders and 2D shape configs (the game has none), joint prototypes on prefabs,
  the framework component scripts' Refresh overrides (unused by the game's prefabs)

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
- Engine scene layer (`src/engine/scene`): scene graph of nodes and render instances; prefab and
  scene hierarchies instantiate into it (MeshFilter + MeshRenderer, SkinnedMeshRenderer with bones
  bound to nodes, LODGroup, static-batch submesh ranges and roots; particles/lines/trails/sprites
  counted, not drawn). One handedness conversion for everything: X mirrored for positions,
  rotations (x, -y, -z, w), matrices (S M S), bind poses, bounds and LOD reference points; triangle
  winding reversed; tangent sign flipped. Material mapping: the shader's name (known families),
  sub-shader tags (Queue, UniversalMaterialType, ShaderGraphTargetId) and forward-pass state
  (blend factors, ZWrite, Cull, with the material's overriding properties) pick one of six
  `scene_*` programs (unlit / lit / lit + normal map, each also skinned) and the pipeline state;
  without the shader, the material's keywords, queue and surface properties decide. Albedo,
  normal map, tiling, base colour (linearised), cut-off and normal strength are passed on. Scene
  renderer: LOD selection by screen-relative height, frustum culling, render-queue order with
  opaque front-to-back and transparent back-to-front, GPU skinning (bone world * bind pose, up to
  256 bones) through a new per-draw uniform block (`SetDrawUniforms`: dynamic uniform buffer on
  Vulkan, UBO on OpenGL), additive/premultiplied blending, camera controller and render loop.
  Shader variants come from one source (`name=vert:frag:DEFINES` in `rr_add_shader_programs`).
  Synthetic demo scene (lit, normal-mapped, unlit vertex-coloured, alpha-tested, transparent,
  additive, two animated skinned meshes, LOD group, culled objects): `sceneview --demo` and
  `RangerRumble --scene-demo`, 1 differing pixel of 921,600 between backends.
  On the 69 staged bundles (`sceneview --stats`, numbers only): 584 root objects, 12,923 nodes,
  518 mesh + 151 skinned renderers, 50 LOD groups, 0 missing bones; 225 meshes, 126 materials and
  109 textures resolved (0 failures); 307 material slots and 148 meshes point into bundles not
  staged yet (131 of those meshes are built-in primitives); 4 materials found their shader (all
  known families), 122 were mapped without it. Drawn with both backends from a framing camera:
  467 draw calls (127 skinned), 577k triangles, 9 pipelines, no errors (Vulkan validation clean);
  116 skinned instances skinned on the CPU land inside their stored bounds (110 within 5%)

- Offline matches (`RangerRumble --match`, `src/app/match`; `Quantum.QuantumGame` in
  `quantum_code/quantum_game.*`): content startup (bundles with the engine's field-name table,
  QuantumContentProvider and the asset factories behind a ResourceManagerDynamic, the player's
  Resources table from `globalgamemanagers` with TextAsset reading, `global-metadata.dat`
  array-initialiser data), FPMath tables loaded from the user's files (small tables from the
  metadata, large ones from Resources or exported `.bytes`; `--approx-math` computes stand-ins
  when the files are missing); QuantumGame as the IDeterministicGame (frame context sized from
  the simulation config and session, systems from SystemSetup, frames, OnGameStart/InitSystems,
  OnSimulate with per-system exception capture, input poll/serialise/deserialise in input mode,
  ApplyInputs, OnUpdateDone with interpolation factor) in a local DeterministicSession at the
  fixed tick rate; `Frame.CalculateChecksum` (CRC64 of the global block and the entity store,
  seeded with the frame number) and `_globals_.Serialize`; the Frame constructor now takes the
  map and RNG seed from the runtime config and sets the start-enabled system bits; RuntimeConfig
  built from the content (map, mode, tables); default keyboard/mouse/gamepad bindings to
  `Quantum.Input` (camera-relative movement, camera sight/orbit); a view layer that instantiates
  each entity's EntityView prefab through `src/engine/scene`, interpolates transforms and orbits a
  camera around the local player's actor, drawn with either renderer; `--headless --ticks N`
  prints entities per component, systems run/failing, events and a checksum every 60 ticks.
  Determinism: two runs give identical checksums (`tests/test_quantum_game.cpp` with content
  built in code; `tools/match_determinism.py` on the real content). Pending: RuntimePlayer and
  `Frame.UpdatePlayerData` (no player data reaches the simulation, so no player actor spawns),
  event confirmation/dispatch (events are counted and released), the checksum/replay snapshot
  buffers, command factories, the Rewired input maps, decoding Quantum assets stored in the
  player's Resources (they have no type trees)

## Known issues

- The staged files lack the player's Resources data files: `globalgamemanagers` lists 1,800+
  Resources paths in GUID-named files in `bin/Data` (842 externals) that aren't in the staged
  base data or asset pack folders. They hold FPMath's large tables (`LUT/FP*`) and Quantum assets
  (`db/configs/simulationconfig`, `deterministicconfig`, `db/elementaldamagetable`,
  `db/ai/aibotsdkconfig`, `db/items/*`...). Matches run with `--approx-math` and class-default
  configs until they are staged (and, for the assets, decodable without type trees).
- On Mesa's software Vulkan driver (lavapipe), line primitives crossing the near plane
  rasterised incorrectly. The placeholder grid avoids it; needs checking on real GPUs.
- Swapchain resize is implemented but has only been exercised at a fixed size so far.
- Buffer/pipeline/texture destruction on Vulkan stalls the GPU; replace with deferred deletion.
  Texture uploads also wait for the queue (fine at load time, not for streaming).
- Lavapipe has no ASTC, so the native ASTC upload path on Vulkan is untested (it falls back to
  software decoding there); the OpenGL native path is tested on llvmpipe.
- Mesh bounds (`m_LocalAABB`) of some skinned meshes are larger than their vertices; the
  submesh bounds match. Not investigated further.
- Scene renderer: most of the staged materials point at shader objects in bundles that aren't
  staged yet, so they're mapped from their own keywords/queue/properties (see the numbers below);
  built-in primitive meshes live in `unity default resources`, which has no type descriptions and
  isn't read yet; blend shapes, particles, lines, trails, sprites, emission, fog and shadows aren't
  drawn; lighting is one directional light plus ambient, evaluated in object space (approximate
  under non-uniform scale). A few NPC rigs skin outside their stored bounds at their prefab pose
  (6 of 116 instances; heroes match exactly) - probably posed rigs, not checked further.

## Next

1. quantum.core: memory/collections (in progress), collision geometry, frame/ECS
2. Matches: RuntimePlayer and Frame player data so player and bot actors spawn; decode the Quantum
   assets in the player's Resources (SimulationConfig, DeterministicConfig, ElementalDamageTable,
   AIBotSDKConfig, db/items...) without type trees (layouts from the metadata); stage the
   `bin/Data` Resources files (FPMath tables)
3. Renderer: level scenes once their bundles are staged, the shader bundle (exact material
   mapping), built-in primitive meshes, animation clips and blend shapes, particles, shadows/fog,
   render targets
4. Asset reader for the game's content files (from the user's own copy)
5. Gameplay systems from quantum.code.dll: damage pipeline and respawning next, then movement,
   weapons and projectiles (need the 3D scene from quantum.core)
4. Asset reader for the game's content files: scenes, textures, meshes
5. Gameplay systems from quantum.code.dll, starting with movement and damage
