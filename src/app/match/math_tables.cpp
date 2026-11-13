#include "math_tables.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_lut.h"
#include "rr/assets/il2cpp_metadata.h"

#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <optional>

namespace rr::app {

namespace {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPLut;
using Bytes = std::vector<std::uint8_t>;

// The small tables are told apart by values that follow from what they approximate:
// log2(1 + i/64) * 2^31 is 0 at i = 0 and 2^31 at i = 64; e^n * 2^16 is 2^16 at n = 0
// (index 30).
std::optional<Bytes> FindLog2Table(const std::vector<assets::Il2CppMetadata::StaticData>& candidates) {
    const std::size_t size = FPLut::Log2ApproxCount * sizeof(std::uint32_t);
    for (const auto& c : candidates) {
        if (c.available.size() < size) continue;
        std::uint32_t first = 0, at64 = 0;
        std::memcpy(&first, c.available.data(), 4);
        std::memcpy(&at64, c.available.data() + 64 * 4, 4);
        if (first == 0 && at64 == 0x80000000u) return Bytes(c.available.begin(), c.available.begin() + static_cast<std::ptrdiff_t>(size));
    }
    return std::nullopt;
}

std::optional<Bytes> FindExpTable(const std::vector<assets::Il2CppMetadata::StaticData>& candidates) {
    const std::size_t size = FPLut::ExpIntegralCount * sizeof(std::int64_t);
    for (const auto& c : candidates) {
        if (c.available.size() < size) continue;
        std::int64_t atZero = 0, atOne = 0;
        std::memcpy(&atZero, c.available.data() + 30 * 8, 8);
        std::memcpy(&atOne, c.available.data() + 31 * 8, 8);
        // e * 2^16 = 178145.6...
        if (atZero == FP::RAW_ONE && atOne > 178000 && atOne < 178300) {
            return Bytes(c.available.begin(), c.available.begin() + static_cast<std::ptrdiff_t>(size));
        }
    }
    return std::nullopt;
}

std::optional<Bytes> ReadFile(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) return std::nullopt;
    return Bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

template <class T> Bytes AsBytes(const std::vector<T>& values) {
    Bytes out(values.size() * sizeof(T));
    if (!out.empty()) std::memcpy(out.data(), values.data(), out.size());
    return out;
}

// Stand-ins computed from the functions the tables sample (same layouts as the originals,
// see fp_lut.h). Values can differ from the game's in the last bits.
std::map<std::string, Bytes> ApproximateTables() {
    const double one = static_cast<double>(FP::RAW_ONE);
    const auto raw = [one](double v) { return static_cast<std::int64_t>(std::llround(v * one)); };
    std::map<std::string, Bytes> tables;

    const std::int64_t angles = FP::PiTimes2().RawValue();
    std::vector<std::int64_t> sinCos(static_cast<std::size_t>(angles));
    for (std::int64_t i = 0; i < angles; ++i) {
        const double a = static_cast<double>(i) / one;
        const auto s = static_cast<std::uint32_t>(static_cast<std::int32_t>(raw(std::sin(a))));
        const auto c = static_cast<std::uint32_t>(static_cast<std::int32_t>(raw(std::cos(a))));
        sinCos[static_cast<std::size_t>(i)] = static_cast<std::int64_t>((std::uint64_t{c} << 32) | s);
    }
    tables[FPLut::SinCosName] = AsBytes(sinCos);

    const auto inverseCount = static_cast<std::size_t>(2 * FP::RAW_ONE + 1);
    std::vector<std::int64_t> asin(inverseCount), acos(inverseCount);
    for (std::size_t i = 0; i < inverseCount; ++i) {
        const double x = std::fmin(1.0, std::fmax(-1.0, (static_cast<double>(i) - one) / one));
        asin[i] = raw(std::asin(x));
        acos[i] = raw(std::acos(x));
    }
    tables[FPLut::AsinName] = AsBytes(asin);
    tables[FPLut::AcosName] = AsBytes(acos);

    // atan(|x|) per raw step up to 6, every 1/16 up to 250, every 16 up to 10000.
    std::vector<std::int64_t> atan;
    for (std::int64_t r = 0; r <= 6 * FP::RAW_ONE; ++r) atan.push_back(raw(std::atan(static_cast<double>(r) / one)));
    for (double x = 6.0 + 1.0 / 16; x <= 250.0; x += 1.0 / 16) atan.push_back(raw(std::atan(x)));
    for (double x = 250.0 + 16; x <= 10000.0 + 16; x += 16) atan.push_back(raw(std::atan(x)));
    tables[FPLut::AtanName] = AsBytes(atan);

    tables[FPLut::TanName] = AsBytes(std::vector<std::int64_t>{0}); // not read by FPMath

    std::vector<std::int32_t> sqrt(65537);
    for (std::size_t i = 0; i < sqrt.size(); ++i) {
        sqrt[i] = static_cast<std::int32_t>(std::llround(std::sqrt(static_cast<double>(i)) * 16384.0));
    }
    tables[FPLut::SqrtName] = AsBytes(sqrt);

    std::vector<std::uint32_t> log2(FPLut::Log2ApproxCount);
    for (std::size_t i = 0; i < log2.size(); ++i) {
        log2[i] = static_cast<std::uint32_t>(std::llround(std::log2(1.0 + static_cast<double>(i) / 64.0) * 2147483648.0));
    }
    tables[FPLut::Log2ApproxName] = AsBytes(log2);

    std::vector<std::int64_t> exp(FPLut::ExpIntegralCount);
    for (std::size_t i = 0; i < exp.size(); ++i) {
        const int n = static_cast<int>(i) - 30;
        const double scale = n < 0 ? 4398046511104.0 : (n < 21 ? 65536.0 : 1.0);
        exp[i] = static_cast<std::int64_t>(std::llround(std::exp(static_cast<double>(n)) * scale));
    }
    tables[FPLut::ExpIntegralName] = AsBytes(exp);
    return tables;
}

} // namespace

MathTablesReport LoadMathTables(assets::PlayerResources* resources, const std::filesystem::path& metadataFile,
                                const std::filesystem::path& lutDir, bool allowApproximation) {
    MathTablesReport report;
    std::map<std::string, Bytes> found;

    // Small tables from the metadata.
    if (auto metadata = assets::Il2CppMetadata::Load(metadataFile)) {
        const auto candidates = metadata->PrivateImplementationData("PhotonDeterministic.dll");
        if (auto log2 = FindLog2Table(candidates)) found[FPLut::Log2ApproxName] = std::move(*log2);
        else report.problems.push_back(std::string(FPLut::Log2ApproxName) + ": not found in " + metadataFile.string());
        if (auto exp = FindExpTable(candidates)) found[FPLut::ExpIntegralName] = std::move(*exp);
        else report.problems.push_back(std::string(FPLut::ExpIntegralName) + ": not found in " + metadataFile.string());
    } else {
        report.problems.push_back("global-metadata.dat not readable: " + metadataFile.string());
    }

    // Large tables: exported files first, then the player's Resources.
    for (const char* name : {FPLut::SinCosName, FPLut::TanName, FPLut::AsinName, FPLut::AcosName, FPLut::AtanName,
                             FPLut::SqrtName}) {
        if (!lutDir.empty()) {
            if (auto bytes = ReadFile(lutDir / (std::string(name) + ".bytes"))) {
                found[name] = std::move(*bytes);
                continue;
            }
        }
        std::string error = "no player Resources (globalgamemanagers not found)";
        if (resources != nullptr) {
            if (auto bytes = resources->LoadTextAsset(std::string("LUT/") + name, &error)) {
                found[name] = std::move(*bytes);
                continue;
            }
        }
        report.problems.push_back(std::string(name) + ": " + error);
    }

    std::map<std::string, Bytes> approximations;
    const FPLut::LutProvider provider = [&](const std::string& name) -> std::optional<Bytes> {
        if (auto it = found.find(name); it != found.end()) {
            report.fromGame.push_back(name);
            return it->second;
        }
        if (!allowApproximation) return std::nullopt;
        if (approximations.empty()) approximations = ApproximateTables();
        if (auto it = approximations.find(name); it != approximations.end()) {
            report.approximated.push_back(name);
            return it->second;
        }
        return std::nullopt;
    };
    try {
        FPLut::Init(provider);
        report.loaded = FPLut::IsLoaded();
    } catch (const std::exception& e) {
        report.problems.push_back(std::string("FPLut.Init: ") + e.what());
        report.loaded = false;
    }
    report.exact = report.loaded && report.approximated.empty();
    return report;
}

} // namespace rr::app
