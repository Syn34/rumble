// fp_lut.cpp - Photon.Deterministic.FPLut: loading the lookup tables.
#include "fp_lut.h"

#include <type_traits>

namespace Photon::Deterministic {

namespace {

// Reads one little-endian integer of type T (the original copied the bytes on
// a little-endian device, so the files are little-endian).
template <typename T> T ReadLittleEndian(const std::uint8_t* bytes) {
    using Unsigned = std::make_unsigned_t<T>;
    Unsigned value = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        value = static_cast<Unsigned>(value | (static_cast<Unsigned>(bytes[i]) << (8 * i)));
    }
    return static_cast<T>(value);
}

std::vector<std::uint8_t> Fetch(const FPLut::LutProvider& lutProvider, const std::string& path) {
    if (!lutProvider) {
        throw std::logic_error("FPLut: no lookup table provider");
    }
    std::optional<std::vector<std::uint8_t>> bytes = lutProvider(path);
    if (!bytes) {
        throw std::runtime_error("FPLut: lookup table '" + path + "' not found");
    }
    return std::move(*bytes);
}

template <typename T>
std::vector<T> Decode(const std::vector<std::uint8_t>& bytes, const std::string& path) {
    if (bytes.size() % sizeof(T) != 0) {
        throw std::invalid_argument("FPLut: size of lookup table '" + path +
                                    "' is not a whole number of entries");
    }
    std::vector<T> values(bytes.size() / sizeof(T));
    for (std::size_t i = 0; i < values.size(); ++i) {
        values[i] = ReadLittleEndian<T>(bytes.data() + i * sizeof(T));
    }
    return values;
}

} // namespace

// 0x4e32fb8 (FPLut.Load<long>), 0x4e32f00 (FPLut.Load<int>)
template <typename T>
void FPLut::Load(const LutProvider& lutProvider, const std::string& path, std::vector<T>& lut) {
    lut = Decode<T>(Fetch(lutProvider, path), path);
}

template void FPLut::Load<std::int32_t>(const LutProvider&, const std::string&,
                                        std::vector<std::int32_t>&);
template void FPLut::Load<std::int64_t>(const LutProvider&, const std::string&,
                                        std::vector<std::int64_t>&);
template void FPLut::Load<std::uint32_t>(const LutProvider&, const std::string&,
                                         std::vector<std::uint32_t>&);

// 0x7a882b8
bool FPLut::IsLoaded() {
    return !sin_cos_lut.empty() && !tan_lut.empty() && !asin_lut.empty() && !acos_lut.empty() &&
           !atan_lut.empty() && !sqrt_aprox_lut.empty();
}

// 0x7a88368
void FPLut::Init(const LutProvider& lutProvider) {
    Load(lutProvider, SinCosName, sin_cos_lut);
    Load(lutProvider, TanName, tan_lut);
    Load(lutProvider, AsinName, asin_lut);
    Load(lutProvider, AcosName, acos_lut);
    Load(lutProvider, AtanName, atan_lut);
    Load(lutProvider, SqrtName, sqrt_aprox_lut);
    InitSmallLut(lutProvider);
}

// 0x7a8850c
void FPLut::InitSmallLut(const LutProvider& lutProvider) {
    // Decode both before assigning so a bad second table leaves no half state.
    std::vector<std::uint32_t> log2 =
        Decode<std::uint32_t>(Fetch(lutProvider, Log2ApproxName), Log2ApproxName);
    std::vector<std::int64_t> exp =
        Decode<std::int64_t>(Fetch(lutProvider, ExpIntegralName), ExpIntegralName);
    if (log2.size() != Log2ApproxCount || exp.size() != ExpIntegralCount) {
        throw std::invalid_argument("FPLut: embedded lookup tables have the wrong size");
    }
    log2_approx_lut = std::move(log2);
    exp_integral_lut = std::move(exp);
}

} // namespace Photon::Deterministic
