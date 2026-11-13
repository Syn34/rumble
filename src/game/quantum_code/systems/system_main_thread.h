// system_main_thread.h - the system base classes of quantum.code.dll: SystemMainThread,
// SystemMainThreadFilter<T>, SystemMainThreadGroup, SystemSignalsOnly and the generic
// component-driven enablers (GenericSystemEnabler / GenericSystemDisabler).
#pragma once

#include "quantum_code/frame.h"
#include "quantum_code/system_base.h"
#include "quantum_core/component_filter_struct.h"

#include <memory>
#include <string>
#include <vector>

namespace Quantum {

// A system whose work is one Update call per frame on the main thread.
class SystemMainThread : public SystemBase {
  public:
    // 0x7cb06dc - Names the profiling sample after the system's type.
    SystemMainThread();
    // 0x7cb066c - Names the profiling sample after `name`.
    explicit SystemMainThread(const std::string& name);

    // 0x7cb0864 - Same as ScheduleUpdate.
    void Schedule(Frame* f) override { ScheduleUpdate(f); }

    // Quantum.SystemMainThread.Update (abstract).
    virtual void Update(Frame* f) = 0;

    const std::string& SampleName() const { return _sampleName; }

  protected:
    // 0x7cb075c - Registers the update as a main-thread task. Tasks run in scheduling
    // order, so the PC build runs it at once (see SystemBase::OnSchedule).
    void ScheduleUpdate(Frame* f) { TaskCallback(f); }
    // 0x7cb0868 - Runs Update; when the simulation commits entity commands between
    // systems, commits them afterwards.
    void TaskCallback(Frame* f);

  private:
    std::string _sampleName;
};

// A main-thread system that runs Update(f, filter) for every entity matching the filter
// struct TFilter (see component_filter_struct.h).
template <class TFilter> class SystemMainThreadFilter : public SystemMainThread {
  public:
    using Filter = TFilter;
    using SystemMainThread::SystemMainThread;

    // Generic (shared instance 0x69f0ee8) - Culled entities are skipped by default.
    virtual bool get_UseCulling() const { return true; }
    // Generic (shared instance 0x69f0ef0) - Components the entities must not have.
    virtual ComponentSet get_Without() const { return ComponentSet{}; }
    // Generic (shared instance 0x69f0efc) - Components of which the entities need one.
    virtual ComponentSet get_Any() const { return ComponentSet{}; }

    // Generic (shared instance 0x69f0f08) - One call per matching entity, in the order of
    // the smallest of the filter's component buffers.
    void Update(Frame* f) final {
        const ComponentSet without = get_Without();
        const ComponentSet any = get_Any();
        ComponentFilterStruct<TFilter> filter = FilterStruct<TFilter>(f, without, any);
        filter.set_UseCulling(get_UseCulling());
        TFilter current{};
        while (filter.Next(current)) {
            Update(f, current);
        }
    }

    // The per-entity work (abstract).
    virtual void Update(Frame* f, TFilter& filter) = 0;
};

// A system that owns child systems and schedules the enabled ones in order.
class SystemMainThreadGroup : public SystemMainThread {
  public:
    // 0x7cb093c - Takes ownership of the children and becomes their parent.
    SystemMainThreadGroup(const std::string& name, std::vector<std::unique_ptr<SystemMainThread>> children);

    // 0x7cb0a08
    std::vector<SystemBase*> get_ChildSystems() const override;

    // 0x7cb0a10 - Schedules every child whose own enable bit is set. An exception thrown
    // by one child is logged and the remaining children still run.
    void Schedule(Frame* f) override;
    // 0x7cb0b88 - Forwards to the children whose own enable bit is set.
    void OnEnabled(Frame* f) override;
    // 0x7cb0c28
    void OnDisabled(Frame* f) override;
    // 0x7cb0cc8 - Groups have no work of their own.
    void Update(Frame* f) override { (void)f; }

  private:
    std::vector<std::unique_ptr<SystemMainThread>> _children;
};

// A system that only reacts to signals: scheduling it does nothing.
class SystemSignalsOnly : public SystemBase {
  public:
    // 0x7cb0cd8
    SystemSignalsOnly() = default;
    // 0x7cb0ccc
    void Schedule(Frame* f) override { (void)f; }
};

// Enables system TSystem when a TComponent is added to any entity.
template <class TComponent, class TSystem>
class GenericSystemEnabler : public SystemSignalsOnly, public ISignalOnComponentAdded<TComponent> {
  public:
    // Generic (e.g. 0x5a45744 for KillZone) - The system type is enabled; enabling an
    // enabled system does nothing.
    void OnAdded(Frame* f, EntityRef entity, TComponent* component) override {
        (void)entity;
        (void)component;
        f->template SystemEnable<TSystem>();
    }
};

// Disables system TSystem when the last TComponent is removed.
template <class TComponent, class TSystem>
class GenericSystemDisabler : public SystemSignalsOnly, public ISignalOnComponentRemoved<TComponent> {
  public:
    // Generic (e.g. 0x5a45288 for KillZone) - Disables the system once no TComponent is
    // left (instances pending removal are not counted).
    void OnRemoved(Frame* f, EntityRef entity, TComponent* component) override {
        (void)entity;
        (void)component;
        if (f->template ComponentCount<TComponent>(false) == 0) {
            f->template SystemDisable<TSystem>();
        }
    }
};

} // namespace Quantum
