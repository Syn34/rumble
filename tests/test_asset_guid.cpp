#include "test_framework.h"

#include "quantum_core/asset_guid.h"

using Quantum::AssetGuid;

TEST_CASE("AssetGuid flags and formatting") {
    CHECK(!AssetGuid().IsValid());
    CHECK(AssetGuid(5).IsValid());
    CHECK(AssetGuid(-1).IsDynamic());
    CHECK(!AssetGuid(5).IsDynamic());
    CHECK(AssetGuid(0x1A2B).ToString() == "[0000000000001A2B]");
    CHECK(AssetGuid(0x1A2B).ToString(false) == "0000000000001A2B");
    CHECK(AssetGuid(0x0000000100000002).GetHashCode() == 3);
}

TEST_CASE("AssetGuid::NewGuid avoids reserved values and flag bits") {
    for (int i = 0; i < 1000; ++i) {
        const AssetGuid g = AssetGuid::NewGuid();
        CHECK(!(g.Value >= 1 && g.Value <= 255));
        CHECK(!g.IsDynamic());
        CHECK((g.Value & AssetGuid::ReservedBits) == 0);
    }
}
