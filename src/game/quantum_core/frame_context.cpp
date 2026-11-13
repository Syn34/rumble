#include "frame_context.h"

#include "photon_deterministic/log.h"
#include "quantum_core/frame_base.h"

#include <exception>

namespace Quantum::Core {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;

FrameContext::FrameContext(const Args& args) {
    if (args.PlatformInfo != nullptr) {
        _allocator = args.PlatformInfo->Allocator;
        _taskRunner = args.PlatformInfo->TaskRunner;
    }
    // (Missing, see the header: the asset database - which keeps a back reference to the
    // context -, the initial dynamic assets and the asset serializer.)
    _commandSerializer = args.CommandSerializer;

    if (args.HeapConfig.HEAP_COUNT >= 1) {
        _events = std::make_unique<EventList>();
        _heapGroup = Quantum::Allocator::Heap::Group::Allocate(args.HeapConfig, *_allocator);
        _heapTrackingMode = args.HeapTrackingMode;
        _heapTracker = Quantum::Allocator::FrameHeap::GetAllocationTrackerForMode(_heapTrackingMode);
        // (Missing, see the header: the task context, the physics layer table,
        // PhysicsEngine2D/3D and Navigation are created here when enabled.)
        if (args.UseCullingArea) {
            _culling = true;
        }
        _eventPools.assign(InitialEventPoolCount, nullptr);
    }

    // The prediction area starts centred on the origin with the largest radius.
    _cullingArea.Center = FPVector3::Zero();
    _cullingArea.RawCenterX = _cullingArea.Center.X.RawValue();
    _cullingArea.RawCenterY = _cullingArea.Center.Y.RawValue();
    _cullingArea.RawCenterZ = _cullingArea.Center.Z.RawValue();
    _cullingArea.Radius = FP::FromRaw(0x7FFFFFFF);
    _cullingArea.RawRadius = 0x7FFFFFFF;
    _cullingArea.RawRadiusSqr = (std::int64_t{0x7FFFFFFF} * 0x7FFFFFFF) >> 16;

    _isLocalPlayer = args.IsLocalPlayer;
    _isServer = args.IsServer;
    _useSharedChecksumSerializer = args.UseSharedChecksumSerialized;
    _checksumStream = std::make_unique<FrameChecksumerBitStream>();
    _sharedChecksumSerializer = std::make_unique<FrameSerializer>(
        Photon::Deterministic::DeterministicFrameSerializeMode::Serialize, nullptr,
        _checksumStream.get());
}

FrameContext::~FrameContext() { Dispose(); }

void FrameContext::OnFrameSimulationBegin(FrameBase* /*f*/) {}

void FrameContext::OnFrameSimulationEnd() {
    // (PhysicsEngine2D/3D.OnFrameSimulationEnd go here once the engines exist.)
    FreeTempAllocations();
}

void* FrameContext::TempAllocate(int count, int alignment) {
    void* memory = _allocator->Alloc(count, alignment);
    std::lock_guard<std::mutex> guard(_tempAllocationsLock);
    _tempAllocations.push_back(TempAllocation{memory});
    return memory;
}

void* FrameContext::TempAllocateAndClear(int count, int alignment) {
    void* memory = _allocator->AllocAndClear(count, alignment);
    std::lock_guard<std::mutex> guard(_tempAllocationsLock);
    _tempAllocations.push_back(TempAllocation{memory});
    return memory;
}

void FrameContext::FreeTempAllocations() {
    std::lock_guard<std::mutex> guard(_tempAllocationsLock);
    for (const TempAllocation& allocation : _tempAllocations) {
        try {
            _allocator->Free(allocation.Ptr);
        } catch (const std::exception& e) {
            Photon::Deterministic::Log::Error("{0}", e.what());
        }
    }
    _tempAllocations.clear();
}

void FrameContext::Dispose() {
    if (_disposed) {
        return;
    }
    _disposed = true;
    if (_allocator != nullptr) {
        FreeTempAllocations();
    }
    // (Culling, Navigation, PhysicsEngine2D/3D, the asset database's resources and the
    // task context are disposed here once they exist.)
    _culling = false;
    if (_heapGroup != nullptr) {
        Quantum::Allocator::Heap::Group::Free(_heapGroup, *_allocator);
        _heapGroup = nullptr;
    }
    if (_allocator != nullptr) {
        _allocator->Dispose();
    }
    // (The original also disposes the task runner if it is IDisposable; the runner is not
    // owned here and has no such interface in this project.)
    for (EventBase*& head : _eventPools) {
        while (head != nullptr) {
            EventBase* next = head->Next;
            delete head;
            head = next;
        }
    }
}

void FrameContext::EnsureEventPool(int id) {
    if (static_cast<int>(_eventPools.size()) <= id) {
        _eventPools.resize(static_cast<std::size_t>(id) * 2, nullptr);
    }
}

void FrameContext::ReleaseEvent(EventBase* ev) {
    ev->OnReleased();
    EnsureEventPool(ev->Id);
    ev->Next = _eventPools[static_cast<std::size_t>(ev->Id)];
    _eventPools[static_cast<std::size_t>(ev->Id)] = ev;
}

void FrameContext::SetPredictionArea(FPVector3 position, FP radius) {
    _cullingArea.Center = position;
    _cullingArea.Radius = radius;
    _cullingArea.RawRadius = radius.RawValue();
    _cullingArea.RawRadiusSqr = (radius.RawValue() * radius.RawValue()) >> 16;
    _cullingArea.RawCenterX = position.X.RawValue();
    _cullingArea.RawCenterY = position.Y.RawValue();
    _cullingArea.RawCenterZ = position.Z.RawValue();
}

bool FrameContext::InPredictionArea(const FrameBase* f, FPVector3 position) const {
    if (!_culling || f->Verified) {
        return true;
    }
    const std::int64_t dx = position.X.RawValue() - _cullingArea.RawCenterX;
    const std::int64_t dy = position.Y.RawValue() - _cullingArea.RawCenterY;
    const std::int64_t dz = position.Z.RawValue() - _cullingArea.RawCenterZ;
    const std::int64_t distanceSqr = ((dy * dy) >> 16) + ((dx * dx) >> 16) + ((dz * dz) >> 16);
    return _cullingArea.RawRadiusSqr > distanceSqr;
}

bool FrameContext::InPredictionArea(const FrameBase* f, FPVector2 position) const {
    if (!_culling || f->Verified) {
        return true;
    }
    const std::int64_t dx = position.X.RawValue() - _cullingArea.RawCenterX;
    const std::int64_t dz = position.Y.RawValue() - _cullingArea.RawCenterZ;
    const std::int64_t distanceSqr = ((dz * dz) >> 16) + ((dx * dx) >> 16);
    return _cullingArea.RawRadiusSqr > distanceSqr;
}

void FrameContext::HookHeapAllocTrackerToFrame(FrameBase* f) const {
    f->InitTrackedHeap(_heapTracker);
}

} // namespace Quantum::Core
