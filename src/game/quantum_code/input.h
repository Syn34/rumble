// input.h - Quantum.Input (quantum.code.dll): one player's input for one simulation tick.
//
// Directions are compressed to fit the per-tick input packet: the move direction to one
// byte (2 degree steps, 0 = no input), the camera orbit distance to one byte (0..16 in
// 255 steps) and three directions to EncodedDirection3. Buttons carry their own
// pressed/released history (quantum_core/button.h).
#pragma once

#include "quantum_code/core_types.h"

#include <cstdint>

namespace Quantum {

struct alignas(4) Input {
    static constexpr int SIZE = 100;
    static constexpr int ALIGNMENT = 4;
    // Players the frame keeps input for (_globals_ holds one Input per player).
    static constexpr int MAX_COUNT = 32;

    std::uint8_t EncodedCameraOrbitOffset;
    std::uint8_t EncodedDirection;
    EncodedDirection3 EncodedCameraOrbitDirection;
    EncodedDirection3 EncodedCameraSight;
    EncodedDirection3 EncodedLookInputVector;
    std::uint8_t _pad0[2];
    Button Dash;
    Button Fire;
    Button Gadget;
    Button Jump;
    Button Melee;
    Button Reload;
    Button Ultimate;

    // 0x7b99168 - Up rotated by (EncodedDirection - 1) * 2 degrees; zero for no input.
    FPVector2 get_Direction() const;
    // 0x7b991a8 - The signed angle from Up, wrapped to [0, 360) degrees, halved and stored
    // one-based (0 is kept for the zero vector).
    void set_Direction(FPVector2 value);
    // 0x7b99238 - The camera's view direction.
    FPVector3 get_CameraSight() const;
    // 0x7b9930c - Where the camera orbits: the transform's position plus the orbit
    // direction scaled by the orbit distance. Throws for a null transform.
    FPVector3 GetCameraPosition(const Transform3D* transform) const;
    // 0x7b9945c
    FP get_CameraOrbitOffset() const;
    // 0x7b99490 - Clamped to [0, 16].
    void set_CameraOrbitOffset(FP value);
    // 0x7b994d0
    FPVector3 get_LookInputVector() const;
    // 0x7b995a4
    void set_LookInputVector(const FPVector3& value);

    // 0x7b98d70
    std::int32_t GetHashCode() const;
    // 0x7b98fa0 - Fields in layout order.
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7b98f30 - A zeroed Input filled from the serializer.
    static Input Read(FrameSerializer* serializer);
    // 0x7b99158
    static void Write(FrameSerializer* serializer, Input input);
};
static_assert(sizeof(Input) == Input::SIZE);

} // namespace Quantum
