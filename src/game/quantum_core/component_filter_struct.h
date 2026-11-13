// component_filter_struct.h - filters into user structs (Quantum.ComponentFilterStruct`1,
// Quantum.ComponentGetter`1, Quantum.ComponentFilterStructMeta, FrameBaseUnsafe.FilterStruct /
// ComponentGetter; quantum.core.dll).
//
// A filter struct holds one EntityRef and one pointer per component. The original finds
// those fields by reflection (ComponentFilterStructMeta.Create<T>); here each struct names
// them itself:
//
//     struct Filter {
//         EntityRef Entity;
//         Transform3D* Transform;
//         Health* Health;
//         static constexpr auto FilterEntity = &Filter::Entity;
//         static constexpr auto FilterComponents = std::make_tuple(&Filter::Transform, &Filter::Health);
//     };
//
// The component pointers must be listed in declaration order: it decides which buffer is
// walked when several are equally small (see the constructor).
#pragma once

#include "quantum_core/component_data_buffer.h"
#include "quantum_core/component_set.h"
#include "quantum_core/component_type_id.h"
#include "quantum_core/frame_base.h"

#include <array>
#include <tuple>
#include <utility>

namespace Quantum {

namespace detail {
template <class TMember> struct FilterMemberComponent;
template <class TStruct, class TComponent> struct FilterMemberComponent<TComponent* TStruct::*> {
    using Type = TComponent;
};
} // namespace detail

// ComponentFilterStructMeta: the component set of a filter struct and the type ids of its
// pointer fields in declaration order.
template <class TFilter> struct ComponentFilterStructMeta {
    static constexpr std::size_t ComponentCount = std::tuple_size_v<decltype(TFilter::FilterComponents)>;

    // Generic (TODO-listed as 0x4c81f24 in quantum.core): the "with" set.
    static ComponentSet ComponentSetWith() {
        ComponentSet set;
        std::apply(
            [&](auto... members) {
                (set.Add(ComponentTypeId<typename detail::FilterMemberComponent<decltype(members)>::Type>::Id), ...);
            },
            TFilter::FilterComponents);
        return set;
    }

    // Writes the entity and the pointers to its components into `out`.
    static void Fill(Core::FrameBase* f, EntityRef entity, TFilter& out) {
        out.*(TFilter::FilterEntity) = entity;
        std::apply(
            [&](auto... members) {
                ((out.*members = static_cast<typename detail::FilterMemberComponent<decltype(members)>::Type*>(
                      f->GetDataBuffer(ComponentTypeId<typename detail::FilterMemberComponent<decltype(members)>::Type>::Id)
                          ->GetDataPointerFastUnsafe(entity))),
                 ...);
            },
            TFilter::FilterComponents);
    }

    template <std::size_t I> static int ComponentId() {
        using Member = std::tuple_element_t<I, std::remove_const_t<decltype(TFilter::FilterComponents)>>;
        return ComponentTypeId<typename detail::FilterMemberComponent<Member>::Type>::Id;
    }

    static std::array<int, ComponentCount> ComponentIds() {
        return ComponentIdsImpl(std::make_index_sequence<ComponentCount>{});
    }

  private:
    template <std::size_t... I> static std::array<int, ComponentCount> ComponentIdsImpl(std::index_sequence<I...>) {
        return {ComponentId<I>()...};
    }
};

template <class TFilter> class ComponentFilterStruct {
  public:
    using Meta = ComponentFilterStructMeta<TFilter>;

    // Generic ctor (shared instance 0x70d7930) - Walks the smallest buffer among the
    // filter's components (the first one on ties). Culled entities are skipped unless the
    // frame is verified; set_UseCulling overrides that.
    ComponentFilterStruct(Core::FrameBase* f, const ComponentSet& without, const ComponentSet& any)
        : _anyEmpty(any.IsEmpty()), _withoutEmpty(without.IsEmpty()), _skipCulling(f->Verified),
          _f(f), _any(any), _with(Meta::ComponentSetWith()), _without(without) {
        const auto ids = Meta::ComponentIds();
        ComponentDataBuffer* smallest = f->GetDataBuffer(ids[0]);
        for (std::size_t i = 1; i < ids.size(); ++i) {
            ComponentDataBuffer* candidate = f->GetDataBuffer(ids[i]);
            if (candidate->get_Count() < smallest->get_Count()) {
                smallest = candidate;
            }
        }
        _iterator = ComponentBlockIterator<>(smallest);
    }

    // Generic (shared instance 0x70d7924)
    void set_UseCulling(bool value) { _skipCulling = !value; }

    // Generic (shared instance 0x70d7b4c) - The next entity that has every component of
    // the filter, matches the any/without sets and is not culled; its entity and component
    // pointers are written to `out`. False at the end (`out` is left as it was).
    bool Next(TFilter& out) {
        for (;;) {
            if (_blockEntities == nullptr) {
                std::uint8_t* data = nullptr;
                if (!_iterator.NextBlock(data, _blockEntities, _blockCount) || _blockCount == 0) {
                    _blockEntities = nullptr;
                    return false;
                }
                _blockIndex = 0;
            }
            while (_blockIndex < _blockCount) {
                const EntityRef candidate = _blockEntities[_blockIndex++];
                if (Matches(candidate)) {
                    Meta::Fill(_f, candidate, out);
                    return true;
                }
            }
            _blockEntities = nullptr;
        }
    }

  private:
    bool Matches(EntityRef candidate) const {
        if (!_f->Exists(candidate)) {
            return false;
        }
        const ComponentSet& mask = _f->ComponentMask(candidate.Index);
        if (!mask.IsSupersetOf(_with)) {
            return false;
        }
        if (!_anyEmpty && !mask.Overlaps(_any)) {
            return false;
        }
        if (!_withoutEmpty && mask.Overlaps(_without)) {
            return false;
        }
        return _skipCulling || !_f->IsCulled(candidate);
    }

    bool _anyEmpty;
    bool _withoutEmpty;
    bool _skipCulling;
    Core::FrameBase* _f;
    ComponentSet _any;
    ComponentSet _with;
    ComponentSet _without;
    ComponentBlockIterator<> _iterator;
    EntityRef* _blockEntities = nullptr;
    int _blockIndex = 0;
    int _blockCount = 0;
};

// FrameBase.FrameBaseUnsafe.FilterStruct<T> (generic, TODO-listed as 0x52b7474).
template <class TFilter>
ComponentFilterStruct<TFilter> FilterStruct(Core::FrameBase* f, const ComponentSet& without = ComponentSet{},
                                            const ComponentSet& any = ComponentSet{}) {
    return ComponentFilterStruct<TFilter>(f, without, any);
}

// Quantum.ComponentGetter<T>: fills a filter struct for one known entity.
template <class TFilter> struct ComponentGetter {
    using Meta = ComponentFilterStructMeta<TFilter>;
    ComponentSet _with = Meta::ComponentSetWith();

    // Generic (instance 0x7130990) - True (and `out` filled) when the entity exists and
    // has every component of the filter; otherwise `out` is reset.
    bool TryGet(Core::FrameBase* f, EntityRef entity, TFilter& out) const {
        if (!f->Exists(entity) || !f->ComponentMask(entity.Index).IsSupersetOf(_with)) {
            out = TFilter{};
            return false;
        }
        Meta::Fill(f, entity, out);
        return true;
    }
};

} // namespace Quantum
