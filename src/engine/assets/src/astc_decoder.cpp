// ASTC LDR block decoder, written from the Khronos ASTC specification.
//
// Terminology follows the specification: a block has a "block mode" (weight grid
// size, weight quantisation, dual plane), 1-4 partitions each with a "colour
// endpoint mode" (CEM) and a pair of endpoint colours, and a grid of weights that
// is bilinearly infilled to the block's texels. Integers are stored with the
// "integer sequence encoding" (ISE): plain bits, or bits plus trits (base 3,
// packed 5 per 8 bits) or quints (base 5, packed 3 per 7 bits).
#include "rr/assets/astc_decoder.h"

#include <algorithm>
#include <array>
#include <cstring>

namespace rr::assets::astc {
namespace {

constexpr int kMaxTexels = 12 * 12;
constexpr int kMaxWeights = 64;
constexpr int kMaxColorValues = 18;

// --- bit access -----------------------------------------------------------------

// Reads `count` bits starting at bit `start` of a little-endian bit string.
std::uint32_t ReadBits(const std::uint8_t* bytes, int start, int count) {
    std::uint32_t value = 0;
    for (int i = 0; i < count; ++i) {
        const int bit = start + i;
        value |= static_cast<std::uint32_t>((bytes[bit >> 3] >> (bit & 7)) & 1) << i;
    }
    return value;
}

// Sequential reader over a bit range; bits at or past `end` read as zero, which is
// how the specification pads the last, incomplete trit/quint group.
class BitStream {
public:
    BitStream(const std::uint8_t* bytes, int start, int end) : bytes_(bytes), pos_(start), end_(end) {}

    std::uint32_t Read(int count) {
        std::uint32_t value = 0;
        for (int i = 0; i < count; ++i, ++pos_) {
            if (pos_ < end_) value |= static_cast<std::uint32_t>((bytes_[pos_ >> 3] >> (pos_ & 7)) & 1) << i;
        }
        return value;
    }

private:
    const std::uint8_t* bytes_;
    int pos_;
    int end_;
};

// --- integer sequence encoding --------------------------------------------------

struct QuantMethod {
    int levels;
    int trits; // 0 or 1
    int quints;
    int bits;
};

// All quantisation levels, in the order the specification numbers them.
constexpr std::array<QuantMethod, 21> kQuantMethods = {{
    {2, 0, 0, 1},   {3, 1, 0, 0},   {4, 0, 0, 2},   {5, 0, 1, 0},   {6, 1, 0, 1},
    {8, 0, 0, 3},   {10, 0, 1, 1},  {12, 1, 0, 2},  {16, 0, 0, 4},  {20, 0, 1, 2},
    {24, 1, 0, 3},  {32, 0, 0, 5},  {40, 0, 1, 3},  {48, 1, 0, 4},  {64, 0, 0, 6},
    {80, 0, 1, 4},  {96, 1, 0, 5},  {128, 0, 0, 7}, {160, 0, 1, 5}, {192, 1, 0, 6},
    {256, 0, 0, 8},
}};
constexpr int kQuant6Index = 4; // colour endpoints need at least 6 levels

int IseBitCount(int count, const QuantMethod& q) {
    int bits = count * q.bits;
    if (q.trits) bits += (count * 8 + 4) / 5;
    if (q.quints) bits += (count * 7 + 2) / 3;
    return bits;
}

struct IseValue {
    std::uint8_t high; // trit or quint value (0 when the range is plain bits)
    std::uint8_t low;  // the plain-bit part
};

void UnpackTrits(std::uint32_t t, int out[5]) {
    auto bit = [](std::uint32_t v, int i) { return static_cast<int>((v >> i) & 1); };
    std::uint32_t c;
    if (((t >> 2) & 7) == 7) {
        c = (((t >> 5) & 7) << 2) | (t & 3);
        out[4] = 2;
        out[3] = 2;
    } else {
        c = t & 0x1F;
        if (((t >> 5) & 3) == 3) {
            out[4] = 2;
            out[3] = bit(t, 7);
        } else {
            out[4] = bit(t, 7);
            out[3] = static_cast<int>((t >> 5) & 3);
        }
    }
    if ((c & 3) == 3) {
        out[2] = 2;
        out[1] = bit(c, 4);
        out[0] = (bit(c, 3) << 1) | (bit(c, 2) & (bit(c, 3) ^ 1));
    } else if (((c >> 2) & 3) == 3) {
        out[2] = 2;
        out[1] = 2;
        out[0] = static_cast<int>(c & 3);
    } else {
        out[2] = bit(c, 4);
        out[1] = static_cast<int>((c >> 2) & 3);
        out[0] = (bit(c, 1) << 1) | (bit(c, 0) & (bit(c, 1) ^ 1));
    }
}

void UnpackQuints(std::uint32_t q, int out[3]) {
    auto bit = [](std::uint32_t v, int i) { return static_cast<int>((v >> i) & 1); };
    if (((q >> 1) & 3) == 3 && ((q >> 5) & 3) == 0) {
        const int q0 = bit(q, 0);
        out[2] = (q0 << 2) | ((bit(q, 4) & (q0 ^ 1)) << 1) | (bit(q, 3) & (q0 ^ 1));
        out[1] = 4;
        out[0] = 4;
        return;
    }
    std::uint32_t c;
    if (((q >> 1) & 3) == 3) {
        out[2] = 4;
        c = (((q >> 3) & 3) << 3) | (((~q >> 5) & 3) << 1) | (q & 1);
    } else {
        out[2] = static_cast<int>((q >> 5) & 3);
        c = q & 0x1F;
    }
    if ((c & 7) == 5) {
        out[1] = 4;
        out[0] = static_cast<int>((c >> 3) & 3);
    } else {
        out[1] = static_cast<int>((c >> 3) & 3);
        out[0] = static_cast<int>(c & 7);
    }
}

// Decodes `count` integers stored from bit `start` with quantisation `q`.
void DecodeIse(const std::uint8_t* bytes, int start, int count, const QuantMethod& q, IseValue* out) {
    BitStream s(bytes, start, start + IseBitCount(count, q));
    const int b = q.bits;
    int done = 0;
    if (q.trits) {
        while (done < count) {
            std::uint32_t m[5];
            std::uint32_t t = 0;
            m[0] = s.Read(b);
            t |= s.Read(2);
            m[1] = s.Read(b);
            t |= s.Read(2) << 2;
            m[2] = s.Read(b);
            t |= s.Read(1) << 4;
            m[3] = s.Read(b);
            t |= s.Read(2) << 5;
            m[4] = s.Read(b);
            t |= s.Read(1) << 7;
            int trits[5];
            UnpackTrits(t, trits);
            for (int i = 0; i < 5 && done < count; ++i, ++done) {
                out[done] = {static_cast<std::uint8_t>(trits[i]), static_cast<std::uint8_t>(m[i])};
            }
        }
    } else if (q.quints) {
        while (done < count) {
            std::uint32_t m[3];
            std::uint32_t v = 0;
            m[0] = s.Read(b);
            v |= s.Read(3);
            m[1] = s.Read(b);
            v |= s.Read(2) << 3;
            m[2] = s.Read(b);
            v |= s.Read(2) << 5;
            int quints[3];
            UnpackQuints(v, quints);
            for (int i = 0; i < 3 && done < count; ++i, ++done) {
                out[done] = {static_cast<std::uint8_t>(quints[i]), static_cast<std::uint8_t>(m[i])};
            }
        }
    } else {
        for (; done < count; ++done) out[done] = {0, static_cast<std::uint8_t>(s.Read(b))};
    }
}

// Repeats the `bits`-bit value `v` to fill `toBits` bits (MSB-first replication).
int Replicate(int v, int bits, int toBits) {
    if (bits == 0) return 0;
    int result = 0;
    int filled = 0;
    while (filled < toBits) {
        result = (result << bits) | v;
        filled += bits;
    }
    return result >> (filled - toBits);
}

// --- unquantisation ----------------------------------------------------------------

int Bit(int v, int i) { return (v >> i) & 1; }

// Colour endpoint value to 0..255 (specification table "colour unquantisation").
int UnquantizeColor(const IseValue& v, const QuantMethod& q) {
    const int m = v.low;
    if (!q.trits && !q.quints) return Replicate(m, q.bits, 8);

    const int A = Bit(m, 0) ? 0x1FF : 0;
    const int b = Bit(m, 1), c = Bit(m, 2), d = Bit(m, 3), e = Bit(m, 4), f = Bit(m, 5);
    int B = 0;
    int C = 0;
    if (q.trits) {
        switch (q.bits) {
        case 1: C = 204; break;
        case 2: C = 93; B = (b << 8) | (b << 4) | (b << 2) | (b << 1); break;
        case 3: C = 44; B = (c << 8) | (b << 7) | (c << 3) | (b << 2) | (c << 1) | b; break;
        case 4: C = 22; B = (d << 8) | (c << 7) | (b << 6) | (d << 2) | (c << 1) | b; break;
        case 5: C = 11; B = (e << 8) | (d << 7) | (c << 6) | (b << 5) | (e << 1) | d; break;
        case 6: C = 5; B = (f << 8) | (e << 7) | (d << 6) | (c << 5) | (b << 4) | f; break;
        default: break;
        }
    } else {
        switch (q.bits) {
        case 1: C = 113; break;
        case 2: C = 54; B = (b << 8) | (b << 3) | (b << 2); break;
        case 3: C = 26; B = (c << 8) | (b << 7) | (c << 2) | (b << 1) | c; break;
        case 4: C = 13; B = (d << 8) | (c << 7) | (b << 6) | (d << 1) | c; break;
        case 5: C = 6; B = (e << 8) | (d << 7) | (c << 6) | (b << 5) | e; break;
        default: break;
        }
    }
    int t = v.high * C + B;
    t ^= A;
    return (A & 0x80) | (t >> 2);
}

// Weight value to 0..64 (specification table "weight unquantisation").
int UnquantizeWeight(const IseValue& v, const QuantMethod& q) {
    const int m = v.low;
    if (!q.trits && !q.quints) {
        const int w = Replicate(m, q.bits, 6);
        return w > 32 ? w + 1 : w;
    }
    if (q.bits == 0) return q.trits ? v.high * 32 : v.high * 16;

    const int A = Bit(m, 0) ? 0x7F : 0;
    const int b = Bit(m, 1), c = Bit(m, 2);
    int B = 0;
    int C = 0;
    if (q.trits) {
        switch (q.bits) {
        case 1: C = 50; break;
        case 2: C = 23; B = (b << 6) | (b << 2) | b; break;
        case 3: C = 11; B = (c << 6) | (b << 5) | (c << 1) | b; break;
        default: break;
        }
    } else {
        switch (q.bits) {
        case 1: C = 28; break;
        case 2: C = 13; B = (b << 6) | (b << 1); break;
        default: break;
        }
    }
    int t = v.high * C + B;
    t ^= A;
    t = (A & 0x20) | (t >> 2);
    return t > 32 ? t + 1 : t;
}

// --- block mode ------------------------------------------------------------------

struct BlockMode {
    int gridWidth = 0;
    int gridHeight = 0;
    bool dualPlane = false;
    int weightQuant = 0; // index into kQuantMethods
};

bool DecodeBlockMode(std::uint32_t mode, BlockMode& out) {
    int r = static_cast<int>((mode >> 4) & 1);
    int h = static_cast<int>((mode >> 9) & 1);
    int d = static_cast<int>((mode >> 10) & 1);
    const int a = static_cast<int>((mode >> 5) & 3);
    int w = 0, hh = 0;

    if ((mode & 3) != 0) {
        r |= static_cast<int>(mode & 3) << 1;
        int b = static_cast<int>((mode >> 7) & 3);
        switch ((mode >> 2) & 3) {
        case 0: w = b + 4; hh = a + 2; break;
        case 1: w = b + 8; hh = a + 2; break;
        case 2: w = a + 2; hh = b + 8; break;
        default:
            b &= 1;
            if (mode & 0x100) {
                w = b + 2;
                hh = a + 2;
            } else {
                w = a + 2;
                hh = b + 6;
            }
            break;
        }
    } else {
        r |= static_cast<int>((mode >> 2) & 3) << 1;
        if (((mode >> 2) & 3) == 0) return false; // reserved
        const int b = static_cast<int>((mode >> 9) & 3);
        switch ((mode >> 7) & 3) {
        case 0: w = 12; hh = a + 2; break;
        case 1: w = a + 2; hh = 12; break;
        case 2:
            w = a + 6;
            hh = b + 6;
            d = 0; // these bits are part of B in this layout
            h = 0;
            break;
        default:
            if (a == 0) {
                w = 6;
                hh = 10;
            } else if (a == 1) {
                w = 10;
                hh = 6;
            } else {
                return false; // reserved
            }
            break;
        }
    }
    out.gridWidth = w;
    out.gridHeight = hh;
    out.dualPlane = d != 0;
    out.weightQuant = (r - 2) + 6 * h;
    return out.weightQuant >= 0 && out.weightQuant < 12;
}

// --- partitioning ------------------------------------------------------------------

std::uint32_t Hash52(std::uint32_t p) {
    p ^= p >> 15;
    p -= p << 17;
    p += p << 7;
    p += p << 4;
    p ^= p >> 5;
    p += p << 16;
    p ^= p >> 7;
    p ^= p >> 3;
    p ^= p << 6;
    p ^= p >> 17;
    return p;
}

// The specification's partition selection function (2D blocks: z = 0).
int SelectPartition(int seed, int x, int y, int partitionCount, bool smallBlock) {
    if (smallBlock) {
        x <<= 1;
        y <<= 1;
    }
    seed += (partitionCount - 1) * 1024;
    const std::uint32_t rnum = Hash52(static_cast<std::uint32_t>(seed));
    std::uint32_t s[12] = {
        rnum & 0xF,         (rnum >> 4) & 0xF,  (rnum >> 8) & 0xF,  (rnum >> 12) & 0xF,
        (rnum >> 16) & 0xF, (rnum >> 20) & 0xF, (rnum >> 24) & 0xF, (rnum >> 28) & 0xF,
        (rnum >> 18) & 0xF, (rnum >> 22) & 0xF, (rnum >> 26) & 0xF, ((rnum >> 30) | (rnum << 2)) & 0xF,
    };
    for (auto& v : s) v *= v;

    int sh1, sh2;
    if (seed & 1) {
        sh1 = (seed & 2) ? 4 : 5;
        sh2 = partitionCount == 3 ? 6 : 5;
    } else {
        sh1 = partitionCount == 3 ? 6 : 5;
        sh2 = (seed & 2) ? 4 : 5;
    }
    const int sh3 = (seed & 0x10) ? sh1 : sh2;
    for (int i = 0; i < 8; ++i) s[i] >>= (i % 2 == 0) ? sh1 : sh2;
    for (int i = 8; i < 12; ++i) s[i] >>= sh3;

    const auto ux = static_cast<std::uint32_t>(x);
    const auto uy = static_cast<std::uint32_t>(y);
    // z is zero for 2D blocks, so seeds 9-12 (the z multipliers) drop out.
    std::uint32_t a = s[0] * ux + s[1] * uy + (rnum >> 14);
    std::uint32_t b = s[2] * ux + s[3] * uy + (rnum >> 10);
    std::uint32_t c = s[4] * ux + s[5] * uy + (rnum >> 6);
    std::uint32_t d = s[6] * ux + s[7] * uy + (rnum >> 2);
    a &= 0x3F;
    b &= 0x3F;
    c &= 0x3F;
    d &= 0x3F;
    if (partitionCount < 4) d = 0;
    if (partitionCount < 3) c = 0;
    if (a >= b && a >= c && a >= d) return 0;
    if (b >= c && b >= d) return 1;
    if (c >= d) return 2;
    return 3;
}

// --- colour endpoints ----------------------------------------------------------------

struct Rgba {
    int c[4];
};

int Clamp255(int v) { return std::clamp(v, 0, 255); }

void BitTransferSigned(int& a, int& b) {
    b = (b >> 1) | (a & 0x80);
    a = (a >> 1) & 0x3F;
    if (a & 0x20) a -= 0x40;
}

Rgba BlueContract(int r, int g, int b, int a) { return {{(r + b) >> 1, (g + b) >> 1, b, a}}; }

bool IsHdrMode(int cem) {
    return cem == 2 || cem == 3 || cem == 7 || cem == 11 || cem == 14 || cem == 15;
}

// Decodes one partition's endpoints from its unquantised values (LDR modes only).
void DecodeEndpoints(int cem, const int* v, Rgba& e0, Rgba& e1) {
    switch (cem) {
    case 0: // luminance, direct
        e0 = {{v[0], v[0], v[0], 255}};
        e1 = {{v[1], v[1], v[1], 255}};
        break;
    case 1: { // luminance, base + offset
        const int l0 = (v[0] >> 2) | (v[1] & 0xC0);
        const int l1 = std::min(l0 + (v[1] & 0x3F), 255);
        e0 = {{l0, l0, l0, 255}};
        e1 = {{l1, l1, l1, 255}};
        break;
    }
    case 4: // luminance + alpha, direct
        e0 = {{v[0], v[0], v[0], v[2]}};
        e1 = {{v[1], v[1], v[1], v[3]}};
        break;
    case 5: { // luminance + alpha, base + offset
        int v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
        BitTransferSigned(v1, v0);
        BitTransferSigned(v3, v2);
        e0 = {{v0, v0, v0, v2}};
        const int l = Clamp255(v0 + v1);
        e1 = {{l, l, l, Clamp255(v2 + v3)}};
        break;
    }
    case 6: // RGB, base + scale
        e0 = {{(v[0] * v[3]) >> 8, (v[1] * v[3]) >> 8, (v[2] * v[3]) >> 8, 255}};
        e1 = {{v[0], v[1], v[2], 255}};
        break;
    case 8:    // RGB, direct
    case 12: { // RGBA, direct
        const int a0 = cem == 12 ? v[6] : 255;
        const int a1 = cem == 12 ? v[7] : 255;
        const int s0 = v[0] + v[2] + v[4];
        const int s1 = v[1] + v[3] + v[5];
        if (s1 >= s0) {
            e0 = {{v[0], v[2], v[4], a0}};
            e1 = {{v[1], v[3], v[5], a1}};
        } else {
            e0 = BlueContract(v[1], v[3], v[5], a1);
            e1 = BlueContract(v[0], v[2], v[4], a0);
        }
        break;
    }
    case 9:    // RGB, base + offset
    case 13: { // RGBA, base + offset
        int t[8] = {v[0], v[1], v[2], v[3], v[4], v[5], cem == 13 ? v[6] : 255, cem == 13 ? v[7] : 0};
        BitTransferSigned(t[1], t[0]);
        BitTransferSigned(t[3], t[2]);
        BitTransferSigned(t[5], t[4]);
        if (cem == 13) BitTransferSigned(t[7], t[6]);
        if (t[1] + t[3] + t[5] >= 0) {
            e0 = {{t[0], t[2], t[4], t[6]}};
            e1 = {{t[0] + t[1], t[2] + t[3], t[4] + t[5], t[6] + t[7]}};
        } else {
            e0 = BlueContract(t[0] + t[1], t[2] + t[3], t[4] + t[5], t[6] + t[7]);
            e1 = BlueContract(t[0], t[2], t[4], t[6]);
        }
        for (int i = 0; i < 4; ++i) {
            e0.c[i] = Clamp255(e0.c[i]);
            e1.c[i] = Clamp255(e1.c[i]);
        }
        break;
    }
    case 10: // RGB, base + scale, plus two alphas
        e0 = {{(v[0] * v[3]) >> 8, (v[1] * v[3]) >> 8, (v[2] * v[3]) >> 8, v[4]}};
        e1 = {{v[0], v[1], v[2], v[5]}};
        break;
    default: break;
    }
}

// --- output ------------------------------------------------------------------------

// Converts a 16-bit decoded component to 8 bits. sRGB colour channels keep the
// top byte (their endpoints were expanded with 0x80 so this rounds); everything
// else is UNORM16 -> UNORM8 with rounding.
std::uint8_t ToUnorm8(int value16, bool srgbChannel) {
    if (srgbChannel) return static_cast<std::uint8_t>(value16 >> 8);
    return static_cast<std::uint8_t>((value16 * 255 + 32767) / 65535);
}

void FillError(std::uint8_t* out, int texels) {
    for (int i = 0; i < texels; ++i) {
        out[i * 4 + 0] = 0xFF;
        out[i * 4 + 1] = 0x00;
        out[i * 4 + 2] = 0xFF;
        out[i * 4 + 3] = 0xFF;
    }
}

bool DecodeVoidExtent(const std::uint8_t* block, int texels, Profile profile, std::uint8_t* out) {
    const bool hdr = ReadBits(block, 9, 1) != 0;
    if (hdr || ReadBits(block, 10, 2) != 3) {
        FillError(out, texels);
        return false;
    }
    // The extent coordinates (bits 12-63) only help encoders; the colour applies
    // to the whole block either way.
    std::uint8_t rgba[4];
    for (int c = 0; c < 4; ++c) {
        const auto value = static_cast<int>(ReadBits(block, 64 + 16 * c, 16));
        rgba[c] = ToUnorm8(value, profile == Profile::Srgb && c < 3);
    }
    for (int i = 0; i < texels; ++i) std::memcpy(out + i * 4, rgba, 4);
    return true;
}

// Bilinear weight infill from the weight grid to the block's texels.
void InfillWeights(const int* grid, int gridW, int gridH, int blockW, int blockH, int* out) {
    const int ds = (1024 + blockW / 2) / (blockW - 1);
    const int dt = (1024 + blockH / 2) / (blockH - 1);
    auto at = [&](int x, int y) {
        x = std::min(x, gridW - 1);
        y = std::min(y, gridH - 1);
        return grid[y * gridW + x];
    };
    for (int t = 0; t < blockH; ++t) {
        for (int s = 0; s < blockW; ++s) {
            const int gs = (ds * s * (gridW - 1) + 32) >> 6;
            const int gt = (dt * t * (gridH - 1) + 32) >> 6;
            const int js = gs >> 4, fs = gs & 0xF;
            const int jt = gt >> 4, ft = gt & 0xF;
            const int w11 = (fs * ft + 8) >> 4;
            const int w10 = ft - w11;
            const int w01 = fs - w11;
            const int w00 = 16 - fs - ft + w11;
            const int sum = at(js, jt) * w00 + at(js + 1, jt) * w01 + at(js, jt + 1) * w10 +
                            at(js + 1, jt + 1) * w11;
            out[t * blockW + s] = (sum + 8) >> 4;
        }
    }
}

} // namespace

bool DecodeBlock(const std::uint8_t* block, int blockWidth, int blockHeight, Profile profile,
                 std::uint8_t* out) {
    const int texels = blockWidth * blockHeight;
    if (blockWidth < 4 || blockHeight < 4 || blockWidth > 12 || blockHeight > 12) return false;

    const std::uint32_t mode = ReadBits(block, 0, 11);
    if ((mode & 0x1FF) == 0x1FC) return DecodeVoidExtent(block, texels, profile, out);

    BlockMode bm;
    if (!DecodeBlockMode(mode, bm) || bm.gridWidth > blockWidth || bm.gridHeight > blockHeight) {
        FillError(out, texels);
        return false;
    }
    const int planes = bm.dualPlane ? 2 : 1;
    const int weightCount = bm.gridWidth * bm.gridHeight * planes;
    const QuantMethod& weightQuant = kQuantMethods[static_cast<std::size_t>(bm.weightQuant)];
    const int weightBits = IseBitCount(weightCount, weightQuant);
    if (weightCount > kMaxWeights || weightBits < 24 || weightBits > 96) {
        FillError(out, texels);
        return false;
    }

    const int partitions = static_cast<int>(ReadBits(block, 11, 2)) + 1;
    if (partitions == 4 && bm.dualPlane) {
        FillError(out, texels);
        return false;
    }

    // Colour endpoint modes.
    int cems[4] = {};
    int belowWeights = 128 - weightBits;
    int colorStart = 0;
    int partitionSeed = 0;
    if (partitions == 1) {
        cems[0] = static_cast<int>(ReadBits(block, 13, 4));
        colorStart = 17;
    } else {
        partitionSeed = static_cast<int>(ReadBits(block, 13, 10));
        std::uint32_t encoded = ReadBits(block, 23, 6);
        colorStart = 29;
        if ((encoded & 3) == 0) {
            for (int i = 0; i < partitions; ++i) cems[i] = static_cast<int>(encoded >> 2);
        } else {
            const int extraBits = 3 * partitions - 4;
            belowWeights -= extraBits;
            encoded |= ReadBits(block, belowWeights, extraBits) << 6;
            const int baseClass = static_cast<int>(encoded & 3) - 1;
            int pos = 2;
            for (int i = 0; i < partitions; ++i, ++pos) cems[i] = baseClass + static_cast<int>((encoded >> pos) & 1);
            for (int i = 0; i < partitions; ++i, pos += 2) cems[i] = (cems[i] << 2) | static_cast<int>((encoded >> pos) & 3);
        }
    }
    int planeComponent = -1;
    if (bm.dualPlane) {
        belowWeights -= 2;
        planeComponent = static_cast<int>(ReadBits(block, belowWeights, 2));
    }

    int colorValueCount = 0;
    for (int i = 0; i < partitions; ++i) {
        if (IsHdrMode(cems[i])) {
            FillError(out, texels);
            return false;
        }
        colorValueCount += ((cems[i] >> 2) + 1) * 2;
    }
    const int colorBits = belowWeights - colorStart;
    if (colorValueCount > kMaxColorValues || colorBits <= 0) {
        FillError(out, texels);
        return false;
    }
    // Endpoints use the finest quantisation that fits in the remaining bits.
    int colorQuant = -1;
    for (int q = static_cast<int>(kQuantMethods.size()) - 1; q >= 0; --q) {
        if (IseBitCount(colorValueCount, kQuantMethods[static_cast<std::size_t>(q)]) <= colorBits) {
            colorQuant = q;
            break;
        }
    }
    if (colorQuant < kQuant6Index) {
        FillError(out, texels);
        return false;
    }

    IseValue raw[kMaxWeights];
    const QuantMethod& cq = kQuantMethods[static_cast<std::size_t>(colorQuant)];
    DecodeIse(block, colorStart, colorValueCount, cq, raw);
    int colorValues[kMaxColorValues];
    for (int i = 0; i < colorValueCount; ++i) colorValues[i] = UnquantizeColor(raw[i], cq);

    Rgba endpoints[4][2];
    for (int i = 0, offset = 0; i < partitions; ++i) {
        DecodeEndpoints(cems[i], colorValues + offset, endpoints[i][0], endpoints[i][1]);
        offset += ((cems[i] >> 2) + 1) * 2;
    }

    // Weights are stored bit-reversed from the top of the block.
    std::uint8_t reversed[kBlockBytes];
    for (int i = 0; i < kBlockBytes; ++i) {
        const std::uint8_t b = block[kBlockBytes - 1 - i];
        std::uint8_t r = 0;
        for (int bit = 0; bit < 8; ++bit) r = static_cast<std::uint8_t>(r | (((b >> bit) & 1) << (7 - bit)));
        reversed[i] = r;
    }
    DecodeIse(reversed, 0, weightCount, weightQuant, raw);

    int planeWeights[2][kMaxTexels];
    for (int p = 0; p < planes; ++p) {
        int grid[kMaxWeights];
        for (int i = 0; i < bm.gridWidth * bm.gridHeight; ++i) {
            grid[i] = UnquantizeWeight(raw[i * planes + p], weightQuant);
        }
        InfillWeights(grid, bm.gridWidth, bm.gridHeight, blockWidth, blockHeight, planeWeights[p]);
    }

    const bool smallBlock = texels < 31;
    const bool srgb = profile == Profile::Srgb;
    for (int y = 0; y < blockHeight; ++y) {
        for (int x = 0; x < blockWidth; ++x) {
            const int texel = y * blockWidth + x;
            const int part = partitions > 1 ? SelectPartition(partitionSeed, x, y, partitions, smallBlock) : 0;
            const Rgba& e0 = endpoints[part][0];
            const Rgba& e1 = endpoints[part][1];
            for (int c = 0; c < 4; ++c) {
                const bool srgbChannel = srgb && c < 3;
                const int c0 = srgbChannel ? (e0.c[c] << 8) | 0x80 : e0.c[c] * 257;
                const int c1 = srgbChannel ? (e1.c[c] << 8) | 0x80 : e1.c[c] * 257;
                const int w = planeWeights[c == planeComponent ? 1 : 0][texel];
                const int value = (c0 * (64 - w) + c1 * w + 32) >> 6;
                out[texel * 4 + c] = ToUnorm8(value, srgbChannel);
            }
        }
    }
    return true;
}

std::vector<std::uint8_t> DecodeImage(std::span<const std::uint8_t> data, int width, int height,
                                      int blockWidth, int blockHeight, Profile profile) {
    if (width <= 0 || height <= 0 || blockWidth < 4 || blockHeight < 4 || blockWidth > 12 ||
        blockHeight > 12) {
        return {};
    }
    const int blocksX = (width + blockWidth - 1) / blockWidth;
    const int blocksY = (height + blockHeight - 1) / blockHeight;
    const auto needed = static_cast<std::size_t>(blocksX) * static_cast<std::size_t>(blocksY) * kBlockBytes;
    if (data.size() < needed) return {};

    std::vector<std::uint8_t> image(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);
    std::uint8_t texels[kMaxTexels * 4];
    for (int by = 0; by < blocksY; ++by) {
        for (int bx = 0; bx < blocksX; ++bx) {
            const auto index = static_cast<std::size_t>(by) * static_cast<std::size_t>(blocksX) + static_cast<std::size_t>(bx);
            DecodeBlock(data.data() + index * kBlockBytes, blockWidth, blockHeight, profile, texels);
            for (int y = 0; y < blockHeight; ++y) {
                const int py = by * blockHeight + y;
                if (py >= height) break;
                for (int x = 0; x < blockWidth; ++x) {
                    const int px = bx * blockWidth + x;
                    if (px >= width) break;
                    std::memcpy(&image[(static_cast<std::size_t>(py) * static_cast<std::size_t>(width) + static_cast<std::size_t>(px)) * 4],
                                &texels[(y * blockWidth + x) * 4], 4);
                }
            }
        }
    }
    return image;
}

} // namespace rr::assets::astc
