// fp_lut.h - Photon.Deterministic.FPLut: the lookup tables behind FPMath's
// trigonometry, square root and exponential.
//
// The tables are game data, so none of their contents live in this repository;
// they are loaded at runtime from the user's own copy of the game:
//
//   * Six large tables are Unity TextAssets the original loads with
//     Resources.Load<TextAsset>("LUT/" + name).bytes (PhotonQuantum's
//     FPMathUtils.LoadLookupTables supplies that provider). Each file is a raw
//     little-endian array of its element type, copied as-is into the table.
//
//       name       table           element  size implied by how FPMath indexes it
//       FPSinCos   sin_cos_lut     int64    411774 entries (one per raw value in
//                                           [0, 2*pi)); low 32 bits = sin, high
//                                           32 bits = cos, both raw FP
//       FPAsin     asin_lut        int64    131073 entries (raw -1.0 .. +1.0)
//       FPAcos     acos_lut        int64    131073 entries (raw -1.0 .. +1.0)
//       FPAtan     atan_lut        int64    at least 397730 entries (see Atan);
//                                           the last entry is used for |x| > 10000
//       FPTan      tan_lut         int64    not read by any FPMath function in
//                                           this build; loaded for IsLoaded only
//       FPSqrt     sqrt_aprox_lut  int32    65537 entries: sqrt(i) * 2^14
//
//   * Two small tables were compiled into the original code as array
//     initialisers (static data of <PrivateImplementationDetails> stored in
//     global-metadata.dat). We can't embed them either, so our InitSmallLut
//     requests them from the same provider under their field names:
//
//       log2_approx_lut   uint32 x 66 (264 bytes): log2(1 + i/64) * 2^31
//       exp_integral_lut  int64  x 63 (504 bytes): e^n for n = -30..32, scaled
//                         by 2^42 (n < 0), 2^16 (0 <= n < 21) or 1 (n >= 21)
//
//     (The formulas describe what the values approximate; the original values
//     are not reproducible by a simple rounding rule, hence loading them.)
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace Photon::Deterministic {

class FPLut {
  public:
    // Photon.Deterministic.LutProvider: returns the bytes of the named table,
    // or nullopt if it can't be found (the original delegate returned null).
    using LutProvider =
        std::function<std::optional<std::vector<std::uint8_t>>(const std::string& path)>;

    // Names the original passes to the provider.
    static constexpr const char* SinCosName = "FPSinCos";
    static constexpr const char* TanName = "FPTan";
    static constexpr const char* AsinName = "FPAsin";
    static constexpr const char* AcosName = "FPAcos";
    static constexpr const char* AtanName = "FPAtan";
    static constexpr const char* SqrtName = "FPSqrt";
    // Our names for the two tables the original embedded in code.
    static constexpr const char* Log2ApproxName = "log2_approx_lut";
    static constexpr const char* ExpIntegralName = "exp_integral_lut";

    // Fixed sizes of the embedded tables (new uint[66] / new long[63]).
    static constexpr std::size_t Log2ApproxCount = 66;
    static constexpr std::size_t ExpIntegralCount = 63;

    // 0x7a882b8 FPLut.IsLoaded: true once the six file tables are non-empty.
    static bool IsLoaded();

    // 0x7a88368 FPLut.Init: loads the six file tables, then InitSmallLut.
    // Throws std::runtime_error if the provider has no data for a table.
    static void Init(const LutProvider& lutProvider);

    // 0x7a8850c FPLut.InitSmallLut. The original filled log2_approx_lut and
    // exp_integral_lut from constants in its code; here the data comes from the
    // provider (see the file comment). Throws std::invalid_argument when a
    // table has the wrong size.
    static void InitSmallLut(const LutProvider& lutProvider);

    // FPLut.Load<T> (generic; instantiated for long at 0x4e32fb8 and for int
    // at 0x4e32f00): fetches the bytes and reinterprets them as an array of T.
    // Like the original's Buffer.BlockCopy, a byte count that isn't a multiple
    // of sizeof(T) is an error (std::invalid_argument).
    template <typename T>
    static void Load(const LutProvider& lutProvider, const std::string& path, std::vector<T>& lut);

    // The tables (original static fields).
    static inline std::vector<std::int32_t> sqrt_aprox_lut;
    static inline std::vector<std::int64_t> asin_lut;
    static inline std::vector<std::int64_t> acos_lut;
    static inline std::vector<std::int64_t> atan_lut;
    static inline std::vector<std::int64_t> sin_cos_lut;
    static inline std::vector<std::int64_t> tan_lut;
    static inline std::vector<std::uint32_t> log2_approx_lut;
    static inline std::vector<std::int64_t> exp_integral_lut;

    // Bounds-checked read used by FPMath. The original threw
    // NullReferenceException for a table that was never loaded and
    // IndexOutOfRangeException for a bad index; we throw std::logic_error and
    // std::out_of_range respectively.
    template <typename T> static T Entry(const std::vector<T>& lut, std::int64_t index) {
        if (lut.empty()) {
            throw std::logic_error("FPLut: lookup table used before FPLut::Init");
        }
        if (index < 0 || static_cast<std::uint64_t>(index) >= lut.size()) {
            throw std::out_of_range("FPLut: lookup table index out of range");
        }
        return lut[static_cast<std::size_t>(index)];
    }
};

} // namespace Photon::Deterministic
