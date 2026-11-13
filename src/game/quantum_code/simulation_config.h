// simulation_config.h - Quantum.SimulationConfig (quantum.code.dll): the asset with the
// simulation's framework settings (entity capacities, threads, heap sizes...).
#pragma once

#include "quantum_code/enums.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/entities_config.h"
#include "quantum_core/heap.h"

#include "photon_deterministic/fp.h"

#include <memory>

namespace Quantum {

class SimulationConfig : public AssetObject {
  public:
    static constexpr long long DEFAULT_ID = 5;

    // 0x7ca3cd8 - Defaults: scene auto-loading mode 2, two threads, three seconds of
    // checksum snapshot history, heaps of 256 pages of 32 KiB.
    SimulationConfig() = default;

    // Navigation and physics engine configurations: owned by those engines' work and not
    // reimplemented here yet.
    std::shared_ptr<void> Navigation;
    std::shared_ptr<void> Physics;
    ::Quantum::Core::EntitiesConfig Entities;
    int AutoLoadSceneFromMap = 2;
    SimulationUpdateTime DeltaTimeType{};
    int ThreadCount = 2;
    Photon::Deterministic::FP ChecksumSnapshotHistoryLengthSeconds = Photon::Deterministic::FP::FromRaw(3 << 16);
    SimulationConfigChecksumErrorDumpOptions ChecksumErrorDumpOptions{};
    ::Quantum::Allocator::HeapTrackingMode HeapTrackingMode = ::Quantum::Allocator::HeapTrackingMode::Disabled;
    int HeapPageShift = 15;
    int HeapPageCount = 256;
    int HeapExtraCount = 0;
};

} // namespace Quantum
