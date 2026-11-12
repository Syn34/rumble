#include "fp_vector2.h"
#include "deterministic_frame_serializer.h"

#include "dotnet_number_format.h"
#include "fp_math.h"
#include "fp_vector3.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace Photon::Deterministic {

namespace {

// Shared by both Normalize overloads. The squared length is taken from the unshifted
// products (32 fractional bits) so short vectors keep their precision; FPMath then
// returns sqrt of it as a mantissa/exponent pair, and each component is scaled by
// 2^44 / mantissa and shifted back down. A zero vector stays zero.
struct NormalizeResult {
    FPVector2 direction;
    FP magnitude;
};

NormalizeResult NormalizeWithMagnitude(FPVector2 value) {
    const FP::Raw x = value.X.RawValue();
    const FP::Raw y = value.Y.RawValue();
    const auto squaredLength =
        static_cast<std::uint64_t>(detail::WrapAdd(detail::WrapMul(x, x), detail::WrapMul(y, y)));
    if (squaredLength == 0) {
        return {FPVector2::Zero(), FP::Zero()};
    }

    const FPMath::ExponentMantisaPair root = FPMath::GetSqrtExponentMantissa(squaredLength);
    const FP::Raw mantissa = root.Mantissa;
    const FP::Raw reciprocal = (FP::Raw{1} << 44) / mantissa;
    const int shift = (root.Exponent + 14) & 63; // the hardware shift uses the low 6 bits

    NormalizeResult result;
    result.direction = {FP::FromRaw(detail::WrapMul(reciprocal, x) >> shift),
                        FP::FromRaw(detail::WrapMul(reciprocal, y) >> shift)};
    const auto scaledMantissa =
        static_cast<FP::Raw>(static_cast<std::uint64_t>(mantissa) << (root.Exponent & 63));
    result.magnitude = FP::FromRaw(scaledMantissa >> 14);
    return result;
}

// Clamps a dot product of unit vectors into Acos's domain.
FP ClampCosine(FP cosine) {
    return FPMath::Clamp(cosine, -FP::One(), FP::One());
}

// +1 when b is counter-clockwise from a (or parallel), -1 otherwise. The original
// compares the two products of the cross term instead of subtracting them.
FP CounterClockwiseSign(FPVector2 a, FPVector2 b) {
    return a.X * b.Y >= a.Y * b.X ? FP::One() : -FP::One();
}

} // namespace

// 0x7a7186c
FPVector3 FPVector2::XYY() const {
    return {X, Y, Y};
}

// 0x7a72840
FPVector3 FPVector2::XOY() const {
    return {X, FP::Zero(), Y};
}

// 0x7a7285c
FPVector3 FPVector2::XYO() const {
    return {X, Y, FP::Zero()};
}

// 0x7a727f4
FP FPVector2::Magnitude() const {
    return FPMath::Sqrt(SqrMagnitude());
}

// 0x7a72824
FPVector2 FPVector2::Normalized() const {
    return Normalize(*this);
}

// 0x7a717a8
FPVector2 FPVector2::Normalize(FPVector2 value) {
    return NormalizeWithMagnitude(value).direction;
}

// 0x7a71800
FPVector2 FPVector2::Normalize(FPVector2 value, FP& magnitude) {
    const NormalizeResult result = NormalizeWithMagnitude(value);
    magnitude = result.magnitude;
    return result.direction;
}

// 0x7a71a74
FPVector2 FPVector2::ClampMagnitude(FPVector2 vector, FP maxLength) {
    if (vector.SqrMagnitude() <= maxLength * maxLength) {
        return vector;
    }
    // Unlike FPVector3.ClampMagnitude this divides by the length directly, and treats
    // anything no longer than FP.EN3 as zero.
    const FP length = FPMath::Sqrt(vector.SqrMagnitude());
    if (length <= detail::Epsilon) {
        return Zero();
    }
    return {vector.X / length * maxLength, vector.Y / length * maxLength};
}

// 0x7a7187c
FP FPVector2::Distance(FPVector2 a, FPVector2 b) {
    return FPMath::Sqrt(DistanceSquared(a, b));
}

// 0x7a71af0
FPVector2 FPVector2::Rotate(FPVector2 vector, FP radians) {
    FP sin;
    FP cos;
    FPMath::SinCos(radians, sin, cos);
    return Rotate(vector, sin, cos);
}

// 0x7a71d38
FP FPVector2::Radians(FPVector2 a, FPVector2 b) {
    return FPMath::Acos(ClampCosine(Dot(Normalize(a), Normalize(b))));
}

// 0x7a71e28
FP FPVector2::RadiansSkipNormalize(FPVector2 a, FPVector2 b) {
    return FPMath::Acos(ClampCosine(Dot(a, b)));
}

// 0x7a71e54
FP FPVector2::RadiansSigned(FPVector2 a, FPVector2 b) {
    return Radians(a, b) * CounterClockwiseSign(a, b);
}

// 0x7a71eb4
FP FPVector2::RadiansSignedSkipNormalize(FPVector2 a, FPVector2 b) {
    return RadiansSkipNormalize(a, b) * CounterClockwiseSign(a, b);
}

// 0x7a71f28
bool FPVector2::IsClockWise(std::span<const FPVector2> vertices) {
    if (vertices.size() < 3) {
        throw std::out_of_range("FPVector2.IsClockWise needs at least three vertices");
    }
    const FPVector2 firstEdge = vertices[1] - vertices[0];
    const FPVector2 secondEdge = vertices[2] - vertices[1];
    // Cross(firstEdge, secondEdge) < 0, compared term against term.
    return secondEdge.Y * firstEdge.X < secondEdge.X * firstEdge.Y;
}

// 0x7a71f84
void FPVector2::MakeCounterClockWise(std::span<FPVector2> vertices) {
    if (IsClockWise(vertices)) {
        FlipWindingOrder(vertices);
    }
}

// 0x7a71fac
void FPVector2::FlipWindingOrder(std::span<FPVector2> vertices) {
    std::reverse(vertices.begin(), vertices.end());
}

// 0x7a71fb4
std::vector<FPVector2> FPVector2::CalculatePolygonNormals(std::span<const FPVector2> vertices) {
    std::vector<FPVector2> normals(vertices.size());
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const std::size_t next = i + 1 < vertices.size() ? i + 1 : 0;
        const FPVector2 edge = vertices[next] - vertices[i];
        // Right-hand perpendicular: outward for counter-clockwise polygons.
        normals[i] = Normalize(FPVector2{edge.Y, -edge.X});
    }
    return normals;
}

// 0x7a720b0
std::vector<FPVector2> FPVector2::RecenterPolygon(std::span<const FPVector2> vertices) {
    const FPVector2 centroid = CalculatePolygonCentroid(vertices);
    std::vector<FPVector2> recentred(vertices.begin(), vertices.end());
    for (FPVector2& vertex : recentred) {
        vertex = vertex - centroid;
    }
    return recentred;
}

// 0x7a722c4
FP FPVector2::CalculatePolygonArea(std::span<const FPVector2> vertices) {
    FP twiceArea;
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const FPVector2 current = vertices[i];
        const FPVector2 next = vertices[(i + 1) % vertices.size()];
        twiceArea += current.X * next.Y - current.Y * next.X;
    }
    return detail::DivInt(twiceArea, 2); // raw integer halving, truncating toward zero
}

// 0x7a721a0
FPVector2 FPVector2::CalculatePolygonCentroid(std::span<const FPVector2> vertices) {
    FP twiceArea;
    FPVector2 weightedSum;
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const FPVector2 current = vertices[i];
        const FPVector2 next = vertices[(i + 1) % vertices.size()];
        const FP cross = Cross(current, next);
        twiceArea += cross;
        weightedSum.X += cross * (current.X + next.X);
        weightedSum.Y += cross * (current.Y + next.Y);
    }
    if (twiceArea == FP::Zero()) {
        // The original throws an exception carrying this message.
        throw std::invalid_argument("Not a valid polygon");
    }
    const FP area = twiceArea * FP::Half();
    const FP divisor = area * FP::FromInt(6);
    // A raw area of +-1 still reaches here with a zero divisor; see DivOrZero.
    return {detail::DivOrZero(weightedSum.X, divisor), detail::DivOrZero(weightedSum.Y, divisor)};
}

// 0x7a72348
FP FPVector2::CalculatePolygonInertiaFactor(std::span<const FPVector2> vertices) {
    FP numerator;
    FP denominator;
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const FPVector2 current = vertices[i];
        const FPVector2 next = vertices[(i + 1) % vertices.size()];
        const FP cross = Cross(current, next);
        const FP squares = Dot(current, current) + Dot(current, next) + Dot(next, next);
        numerator += squares * cross;
        denominator += cross;
    }
    // An empty or zero-area polygon divides by zero here; the original then returns
    // garbage, we return zero.
    return detail::DivOrZero(numerator, denominator * FP::FromInt(6));
}

// 0x7a72418
FPVector2 FPVector2::CalculatePolygonLocalSupport(const FPVector2* vertices, int verticesCount,
                                                  const FPVector2& localDir) {
    const FP firstDot = Dot(vertices[0], localDir);
    FP bestDot = Dot(vertices[1], localDir);
    if (bestDot > firstDot) {
        // Climb forwards while the next vertex is strictly further along localDir.
        int best = 1;
        while (best + 1 < verticesCount) {
            const FP nextDot = Dot(vertices[best + 1], localDir);
            if (nextDot <= bestDot) {
                break;
            }
            bestDot = nextDot;
            ++best;
        }
        return vertices[best];
    }

    // Otherwise climb backwards from the last vertex. The backward walk never steps
    // below index 2 (vertex 1 was already ruled out above).
    int best = verticesCount - 1;
    bestDot = Dot(vertices[best], localDir);
    if (bestDot <= firstDot) {
        return vertices[0];
    }
    while (best - 1 >= 2) {
        const FP nextDot = Dot(vertices[best - 1], localDir);
        if (nextDot <= bestDot) {
            break;
        }
        bestDot = nextDot;
        --best;
    }
    return vertices[best];
}

// 0x7a72558
FP FPVector2::CalculatePolygonRadius(std::span<const FPVector2> vertices) {
    FP radius;
    for (const FPVector2& vertex : vertices) {
        radius = FPMath::Max(radius, vertex.Magnitude());
    }
    return radius;
}

// 0x7a72940
std::string FPVector2::ToString() const {
    return "(" + detail::FormatSingle(X.AsFloat()) + ", " + detail::FormatSingle(Y.AsFloat()) + ")";
}

// TODO(0x7a726a0, Photon.Deterministic.FPVector2.Serialize): serialises X then Y through
// FP.Serialize (0x7a726d0) with the frame serializer; needs IDeterministicFrameSerializer
// and FP.Serialize, which belong to the serialisation work.
void FPVector2::Serialize(void* ptr, IDeterministicFrameSerializer* serializer) {
    // Each component goes through the stream in declaration order.
    auto* value = static_cast<FPVector2*>(ptr);
    assert(value != nullptr && serializer != nullptr);
    FP::Serialize(&value->X, *serializer);
    FP::Serialize(&value->Y, *serializer);
}

} // namespace Photon::Deterministic
