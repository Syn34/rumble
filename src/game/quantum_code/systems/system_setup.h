// system_setup.h - Quantum.SystemSetup: the game's list of simulation systems, plus the
// PC driver's helpers that start the systems and run them each frame.
#pragma once

#include "quantum_code/frame.h"
#include "quantum_code/system_base.h"

#include <memory>
#include <vector>

namespace Quantum {

using SystemList = std::vector<std::unique_ptr<SystemBase>>;

class SystemSetup {
  public:
    // 0x7c12130 - Static set-up of the frame type (component registration), then the root
    // systems: enablers first (so a component added during a frame enables its system
    // before the system list runs), the main systems, the disablers last. The configs are
    // not used.
    static SystemList CreateSystems(RuntimeConfig* gameConfig, SimulationConfig* simulationConfig);
    // 0x7c127e8 - The main systems in scheduling order (groups own further systems).
    static SystemList GetMainSystems();
    // 0x7c121c0
    static SystemList GetEnablerSystems();
    // 0x7c15f80
    static SystemList GetDisablerSystems();
};

// --- PC driver -------------------------------------------------------------------------
// New code standing in for the session layer the original runs the systems from
// (PhotonQuantum's game runner); kept small and in the same order of operations.

// Every system of the hierarchy, depth first, in runtime-index order; assigns the runtime
// indices (system enable bits) on first use.
std::vector<SystemBase*> FlattenSystems(const SystemList& roots);

// Sets the enable bit of every system that starts enabled, then calls OnInit on every
// system and OnEnabled on those enabled in the hierarchy.
void StartSystems(Frame* f);

// Schedules (runs) every enabled root system in order.
void UpdateSystems(Frame* f);

} // namespace Quantum
