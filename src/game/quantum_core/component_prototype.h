// component_prototype.h - the prototype system of quantum.core.dll: component prototypes,
// entity prototype containers and the context they are materialised in.
//
// An entity prototype is a list of component prototypes (EntityPrototypeContainer). Each
// component prototype knows which component type it makes and how to add a filled-in
// component to an entity (AddToEntity). Materialising a prototype into a frame creates an
// entity and lets every component prototype add its component, in component-index order;
// prototypes that need to see the finished entity (joints) implement
// IEntityPrototypeMaterializedListener and are called once all components are in place.
//
// Prototypes come from assets: an EntityPrototype asset holds one container, a Map holds
// one per baked map entity (Map::MapEntities). Map entities refer to each other with
// MapEntityId; the context resolves those to the entities created for the map.
//
// C# type identity becomes std::type_info here. A component prototype whose component
// type has no C++ struct yet (physics components, still being written) answers with the
// registered component name instead; see PendingComponentPrototype.
#pragma once

#include "quantum_core/component_prototype_ref.h"
#include "quantum_core/component_type_id.h"
#include "quantum_core/entity_prototype_ref.h"
#include "quantum_core/entity_ref.h"
#include "quantum_core/map_entity_id.h"

#include <atomic>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace Quantum {

namespace Core {
class FrameBase;
}
namespace Prototypes {
class ComponentPrototypeVisitorBase;
}

class EntityPrototypeContainer;

// Marker of every prototype type (Quantum.IPrototype).
class IPrototype {
  public:
    virtual ~IPrototype() = default;
};

// Base classes the code generator uses for prototypes of dictionary entries, unions and
// plain structs. They carry no data. (Deriving them from IPrototype is this project's
// reading; the original only uses them as base types.)
class DictionaryEntryPrototype : public IPrototype {
  public:
    // 0x7ceae24
    DictionaryEntryPrototype() = default;
};

class UnionPrototype : public IPrototype {
  public:
    // 0x7ceae2c
    UnionPrototype() = default;
};

class StructPrototype : public IPrototype {
  public:
    // 0x7ceae34
    StructPrototype() = default;
};

// What a component prototype is being materialised for: the entity, the prototype it
// comes from, the component index, the container, and (for map entities) the entities
// created for the whole map, indexed by scene index.
struct PrototypeMaterializationContext {
    EntityRef Entity;
    ::Quantum::EntityPrototypeRef EntityPrototypeRef;
    int ComponentTypeIndex = 0;
    ::Quantum::EntityPrototypeContainer* EntityPrototypeContainer = nullptr;
    // Null when a single prototype is materialised (not a map).
    const std::vector<EntityRef>* _entities = nullptr;

    PrototypeMaterializationContext() = default;
    // 0x7cef5d8
    PrototypeMaterializationContext(EntityRef entity, ::Quantum::EntityPrototypeRef prototypeRef,
                                    ::Quantum::EntityPrototypeContainer* container,
                                    int componentIndex, const std::vector<EntityRef>* entities)
        : Entity(entity), EntityPrototypeRef(prototypeRef), ComponentTypeIndex(componentIndex),
          EntityPrototypeContainer(container), _entities(entities) {}

    // 0x7cef61c - the prototype reference plus this component's index.
    ComponentPrototypeRef get_ComponentPrototypeRef() const {
        ComponentPrototypeRef ref;
        ref.Asset = EntityPrototypeRef.Asset;
        ref.InnerAssetId = EntityPrototypeRef.InnerAssetId;
        ref.ComponentTypeIndex = ComponentTypeIndex;
        return ref;
    }

    // 0x7cef630 - The entity created for a map entity id. Without a map entity list, scene
    // index 0 stands for the entity being materialised itself. False (and None) for ids
    // out of range or entities that were not created.
    bool TryGetEntityRef(MapEntityId id, EntityRef& entityRef) const;
};

// Implemented by component prototypes that act once the whole entity (or map) has been
// materialised (Quantum.IEntityPrototypeMaterializedListener).
class IEntityPrototypeMaterializedListener {
  public:
    virtual ~IEntityPrototypeMaterializedListener() = default;
    virtual void OnEntityPrototypeMaterialized(Core::FrameBase* f, EntityRef entity,
                                               EntityPrototypeRef prototypeRef) = 0;
};

class ComponentPrototype : public IPrototype {
  public:
    // 0x7ceb170
    ComponentPrototype() = default;
    ComponentPrototype(const ComponentPrototype&) = default;
    ComponentPrototype& operator=(const ComponentPrototype&) = default;

    // The component type this prototype makes (Quantum.ComponentPrototype.ComponentType).
    // Null while that type has no C++ struct; ComponentTypeName() then identifies it.
    virtual const std::type_info* get_ComponentType() const = 0;
    // The registered (C#) name of that component type.
    virtual std::string ComponentTypeName() const = 0;

    // 0x7ceae3c - The registered index of the component type, looked up once and cached.
    // Throws AssertException if the type is not registered.
    int get_ComponentTypeId();

    // Adds (or overwrites) the component on `entity`. True when the component was new,
    // false when the entity already had one (its data is replaced where the prototype
    // uses Set; see each prototype).
    virtual bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                             const PrototypeMaterializationContext& context) = 0;
    // Double dispatch over the framework's prototype types.
    virtual void Dispatch(Prototypes::ComponentPrototypeVisitorBase& visitor) = 0;

    // Readable type name of this prototype, for messages (the original prints its type).
    std::string PrototypeTypeName() const;

    // 0x7ceaf80 PrototypeTypeToComponentType - The component type a prototype type makes.
    // The original reads it from the prototype type's [Prototype] attribute and throws for
    // a null type, a type that is not a component prototype, or a missing attribute; here
    // those are compile-time errors, and the attribute is the prototype's ComponentType
    // alias.
    template <class TPrototype> static const std::type_info& PrototypeTypeToComponentType() {
        static_assert(std::is_base_of_v<ComponentPrototype, TPrototype>,
                      "not a component prototype type");
        return typeid(typename TPrototype::ComponentType);
    }

  protected:
    int _componentTypeId = 0;
};

// Quantum.ComponentPrototype<T>: a prototype of component type T.
template <class T> class TypedComponentPrototype : public ComponentPrototype {
  public:
    using ComponentType = T;

    // get_ComponentType (shared generic code): typeid(T).
    const std::type_info* get_ComponentType() const override { return &typeid(T); }
    std::string ComponentTypeName() const override { return ComponentTypeNameOf<T>(); }
};

// A prototype of a framework component whose C++ struct does not exist yet: the type is
// found by its registered name. Once the struct exists, the prototype derives from
// TypedComponentPrototype<T> instead.
class PendingComponentPrototype : public ComponentPrototype {
  public:
    const std::type_info* get_ComponentType() const override { return nullptr; }
    std::string ComponentTypeName() const override { return _componentName; }

  protected:
    explicit PendingComponentPrototype(const char* componentName) : _componentName(componentName) {}

  private:
    const char* _componentName;
};

// The component prototypes of one entity prototype, kept sorted by component index so
// they materialise in a fixed order and can be found by binary search. Sorting happens
// lazily (and at most once, even with several threads materialising) when the container
// is first used; SetDirty forces a new sort after Components changed.
class EntityPrototypeContainer {
  public:
    static constexpr int SortState_Unsorted = 0;
    static constexpr int SortState_Sorting = 1;
    static constexpr int SortState_Sorted = 2;

    std::vector<std::shared_ptr<ComponentPrototype>> Components;
    // Set by the sort when any component prototype is an
    // IEntityPrototypeMaterializedListener.
    bool HasAnyListeners = false;

    // 0x7cf0f48 - no components.
    EntityPrototypeContainer() = default;
    EntityPrototypeContainer(const EntityPrototypeContainer&) = delete;
    EntityPrototypeContainer& operator=(const EntityPrototypeContainer&) = delete;

    // 0x7cf0cc8 - Sorts Components by component index unless already sorted. One caller
    // sorts; concurrent callers yield until it has finished. Also notes whether any
    // component prototype is a listener. Throws std::invalid_argument for a null entry
    // (the original fails inside the sort with a null reference).
    void EnsureIsSorted();
    // 0x7cf0de4 - Marks the container unsorted (and without listeners) again.
    void SetDirty();
    // 0x7cf0dfc - The prototype for a component type; null if there is none. Throws
    // std::logic_error for an unregistered type.
    ComponentPrototype* GetComponentPrototype(const std::type_info& type);
    // 0x7cf0e1c - The prototype for a component index; null if there is none.
    ComponentPrototype* GetComponentPrototype(int componentTypeId);

    // 0x7cf0f0c (IComparer<ComponentPrototype>.Compare) - by component index.
    static int Compare(ComponentPrototype& x, ComponentPrototype& y) {
        return x.get_ComponentTypeId() - y.get_ComponentTypeId();
    }
    // 0x7cf0e78 - Binary search of a sorted list by component index: the position, or the
    // bitwise complement of the insertion point when absent.
    static int InternalBinarySearch(const std::vector<std::shared_ptr<ComponentPrototype>>& array,
                                    int componentTypeId);

  private:
    std::atomic<int> _sortState{SortState_Unsorted};
};

// Diagnostics shared by materialisation code (Quantum.PrototypeValidator). Messages are
// worded by this project.
struct PrototypeValidator {
    // 0x7cf04ec - The number of elements to use from an array of `length` elements
    // (0 for a null array) where `expectedLength` are expected: the smaller one, with a
    // warning when they differ.
    static int CheckLength(int length, int expectedLength,
                           const PrototypeMaterializationContext& context);
    // 0x7cf0760 - Error: `entity` already had the component `componentPrototype` made,
    // and its data was replaced.
    static void ComponentOverwritten(EntityRef entity, const ComponentPrototype& componentPrototype,
                                     const PrototypeMaterializationContext& context);
    // 0x7cf0960 - Error: a union prototype names a field the union does not have; the
    // union stays unset.
    static void UnknownUnionField(const std::string& field,
                                  const PrototypeMaterializationContext& context);
    // 0x7cf0af8 - The entity a map entity id refers to: None for an invalid id, and an
    // error (result None) when the context cannot resolve it.
    static void FindMapEntity(MapEntityId link, const PrototypeMaterializationContext& context,
                              EntityRef& result);
    // AddToDictionary<K, V> (shared generic code, no address of its own): adds an entry of
    // a prototype dictionary; a key that is already present is reported as an error and
    // the first value is kept.
    template <class TDictionary, class K, class V>
    static void AddToDictionary(TDictionary& dictionary, const K& key, const V& value,
                                const PrototypeMaterializationContext& context) {
        if (!dictionary.TryAdd(key, value)) DuplicateKey(KeyText(key), context);
    }
    static void DuplicateKey(const std::string& key, const PrototypeMaterializationContext& context);

  private:
    template <class K> static std::string KeyText(const K& key) {
        if constexpr (std::is_arithmetic_v<K>) {
            return std::to_string(key);
        } else if constexpr (requires { key.ToString(); }) {
            return key.ToString();
        } else {
            return "?";
        }
    }
};

// Marks a component type for the code generator (Quantum.Prototypes.PrototypeAttribute):
// the component type a prototype class makes. At run time the prototype's ComponentType
// alias plays this role.
struct PrototypeAttribute {
    const std::type_info* _type = nullptr;

    // 0x7d7fc70
    explicit PrototypeAttribute(const std::type_info& componentType) : _type(&componentType) {}
    // 0x7d7fca0
    const std::type_info* get_Type() const { return _type; }
    // 0x7d7fca8
    void set_Type(const std::type_info* value) { _type = value; }
};

// Excludes a component field from prototypes (editor-side only).
// Original: Quantum.Core.ExcludeFromPrototypeAttribute.
struct ExcludeFromPrototypeAttribute {
    // 0x7d194f0
    ExcludeFromPrototypeAttribute() = default;
};

} // namespace Quantum
