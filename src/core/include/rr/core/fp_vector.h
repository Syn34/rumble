// fp_vector.h - 2D/3D fixed-point vectors for the deterministic simulation.
#pragma once

#include "rr/core/fp.h"

namespace rr::det {

struct FPVector2 {
    FP x;
    FP y;

    static constexpr FPVector2 Zero() { return {}; }

    friend constexpr FPVector2 operator+(FPVector2 a, FPVector2 b) { return {a.x + b.x, a.y + b.y}; }
    friend constexpr FPVector2 operator-(FPVector2 a, FPVector2 b) { return {a.x - b.x, a.y - b.y}; }
    friend constexpr FPVector2 operator*(FPVector2 v, FP s) { return {v.x * s, v.y * s}; }
    friend constexpr FPVector2 operator*(FP s, FPVector2 v) { return v * s; }
    friend constexpr bool operator==(FPVector2 a, FPVector2 b) = default;

    static constexpr FP Dot(FPVector2 a, FPVector2 b) {
        return FP::FromRaw(FP::MulRaw(a.x.RawValue(), b.x.RawValue()) +
                           FP::MulRaw(a.y.RawValue(), b.y.RawValue()));
    }

    static constexpr FP DistanceSquared(FPVector2 a, FPVector2 b) {
        const FPVector2 d = a - b;
        return Dot(d, d);
    }

    constexpr FP SqrMagnitude() const { return Dot(*this, *this); }
};

struct FPVector3 {
    FP x;
    FP y;
    FP z;

    static constexpr FPVector3 Zero() { return {}; }
    static constexpr FPVector3 Up() { return {FP::Zero(), FP::One(), FP::Zero()}; }

    friend constexpr FPVector3 operator+(FPVector3 a, FPVector3 b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }
    friend constexpr FPVector3 operator-(FPVector3 a, FPVector3 b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }
    friend constexpr FPVector3 operator-(FPVector3 v) { return {-v.x, -v.y, -v.z}; }
    friend constexpr FPVector3 operator*(FPVector3 v, FP s) { return {v.x * s, v.y * s, v.z * s}; }
    friend constexpr FPVector3 operator*(FP s, FPVector3 v) { return v * s; }
    friend constexpr bool operator==(FPVector3 a, FPVector3 b) = default;

    // Each product is shifted before summing; this ordering affects the low bits
    // and must be kept for results to match the original simulation.
    static constexpr FP Dot(FPVector3 a, FPVector3 b) {
        return FP::FromRaw(FP::MulRaw(a.x.RawValue(), b.x.RawValue()) +
                           FP::MulRaw(a.y.RawValue(), b.y.RawValue()) +
                           FP::MulRaw(a.z.RawValue(), b.z.RawValue()));
    }

    static constexpr FP DistanceSquared(FPVector3 a, FPVector3 b) {
        const FPVector3 d = a - b;
        return Dot(d, d);
    }

    constexpr FP SqrMagnitude() const { return Dot(*this, *this); }
};

} // namespace rr::det
