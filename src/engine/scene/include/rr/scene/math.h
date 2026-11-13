// math.h - small single-precision maths for the scene layer.
//
// Conventions: right-handed, Y up, the camera looks down -Z; matrices are
// column-major (like GLSL) and act on column vectors; clip space has depth in
// [0, 1] (the renderer's convention, see rr/render/render_device.h).
#pragma once

#include <array>
#include <cmath>

namespace rr::scene {

struct Vec3 {
    float x = 0, y = 0, z = 0;

    friend Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
    friend Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
    friend Vec3 operator-(Vec3 a) { return {-a.x, -a.y, -a.z}; }
    friend Vec3 operator*(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
    friend Vec3 operator*(float s, Vec3 a) { return a * s; }
    friend bool operator==(Vec3 a, Vec3 b) = default;
};

inline float Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 Cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline float Length(Vec3 a) { return std::sqrt(Dot(a, a)); }
inline Vec3 Normalize(Vec3 a) {
    const float l = Length(a);
    return l > 0 ? a * (1.0f / l) : Vec3{};
}
inline Vec3 Min(Vec3 a, Vec3 b) { return {std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z)}; }
inline Vec3 Max(Vec3 a, Vec3 b) { return {std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z)}; }

struct Quat {
    float x = 0, y = 0, z = 0, w = 1;

    friend Quat operator*(Quat a, Quat b) {
        return {a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y, a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
                a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x, a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z};
    }
};

Quat AxisAngle(Vec3 axis, float radians);
Vec3 Rotate(Quat q, Vec3 v);
Quat Normalize(Quat q);

struct Mat4 {
    std::array<float, 16> m{}; // column-major: element (row, col) at m[col * 4 + row]

    static Mat4 Identity();
    float& At(int row, int col) { return m[static_cast<std::size_t>(col * 4 + row)]; }
    float At(int row, int col) const { return m[static_cast<std::size_t>(col * 4 + row)]; }
    Vec3 Translation() const { return {At(0, 3), At(1, 3), At(2, 3)}; }
    // Column `c` as a vector (the image of an axis for c < 3).
    Vec3 Axis(int c) const { return {At(0, c), At(1, c), At(2, c)}; }

    friend Mat4 operator*(const Mat4& a, const Mat4& b);
};

Mat4 Translation(Vec3 t);
Mat4 ScaleMatrix(Vec3 s);
Mat4 RotationMatrix(Quat q);
// translation * rotation * scale (the order transforms compose in).
Mat4 Trs(Vec3 t, Quat r, Vec3 s);
Vec3 TransformPoint(const Mat4& m, Vec3 p);
Vec3 TransformVector(const Mat4& m, Vec3 v);
// General inverse; returns the identity for singular matrices.
Mat4 Inverse(const Mat4& m);
Mat4 Transpose(const Mat4& m);
// Right-handed perspective projection with depth in [0, 1].
Mat4 Perspective(float fovY, float aspect, float zNear, float zFar);
// View matrix of a camera at `eye` looking at `target`.
Mat4 LookAt(Vec3 eye, Vec3 target, Vec3 up);
// Largest scale factor along the matrix's three axes.
float MaxAxisScale(const Mat4& m);

// Axis-aligned box.
struct Aabb {
    Vec3 min{1e30f, 1e30f, 1e30f};
    Vec3 max{-1e30f, -1e30f, -1e30f};

    static Aabb FromCenterExtent(Vec3 center, Vec3 extent) { return {center - extent, center + extent}; }
    bool Valid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }
    Vec3 Center() const { return (min + max) * 0.5f; }
    Vec3 Extent() const { return (max - min) * 0.5f; }
    void Add(Vec3 p) {
        min = Min(min, p);
        max = Max(max, p);
    }
    void Add(const Aabb& b) {
        if (!b.Valid()) return;
        min = Min(min, b.min);
        max = Max(max, b.max);
    }
};

// Bounds of a box after transforming it (exact for the box's corners).
Aabb TransformAabb(const Mat4& m, const Aabb& box);

// Six clip planes (ax + by + cz + d >= 0 inside) of a view-projection matrix in
// the [0, 1] depth convention: left, right, bottom, top, near, far.
struct Frustum {
    std::array<std::array<float, 4>, 6> planes{};

    static Frustum FromViewProjection(const Mat4& viewProjection);
    // False only when the box is certainly outside.
    bool Intersects(const Aabb& box) const;
};

// --- handedness ---------------------------------------------------------------------
//
// The game's content is left-handed (Y up, Z forward, clockwise front faces). The
// scene layer mirrors the X axis to get the same shapes in the renderer's
// right-handed convention: positions and directions (x, y, z) -> (-x, y, z),
// rotations (x, y, z, w) -> (x, -y, -z, w), scales unchanged, matrices M -> S M S
// with S = diag(-1, 1, 1, 1), and every triangle's vertex order reversed.

inline Vec3 MirrorPosition(Vec3 p) { return {-p.x, p.y, p.z}; }
inline Quat MirrorRotation(Quat q) { return {q.x, -q.y, -q.z, q.w}; }
Mat4 MirrorMatrix(const Mat4& m);

} // namespace rr::scene
