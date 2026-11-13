// component_type_id.h - Quantum.ComponentTypeId and Quantum.ComponentTypeId<T>: the
// process-wide registry of component types.
//
// Every component type gets a small integer index (its bit in ComponentSet, its slot in
// the frame's component buffer table), a byte size, flags and serialisation/lifetime
// callbacks. Registration happens once, in Setup: index 0 is reserved, the framework's
// own components (transforms, physics, navigation, view, map link) come next in a fixed
// order, then the game registers its components from the callback passed to Setup. The
// order therefore decides the indices, and the indices are part of the serialised and
// checksummed frame state - it must never change.
//
// The C# class pair ComponentTypeId / ComponentTypeId<T> becomes one class template here:
// ComponentTypeId<> (= ComponentTypeId<void>) is the non-generic registry, and
// ComponentTypeId<T> holds the per-type statics.
//
// System.Type has no C++ equivalent; a registered type is identified by its
// std::type_info and carries its C# type name (used by debug output and by the
// per-component block capacity configuration, which matches on names). Framework
// components whose C++ types do not exist yet are registered by name and size only, so
// that all later indices stay correct.
#pragma once

#include "quantum_core/entity_ref.h"

#include <atomic>
#include <concepts>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace Quantum {

class FrameSerializer;

namespace Core {
class FrameBase;
}

enum class ComponentFlags : int {
    None = 0,
    // At most one instance may exist per frame.
    Singleton = 1,
};

// Quantum.ComponentSerializeDelegate - (component data, serializer).
using ComponentSerializeDelegate = void (*)(void* component, FrameSerializer* serializer);
// Quantum.ComponentChangedDelegate - (frame, entity, component data).
using ComponentChangedDelegate = void (*)(Core::FrameBase* f, EntityRef entity, void* component);

struct ComponentCallbacks {
    ComponentSerializeDelegate Serialize = nullptr;
    ComponentChangedDelegate OnAdded = nullptr;
    ComponentChangedDelegate OnRemoved = nullptr;

    ComponentCallbacks() = default;
    // 0x7ceaad4
    explicit ComponentCallbacks(ComponentSerializeDelegate serialize) : Serialize(serialize) {}
    // 0x7ceab08
    ComponentCallbacks(ComponentSerializeDelegate serialize, ComponentChangedDelegate onAdded,
                       ComponentChangedDelegate onRemoved)
        : Serialize(serialize), OnAdded(onAdded), OnRemoved(onRemoved) {}
};

// A component type may name itself through a static `ComponentTypeName` member (its C#
// name); otherwise the implementation-defined typeid name is used.
template <class T>
concept NamedComponent = requires {
    { T::ComponentTypeName } -> std::convertible_to<std::string_view>;
};

template <class T> std::string ComponentTypeNameOf() {
    if constexpr (NamedComponent<T>) {
        return std::string(std::string_view(T::ComponentTypeName));
    } else {
        return typeid(T).name();
    }
}

template <class T = void> struct ComponentTypeId;

// The non-generic registry (Quantum.ComponentTypeId).
template <> struct ComponentTypeId<void> {
    // Index -> type (null for the reserved index 0 and for framework components without a
    // C++ type yet), name, size, flags, callbacks. Valid once Setup has finished.
    static std::vector<const std::type_info*> Type;
    static std::vector<std::string> Name;
    static std::vector<int> Size;
    static std::vector<ComponentFlags> Flags;
    static std::vector<ComponentCallbacks> Callbacks;

    // 0x7cebfb8 - Registers all component types, once per process. The first caller does
    // the work (reserved index, framework components, then `action` for the game's own
    // components); concurrent callers wait until it has finished, later callers return
    // immediately.
    static void Setup(const std::function<void()>& action);

    // 0x7ceae80 - Index of a registered type; throws std::logic_error for an unknown one.
    static int GetComponentIndex(const std::type_info& type);
    // Index by C# type name (the original's _reverseStringLookup); -1 if unknown.
    static int GetComponentIndex(std::string_view name);
    // 0x7cebf4c
    static int GetComponentSize(int index) { return Size.at(static_cast<std::size_t>(index)); }
    // 0x7ceac64
    static const std::type_info* GetComponentType(int index) {
        return Type.at(static_cast<std::size_t>(index));
    }
    static const std::string& GetComponentName(int index) {
        return Name.at(static_cast<std::size_t>(index));
    }
    // Number of indices in use, including the reserved index 0.
    static int Count() { return static_cast<int>(Type.size()); }
    static bool IsSetUp() { return _initFinished.load(std::memory_order_acquire); }

    // 0x4cfddc4 (instance for AIBlackboardComponent) - Add<T> with separate callbacks.
    template <class T>
    static void Add(ComponentSerializeDelegate serialize, ComponentChangedDelegate onAdded,
                    ComponentChangedDelegate onRemoved, ComponentFlags flags) {
        Add<T>(ComponentCallbacks(serialize, onAdded, onRemoved), flags);
    }

    // 0x4cf438c (instance for Transform3D) - Gives T the next index and records its size,
    // flags and callbacks (in ComponentTypeId<T> and in the registry). Only valid while
    // Setup runs.
    template <class T> static void Add(ComponentCallbacks callbacks, ComponentFlags flags);

    // Registration of a type known only by name and size (framework components whose C++
    // type is not available yet). Returns the index.
    static int AddEntry(const std::type_info* type, std::string name, int size,
                        ComponentCallbacks callbacks, ComponentFlags flags);

    // Test support: forgets every registration so Setup can run again. Not part of the
    // original, which registers once per process.
    static void ResetForTests();

  private:
    static std::atomic<bool> _initAcquired;
    static std::atomic<bool> _initFinished;
    static bool _registering;
    static std::mutex _lock;
    static std::unordered_map<std::type_index, int> _reverseLookup;
    static std::unordered_map<std::string, int> _reverseStringLookup;

    static void RegisterFrameworkComponents();
};

// Per-type statics (Quantum.ComponentTypeId<T>). Id stays 0 until T is registered.
template <class T> struct ComponentTypeId {
    static inline int Id = 0;
    static inline int Size = 0;
    static inline ComponentFlags Flags = ComponentFlags::None;
    static inline ComponentCallbacks Callbacks;
};

template <class T>
void ComponentTypeId<void>::Add(ComponentCallbacks callbacks, ComponentFlags flags) {
    const int index = AddEntry(&typeid(T), ComponentTypeNameOf<T>(),
                               static_cast<int>(sizeof(T)), callbacks, flags);
    ComponentTypeId<T>::Id = index;
    ComponentTypeId<T>::Size = static_cast<int>(sizeof(T));
    ComponentTypeId<T>::Flags = flags;
    ComponentTypeId<T>::Callbacks = callbacks;
}

} // namespace Quantum
