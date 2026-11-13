// frame_context.h - Quantum.Core.FrameContext: the state shared by all frames of one
// simulation.
//
// Frames are snapshots and come and go (prediction, rollback); what does not belong to a
// snapshot lives here: the native allocator, the group of frame heaps every frame takes
// its heap from, the heap allocation tracker, the event pools and the list of raised
// events, temporary per-frame allocations, the prediction (culling) area, the shared
// checksum serializer, and the physics/navigation engines and task system.
#pragma once

#include "photon_deterministic/deterministic_platform_info.h"
#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "photon_deterministic/native.h"
#include "quantum_core/events.h"
#include "quantum_core/frame_checksumer_bit_stream.h"
#include "quantum_core/frame_heap.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/heap.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace Photon::Deterministic {
class DeterministicCommandSerializer;
class IDeterministicPlatformTaskRunner;
} // namespace Photon::Deterministic

namespace Quantum::Core {

class FrameBase;
class FrameThreadSafe;

class FrameContext {
  public:
    using NativeAllocator = Photon::Deterministic::Native::Allocator;

    // Everything a simulation is created with (Quantum.Core.FrameContext.Args).
    struct Args {
        // TODO(0x7d1e568, Quantum.Core.FrameContext..ctor): AssetDatabase, NavigationConfig,
        // PhysicsConfig, AssetSerializer and InitialDynamicAssets are not available yet.
        Photon::Deterministic::DeterministicPlatformInfo* PlatformInfo = nullptr;
        std::function<bool(int)> IsLocalPlayer;
        bool IsServer = false;
        Photon::Deterministic::DeterministicCommandSerializer* CommandSerializer = nullptr;
        Quantum::Allocator::Heap::Config HeapConfig;
        Quantum::Allocator::HeapTrackingMode HeapTrackingMode = Quantum::Allocator::HeapTrackingMode::Disabled;
        bool UseCullingArea = false;
        bool UsePhysics2D = false;
        bool UsePhysics3D = false;
        bool UseNavigation = false;
        bool UseSharedChecksumSerialized = false;
    };

    struct TempAllocation {
        void* Ptr = nullptr;
    };

    // The prediction area: entities outside it are culled on predicted frames. The raw
    // copies avoid FP arithmetic in the hot checks.
    struct CullingArea {
        std::int64_t RawRadius = 0;
        std::int64_t RawRadiusSqr = 0;
        std::int64_t RawCenterX = 0;
        std::int64_t RawCenterY = 0;
        std::int64_t RawCenterZ = 0;
        Photon::Deterministic::FPVector3 Center;
        Photon::Deterministic::FP Radius;
    };

    // Number of event ids the pool table starts with.
    static constexpr int InitialEventPoolCount = 1024;

    // TODO(0x7d1e568, Quantum.Core.FrameContext..ctor): asset database, task context,
    // profiler, physics layers and engines, navigation and the Culling object are missing.
    // Done: allocator and task runner from the platform info, the heap group (when the
    // heap config asks for at least one heap), event list and pools, the heap tracker for
    // the configured mode, the prediction area and the shared checksum serializer.
    explicit FrameContext(const Args& args);
    // Runs Dispose.
    ~FrameContext();

    FrameContext(const FrameContext&) = delete;
    FrameContext& operator=(const FrameContext&) = delete;

    // 0x7d1e510, 0x7d1e518, 0x7d1e530, 0x7d1e550, 0x7d1e558, 0x7d1e560
    NativeAllocator* get_Allocator() const { return _allocator; }
    Photon::Deterministic::IDeterministicPlatformTaskRunner* get_TaskRunner() const {
        return _taskRunner;
    }
    EventList* get_Events() const { return _events.get(); }
    bool get_UseSharedChecksumSerializer() const { return _useSharedChecksumSerializer; }
    FrameSerializer* get_SharedChecksumSerializer() const { return _sharedChecksumSerializer.get(); }
    Quantum::Allocator::HeapTrackingMode get_HeapTrackingMode() const { return _heapTrackingMode; }
    bool get_IsServer() const { return _isServer; }
    Quantum::Allocator::Heap::Group* HeapGroup() const { return _heapGroup; }
    Photon::Deterministic::DeterministicCommandSerializer* CommandSerializer() const {
        return _commandSerializer;
    }
    bool IsLocalPlayer(int player) const { return _isLocalPlayer ? _isLocalPlayer(player) : false; }
    const CullingArea& get_CullingArea() const { return _cullingArea; }

    // TODO(0x7d1e4f8, Quantum.Core.FrameContext.get_Physics2D): needs PhysicsEngine2D.
    // TODO(0x7d1e500, Quantum.Core.FrameContext.get_Physics3D): needs PhysicsEngine3D.
    // TODO(0x7d1e508, Quantum.Core.FrameContext.get_Navigation): needs Navigation.
    // TODO(0x7d1e520, Quantum.Core.FrameContext.get_TaskContext): needs Task.TaskContext.
    // TODO(0x7d1e528, Quantum.Core.FrameContext.get_ProfilerContext): needs ProfilerContext.
    // TODO(0x7d1e538, Quantum.Core.FrameContext.get_Culling): needs FrameBase.Culling.
    // TODO(0x7d1e540, Quantum.Core.FrameContext.get_AssetDB): needs AssetDB.
    // TODO(0x7d1e548, Quantum.Core.FrameContext.get_AssetSerializer): needs IAssetSerializer.

    // TODO(0x7d1eb94, Quantum.Core.FrameContext.OnSimulationBegin): starts the profiler and
    // task context; both are missing, so this does nothing yet.
    void OnSimulationBegin() {}
    // TODO(0x7d1ebc4, Quantum.Core.FrameContext.OnSimulationEnd): ends the task context
    // frame; missing, so this does nothing yet.
    void OnSimulationEnd() {}
    // TODO(0x7d1ebdc, Quantum.Core.FrameContext.OnFrameSimulationBegin): forward to
    // PhysicsEngine2D/3D.OnFrameSimulationBegin once they exist.
    void OnFrameSimulationBegin(FrameBase* f);
    // TODO(0x7d1ec28, Quantum.Core.FrameContext.OnFrameSimulationEnd): the physics
    // engines' end-of-frame work is missing; FreeTempAllocations is done.
    void OnFrameSimulationEnd();

    // 0x7d1c2ac - Allocator memory released at the end of the simulated frame.
    void* TempAllocate(int count, int alignment);
    // 0x7d1eed8 - Zeroed variant.
    void* TempAllocateAndClear(int count, int alignment);
    // 0x7d1ec5c - Frees all temporary allocations. A failing free is logged and the rest
    // are still freed.
    void FreeTempAllocations();

    // TODO(0x7d1f054, Quantum.Core.FrameContext.Dispose): disposing the engines, asset
    // database resources and task context is missing. Done: frees temporary memory and the
    // heap group, disposes the allocator, deletes pooled events. Safe to call twice.
    void Dispose();

    // 0x7d1a7c8, 0x7d1a7d0
    void ReleaseHeap(Quantum::Allocator::Heap* heap) { _heapGroup->Release(heap); }
    Quantum::Allocator::Heap* ReserveHeap() { return _heapGroup->Reserve(); }

    // 0x4f01000 (instance for object) - A pooled event object for `id` (a new T if the
    // pool is empty). The pool table grows to twice `id` when needed.
    template <class T> T* AcquireEvent(int id) {
        EnsureEventPool(id);
        EventBase* pooled = _eventPools[static_cast<std::size_t>(id)];
        if (pooled == nullptr) {
            return new T();
        }
        _eventPools[static_cast<std::size_t>(id)] = pooled->Next;
        pooled->Next = nullptr;
        return static_cast<T*>(pooled);
    }
    // 0x7d1f468 - Calls OnReleased and puts the event back into its id's pool (the pool
    // owns it from then on).
    void ReleaseEvent(EventBase* ev);

    // 0x7d1eb5c
    void SetPredictionArea(Photon::Deterministic::FPVector3 position,
                           Photon::Deterministic::FP radius);
    // 0x7d1f558 - Always true without culling or on verified frames; otherwise whether the
    // position lies strictly inside the area.
    bool InPredictionArea(const FrameBase* f, Photon::Deterministic::FPVector3 position) const;
    // 0x7d1df9c - 2D form; the position's Y is compared with the area's Z.
    bool InPredictionArea(const FrameBase* f, Photon::Deterministic::FPVector2 position) const;

    // 0x7d1f5bc - Gives the frame a tracked heap using this context's tracker.
    void HookHeapAllocTrackerToFrame(FrameBase* f) const;

    const std::shared_ptr<Quantum::Allocator::IHeapAllocationTracker>& HeapTracker() const {
        return _heapTracker;
    }

  private:
    void EnsureEventPool(int id);

    std::function<bool(int)> _isLocalPlayer;
    CullingArea _cullingArea;
    Quantum::Allocator::Heap::Group* _heapGroup = nullptr;
    Photon::Deterministic::DeterministicCommandSerializer* _commandSerializer = nullptr;
    // Stands in for the Culling object (created when UseCullingArea is set); only its
    // presence matters for the prediction-area checks.
    bool _culling = false;
    std::vector<EventBase*> _eventPools;
    std::vector<TempAllocation> _tempAllocations;
    std::mutex _tempAllocationsLock;
    std::unique_ptr<EventList> _events;
    NativeAllocator* _allocator = nullptr;
    Photon::Deterministic::IDeterministicPlatformTaskRunner* _taskRunner = nullptr;
    std::unique_ptr<FrameChecksumerBitStream> _checksumStream;
    std::unique_ptr<FrameSerializer> _sharedChecksumSerializer;
    Quantum::Allocator::HeapTrackingMode _heapTrackingMode = Quantum::Allocator::HeapTrackingMode::Disabled;
    std::shared_ptr<Quantum::Allocator::IHeapAllocationTracker> _heapTracker;
    bool _useSharedChecksumSerializer = false;
    bool _isServer = false;
    bool _disposed = false;
};

} // namespace Quantum::Core
