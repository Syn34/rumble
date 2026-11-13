// view.h - Quantum.View: the component naming the presentation view (EntityView asset)
// of an entity. The simulation only stores and copies the reference.
#pragma once

#include "quantum_core/builtin_assets.h"

namespace Quantum {

class FrameSerializer;

struct View {
    static constexpr int SIZE = 8;
    // Registered component name (see component_type_id.h).
    static constexpr const char* ComponentTypeName = "View";

    AssetRefEntityView Current;

    // 0x7d45c20 - the view reference's GUID.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

static_assert(sizeof(View) == View::SIZE);

} // namespace Quantum
