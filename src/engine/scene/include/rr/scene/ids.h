// ids.h - indices of entries in a scene::ResourceLibrary.
#pragma once

#include <cstdint>

namespace rr::scene {

using MeshId = std::uint32_t;
using TextureId = std::uint32_t;
using MaterialId = std::uint32_t;
inline constexpr std::uint32_t kNoResource = 0xFFFFFFFFu;

} // namespace rr::scene
