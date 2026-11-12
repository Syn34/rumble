// fp_math_utility.h - gameplay maths helpers.
//
// Original: Quantum.FPMathUtility (quantum.code.dll), a static helper class
// used by the deterministic simulation (movement, AI, network quantisation).
#pragma once

#include "rr/core/fp.h"
#include "rr/core/fp_vector.h"

#include <cstdint>

namespace Quantum {

using rr::det::FP;
using rr::det::FPVector2;
using rr::det::FPVector3;

class FPMathUtility {
public:
    // True if any bit of `flag` is set in `flags`.
    static bool Contains(std::int32_t flags, std::int32_t flag);

    // Equality within FP::EN3 (squared distance for vectors, absolute difference for scalars).
    static bool EqualWithEpsilon(const FPVector3& a, const FPVector3& b);
    static bool EqualWithEpsilon(FP a, FP b);

    static FP SquareOf(FP value);

    // Note: evaluated in floating point by the original as well, so this is not
    // bit-exact across platforms. Avoid in new simulation code.
    static FP Pow(FP base, FP exponent);

    static FP DegreesToRadians(FP degrees);
    static FP RadiansToDegrees(FP radians);

    // Remaps `value` from [inMin, inMax] to [outMin, outMax], clamping at the ends.
    static FP Map(FP inMin, FP inMax, FP outMin, FP outMax, FP value);

    // Compresses a [0,1] value into a byte (0..255) for network / state packing.
    static std::uint8_t QuantizeFP01ToByte(FP value01);
    // Compresses a value in [min, max] into a byte.
    static std::uint8_t QuantizeFPToByte(FP value, FP min, FP max);
    // Inverse of QuantizeFPToByte; 0 and 255 map exactly onto min and max.
    static FP UnQuantizeByteToFP(std::uint8_t quantized, FP min, FP max);

    static FPVector3 ComponentMax(const FPVector3& a, const FPVector3& b);
    static FP GetMaxComponent(const FPVector3& v);

    // Closest point to `point` on the segment [lineStart, lineEnd].
    static FPVector3 ClosestPointOnLine(const FPVector3& point, const FPVector3& lineStart,
                                        const FPVector3& lineEnd);

    // Squared distance from `point` to the segment [a, b] on the ground plane.
    static FP DistanceSqrToSegment(FPVector2 point, FPVector2 a, FPVector2 b);

    // TODO(0x7be2620, Quantum.FPMathUtility.RandomOnUnitCircle): needs the
    // simulation RNG (Quantum RNGSession) and the deterministic sin/cos tables,
    // neither of which has been reconstructed yet. Picks a uniformly random
    // angle and returns the unit vector for it.
    //
    // TODO(0x7be2af0, Quantum.FPMathUtility.OrientOnNormal): needs FPQuaternion.
    // Builds a rotation that keeps the input's facing but aligns "up" with a
    // surface normal (special-cased when the normal is almost straight up).
};

} // namespace Quantum
