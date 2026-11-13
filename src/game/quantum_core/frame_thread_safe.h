// frame_thread_safe.h - Quantum.FrameThreadSafe: a frame handle for worker threads.
//
// Multithreaded systems receive a FrameThreadSafe instead of the frame: the same frame
// plus the index of the thread it is used on (0 = main thread), restricted to operations
// that are safe to run concurrently (reads, and writes to components the job owns).
#pragma once

#include "quantum_core/component_iterators.h"
#include "quantum_core/frame_base.h"

namespace Quantum {

struct FrameThreadSafe {
    Core::FrameBase* _frame = nullptr;
    int _thread = 0;

    FrameThreadSafe() = default;
    // 0x7cefab0
    FrameThreadSafe(Core::FrameBase* frame, int thread) : _frame(frame), _thread(thread) {}

    // 0x7cef9c8
    int get_Thread() const { return _thread; }
    // 0x7cef9d0
    bool get_IsMainThread() const { return _frame != nullptr && _thread == 0; }
    // 0x7cef9f0
    bool get_IsVerified() const { return _frame->Verified; }
    // 0x7cefa08
    Core::FrameContext* get_Context() const { return _frame->get_Context(); }
    // 0x7cefa3c
    Photon::Deterministic::FP get_DeltaTime() const { return _frame->get_DeltaTime(); }
    // 0x7cefa20
    Map* get_Map() const { return _frame->get_Map(); }
    // TODO(0x7cefa5c, Quantum.FrameThreadSafe.get_Physics3D): the per-thread
    // PhysicsEngine3D.ThreadSafeApi; needs the 3D physics engine.

    // 0x7cefad8 - Address of a heap block.
    void* Void(Ptr ptr) const { return _frame->get_TrackedHeap()->Void(ptr); }
    // 0x7cefb04 - Back to the frame.
    operator Core::FrameBase*() const { return _frame; }
    // 0x7ce3448 - The main-thread handle of a frame.
    static FrameThreadSafe FromFrame(Core::FrameBase* frame) { return FrameThreadSafe(frame, 0); }

    // 0x4f0d100 (instance for Transform2DVertical)
    template <class T> bool Has(EntityRef entityRef) const { return _frame->Has<T>(entityRef); }
    // 0x7cefb08
    bool Exists(EntityRef entityRef) const { return _frame->Exists(entityRef); }
    // 0x7cefb54
    bool IsCulled(EntityRef entityRef) const { return _frame->IsCulled(entityRef); }
    // 0x4f0d720 (instance for Transform3D)
    template <class T> bool TryGetPointer(EntityRef entityRef, T*& value) const {
        return _frame->TryGetPointer<T>(entityRef, value);
    }
    // 0x4f0d024 (instance for Transform3D)
    template <class T> T* GetPointer(EntityRef entityRef) const {
        return _frame->GetPointer<T>(entityRef);
    }
    // 0x4f0ce7c (instance for PhysicsCollider2D)
    template <class T> ComponentBlockIterator<T> GetComponentBlockIterator() const {
        return _frame->GetComponentBlockIterator<T>();
    }
    // 0x4f0d270 (instance for AssetRefConsideration)
    template <class T> Collections::QList<T> ResolveList(Collections::QListPtr<T> ptr) const {
        return _frame->ResolveList(ptr);
    }
    // 0x4f0d194 (instance for <AssetRefConsideration, FP>)
    template <class K, class V>
    Collections::QDictionary<K, V> ResolveDictionary(Collections::QDictionaryPtr<K, V> ptr) const {
        return _frame->ResolveDictionary(ptr);
    }
    // 0x4f0ce28 (shared generic code) - FrameBase::FindAsset with this thread's loading
    // path (worker threads queue loads for the main thread).
    template <class T> T* FindAsset(AssetGuid guid) const {
        return _frame->FindAssetInternal<T>(guid, _thread == 0);
    }
    template <class T> T* FindAsset(AssetRef assetRef) const { return FindAsset<T>(assetRef.Id); }
    // 0x4f0d34c (shared generic code)
    template <class T> bool TryFindAsset(AssetGuid guid, T*& asset) const {
        return _frame->TryFindAssetInternal<T>(guid, _thread == 0, asset);
    }
};

} // namespace Quantum
