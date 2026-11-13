#include "test_framework.h"

#include "math_tables.h"
#include "photon_deterministic/deterministic_platform_info.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_code/frame.h"
#include "quantum_code/systems/all_systems.h"
#include "quantum_code/systems/enablers.h"
#include "quantum_code/systems/system_setup.h"
#include "quantum_core/frame_context.h"

#include <memory>
#include <vector>

using namespace Quantum;
using Photon::Deterministic::FP;
using Photon::Deterministic::SystemAllocator;
using Quantum::Allocator::Heap;
using Quantum::Core::FrameContext;

namespace {

// A frame running the given root systems.
struct SystemsSimulation {
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<FrameContext> context;
    Photon::Deterministic::DeterministicSessionConfig session;
    RuntimeConfig runtime;
    SimulationConfig simulation;
    SystemList roots;
    std::vector<SystemBase*> all;
    std::unique_ptr<Frame> frame;

    explicit SystemsSimulation(SystemList systems) : roots(std::move(systems)) {
        ComponentTypeId<>::ResetForTests();
        platform.Allocator = &allocator;
        FrameContext::Args args;
        args.PlatformInfo = &platform;
        args.HeapConfig = Heap::Config(12, 256, 4);
        context = std::make_unique<FrameContext>(args);
        session.PlayerCount = 4;
        simulation.Entities.DefaultEntityCapacity = 64;
        simulation.Entities.DefaultComponentBlockCapacity = 8;
        all = FlattenSystems(roots);
        std::vector<SystemBase*> rootPointers;
        for (const auto& root : roots) {
            rootPointers.push_back(root.get());
        }
        // 1/60 s
        frame = std::make_unique<Frame>(context.get(), all, rootPointers, &session, &runtime, &simulation,
                                        FP::FromRaw(1092));
    }

    ~SystemsSimulation() {
        frame->Free();
        frame.reset();
        context->Dispose();
        ComponentTypeId<>::ResetForTests();
    }

    template <class T> T* Find() {
        for (SystemBase* system : all) {
            if (auto* typed = dynamic_cast<T*>(system)) {
                return typed;
            }
        }
        return nullptr;
    }
};

template <class... TSystems> SystemList MakeSystems() {
    SystemList list;
    (list.push_back(std::make_unique<TSystems>()), ...);
    return list;
}

} // namespace

TEST_CASE("quantum.code systems: the system list has the original's shape") {
    SystemList roots = SystemSetup::CreateSystems(nullptr, nullptr);
    CHECK(roots.size() == 16u + 154u + 16u);
    std::vector<SystemBase*> all = FlattenSystems(roots);
    // Groups contribute their children: 11 + 12 + 13 + 5 + 2 + 22.
    CHECK(all.size() == roots.size() + 65u);
    CHECK(dynamic_cast<KillZoneSystemEnabler*>(all[0]) != nullptr);
    CHECK(dynamic_cast<Core::CullingSystem3D*>(all[16]) != nullptr);
    CHECK(all[16]->get_RuntimeIndex() == 16);
    // Systems driven by an enabler start disabled.
    int startDisabled = 0;
    for (SystemBase* system : all) {
        startDisabled += system->get_StartEnabled() ? 0 : 1;
    }
    CHECK(startDisabled == 34);
    ComponentTypeId<>::ResetForTests();
}

TEST_CASE("quantum.code systems: start, enablers and group scheduling") {
    SystemsSimulation sim(SystemSetup::CreateSystems(nullptr, nullptr));
    Frame& f = *sim.frame;
    StartSystems(&f);
    auto* moveTo = sim.Find<MoveToSystem>();
    CHECK(moveTo != nullptr);
    CHECK(!f.SystemIsEnabledSelf(moveTo));
    const EntityRef entity = f.Create();
    f.Add<MoveTo>(entity);
    CHECK(f.SystemIsEnabledSelf(moveTo));
    f.Remove<MoveTo>(entity);
    CHECK(!f.SystemIsEnabledSelf(moveTo));
    // A whole frame of systems runs without touching anything unexpected.
    UpdateSystems(&f);
    CHECK(f.Exists(entity));
}
