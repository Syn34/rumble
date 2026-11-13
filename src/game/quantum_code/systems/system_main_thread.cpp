// system_main_thread.cpp - SystemMainThread, SystemMainThreadGroup.
#include "quantum_code/systems/system_main_thread.h"

#include "photon_deterministic/log.h"

#include <exception>

namespace Quantum {

SystemMainThread::SystemMainThread() : _sampleName("SystemMainThread") {}

SystemMainThread::SystemMainThread(const std::string& name) : _sampleName(name) {}

void SystemMainThread::TaskCallback(Frame* f) {
    Update(f);
    if (f->get_CommitCommandsMode() == CommitCommandsModes::InBetweenSystems) {
        f->CommitAllCommands();
    }
}

SystemMainThreadGroup::SystemMainThreadGroup(const std::string& name,
                                             std::vector<std::unique_ptr<SystemMainThread>> children)
    : SystemMainThread(name), _children(std::move(children)) {
    for (const auto& child : _children) {
        child->set_ParentSystem(this);
    }
}

std::vector<SystemBase*> SystemMainThreadGroup::get_ChildSystems() const {
    std::vector<SystemBase*> result;
    result.reserve(_children.size());
    for (const auto& child : _children) {
        result.push_back(child.get());
    }
    return result;
}

void SystemMainThreadGroup::Schedule(Frame* f) {
    for (const auto& child : _children) {
        if (!f->SystemIsEnabledSelf(child.get())) {
            continue;
        }
        try {
            child->OnSchedule(f);
        } catch (const std::exception& e) {
            Photon::Deterministic::Log::Exception(e);
        }
    }
}

void SystemMainThreadGroup::OnEnabled(Frame* f) {
    for (const auto& child : _children) {
        if (f->SystemIsEnabledSelf(child.get())) {
            child->OnEnabled(f);
        }
    }
}

void SystemMainThreadGroup::OnDisabled(Frame* f) {
    for (const auto& child : _children) {
        if (f->SystemIsEnabledSelf(child.get())) {
            child->OnDisabled(f);
        }
    }
}

} // namespace Quantum
