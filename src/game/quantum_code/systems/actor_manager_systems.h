// actor_manager_systems.h - spawn points and respawning: ActorManagerSystem (drives the
// ActorManager singleton's respawn timers and spawn point lists) and AISystemsLib.
#pragma once

#include "quantum_code/systems/system_main_thread.h"
#include "quantum_code/systems/transform_systems.h"

namespace Quantum {

class AISystemsLib {
  public:
    // TODO(0x7cc6d84, Quantum.AISystemsLib.SetEnabled): records whether bots are inactive and
    //   enables or disables the bot SDK, navigation and move-forward AI systems; disabling
    //   should first stop every agent's path finding (AINavigationSystem.StopPathFinding, not
    //   reimplemented yet), which is skipped for now.
    static void SetEnabled(Frame* f, bool enabled);
};

class ActorManagerSystem final : public SystemMainThread,
                                 public ISignalOnMapChanged,
                                 public ISignalOnComponentAdded<PlayerActor>,
                                 public ISignalOnComponentAdded<ActorManager>,
                                 public ISignalGameMode_OnPlayerFinished,
                                 public ISignalPlayerActor_OnAfterInitialized {
  public:
    // 0x7cb2b50
    ActorManagerSystem() = default;
    // 0x7cb2834 - Reads the map's spawn points; without a game mode handler the AI runs
    // from the start.
    void OnInit(Frame* f) override;
    // 0x7cb2924
    void Update(Frame* frame) override;
    // 0x7cb299c - A new map means new spawn points.
    void OnMapChanged(Frame* f, AssetRefMap previousMap) override;
    // 0x7cb2a18 - Every player gets an ActorInput.
    void OnAdded(Frame* f, EntityRef entity, PlayerActor* pActor) override;
    // 0x7cb2a70 - The manager is never culled.
    void OnAdded(Frame* f, EntityRef entity, ActorManager* component) override;
    // 0x7cb2a90 - Finished players no longer respawn.
    void GameMode_OnPlayerFinished(Frame* f, EntityRef entity) override;
    // 0x7cb2b2c
    void PlayerActor_OnAfterInitialized(Frame* f, EntityRef actorEntity, PlayerActor* actor) override;
};

} // namespace Quantum
