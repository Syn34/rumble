#include "component_type_id.h"

#include "quantum_core/transform.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/map_entity_id.h"
#include "quantum_core/navigation/navmesh_components.h"
#include "quantum_core/view.h"

#include <stdexcept>
#include <thread>

namespace Quantum {

std::vector<const std::type_info*> ComponentTypeId<void>::Type;
std::vector<std::string> ComponentTypeId<void>::Name;
std::vector<int> ComponentTypeId<void>::Size;
std::vector<ComponentFlags> ComponentTypeId<void>::Flags;
std::vector<ComponentCallbacks> ComponentTypeId<void>::Callbacks;
std::atomic<bool> ComponentTypeId<void>::_initAcquired{false};
std::atomic<bool> ComponentTypeId<void>::_initFinished{false};
bool ComponentTypeId<void>::_registering = false;
std::mutex ComponentTypeId<void>::_lock;
std::unordered_map<std::type_index, int> ComponentTypeId<void>::_reverseLookup;
std::unordered_map<std::string, int> ComponentTypeId<void>::_reverseStringLookup;

namespace {

// 0x7ced6a8 (<>c.<Setup>b__18_0) - Serializer of the reserved index 0; never meant to run.
void SerializeReservedSlot(void* /*component*/, FrameSerializer* /*serializer*/) {
    throw std::logic_error("ComponentTypeId: component index 0 is reserved");
}

// The navigation components serialise through the interface type; component callbacks
// receive the frame serializer.
void SerializeNavMeshSteeringAgent(void* component, FrameSerializer* serializer) {
    NavMeshSteeringAgent::Serialize(component, serializer);
}
void SerializeNavMeshAvoidanceAgent(void* component, FrameSerializer* serializer) {
    NavMeshAvoidanceAgent::Serialize(component, serializer);
}
void SerializeNavMeshAvoidanceObstacle(void* component, FrameSerializer* serializer) {
    NavMeshAvoidanceObstacle::Serialize(component, serializer);
}

void WaitUntilFinished(const std::atomic<bool>& finished) {
    while (!finished.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
}

} // namespace

int ComponentTypeId<void>::AddEntry(const std::type_info* type, std::string name, int size,
                                    ComponentCallbacks callbacks, ComponentFlags flags) {
    if (!_registering) {
        throw std::logic_error("ComponentTypeId: components can only be added during Setup");
    }
    const int index = static_cast<int>(Type.size());
    if (type != nullptr && !_reverseLookup.emplace(std::type_index(*type), index).second) {
        throw std::invalid_argument("ComponentTypeId: " + name + " is already registered");
    }
    if (!_reverseStringLookup.emplace(name, index).second) {
        throw std::invalid_argument("ComponentTypeId: the name " + name + " is already taken");
    }
    Type.push_back(type);
    Name.push_back(std::move(name));
    Size.push_back(size);
    Callbacks.push_back(callbacks);
    Flags.push_back(flags);
    return index;
}

// The framework's own components, in the original's order. Types that exist in this
// project are registered with their C++ type; the others by C# name and original byte
// size (their callbacks are added when the types are implemented).
void ComponentTypeId<void>::RegisterFrameworkComponents() {
    const auto none = ComponentFlags::None;
    Add<Transform2D>(ComponentCallbacks(&Transform2D::Serialize), none);
    Add<Transform3D>(ComponentCallbacks(&Transform3D::Serialize), none);
    Add<Transform2DVertical>(ComponentCallbacks(&Transform2DVertical::Serialize), none);
    // Pending their C++ types: once they exist these become Add<T> calls with their
    // Serialize/OnAdded/OnRemoved callbacks. Name and size already keep every later index
    // and buffer layout identical to the original's.
    AddEntry(nullptr, "PhysicsCollider2D", 80, {}, none);
    AddEntry(nullptr, "PhysicsBody2D", 120, {}, none);
    AddEntry(nullptr, "PhysicsCollider3D", 112, {}, none);
    AddEntry(nullptr, "PhysicsBody3D", 184, {}, none);
    AddEntry(nullptr, "PhysicsJoints2D", 8, {}, none);
    AddEntry(nullptr, "PhysicsJoints3D", 8, {}, none);
    AddEntry(nullptr, "PhysicsCallbacks2D", 16, {}, none);
    AddEntry(nullptr, "PhysicsCallbacks3D", 16, {}, none);
    AddEntry(nullptr, "CharacterController2D", 40, {}, none);
    AddEntry(nullptr, "CharacterController3D", 48, {}, none);
    AddEntry(nullptr, "NavMeshPathfinder", 96, {}, none);
    // The navigation agents' OnAdded/OnRemoved callbacks are still TODO (null here).
    Add<NavMeshSteeringAgent>(ComponentCallbacks(&SerializeNavMeshSteeringAgent), none);
    Add<NavMeshAvoidanceAgent>(ComponentCallbacks(&SerializeNavMeshAvoidanceAgent), none);
    Add<NavMeshAvoidanceObstacle>(ComponentCallbacks(&SerializeNavMeshAvoidanceObstacle), none);
    Add<View>(ComponentCallbacks(&View::Serialize), none);
    Add<MapEntityLink>(ComponentCallbacks(&MapEntityLink::Serialize), none);
}

void ComponentTypeId<void>::Setup(const std::function<void()>& action) {
    if (_initAcquired.load(std::memory_order_acquire)) {
        WaitUntilFinished(_initFinished);
        return;
    }
    std::lock_guard<std::mutex> guard(_lock);
    if (_initAcquired.load(std::memory_order_acquire)) {
        WaitUntilFinished(_initFinished);
        return;
    }
    _initAcquired.store(true, std::memory_order_release);

    Type.clear();
    Name.clear();
    Size.clear();
    Flags.clear();
    Callbacks.clear();
    _reverseLookup.clear();
    _reverseStringLookup.clear();

    // Index 0 is reserved: no type, size -1, a serializer that refuses to run.
    Type.push_back(nullptr);
    Name.emplace_back();
    Size.push_back(-1);
    Flags.push_back(ComponentFlags::None);
    Callbacks.emplace_back(&SerializeReservedSlot);

    _registering = true;
    try {
        RegisterFrameworkComponents();
        if (action) {
            action();
        }
    } catch (...) {
        _registering = false;
        throw;
    }
    _registering = false;
    _initFinished.store(true, std::memory_order_release);
}

int ComponentTypeId<void>::GetComponentIndex(const std::type_info& type) {
    auto it = _reverseLookup.find(std::type_index(type));
    if (it == _reverseLookup.end()) {
        throw std::logic_error(std::string("ComponentTypeId: ") + type.name() +
                               " is not a registered component type");
    }
    return it->second;
}

int ComponentTypeId<void>::GetComponentIndex(std::string_view name) {
    auto it = _reverseStringLookup.find(std::string(name));
    return it != _reverseStringLookup.end() ? it->second : -1;
}

void ComponentTypeId<void>::ResetForTests() {
    std::lock_guard<std::mutex> guard(_lock);
    Type.clear();
    Name.clear();
    Size.clear();
    Flags.clear();
    Callbacks.clear();
    _reverseLookup.clear();
    _reverseStringLookup.clear();
    _initFinished.store(false, std::memory_order_release);
    _initAcquired.store(false, std::memory_order_release);
}

} // namespace Quantum
