// system_base.cpp - Quantum.SystemBase.
#include "quantum_code/system_base.h"

#include "photon_deterministic/log.h"

#include <stdexcept>
#include <typeinfo>

namespace Quantum {

SystemBase::SystemBase() : _scheduleSample("System") {}

int SystemBase::get_RuntimeIndex() const {
    if (!_runtimeIndex) {
        throw std::logic_error("SystemBase: runtime index not assigned");
    }
    return *_runtimeIndex;
}

void SystemBase::set_RuntimeIndex(int value) {
    if (_runtimeIndex) {
        Photon::Deterministic::Log::Error("SystemBase: runtime index already assigned");
        return;
    }
    _runtimeIndex = value;
}

std::vector<SystemBase*> SystemBase::get_Hierarchy() {
    std::vector<SystemBase*> result{this};
    for (SystemBase* child : get_ChildSystems()) {
        for (SystemBase* system : child->get_Hierarchy()) {
            result.push_back(system);
        }
    }
    return result;
}

} // namespace Quantum
