// dispatcher_base.h - Quantum.DispatcherBase: type-indexed publish/subscribe used for the
// simulation's callbacks and events on the presentation side.
//
// Only the parts needed by the frame core are implemented so far; the subscription
// machinery (delegates per dispatchable type, listener liveness, one-shot and filtered
// handlers) is listed below as TODO.
#pragma once

#include <cstdint>

namespace Quantum {

class DispatcherBase {
  public:
    enum class ListenerStatus : int {
        Active = 0,
        Inactive = 1,
        Dead = 2,
    };

    virtual ~DispatcherBase() = default;

    // 0x7ced7fc - Next dispatcher in a chain.
    DispatcherBase* get_Next() const { return _next; }
    void set_Next(DispatcherBase* next) { _next = next; }

    // 0x7cee268 - True if every bit of `flag` is set in `flags`.
    static bool CheckFlag(std::uint32_t flags, std::uint32_t flag) { return (flag & ~flags) == 0; }

    // 0x7cee274 - The base implementation only knows that a missing listener is dead;
    // subclasses check liveness of engine objects.
    virtual ListenerStatus GetListenerStatus(const void* listener, std::uint32_t /*flags*/) const {
        return listener == nullptr ? ListenerStatus::Dead : ListenerStatus::Active;
    }

    // TODO(0x7ced804, Quantum.DispatcherBase..ctor): validates the type->index map (every
    // type must implement IDispatchable, ids unique and in range), creates the meta table.
    // The subscription machinery needs a C++ model of delegates and listeners:
    // TODO(0x7cedcd8, Quantum.DispatcherBase.Unsubscribe)
    // TODO(0x7cedd28, Quantum.DispatcherBase.Clear)
    // TODO(0x7cedebc, Quantum.DispatcherBase.RemoveDeadListners)
    // TODO(0x7cee0a0, Quantum.DispatcherBase.UnsubscribeListener)
    // TODO(0x7cee284, Quantum.DispatcherBase.InvokeMeta)
    // TODO(0x7cee92c, Quantum.DispatcherBase.UnsubscribeInternal(meta, listener, handler))
    // TODO(0x7cedce0, Quantum.DispatcherBase.UnsubscribeInternal(meta, id))
    // TODO(0x7cedf2c, Quantum.DispatcherBase.RemoveDeadListners(meta))
    // TODO(0x4d73660, Quantum.DispatcherBase.GetIndex<T>)
    // TODO(0x4d73778, Quantum.DispatcherBase.Subscribe<T>)
    // TODO(0x4d73898, Quantum.DispatcherBase.SubscribeInternal<T>)
    // TODO(0x4d73bdc, Quantum.DispatcherBase.SubscribeManual<T>)
    // TODO(0x4d73d48, Quantum.DispatcherBase.SubscribeManualInternal<T>)
    // TODO(0x4d73ef4, Quantum.DispatcherBase.UnsubscribeListener<T>)
    // TODO(0x7cee828, Quantum.DispatcherBase.DispatchableMeta.get_Count)
    // TODO(0x7ceecc8, Quantum.DispatcherBase.DispatchableMeta.Add)
    // TODO(0x7cedd88, Quantum.DispatcherBase.DispatchableMeta.Clear)
    // TODO(0x7ceeb20, Quantum.DispatcherBase.DispatchableMeta.NullifyAt)
    // TODO(0x7cee96c, Quantum.DispatcherBase.DispatchableMeta.Remove(id))
    // TODO(0x7cee128, Quantum.DispatcherBase.DispatchableMeta.Remove(handler, listener))
    // TODO(0x7ceea3c, Quantum.DispatcherBase.DispatchableMeta.RemoveAt)
    // TODO(0x7cee870, Quantum.DispatcherBase.DispatchableMeta.SetFlag)
    // TODO(0x7ceef64, Quantum.DispatcherBase.DispatchableMeta..ctor)
    // TODO(0x7ceec18, Quantum.DispatcherBase.HandlerDisposable.Dispose)

  private:
    DispatcherBase* _next = nullptr;
};

} // namespace Quantum
