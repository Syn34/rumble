#include "rr/scene/math.h"

#include <algorithm>

namespace rr::scene {

Quat AxisAngle(Vec3 axis, float radians) {
    const Vec3 a = Normalize(axis);
    const float s = std::sin(radians * 0.5f);
    return {a.x * s, a.y * s, a.z * s, std::cos(radians * 0.5f)};
}

Vec3 Rotate(Quat q, Vec3 v) {
    // v' = v + 2w(u x v) + 2u x (u x v), with u the vector part of q.
    const Vec3 u{q.x, q.y, q.z};
    const Vec3 t = Cross(u, v) * 2.0f;
    return v + t * q.w + Cross(u, t);
}

Quat Normalize(Quat q) {
    const float l = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (l <= 0) return {};
    return {q.x / l, q.y / l, q.z / l, q.w / l};
}

Mat4 Mat4::Identity() {
    Mat4 r;
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 r;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) sum += a.At(row, k) * b.At(k, col);
            r.At(row, col) = sum;
        }
    }
    return r;
}

Mat4 Translation(Vec3 t) {
    Mat4 r = Mat4::Identity();
    r.At(0, 3) = t.x;
    r.At(1, 3) = t.y;
    r.At(2, 3) = t.z;
    return r;
}

Mat4 ScaleMatrix(Vec3 s) {
    Mat4 r;
    r.At(0, 0) = s.x;
    r.At(1, 1) = s.y;
    r.At(2, 2) = s.z;
    r.At(3, 3) = 1.0f;
    return r;
}

Mat4 RotationMatrix(Quat q) {
    const float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    const float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
    const float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
    Mat4 r = Mat4::Identity();
    r.At(0, 0) = 1 - 2 * (yy + zz);
    r.At(0, 1) = 2 * (xy - wz);
    r.At(0, 2) = 2 * (xz + wy);
    r.At(1, 0) = 2 * (xy + wz);
    r.At(1, 1) = 1 - 2 * (xx + zz);
    r.At(1, 2) = 2 * (yz - wx);
    r.At(2, 0) = 2 * (xz - wy);
    r.At(2, 1) = 2 * (yz + wx);
    r.At(2, 2) = 1 - 2 * (xx + yy);
    return r;
}

Mat4 Trs(Vec3 t, Quat r, Vec3 s) {
    Mat4 m = RotationMatrix(r);
    for (int row = 0; row < 3; ++row) {
        m.At(row, 0) *= s.x;
        m.At(row, 1) *= s.y;
        m.At(row, 2) *= s.z;
    }
    m.At(0, 3) = t.x;
    m.At(1, 3) = t.y;
    m.At(2, 3) = t.z;
    return m;
}

Vec3 TransformPoint(const Mat4& m, Vec3 p) {
    return {m.At(0, 0) * p.x + m.At(0, 1) * p.y + m.At(0, 2) * p.z + m.At(0, 3),
            m.At(1, 0) * p.x + m.At(1, 1) * p.y + m.At(1, 2) * p.z + m.At(1, 3),
            m.At(2, 0) * p.x + m.At(2, 1) * p.y + m.At(2, 2) * p.z + m.At(2, 3)};
}

Vec3 TransformVector(const Mat4& m, Vec3 v) {
    return {m.At(0, 0) * v.x + m.At(0, 1) * v.y + m.At(0, 2) * v.z,
            m.At(1, 0) * v.x + m.At(1, 1) * v.y + m.At(1, 2) * v.z,
            m.At(2, 0) * v.x + m.At(2, 1) * v.y + m.At(2, 2) * v.z};
}

Mat4 Inverse(const Mat4& a) {
    // Cofactor expansion over 2x2 sub-determinants.
    const auto& m = a.m;
    std::array<float, 16> inv{};
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +
             m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -
             m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +
             m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -
              m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] -
             m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] +
             m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] -
             m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] +
              m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
             m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] -
             m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] +
              m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] -
              m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -
             m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
              m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
    const float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (std::fabs(det) < 1e-30f) return Mat4::Identity();
    Mat4 r;
    for (std::size_t i = 0; i < 16; ++i) r.m[i] = inv[i] / det;
    return r;
}

Mat4 Transpose(const Mat4& a) {
    Mat4 r;
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col) r.At(row, col) = a.At(col, row);
    return r;
}

Mat4 Perspective(float fovY, float aspect, float zNear, float zFar) {
    const float f = 1.0f / std::tan(fovY * 0.5f);
    Mat4 r;
    r.At(0, 0) = f / aspect;
    r.At(1, 1) = f;
    r.At(2, 2) = zFar / (zNear - zFar);
    r.At(2, 3) = zNear * zFar / (zNear - zFar);
    r.At(3, 2) = -1.0f;
    return r;
}

Mat4 LookAt(Vec3 eye, Vec3 target, Vec3 up) {
    const Vec3 f = Normalize(target - eye);
    Vec3 s = Cross(f, up);
    if (Dot(s, s) < 1e-12f) s = Cross(f, Vec3{0, 0, 1});
    s = Normalize(s);
    const Vec3 u = Cross(s, f);
    Mat4 r = Mat4::Identity();
    r.At(0, 0) = s.x;
    r.At(0, 1) = s.y;
    r.At(0, 2) = s.z;
    r.At(1, 0) = u.x;
    r.At(1, 1) = u.y;
    r.At(1, 2) = u.z;
    r.At(2, 0) = -f.x;
    r.At(2, 1) = -f.y;
    r.At(2, 2) = -f.z;
    r.At(0, 3) = -Dot(s, eye);
    r.At(1, 3) = -Dot(u, eye);
    r.At(2, 3) = Dot(f, eye);
    return r;
}

float MaxAxisScale(const Mat4& m) {
    return std::max({Length(m.Axis(0)), Length(m.Axis(1)), Length(m.Axis(2))});
}

Aabb TransformAabb(const Mat4& m, const Aabb& box) {
    if (!box.Valid()) return box;
    // Centre/extent form: the new extent sums the absolute matrix columns.
    const Vec3 c = TransformPoint(m, box.Center());
    const Vec3 e = box.Extent();
    Vec3 extent;
    extent.x = std::fabs(m.At(0, 0)) * e.x + std::fabs(m.At(0, 1)) * e.y + std::fabs(m.At(0, 2)) * e.z;
    extent.y = std::fabs(m.At(1, 0)) * e.x + std::fabs(m.At(1, 1)) * e.y + std::fabs(m.At(1, 2)) * e.z;
    extent.z = std::fabs(m.At(2, 0)) * e.x + std::fabs(m.At(2, 1)) * e.y + std::fabs(m.At(2, 2)) * e.z;
    return Aabb::FromCenterExtent(c, extent);
}

Frustum Frustum::FromViewProjection(const Mat4& vp) {
    // Rows of the matrix; clip-space tests -w <= x, y <= w and 0 <= z <= w.
    auto row = [&](int r) { return std::array<float, 4>{vp.At(r, 0), vp.At(r, 1), vp.At(r, 2), vp.At(r, 3)}; };
    const auto r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);
    Frustum f;
    for (int i = 0; i < 4; ++i) {
        const auto k = static_cast<std::size_t>(i);
        f.planes[0][k] = r3[k] + r0[k]; // left
        f.planes[1][k] = r3[k] - r0[k]; // right
        f.planes[2][k] = r3[k] + r1[k]; // bottom
        f.planes[3][k] = r3[k] - r1[k]; // top
        f.planes[4][k] = r2[k];         // near (z >= 0)
        f.planes[5][k] = r3[k] - r2[k]; // far
    }
    for (auto& p : f.planes) {
        const float l = std::sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
        if (l > 0)
            for (float& v : p) v /= l;
    }
    return f;
}

bool Frustum::Intersects(const Aabb& box) const {
    if (!box.Valid()) return false;
    const Vec3 c = box.Center();
    const Vec3 e = box.Extent();
    for (const auto& p : planes) {
        const float distance = p[0] * c.x + p[1] * c.y + p[2] * c.z + p[3];
        const float radius = std::fabs(p[0]) * e.x + std::fabs(p[1]) * e.y + std::fabs(p[2]) * e.z;
        if (distance + radius < 0) return false;
    }
    return true;
}

Mat4 MirrorMatrix(const Mat4& m) {
    // S M S with S = diag(-1, 1, 1, 1): negate row 0 and column 0 (the corner twice).
    Mat4 r = m;
    for (int i = 0; i < 4; ++i) {
        r.At(0, i) = -r.At(0, i);
        r.At(i, 0) = -r.At(i, 0);
    }
    return r;
}

} // namespace rr::scene
