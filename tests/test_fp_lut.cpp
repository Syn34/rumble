#include "test_framework.h"

#include "photon_deterministic/fp_lut.h"

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

using Photon::Deterministic::FPLut;

namespace {

template <typename Exception, typename Fn> bool Throws(Fn&& fn) {
    try {
        fn();
    } catch (const Exception&) {
        return true;
    }
    return false;
}

template <typename T> std::vector<std::uint8_t> LittleEndianBytes(const std::vector<T>& values) {
    std::vector<std::uint8_t> bytes;
    for (T value : values) {
        auto bits = static_cast<std::uint64_t>(value);
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            bytes.push_back(static_cast<std::uint8_t>(bits & 0xFF));
            bits >>= 8;
        }
    }
    return bytes;
}

FPLut::LutProvider ProviderFor(std::map<std::string, std::vector<std::uint8_t>> files) {
    return [files = std::move(files)](
               const std::string& path) -> std::optional<std::vector<std::uint8_t>> {
        const auto it = files.find(path);
        if (it == files.end()) {
            return std::nullopt;
        }
        return it->second;
    };
}

} // namespace

TEST_CASE("FPLut::Load decodes little-endian arrays") {
    const auto provider = ProviderFor({
        {"longs", {0x01, 0, 0, 0, 0, 0, 0, 0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
        {"ints", {0x78, 0x56, 0x34, 0x12}},
        {"odd", {1, 2, 3}},
    });
    std::vector<std::int64_t> longs;
    FPLut::Load(provider, "longs", longs);
    CHECK(longs.size() == 2 && longs[0] == 1 && longs[1] == -2);

    std::vector<std::int32_t> ints;
    FPLut::Load(provider, "ints", ints);
    CHECK(ints.size() == 1 && ints[0] == 0x12345678);

    CHECK(Throws<std::invalid_argument>([&] { FPLut::Load(provider, "odd", ints); }));
    CHECK(Throws<std::runtime_error>([&] { FPLut::Load(provider, "missing", ints); }));
}

TEST_CASE("FPLut::Init requests the original table names and IsLoaded checks them") {
    std::map<std::string, std::vector<std::uint8_t>> files = {
        {"FPSinCos", LittleEndianBytes<std::int64_t>({1, 2})},
        {"FPTan", LittleEndianBytes<std::int64_t>({3})},
        {"FPAsin", LittleEndianBytes<std::int64_t>({4})},
        {"FPAcos", LittleEndianBytes<std::int64_t>({5})},
        {"FPAtan", LittleEndianBytes<std::int64_t>({6})},
        {"FPSqrt", LittleEndianBytes<std::int32_t>({7, 8, 9})},
        {"log2_approx_lut", LittleEndianBytes(std::vector<std::uint32_t>(66, 10u))},
        {"exp_integral_lut", LittleEndianBytes(std::vector<std::int64_t>(63, 11))},
    };
    FPLut::Init(ProviderFor(files));
    CHECK(FPLut::IsLoaded());
    CHECK(FPLut::sin_cos_lut.size() == 2 && FPLut::sin_cos_lut[1] == 2);
    CHECK(FPLut::tan_lut[0] == 3 && FPLut::asin_lut[0] == 4 && FPLut::acos_lut[0] == 5);
    CHECK(FPLut::atan_lut[0] == 6 && FPLut::sqrt_aprox_lut.size() == 3);
    CHECK(FPLut::log2_approx_lut.size() == 66 && FPLut::log2_approx_lut[65] == 10u);
    CHECK(FPLut::exp_integral_lut.size() == 63 && FPLut::exp_integral_lut[0] == 11);

    // The embedded tables have fixed sizes.
    auto wrongSize = files;
    wrongSize["exp_integral_lut"] = LittleEndianBytes(std::vector<std::int64_t>(62, 11));
    CHECK(Throws<std::invalid_argument>([&] { FPLut::InitSmallLut(ProviderFor(wrongSize)); }));

    // An empty table counts as not loaded.
    FPLut::tan_lut.clear();
    CHECK(!FPLut::IsLoaded());
}

TEST_CASE("FPLut::Entry checks for missing tables and bad indices") {
    const std::vector<std::int64_t> empty;
    const std::vector<std::int64_t> three{10, 20, 30};
    CHECK(Throws<std::logic_error>([&] { FPLut::Entry(empty, 0); }));
    CHECK(FPLut::Entry(three, 2) == 30);
    CHECK(Throws<std::out_of_range>([&] { FPLut::Entry(three, 3); }));
    CHECK(Throws<std::out_of_range>([&] { FPLut::Entry(three, -1); }));
}
