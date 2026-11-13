// hit_collection_storage.h - buffer management shared by HitCollection (2D) and
// HitCollection3D, which differ only in the hit type.
#pragma once

#include "quantum_core/frame_context.h"

#include "photon_deterministic/native.h"

#include <cassert>
#include <cstring>

namespace Quantum::Physics2D::hit_storage {

template <class THit, class TModes>
THit* AllocateHits(std::int32_t capacity, TModes mode, Core::FrameContext* context) {
    assert(context != nullptr);
    const int bytes = capacity * static_cast<int>(sizeof(THit));
    if (mode == TModes::Temp) {
        return static_cast<THit*>(context->TempAllocateAndClear(bytes, 8));
    }
    return static_cast<THit*>(context->get_Allocator()->AllocAndClear(bytes));
}

template <class THit, class TModes>
void ExpandHits(THit*& buffer, std::int32_t& capacity, TModes mode, std::int32_t newCapacity,
                Core::FrameContext* context) {
    THit* grown = AllocateHits<THit>(newCapacity, mode, context);
    std::memcpy(grown, buffer, static_cast<std::size_t>(capacity) * sizeof(THit));
    if (mode != TModes::Temp) {
        context->get_Allocator()->Free(buffer);
    }
    buffer = grown;
    capacity = newCapacity;
}

template <class THit, class TModes>
void FreeHits(THit*& buffer, std::int32_t& count, std::int32_t& capacity, TModes mode,
              Photon::Deterministic::Native::Allocator* allocator) {
    if (mode != TModes::Persistent) {
        return;
    }
    if (buffer != nullptr) {
        assert(allocator != nullptr);
        allocator->Free(buffer);
        buffer = nullptr;
    }
    count = 0;
    capacity = 0;
}

} // namespace Quantum::Physics2D::hit_storage
