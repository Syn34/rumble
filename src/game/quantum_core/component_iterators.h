// component_iterators.h - ways of walking the components of a frame:
//
//  * ComponentBlockIterator<T> (Quantum.ComponentBlockIterator`1): pointers to every T,
//    block by block; skips instances pending removal.
//  * ComponentIterator<T> (Quantum.ComponentIterator`1): entity/value pairs; skips
//    instances pending removal and throws if instances move meanwhile.
//  * ComponentFilter<Ts...> (Quantum.ComponentFilter`1..`4): entities that have all of
//    Ts, at least one of an "any" set and none of a "without" set, with pointers to their
//    Ts. On predicted frames culled entities are skipped.
//
// All of them walk the first component type's buffer in slot order, which is what makes
// iteration order deterministic. The C# enumerators (MoveNext/Current) are kept and also
// exposed through begin()/end() for range-based for.
#pragma once

#include "quantum_core/frame_base.h"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <tuple>

namespace Quantum {

// Quantum.EntityComponentPair<T>
template <class T> struct EntityComponentPair {
    EntityRef Entity;
    T Component{};

    // 0x74b03f8 (instance for BlastBallGoal)
    void Deconstruct(EntityRef& entity, T& component) const {
        entity = Entity;
        component = Component;
    }
};

// Quantum.EntityComponentPointerPair<T>
template <class T> struct EntityComponentPointerPair {
    EntityRef Entity;
    T* Component = nullptr;

    // 0x74b0654 (instance for AINavigation)
    void Deconstruct(EntityRef& entity, T*& component) const {
        entity = Entity;
        component = Component;
    }
};

// Adapts a C#-style enumerator (MoveNext / Current) to a C++ input range.
template <class TEnumerator> class EnumeratorRange {
  public:
    explicit EnumeratorRange(TEnumerator enumerator) : _enumerator(enumerator) {}

    struct Sentinel {};
    class Iterator {
      public:
        explicit Iterator(TEnumerator* enumerator) : _enumerator(enumerator) { Advance(); }
        decltype(auto) operator*() const { return _enumerator->get_Current(); }
        Iterator& operator++() {
            Advance();
            return *this;
        }
        friend bool operator==(const Iterator& it, Sentinel) { return it._done; }

      private:
        void Advance() { _done = !_enumerator->MoveNext(); }
        TEnumerator* _enumerator;
        bool _done = false;
    };

    Iterator begin() { return Iterator(&_enumerator); }
    Sentinel end() const { return {}; }

  private:
    TEnumerator _enumerator;
};

// ---------------------------------------------------------------------------------------

template <class T> struct ComponentBlockIterator {
    Core::FrameBase* _f = nullptr;
    ComponentBlockIterator<> _iterator;

    // 0x70d5fb4 (instance for AINavigation)
    ComponentBlockIterator(Core::FrameBase* f, ComponentDataBuffer* buffer)
        : _f(f), _iterator(buffer) {}

    // 0x70d6070 (instance for AINavigation) - The next run of instances.
    bool NextBlock(EntityRef*& entities, T*& components, int& count) {
        std::uint8_t* data = nullptr;
        const bool found = _iterator.NextBlock(data, entities, count);
        components = found ? reinterpret_cast<T*>(data) : nullptr;
        return found;
    }

    struct Enumerator {
        Core::FrameBase* _f = nullptr;
        int _blockIndex = -1;
        int _blockCount = 0;
        EntityRef* _blockEntities = nullptr;
        T* _blockComponents = nullptr;
        ComponentBlockIterator _iterator;

        // 0x752bfa8 (instance for PhysicsCollider3D)
        Enumerator(Core::FrameBase* f, const ComponentBlockIterator& iterator)
            : _f(f), _iterator(iterator) {}

        // 0x752bff4 - Throws std::logic_error before the first MoveNext or after the end.
        EntityComponentPointerPair<T> get_Current() const {
            if (static_cast<std::uint32_t>(_blockIndex) >= static_cast<std::uint32_t>(_blockCount)) {
                throw std::logic_error("ComponentBlockIterator: no current element");
            }
            return {_blockEntities[_blockIndex], &_blockComponents[_blockIndex]};
        }

        // 0x752c04c - Next instance whose entity still has T (instances pending removal are
        // skipped).
        bool MoveNext() {
            for (;;) {
                if (++_blockIndex == _blockCount) {
                    if (!_iterator.NextBlock(_blockEntities, _blockComponents, _blockCount)) {
                        return false;
                    }
                    _blockIndex = 0;
                }
                if (_f->Has(_blockEntities[_blockIndex], ComponentTypeId<T>::Id)) {
                    return true;
                }
            }
        }
    };

    // 0x70d6004 (instance for AINavigation)
    Enumerator GetEnumerator() const { return Enumerator(_f, *this); }
    EnumeratorRange<Enumerator> Range() const { return EnumeratorRange<Enumerator>(GetEnumerator()); }
};

// ---------------------------------------------------------------------------------------

template <class T> struct ComponentIterator {
    Core::FrameBase* _f = nullptr;
    int _count = 0;
    int _version = 0;
    ComponentDataBuffer* _buffer = nullptr;

    // 0x7136af0 (instance for BlastBallGoal)
    ComponentIterator(Core::FrameBase* f, ComponentDataBuffer* buffer)
        : _f(f), _count(buffer->get_Count()), _version(buffer->_version), _buffer(buffer) {}

    // 0x7136bac (instance for BlastBallGoal) - Throws std::logic_error if instances of T
    // were removed (slots moved) since the iterator was made.
    void VerifyVersion() const {
        if (_version != _buffer->_version) {
            throw std::logic_error("ComponentIterator: " + ComponentTypeNameOf<T>() +
                                   " components were removed while iterating over them");
        }
    }

    struct Enumerator {
        Core::FrameBase* _f = nullptr;
        int _index = -1;
        ComponentIterator _iterator;

        // 0x74f97cc (instance for BlastBallGoal)
        Enumerator(Core::FrameBase* f, const ComponentIterator& iterator)
            : _f(f), _iterator(iterator) {}

        // 0x74f9810 - Throws std::logic_error outside the valid range.
        EntityComponentPair<T> get_Current() const {
            if (static_cast<std::uint32_t>(_index) >= static_cast<std::uint32_t>(_iterator._count)) {
                throw std::logic_error("ComponentIterator: no current element");
            }
            const int slot = _index + 1;
            return {_iterator._buffer->GetEntityRefByIndex(slot),
                    *static_cast<const T*>(_iterator._buffer->GetDataPointerByIndex(slot))};
        }

        // 0x74f98a0 - Next instance whose entity still has T.
        bool MoveNext() {
            for (;;) {
                _iterator.VerifyVersion();
                ++_index;
                if (_index >= _iterator._count) {
                    return false;
                }
                const EntityRef entity = _iterator._buffer->GetEntityRefByIndex(_index + 1);
                if (_f->Has(entity, ComponentTypeId<T>::Id)) {
                    return true;
                }
            }
        }
    };

    // 0x7136b30 (instance for BlastBallGoal)
    Enumerator GetEnumerator() const { return Enumerator(_f, *this); }
    EnumeratorRange<Enumerator> Range() const { return EnumeratorRange<Enumerator>(GetEnumerator()); }
};

// ---------------------------------------------------------------------------------------

template <class... Ts> struct ComponentFilter {
    static_assert(sizeof...(Ts) >= 1 && sizeof...(Ts) <= 4, "filters take 1 to 4 components");
    static constexpr std::size_t Count = sizeof...(Ts);

    bool _anySuccess = false;
    bool _withoutSuccess = false;
    bool _cullingSuccess = false;
    Core::FrameBase* _registry = nullptr;
    ComponentSet _any;
    ComponentSet _with;
    ComponentSet _without;
    std::array<ComponentDataBuffer*, Count> _buffers{};
    ComponentBlockIterator<> _blockIterator;
    int _t0Id = 0;
    int _blockIndex = 0;
    int _blockCount = 0;
    std::uint8_t* _blockComponentData = nullptr;
    EntityRef* _blockEntityHandles = nullptr;

    // 0x712abb8 (1 type, AISpawner), 0x712d55c (2), 0x712e7e8 (3), 0x712f804 (4) - The
    // walk follows the first type's buffer. Culling is ignored on verified frames.
    ComponentFilter(Core::FrameBase* f, const ComponentSet& with, const ComponentSet& any,
                    const ComponentSet& without)
        : _anySuccess(any.IsEmpty()), _withoutSuccess(without.IsEmpty()),
          _cullingSuccess(f->Verified), _registry(f), _any(any), _with(with),
          _without(without), _buffers{f->template GetDataBuffer<Ts>()...},
          _blockIterator(_buffers[0]), _t0Id(ComponentTypeId<FirstType>::Id) {}

    // 0x712adb0 (1 type), 0x712d818 (2), 0x712eb44 (3), 0x712fafc (4) - The next matching
    // entity and pointers to its components; false (all null) at the end.
    bool NextUnsafe(EntityRef& entity, Ts*&... components) {
        for (;;) {
            if (_blockComponentData == nullptr) {
                if (!_blockIterator.NextBlock(_blockComponentData, _blockEntityHandles,
                                              _blockCount) ||
                    _blockCount == 0) {
                    entity = EntityRef::None();
                    ((components = nullptr), ...);
                    return false;
                }
                _blockIndex = 0;
            }
            for (; _blockIndex < _blockCount; ++_blockIndex) {
                const EntityRef candidate = _blockEntityHandles[_blockIndex];
                if (!Matches(candidate)) {
                    continue;
                }
                entity = candidate;
                AssignPointers(candidate, std::index_sequence_for<Ts...>{}, components...);
                ++_blockIndex;
                return true;
            }
            _blockComponentData = nullptr;
        }
    }

    // 0x712ad28 (1 type), 0x712d75c (2), 0x712ea64 (3) - Copies of the components.
    bool Next(EntityRef& entity, Ts&... components) {
        std::tuple<Ts*...> pointers;
        const bool found = std::apply(
            [&](auto&... p) { return NextUnsafe(entity, p...); }, pointers);
        CopyOut(found, pointers, std::index_sequence_for<Ts...>{}, components...);
        return found;
    }

  private:
    using FirstType = std::tuple_element_t<0, std::tuple<Ts...>>;

    bool Matches(EntityRef candidate) const {
        const ComponentSet mask = _registry->ComponentMask(candidate.Index);
        if (!_registry->Exists(candidate)) {
            return false;
        }
        // The one-type filter tests the type's bit, the others the whole "with" set; the
        // result is the same.
        if constexpr (Count == 1) {
            if (!mask.IsSet(_t0Id)) {
                return false;
            }
        } else {
            if (!mask.IsSupersetOf(_with)) {
                return false;
            }
        }
        if (!_anySuccess && !mask.Overlaps(_any)) {
            return false;
        }
        if (!_withoutSuccess && mask.Overlaps(_without)) {
            return false;
        }
        return _cullingSuccess || !_registry->IsCulled(candidate);
    }

    template <std::size_t... I>
    void AssignPointers(EntityRef entity, std::index_sequence<I...>, Ts*&... components) {
        ((components = static_cast<Ts*>(_buffers[I]->GetDataPointerFastUnsafe(entity))), ...);
    }

    template <std::size_t... I>
    static void CopyOut(bool found, const std::tuple<Ts*...>& pointers, std::index_sequence<I...>,
                        Ts&... components) {
        ((components = found ? *std::get<I>(pointers) : Ts{}), ...);
    }
};

// Filters into user structs (Quantum.ComponentFilterStruct`1, ComponentGetter`1,
// ComponentFilterStructMeta) reflect over the struct's fields to find an EntityRef and
// component pointers; they need a C++ description of such structs:
// TODO(0x4c81f24, Quantum.ComponentFilterStructMeta.Create<T>)
// TODO(0x70d7930, Quantum.ComponentFilterStruct<T>..ctor)
// TODO(0x70d7b4c, Quantum.ComponentFilterStruct<T>.Next)
// TODO(0x70d7924, Quantum.ComponentFilterStruct<T>.set_UseCulling)
// TODO(0x7130988, Quantum.ComponentGetter<T>..ctor)
// TODO(0x7130990, Quantum.ComponentGetter<T>.TryGet)

// ---------------------------------------------------------------------------------------
// FrameBase members that return these types

namespace Core {

template <class T> ComponentIterator<T> FrameBase::GetComponentIterator() {
    return ComponentIterator<T>(this, GetDataBuffer<T>());
}

template <class T> ComponentBlockIterator<T> FrameBase::GetComponentBlockIterator() {
    return ComponentBlockIterator<T>(this, GetDataBuffer<T>());
}

template <class... Ts>
ComponentFilter<Ts...> FrameBase::Filter(ComponentSet without, ComponentSet any) {
    return ComponentFilter<Ts...>(this, ComponentSet::Create<Ts...>(), any, without);
}

template <class T> ComponentBlockIterator<T> FrameBase::FrameBaseUnsafe::GetComponentBlockIterator() {
    return _f->GetComponentBlockIterator<T>();
}

} // namespace Core
} // namespace Quantum
