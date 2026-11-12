#include "test_framework.h"

#include "photon_deterministic/bit_stream.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using Photon::Deterministic::BitStream;
using Photon::Deterministic::Encoding;
using Photon::Deterministic::FP;

TEST_CASE("BitStream packs bits LSB first") {
    BitStream s(4);
    s.SetWriting(true);
    s.WriteByte(0b101, 3); // bits 0..2
    s.WriteByte(0x7F, 7);  // bits 3..9 straddle into byte 1
    s.WriteBool(true);     // bit 10
    CHECK(s.Position() == 11);
    CHECK(s.BytesRequired() == 2);
    CHECK(s.Data()[0] == static_cast<std::uint8_t>(0b101 | (0x7F << 3)));
    CHECK(s.Data()[1] == static_cast<std::uint8_t>((0x7F >> 5) | (1 << 2)));

    BitStream r(s.Data());
    CHECK(r.ReadByte(3) == 0b101);
    CHECK(r.ReadByte(7) == 0x7F);
    CHECK(r.ReadBool());
    CHECK(!r.ReadBool());
}

TEST_CASE("BitStream WriteByteAt keeps neighbouring bits") {
    std::vector<std::uint8_t> data = {0xFF, 0xFF};
    BitStream::WriteByteAt(data, 2, 3, 0);
    CHECK(data[0] == 0xE3);
    BitStream::WriteByteAt(data, 6, 4, 0b1001);
    CHECK(data[0] == 0x63);
    CHECK(data[1] == 0xFE);
    // Only the low `bits` bits of the value are used.
    BitStream::WriteByteAt(data, 8, 1, 0xFE);
    CHECK(data[1] == 0xFE);
    // Zero bits writes nothing.
    BitStream::WriteByteAt(data, 0, 0, 0);
    CHECK(data[0] == 0x63);
}

TEST_CASE("BitStream multi-byte values are low byte first and use exact bit counts") {
    BitStream s(16);
    s.SetWriting(true);
    s.WriteUInt(0x12345678u);
    CHECK(s.Data()[0] == 0x78);
    CHECK(s.Data()[3] == 0x12);

    s.Reset();
    s.SetWriting(true);
    s.WriteUInt(0x1FFFFu, 17);
    CHECK(s.Position() == 17);
    s.WriteUShort(0x2AB, 10);
    CHECK(s.Position() == 27);
    s.WriteULong(0x1'2345'6789ull, 33);
    CHECK(s.Position() == 60);

    BitStream r(s.Data());
    CHECK(r.ReadUInt(17) == 0x1FFFFu);
    CHECK(r.ReadUShort(10) == 0x2AB);
    CHECK(r.ReadULong(33) == 0x1'2345'6789ull);
    // Out-of-range bit counts do nothing.
    const int before = r.Position();
    CHECK(r.ReadUInt(0) == 0u);
    CHECK(r.ReadUInt(40) == 0u);
    CHECK(r.Position() == before);
}

TEST_CASE("BitStream round-trips every primitive at an unaligned position") {
    BitStream s(128);
    s.SetWriting(true);
    s.WriteBool(true); // misalign everything that follows
    s.WriteByte(0xA5);
    s.WriteSByte(-3);
    s.WriteUShort(0xBEEF);
    s.WriteShort(-12345);
    s.WriteUInt(0xDEADBEEFu);
    s.WriteInt(-123456789);
    s.WriteULong(0xFEDCBA9876543210ull);
    s.WriteLong(-9876543210123ll);
    s.WriteDouble(-1.5e300);
    s.WriteFP(FP::FromRaw(-123456789012ll));
    s.WriteInt(-2, 5);
    s.WriteLong(-1, 40);
    s.WriteShort(-1, 12);

    BitStream r(s.Data());
    CHECK(r.ReadBoolean());
    CHECK(r.ReadByte() == 0xA5);
    CHECK(r.ReadSByte() == -3);
    CHECK(r.ReadUShort() == 0xBEEF);
    CHECK(r.ReadShort() == -12345);
    CHECK(r.ReadUInt() == 0xDEADBEEFu);
    CHECK(r.ReadInt() == -123456789);
    CHECK(r.ReadULong() == 0xFEDCBA9876543210ull);
    CHECK(r.ReadLong() == -9876543210123ll);
    CHECK(r.ReadDouble() == -1.5e300);
    CHECK(r.ReadFP() == FP::FromRaw(-123456789012ll));
    // Narrow reads are not sign-extended.
    CHECK(r.ReadInt(5) == 0x1E);
    CHECK(r.ReadLong(40) == 0xFF'FFFF'FFFFll);
    CHECK(r.ReadShort(12) == 0x0FFF);
}

TEST_CASE("BitStream byte arrays aligned and unaligned") {
    const std::vector<std::uint8_t> payload = {1, 2, 3, 0x80, 0xFF, 0x7E};
    for (int lead : {0, 3}) {
        BitStream s(16);
        s.SetWriting(true);
        s.WriteByte(0x5, lead);
        s.WriteByteArray(payload, 1, 4);
        CHECK(s.Position() == lead + 32);

        BitStream r(s.Data());
        CHECK(r.ReadByte(lead) == (lead == 0 ? 0 : 0x5));
        std::vector<std::uint8_t> out(6, 0xEE);
        r.ReadByteArray(out, 2, 4);
        CHECK(out[0] == 0xEE);
        CHECK(out[2] == 2);
        CHECK(out[5] == 0xFF);
    }
}

TEST_CASE("BitStream length-prefixed arrays and strings") {
    BitStream s(64);
    s.SetWriting(true);
    const std::vector<std::uint8_t> bytes = {9, 8, 7};
    s.WriteByteArrayLengthPrefixed(std::span<const std::uint8_t>(bytes));
    s.WriteByteArrayLengthPrefixed(std::nullopt);
    s.WriteByteArrayLengthPrefixed(std::span<const std::uint8_t>(bytes), 2);
    s.WriteString(std::string_view("héllo"), Encoding::UTF8);
    s.WriteString(std::nullopt, Encoding::UTF8);
    s.WriteString(std::string_view(""), Encoding::UTF8);

    BitStream r(s.Data());
    const auto a = r.ReadByteArrayLengthPrefixed();
    CHECK(a.has_value() && *a == bytes);
    CHECK(!r.ReadByteArrayLengthPrefixed().has_value());
    const auto truncated = r.ReadByteArrayLengthPrefixed();
    CHECK(truncated.has_value() && truncated->size() == 2 && (*truncated)[1] == 8);
    CHECK(r.ReadString(Encoding::UTF8) == std::optional<std::string>("héllo"));
    CHECK(!r.ReadString(Encoding::UTF8).has_value());
    CHECK(r.ReadString(Encoding::UTF8) == std::optional<std::string>(""));
    CHECK(r.Position() == s.Position());

    // The string flag means "is null": a null string is a single 1 bit.
    BitStream n(1);
    n.SetWriting(true);
    n.WriteString(std::nullopt, Encoding::UTF8);
    CHECK(n.Data()[0] == 1);
}

TEST_CASE("BitStream Serialize overloads are symmetric") {
    BitStream w(256);
    w.SetWriting(true);

    bool flag = true;
    std::uint8_t b = 200;
    std::int32_t i = -77;
    std::uint32_t u = 0x80000001u;
    std::uint32_t u9 = 300;
    std::int64_t l = -5;
    std::uint64_t ul = 0x0123456789ABCDEFull;
    std::uint64_t ul40 = 0xAB'CDEF'0123ull;
    FP fp = FP::FromRaw(-42);
    double d = 3.25;
    std::optional<std::string> str = std::string("photon");
    std::optional<std::vector<std::int32_t>> ints = std::vector<std::int32_t>{1, -2, 3};
    std::optional<std::vector<std::uint8_t>> blob = std::vector<std::uint8_t>{4, 5};
    std::optional<std::vector<std::uint8_t>> sized = std::vector<std::uint8_t>{6, 7, 8, 9};
    int sizedLength = 3;
    std::optional<std::vector<std::uint8_t>> fixed = std::vector<std::uint8_t>{1, 2, 3, 4};
    int fixedLength = 0;
    std::int16_t shorts[3] = {-1, 2, -3};
    std::uint64_t longs[2] = {1, ~0ull};
    std::int16_t sv = -300;
    std::int8_t sb = -4;

    CHECK(w.Condition(true));
    w.Serialize(flag);
    w.Serialize(b);
    w.Serialize(i);
    w.Serialize(u);
    w.Serialize(u9, 9);
    w.Serialize(l);
    w.Serialize(ul);
    w.Serialize(ul40, 40);
    w.Serialize(fp);
    w.Serialize(d);
    w.Serialize(str);
    w.Serialize(ints);
    w.Serialize(blob);
    w.Serialize(sized, sizedLength);
    w.Serialize(fixed, fixedLength, 4);
    CHECK(fixedLength == 4);
    w.SerializeBuffer(shorts, 3);
    w.SerializeBuffer(longs, 2);
    w.Serialize(&sv);
    w.Serialize(&sb);

    BitStream r(w.Data());
    CHECK(r.Reading());
    bool flag2 = false;
    std::uint8_t b2 = 0;
    std::int32_t i2 = 0;
    std::uint32_t u2 = 0;
    std::uint32_t u92 = 0;
    std::int64_t l2 = 0;
    std::uint64_t ul2 = 0;
    std::uint64_t ul402 = 0;
    FP fp2;
    double d2 = 0;
    std::optional<std::string> str2;
    std::optional<std::vector<std::int32_t>> ints2;
    std::optional<std::vector<std::uint8_t>> blob2;
    std::optional<std::vector<std::uint8_t>> sized2 = std::vector<std::uint8_t>(10, 0xEE);
    int sizedLength2 = 0;
    std::optional<std::vector<std::uint8_t>> fixed2;
    int fixedLength2 = 0;
    std::int16_t shorts2[3] = {};
    std::uint64_t longs2[2] = {};
    std::int16_t sv2 = 0;
    std::int8_t sb2 = 0;

    CHECK(r.Condition(false));
    r.Serialize(flag2);
    r.Serialize(b2);
    r.Serialize(i2);
    r.Serialize(u2);
    r.Serialize(u92, 9);
    r.Serialize(l2);
    r.Serialize(ul2);
    r.Serialize(ul402, 40);
    r.Serialize(fp2);
    r.Serialize(d2);
    r.Serialize(str2);
    r.Serialize(ints2);
    r.Serialize(blob2);
    r.Serialize(sized2, sizedLength2);
    r.Serialize(fixed2, fixedLength2, 4);
    r.SerializeBuffer(shorts2, 3);
    r.SerializeBuffer(longs2, 2);
    r.Serialize(&sv2);
    r.Serialize(&sb2);

    CHECK(flag2 == flag);
    CHECK(b2 == b);
    CHECK(i2 == i);
    CHECK(u2 == u);
    CHECK(u92 == u9);
    CHECK(l2 == l);
    CHECK(ul2 == ul);
    CHECK(ul402 == ul40);
    CHECK(fp2 == fp);
    CHECK(d2 == d);
    CHECK(str2 == str);
    CHECK(ints2 == ints);
    CHECK(blob2 == blob);
    // The existing, larger array is reused: only the first `length` bytes change.
    CHECK(sizedLength2 == 3);
    CHECK(sized2->size() == 10);
    CHECK((*sized2)[2] == 8);
    CHECK((*sized2)[3] == 0xEE);
    CHECK(fixed2 == fixed);
    CHECK(fixedLength2 == 4);
    CHECK(shorts2[0] == -1 && shorts2[2] == -3);
    CHECK(longs2[1] == ~0ull);
    CHECK(sv2 == sv);
    CHECK(sb2 == sb);
    CHECK(r.Position() == w.Position());
}

TEST_CASE("BitStream null and empty arrays in Serialize") {
    BitStream w(8);
    w.SetWriting(true);
    std::optional<std::vector<std::uint8_t>> none;
    std::optional<std::vector<std::uint8_t>> empty = std::vector<std::uint8_t>{};
    int length = 0;
    w.Serialize(none);
    w.Serialize(empty, length, 4); // empty counts as absent for the fixed-size form
    CHECK(w.Position() == 2);

    BitStream r(w.Data());
    std::optional<std::vector<std::uint8_t>> a = std::vector<std::uint8_t>{1};
    std::optional<std::vector<std::uint8_t>> b = std::vector<std::uint8_t>{1};
    r.Serialize(a);
    r.Serialize(b, length, 4);
    CHECK(!a.has_value());
    CHECK(!b.has_value());
}

TEST_CASE("BitStream windows, position and overflow") {
    std::vector<std::uint8_t> buffer(10, 0xAA);
    BitStream s(buffer, 4, 3);
    CHECK(s.Position() == 0);
    CHECK(s.Capacity() == 4);
    CHECK(s.CanRead(32));
    CHECK(!s.CanRead(33));

    s.SetWriting(true);
    s.Reset(); // zeroes only the window
    CHECK(buffer[2] == 0xAA);
    CHECK(buffer[3] == 0);
    CHECK(buffer[6] == 0);
    CHECK(buffer[7] == 0xAA);

    s.WriteByte(0x3, 2);
    CHECK(buffer[3] == 0x3);
    CHECK(s.RoundToByte() == 4); // absolute byte index
    CHECK(s.Position() == 8);
    s.WriteUShort(0x1234);
    CHECK(s.ToArray() == (std::vector<std::uint8_t>{0x3, 0x34, 0x12}));

    s.SetPosition(-5);
    CHECK(s.Position() == 0);
    s.SetPosition(1000);
    CHECK(s.Position() == 32);
    CHECK(!s.Overflowing());
    s.WriteBool(true);
    CHECK(s.Overflowing());

    const std::vector<std::uint8_t> incoming = {7, 8};
    s.CopyFromArray(incoming);
    CHECK(buffer[3] == 7);
    CHECK(s.Position() == 0);
    CHECK(s.CanRead(16));
    CHECK(!s.CanRead(17));

    s.Reset(1);
    CHECK(!s.CanRead(9));
}

TEST_CASE("BitStream reads past the buffer throw") {
    BitStream s(1);
    s.ReadByte();
    bool threw = false;
    try {
        s.ReadByte();
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("BitStream SerializeArrayLength sends only the count") {
    BitStream w(16);
    w.SetWriting(true);
    std::optional<std::vector<std::uint64_t>> values = std::vector<std::uint64_t>{5, 6, 7};
    std::optional<std::vector<std::uint64_t>> none;
    w.SerializeArrayLength(values);
    w.SerializeArrayLength(none);
    CHECK(w.Position() == 64);

    BitStream r(w.Data());
    std::optional<std::vector<std::uint64_t>> a;
    std::optional<std::vector<std::uint64_t>> b;
    r.SerializeArrayLength(a);
    r.SerializeArrayLength(b);
    CHECK(a.has_value() && a->size() == 3 && (*a)[0] == 0);
    CHECK(b.has_value() && b->empty());
}
