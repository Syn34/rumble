// component_prototype.cpp - ComponentPrototype, EntityPrototypeContainer,
// PrototypeMaterializationContext and PrototypeValidator (quantum.core.dll).
#include "quantum_core/component_prototype.h"

#include "photon_deterministic/log.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/assert.h"

#include <algorithm>
#include <stdexcept>
#include <thread>

namespace Quantum {

using Photon::Deterministic::Log;

// --- PrototypeMaterializationContext ---------------------------------------------------------

bool PrototypeMaterializationContext::TryGetEntityRef(MapEntityId id, EntityRef& entityRef) const {
    const int index = id.SceneIndex();
    if (_entities == nullptr) {
        // A single prototype: only "scene index 0" exists, and it is the entity itself.
        if (index == 0) {
            entityRef = Entity;
            return entityRef.IsValid();
        }
    } else if (index >= 0 && index < static_cast<int>(_entities->size())) {
        entityRef = (*_entities)[static_cast<std::size_t>(index)];
        return entityRef.IsValid();
    }
    entityRef = EntityRef::None();
    return false;
}

// --- ComponentPrototype ----------------------------------------------------------------------

int ComponentPrototype::get_ComponentTypeId() {
    if (_componentTypeId == 0) {
        const std::type_info* type = get_ComponentType();
        _componentTypeId = type != nullptr ? ComponentTypeId<>::GetComponentIndex(*type)
                                           : ComponentTypeId<>::GetComponentIndex(ComponentTypeName());
    }
    Assert::Always(_componentTypeId > 0);
    return _componentTypeId;
}

std::string ComponentPrototype::PrototypeTypeName() const { return AssetTypeName(typeid(*this)); }

// --- EntityPrototypeContainer ----------------------------------------------------------------

void EntityPrototypeContainer::EnsureIsSorted() {
    while (_sortState.load(std::memory_order_acquire) != SortState_Sorted) {
        int expected = SortState_Unsorted;
        if (!_sortState.compare_exchange_strong(expected, SortState_Sorting,
                                                std::memory_order_acq_rel)) {
            // Another thread is sorting.
            std::this_thread::yield();
            continue;
        }

        const bool hasNull = std::any_of(Components.begin(), Components.end(),
                                         [](const auto& component) { return component == nullptr; });
        if (hasNull) {
            _sortState.store(SortState_Unsorted, std::memory_order_release);
            throw std::invalid_argument("EntityPrototypeContainer: null component prototype");
        }

        // A stable sort: for the small lists prototypes have (up to 16 entries) .NET's
        // Array.Sort is an insertion sort, which keeps entries with equal keys in their
        // authored order too. (Two prototypes of one component type do not occur in valid
        // data.)
        std::stable_sort(Components.begin(), Components.end(),
                         [](const std::shared_ptr<ComponentPrototype>& x,
                            const std::shared_ptr<ComponentPrototype>& y) {
                             return Compare(*x, *y) < 0;
                         });

        // The flag only ever turns on here; SetDirty clears it.
        for (std::size_t i = 0; i < Components.size() && !HasAnyListeners; ++i) {
            HasAnyListeners =
                dynamic_cast<IEntityPrototypeMaterializedListener*>(Components[i].get()) != nullptr;
        }

        const int previous = _sortState.exchange(SortState_Sorted, std::memory_order_acq_rel);
        Assert::Always(previous == SortState_Sorting);
        return;
    }
}

void EntityPrototypeContainer::SetDirty() {
    HasAnyListeners = false;
    _sortState.exchange(SortState_Unsorted, std::memory_order_acq_rel);
}

ComponentPrototype* EntityPrototypeContainer::GetComponentPrototype(const std::type_info& type) {
    return GetComponentPrototype(ComponentTypeId<>::GetComponentIndex(type));
}

ComponentPrototype* EntityPrototypeContainer::GetComponentPrototype(int componentTypeId) {
    EnsureIsSorted();
    const int index = InternalBinarySearch(Components, componentTypeId);
    return index >= 0 ? Components[static_cast<std::size_t>(index)].get() : nullptr;
}

int EntityPrototypeContainer::InternalBinarySearch(
    const std::vector<std::shared_ptr<ComponentPrototype>>& array, int componentTypeId) {
    int low = 0;
    int high = static_cast<int>(array.size()) - 1;
    while (low <= high) {
        const int middle = low + (high - low) / 2;
        const std::shared_ptr<ComponentPrototype>& candidate = array[static_cast<std::size_t>(middle)];
        if (candidate == nullptr) {
            throw std::invalid_argument("EntityPrototypeContainer: null component prototype");
        }
        const int id = candidate->get_ComponentTypeId();
        if (id == componentTypeId) {
            return middle;
        }
        if (id < componentTypeId) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    return ~low;
}

// --- PrototypeValidator ----------------------------------------------------------------------

int PrototypeValidator::CheckLength(int length, int expectedLength,
                                    const PrototypeMaterializationContext& context) {
    const int used = std::min(length, expectedLength);
    if (length != expectedLength) {
        Log::Warn("Prototype array has {0} elements where {1} are expected; using {2} ({3})",
                  length, expectedLength, used, context.EntityPrototypeRef.ToString());
    }
    return used;
}

void PrototypeValidator::ComponentOverwritten(EntityRef entity,
                                              const ComponentPrototype& componentPrototype,
                                              const PrototypeMaterializationContext& context) {
    Log::Error("{1} replaced a component entity {0} already had ({2})", entity.ToString(),
               componentPrototype.PrototypeTypeName(), context.EntityPrototypeRef.ToString());
}

void PrototypeValidator::UnknownUnionField(const std::string& field,
                                           const PrototypeMaterializationContext& context) {
    Log::Error("Union prototype names unknown field {0}; the union is left unset ({1})", field,
               context.EntityPrototypeRef.ToString());
}

void PrototypeValidator::FindMapEntity(MapEntityId link, const PrototypeMaterializationContext& context,
                                       EntityRef& result) {
    if (!link.IsValid()) {
        result = EntityRef::None();
        return;
    }
    if (!context.TryGetEntityRef(link, result)) {
        Log::Error("No entity for map entity {0} ({1})", link.ToString(),
                   context.EntityPrototypeRef.ToString());
    }
}

} // namespace Quantum
