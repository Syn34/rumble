// core_systems.h - the framework systems that ship as source with the game
// (namespace Quantum.Core in quantum.code.dll): culling, physics, navigation, map
// entities, player connection tracking and the debug-command hook.
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum::Core {

// Culls entities for the 2D physics/view (not part of the game's system list).
class CullingSystem2D final : public SystemBase {
  public:
    // TODO(0x7c169c4, Quantum.Core.CullingSystem2D.Schedule): schedules Culling.Schedule2D
    // of the frame context; needs quantum.core's Culling.
    void Schedule(Frame* f) override { (void)f; }
};

class CullingSystem3D final : public SystemBase {
  public:
    // 0x7c165a8
    CullingSystem3D() = default;
    // TODO(0x7c16a14, Quantum.Core.CullingSystem3D.Schedule): schedules Culling.Schedule3D
    // of the frame context; needs quantum.core's Culling.
    void Schedule(Frame* f) override { (void)f; }
};

class PhysicsSystem2D final : public SystemBase {
  public:
    // TODO(0x7c16b6c, Quantum.Core.PhysicsSystem2D.Schedule): registers itself as the
    // scene's callback target and runs Scene2D.Update with the frame's delta time; needs
    // quantum.core's Physics2D scene.
    void Schedule(Frame* f) override { (void)f; }
};

// Steps the 3D physics scene and forwards its callbacks as frame signals.
class PhysicsSystem3D final : public SystemBase {
  public:
    // 0x7c165b0
    PhysicsSystem3D() = default;
    // TODO(0x7c16bfc, Quantum.Core.PhysicsSystem3D.OnInit): Scene3D.InitScene(f, f.Map) on
    // the frame's 3D physics engine when it exists; needs quantum.core's Scene3D.
    void OnInit(Frame* f) override { (void)f; }
    // TODO(0x7c16c68, Quantum.Core.PhysicsSystem3D.Schedule): makes this system the scene's
    // callback target and runs Scene3D.Update with the frame's delta time; needs
    // quantum.core's Scene3D.
    void Schedule(Frame* f) override { (void)f; }

    // Physics callbacks (the scene calls them with the frame being simulated); each one
    // raises the frame signal of the same name. They are not signal receivers themselves.
    // 0x7c16d14
    void OnCollision3D(Core::FrameBase* f, const CollisionInfo3D& info);
    // 0x7c16dbc
    void OnCollisionEnter3D(Core::FrameBase* f, const CollisionInfo3D& info);
    // 0x7c16e64
    void OnCollisionExit3D(Core::FrameBase* f, const ExitInfo3D& info);
    // 0x7c16f04
    void OnTrigger3D(Core::FrameBase* f, const TriggerInfo3D& info);
    // 0x7c16fac
    void OnTriggerEnter3D(Core::FrameBase* f, const TriggerInfo3D& info);
    // 0x7c17054
    void OnTriggerExit3D(Core::FrameBase* f, const ExitInfo3D& info);
};

// Updates navmesh agents and forwards the navigation callbacks as frame signals.
class NavigationSystem final : public SystemBase {
  public:
    // 0x7c1660c
    NavigationSystem() = default;
    // TODO(0x7c166e0, Quantum.Core.NavigationSystem.Schedule): remembers the frame and runs
    // the navigation Updater (f.Navigation) with the frame's delta time, this system
    // receiving the agent callbacks; needs quantum.core's navigation Updater.
    void Schedule(Frame* f) override { _currentFrame = f; }

    // Callbacks of the navigation updater for the frame being updated.
    // 0x7c16764
    void OnWaypointReached(EntityRef entity, FPVector3 waypoint, WaypointFlag waypointFlags, bool& resetAgent);
    // 0x7c167d4
    void OnSearchFailed(EntityRef entity, bool& resetAgent);
    // 0x7c167f0
    void OnMoveAgent(EntityRef entity, FPVector2 desiredDirection);

  private:
    Frame* _currentFrame = nullptr;
};

// Creates the map's entities, and replaces them when the map changes.
class EntityPrototypeSystem final : public SystemSignalsOnly {
  public:
    // 0x7c16614
    EntityPrototypeSystem() = default;
    // 0x7c1680c - Same as a change from no map.
    void OnInit(Frame* f) override { OnMapChanged(f, AssetRefMap{}); }
    // 0x7c16814 - If there was a previous map, its entities (those with a MapEntityLink)
    // are destroyed; then the current map's entities are created.
    void OnMapChanged(Frame* f, AssetRefMap previousMap);
};

// Raises OnPlayerConnected / OnPlayerDisconnected when a player's presence flag changes.
class PlayerConnectedSystem final : public SystemMainThread {
  public:
    // 0x7c1661c
    PlayerConnectedSystem() = default;
    // 0x7c16a64 - Only verified frames are looked at.
    void Update(Frame* f) override;
};

class DebugCommand {
  public:
    // The system the debug command needs; it has no work.
    class InternalSystem final : public SystemBase {
      public:
        // 0x7c16680
        InternalSystem() = default;
        // 0x7c166d4
        void Schedule(Frame* f) override { (void)f; }
    };

    // 0x7c165b8
    static std::unique_ptr<SystemBase> CreateSystem() { return std::make_unique<InternalSystem>(); }
    // TODO(0x7c16624, Quantum.Core.DebugCommand.CreateCommand): creates the debug command
    // object (entity create/destroy/component edits from the editor); needs the command
    // serialisation of components.
};

} // namespace Quantum::Core
