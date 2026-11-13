// system_base.h - Quantum.SystemBase (quantum.code.dll): the base of every simulation
// system.
//
// Systems form a hierarchy (groups own child systems). Each gets a runtime index, its bit
// in the frame's system enable set (_globals_::Systems); a system runs - and receives
// signals - only while it and all its parents are enabled.
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Quantum {

class Frame;

class SystemBase {
  public:
    // 0x7cafe4c - Also names the system's profiling sample after its type.
    SystemBase();
    virtual ~SystemBase() = default;

    SystemBase(const SystemBase&) = delete;
    SystemBase& operator=(const SystemBase&) = delete;

    // 0x7cafd74 - Throws std::logic_error before the index is assigned (the original reads
    // Nullable<int>.Value).
    int get_RuntimeIndex() const;
    // 0x7ca6a5c - The index can be assigned once; a second assignment is logged and ignored.
    void set_RuntimeIndex(int value);
    bool HasRuntimeIndex() const { return _runtimeIndex.has_value(); }

    // 0x7cafdbc, 0x7cafdc4
    SystemBase* get_ParentSystem() const { return _parentSystem; }
    void set_ParentSystem(SystemBase* value) { _parentSystem = value; }

    // 0x7cafdcc - Plain systems have no children; groups override.
    virtual std::vector<SystemBase*> get_ChildSystems() const { return {}; }
    // 0x7cabe64 - This system, then the hierarchy of each child in order (the original is
    // a lazy iterator; the order is the same).
    std::vector<SystemBase*> get_Hierarchy();

    // 0x7cafe44
    virtual bool get_StartEnabled() const { return true; }

    // 0x7cafed0
    virtual void OnInit(Frame* f) { (void)f; }
    // 0x7cafed4
    virtual void OnEnabled(Frame* f) { (void)f; }
    // 0x7cafed8
    virtual void OnDisabled(Frame* f) { (void)f; }

    // TODO(0x7ca8624, Quantum.SystemBase.OnSchedule): forwards to the system's scheduling
    // entry point; needs the task system.

    // Name used by the profiling sample ("<type>" in the original; here the C++ type).
    const std::string& ScheduleSample() const { return _scheduleSample; }

  private:
    std::optional<int> _runtimeIndex;
    std::string _scheduleSample;
    SystemBase* _parentSystem = nullptr;
};

} // namespace Quantum
