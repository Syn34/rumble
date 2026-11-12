#include "byte_utils.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <stdexcept>

namespace Photon::Deterministic {

namespace {

// ---------------------------------------------------------------------------------
// Little-endian stores into a byte span, bounds checked (the original wrote through
// an unchecked pointer and would fault instead).

template <typename T> void StoreLittleEndian(std::span<std::uint8_t> buffer, int offset, T value) {
    if (offset < 0 || static_cast<std::size_t>(offset) + sizeof(T) > buffer.size()) {
        throw std::out_of_range("ByteUtils: write past end of buffer");
    }
    auto bits = static_cast<std::uint64_t>(value);
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        buffer[static_cast<std::size_t>(offset) + i] = static_cast<std::uint8_t>(bits & 0xFFu);
        bits >>= 8;
    }
}

std::int32_t LoadInt32LittleEndian(std::span<const std::uint8_t> data, std::size_t offset) {
    if (offset + 4 > data.size()) {
        throw std::out_of_range("ByteUtils: read past end of buffer");
    }
    std::uint32_t v = 0;
    for (std::size_t i = 0; i < 4; ++i) {
        v |= static_cast<std::uint32_t>(data[offset + i]) << (8 * i);
    }
    return static_cast<std::int32_t>(v);
}

void AppendInt32LittleEndian(std::vector<std::uint8_t>& out, std::uint32_t v) {
    for (int i = 0; i < 4; ++i) {
        out.push_back(static_cast<std::uint8_t>(v >> (8 * i)));
    }
}

// ---------------------------------------------------------------------------------
// CRC-32 (IEEE, reflected), as used in the gzip trailer.

std::uint32_t Crc32(std::span<const std::uint8_t> data) {
    static const auto table = [] {
        std::array<std::uint32_t, 256> t{};
        for (std::uint32_t n = 0; n < 256; ++n) {
            std::uint32_t c = n;
            for (int k = 0; k < 8; ++k) {
                c = (c & 1u) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            }
            t[n] = c;
        }
        return t;
    }();
    std::uint32_t crc = 0xFFFFFFFFu;
    for (std::uint8_t b : data) {
        crc = table[(crc ^ b) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

// ---------------------------------------------------------------------------------
// gzip (RFC 1952) container around DEFLATE (RFC 1951).
//
// The original delegated to the runtime's GZipStream (native zlib underneath). The
// writer here emits valid gzip using stored (uncompressed) DEFLATE blocks, so any
// gzip reader -- including the original game -- decodes it to the same bytes, but the
// compressed bytes and their size differ from zlib's output.
// TODO: real DEFLATE compression if compressed sizes ever matter.

[[noreturn]] void CorruptGZip() {
    throw std::runtime_error("GZip: corrupt or truncated data");
}

std::vector<std::uint8_t> GZipEncode(std::span<const std::uint8_t> data) {
    std::vector<std::uint8_t> out;
    out.reserve(data.size() + data.size() / 65535 * 5 + 32);

    // Header: magic, CM = deflate, no flags, no mtime, XFL 0, OS = Unix.
    const std::uint8_t header[] = {0x1F, 0x8B, 8, 0, 0, 0, 0, 0, 0, 3};
    out.insert(out.end(), std::begin(header), std::end(header));

    constexpr std::size_t MaxStored = 65535;
    std::size_t pos = 0;
    do {
        const std::size_t len = std::min(MaxStored, data.size() - pos);
        const bool final = pos + len == data.size();
        out.push_back(final ? 1 : 0); // BFINAL, BTYPE = 00 (stored), padded to the byte
        out.push_back(static_cast<std::uint8_t>(len & 0xFFu));
        out.push_back(static_cast<std::uint8_t>(len >> 8));
        out.push_back(static_cast<std::uint8_t>(~len & 0xFFu));
        out.push_back(static_cast<std::uint8_t>((~len >> 8) & 0xFFu));
        out.insert(out.end(), data.begin() + static_cast<std::ptrdiff_t>(pos),
                   data.begin() + static_cast<std::ptrdiff_t>(pos + len));
        pos += len;
    } while (pos < data.size());

    AppendInt32LittleEndian(out, Crc32(data));
    AppendInt32LittleEndian(out, static_cast<std::uint32_t>(data.size()));
    return out;
}

// Canonical Huffman decoding table: number of codes per length plus symbols in
// code order.
struct Huffman {
    std::array<std::uint16_t, 16> countPerLength{};
    std::vector<std::uint16_t> symbols;

    explicit Huffman(std::span<const std::uint8_t> lengths) : symbols(lengths.size()) {
        for (std::uint8_t len : lengths) {
            if (len >= countPerLength.size()) {
                CorruptGZip();
            }
            ++countPerLength[len];
        }
        // Length 0 marks an unused symbol; those are counted but get no code.
        countPerLength[0] = 0;
        // Index of the first symbol of each code length within `symbols`.
        std::array<std::uint16_t, 16> start{};
        std::uint16_t running = 0;
        for (std::size_t len = 1; len < 16; ++len) {
            start[len] = running;
            running = static_cast<std::uint16_t>(running + countPerLength[len]);
        }
        for (std::size_t sym = 0; sym < lengths.size(); ++sym) {
            if (lengths[sym] != 0) {
                symbols[start[lengths[sym]]++] = static_cast<std::uint16_t>(sym);
            }
        }
    }
};

class Inflater {
  public:
    explicit Inflater(std::span<const std::uint8_t> input, std::size_t start)
        : input_(input), pos_(start) {}

    std::size_t Position() const { return pos_; }

    void Run(std::vector<std::uint8_t>& out) {
        bool last = false;
        while (!last) {
            last = Bits(1) != 0;
            switch (Bits(2)) {
            case 0:
                Stored(out);
                break;
            case 1:
                Fixed(out);
                break;
            case 2:
                Dynamic(out);
                break;
            default:
                CorruptGZip();
            }
        }
        // Discard the rest of the current byte; the trailer is byte aligned.
        bitCount_ = 0;
        bitBuffer_ = 0;
    }

  private:
    std::uint32_t Bits(int need) {
        std::uint32_t value = bitBuffer_;
        while (bitCount_ < need) {
            if (pos_ >= input_.size()) {
                CorruptGZip();
            }
            value |= static_cast<std::uint32_t>(input_[pos_++]) << bitCount_;
            bitCount_ += 8;
        }
        bitBuffer_ = value >> need;
        bitCount_ -= need;
        return value & ((1u << need) - 1u);
    }

    int Decode(const Huffman& h) {
        int code = 0;
        int first = 0;
        int index = 0;
        for (std::size_t len = 1; len < 16; ++len) {
            code |= static_cast<int>(Bits(1));
            const int count = h.countPerLength[len];
            if (code - count < first) {
                return h.symbols[static_cast<std::size_t>(index + (code - first))];
            }
            index += count;
            first += count;
            first <<= 1;
            code <<= 1;
        }
        CorruptGZip();
    }

    void Stored(std::vector<std::uint8_t>& out) {
        bitBuffer_ = 0;
        bitCount_ = 0;
        if (pos_ + 4 > input_.size()) {
            CorruptGZip();
        }
        const unsigned len = input_[pos_] | (input_[pos_ + 1] << 8);
        const unsigned nlen = input_[pos_ + 2] | (input_[pos_ + 3] << 8);
        pos_ += 4;
        if (len != (~nlen & 0xFFFFu) || pos_ + len > input_.size()) {
            CorruptGZip();
        }
        out.insert(out.end(), input_.begin() + static_cast<std::ptrdiff_t>(pos_),
                   input_.begin() + static_cast<std::ptrdiff_t>(pos_ + len));
        pos_ += len;
    }

    void Fixed(std::vector<std::uint8_t>& out) {
        static const Huffman lit = [] {
            std::array<std::uint8_t, 288> l{};
            std::fill(l.begin(), l.begin() + 144, std::uint8_t{8});
            std::fill(l.begin() + 144, l.begin() + 256, std::uint8_t{9});
            std::fill(l.begin() + 256, l.begin() + 280, std::uint8_t{7});
            std::fill(l.begin() + 280, l.end(), std::uint8_t{8});
            return Huffman(l);
        }();
        static const Huffman dist = [] {
            std::array<std::uint8_t, 30> l{};
            l.fill(5);
            return Huffman(l);
        }();
        Codes(out, lit, dist);
    }

    void Dynamic(std::vector<std::uint8_t>& out) {
        static constexpr std::array<std::uint8_t, 19> order = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                                               11, 4,  12, 3, 13, 2, 14, 1, 15};
        const std::size_t nlen = Bits(5) + 257;
        const std::size_t ndist = Bits(5) + 1;
        const std::size_t ncode = Bits(4) + 4;
        if (nlen > 286 || ndist > 30) {
            CorruptGZip();
        }
        std::array<std::uint8_t, 19> codeLengths{};
        for (std::size_t i = 0; i < ncode; ++i) {
            codeLengths[order[i]] = static_cast<std::uint8_t>(Bits(3));
        }
        const Huffman lengthCode(codeLengths);

        std::vector<std::uint8_t> lengths(nlen + ndist, 0);
        std::size_t i = 0;
        while (i < nlen + ndist) {
            const int sym = Decode(lengthCode);
            if (sym < 16) {
                lengths[i++] = static_cast<std::uint8_t>(sym);
                continue;
            }
            std::uint8_t value = 0;
            std::size_t repeat = 0;
            if (sym == 16) {
                if (i == 0) {
                    CorruptGZip();
                }
                value = lengths[i - 1];
                repeat = 3 + Bits(2);
            } else if (sym == 17) {
                repeat = 3 + Bits(3);
            } else {
                repeat = 11 + Bits(7);
            }
            if (i + repeat > nlen + ndist) {
                CorruptGZip();
            }
            std::fill_n(lengths.begin() + static_cast<std::ptrdiff_t>(i), repeat, value);
            i += repeat;
        }
        const Huffman lit(std::span<const std::uint8_t>(lengths).first(nlen));
        const Huffman dist(std::span<const std::uint8_t>(lengths).subspan(nlen));
        Codes(out, lit, dist);
    }

    void Codes(std::vector<std::uint8_t>& out, const Huffman& lit, const Huffman& dist) {
        static constexpr std::array<std::uint16_t, 29> lengthBase = {
            3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23, 27,
            31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
        static constexpr std::array<std::uint8_t, 29> lengthExtra = {
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
        static constexpr std::array<std::uint16_t, 30> distBase = {
            1,   2,   3,   4,   5,   7,    9,    13,   17,   25,   33,   49,   65,    97,    129,
            193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
        static constexpr std::array<std::uint8_t, 30> distExtra = {
            0, 0, 0, 0, 1, 1, 2, 2,  3,  3,  4,  4,  5,  5,  6,
            6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

        for (;;) {
            const int sym = Decode(lit);
            if (sym < 256) {
                out.push_back(static_cast<std::uint8_t>(sym));
                continue;
            }
            if (sym == 256) {
                return;
            }
            const auto li = static_cast<std::size_t>(sym - 257);
            if (li >= lengthBase.size()) {
                CorruptGZip();
            }
            const std::size_t length = lengthBase[li] + Bits(lengthExtra[li]);
            const auto di = static_cast<std::size_t>(Decode(dist));
            if (di >= distBase.size()) {
                CorruptGZip();
            }
            const std::size_t distance = distBase[di] + Bits(distExtra[di]);
            if (distance > out.size()) {
                CorruptGZip();
            }
            const std::size_t from = out.size() - distance;
            for (std::size_t k = 0; k < length; ++k) {
                out.push_back(out[from + k]);
            }
        }
    }

    std::span<const std::uint8_t> input_;
    std::size_t pos_;
    std::uint32_t bitBuffer_ = 0;
    int bitCount_ = 0;
};

std::vector<std::uint8_t> GZipDecode(std::span<const std::uint8_t> data) {
    std::vector<std::uint8_t> out;
    std::size_t pos = 0;
    // Concatenated gzip members decode to the concatenation of their contents.
    while (pos + 2 <= data.size() && data[pos] == 0x1F && data[pos + 1] == 0x8B) {
        if (pos + 10 > data.size() || data[pos + 2] != 8) {
            CorruptGZip();
        }
        const std::uint8_t flags = data[pos + 3];
        pos += 10;
        const auto need = [&](std::size_t n) {
            if (pos + n > data.size()) {
                CorruptGZip();
            }
        };
        if (flags & 0x04) { // FEXTRA
            need(2);
            const std::size_t xlen = data[pos] | (data[pos + 1] << 8);
            need(2 + xlen);
            pos += 2 + xlen;
        }
        for (const std::uint8_t zeroTerminated : {std::uint8_t{0x08}, std::uint8_t{0x10}}) {
            if (flags & zeroTerminated) { // FNAME, FCOMMENT
                while (true) {
                    need(1);
                    if (data[pos++] == 0) {
                        break;
                    }
                }
            }
        }
        if (flags & 0x02) { // FHCRC
            need(2);
            pos += 2;
        }

        const std::size_t memberStart = out.size();
        Inflater inflater(data, pos);
        inflater.Run(out);
        pos = inflater.Position();

        need(8);
        const auto crc = static_cast<std::uint32_t>(LoadInt32LittleEndian(data, pos));
        const auto size = static_cast<std::uint32_t>(LoadInt32LittleEndian(data, pos + 4));
        pos += 8;
        const std::span<const std::uint8_t> member(out.data() + memberStart,
                                                   out.size() - memberStart);
        if (crc != Crc32(member) || size != static_cast<std::uint32_t>(member.size())) {
            CorruptGZip();
        }
    }
    if (pos == 0 && !data.empty()) {
        CorruptGZip(); // not gzip at all
    }
    return out;
}

constexpr char Base64Alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

} // namespace

// ---------------------------------------------------------------------------------

std::vector<std::uint8_t> EncodingGetBytes(Encoding encoding, std::string_view text) {
    switch (encoding) {
    case Encoding::UTF8:
        break;
    }
    return {text.begin(), text.end()};
}

std::string EncodingGetString(Encoding encoding, std::span<const std::uint8_t> bytes) {
    switch (encoding) {
    case Encoding::UTF8:
        break;
    }
    // TODO: .NET replaces invalid UTF-8 sequences with U+FFFD; bytes are kept as-is here.
    return {bytes.begin(), bytes.end()};
}

GZipCompressStream::GZipCompressStream(std::ostream& output)
    : std::ostream(nullptr), output_(output) {
    rdbuf(&buffer_);
}

GZipCompressStream::~GZipCompressStream() {
    Close();
}

void GZipCompressStream::Close() {
    if (buffer_.closed) {
        return;
    }
    buffer_.closed = true;
    const auto encoded = GZipEncode(buffer_.bytes);
    output_.write(reinterpret_cast<const char*>(encoded.data()),
                  static_cast<std::streamsize>(encoded.size()));
    buffer_.bytes.clear();
    buffer_.bytes.shrink_to_fit();
}

GZipCompressStream::Buffer::int_type GZipCompressStream::Buffer::overflow(int_type ch) {
    if (closed) {
        return traits_type::eof();
    }
    if (!traits_type::eq_int_type(ch, traits_type::eof())) {
        bytes.push_back(static_cast<std::uint8_t>(traits_type::to_char_type(ch)));
    }
    return traits_type::not_eof(ch);
}

std::streamsize GZipCompressStream::Buffer::xsputn(const char* s, std::streamsize count) {
    if (closed) {
        return 0;
    }
    bytes.insert(bytes.end(), reinterpret_cast<const std::uint8_t*>(s),
                 reinterpret_cast<const std::uint8_t*>(s) + count);
    return count;
}

// 0x7a84b88
std::vector<std::uint8_t>
ByteUtils::MergeByteBlocks(std::span<const std::vector<std::uint8_t>> blocks) {
    std::size_t total = 0;
    for (const auto& block : blocks) {
        total += block.size();
    }
    std::vector<std::uint8_t> merged;
    merged.reserve(total);
    for (const auto& block : blocks) {
        merged.insert(merged.end(), block.begin(), block.end());
    }
    return merged;
}

// 0x7a86b74
int ByteUtils::AddValueBlock(std::int32_t value, std::span<std::uint8_t> buffer, int offset) {
    StoreLittleEndian<std::int32_t>(buffer, offset, 4);
    StoreLittleEndian(buffer, offset + 4, value);
    return offset + 4 + 4;
}

// 0x7a86be4
int ByteUtils::AddValueBlock(std::int64_t value, std::span<std::uint8_t> buffer, int offset) {
    StoreLittleEndian<std::int32_t>(buffer, offset, 8);
    StoreLittleEndian(buffer, offset + 4, value);
    return offset + 4 + 8;
}

// 0x7a86c54
int ByteUtils::AddValueBlock(std::uint64_t value, std::span<std::uint8_t> buffer, int offset) {
    StoreLittleEndian<std::int32_t>(buffer, offset, 8);
    StoreLittleEndian(buffer, offset + 4, value);
    return offset + 4 + 8;
}

// 0x7a86c94
int ByteUtils::BeginByteBlockHeader(std::span<std::uint8_t> /*buffer*/, int offset,
                                    int& blockStart) {
    blockStart = offset;
    return offset + 4;
}

// 0x7a86ca0
int ByteUtils::EndByteBlockHeader(std::span<std::uint8_t> buffer, int blockStart,
                                  int bytesWritten) {
    StoreLittleEndian(buffer, blockStart, static_cast<std::int32_t>(bytesWritten));
    return blockStart + 4 + bytesWritten;
}

// 0x7a7ea30
std::vector<std::uint8_t>
ByteUtils::PackByteBlocks(std::span<const std::vector<std::uint8_t>> blocks) {
    std::size_t total = 0;
    for (const auto& block : blocks) {
        total += 4 + block.size();
    }
    std::vector<std::uint8_t> packed;
    packed.reserve(total);
    for (const auto& block : blocks) {
        AppendInt32LittleEndian(packed, static_cast<std::uint32_t>(block.size()));
        packed.insert(packed.end(), block.begin(), block.end());
    }
    return packed;
}

// 0x7a7f2f8 (body in <ReadByteBlocks>d__9.MoveNext, 0x7a87dc4)
std::vector<std::vector<std::uint8_t>>
ByteUtils::ReadByteBlocks(std::span<const std::uint8_t> data) {
    std::vector<std::vector<std::uint8_t>> blocks;
    std::size_t offset = 0;
    while (offset < data.size()) {
        const std::int32_t length = LoadInt32LittleEndian(data, offset);
        offset += 4;
        if (length < 0 || offset + static_cast<std::size_t>(length) > data.size()) {
            throw std::out_of_range("ByteUtils::ReadByteBlocks: block runs past end of data");
        }
        const auto first = data.begin() + static_cast<std::ptrdiff_t>(offset);
        blocks.emplace_back(first, first + length);
        offset += static_cast<std::size_t>(length);
    }
    return blocks;
}

// 0x7a86d00
void ByteUtils::CopyTo(std::istream& source, std::ostream& destination) {
    std::array<char, 4096> chunk{};
    for (;;) {
        source.read(chunk.data(), static_cast<std::streamsize>(chunk.size()));
        const std::streamsize got = source.gcount();
        if (got == 0) {
            return;
        }
        destination.write(chunk.data(), got);
    }
}

// 0x7a86dd8
std::string ByteUtils::Base64Encode(std::span<const std::uint8_t> data) {
    std::string text;
    text.reserve((data.size() + 2) / 3 * 4);
    std::size_t i = 0;
    for (; i + 3 <= data.size(); i += 3) {
        const std::uint32_t v =
            (std::uint32_t{data[i]} << 16) | (std::uint32_t{data[i + 1]} << 8) | data[i + 2];
        text += Base64Alphabet[(v >> 18) & 63u];
        text += Base64Alphabet[(v >> 12) & 63u];
        text += Base64Alphabet[(v >> 6) & 63u];
        text += Base64Alphabet[v & 63u];
    }
    const std::size_t rest = data.size() - i;
    if (rest != 0) {
        std::uint32_t v = std::uint32_t{data[i]} << 16;
        if (rest == 2) {
            v |= std::uint32_t{data[i + 1]} << 8;
        }
        text += Base64Alphabet[(v >> 18) & 63u];
        text += Base64Alphabet[(v >> 12) & 63u];
        text += rest == 2 ? Base64Alphabet[(v >> 6) & 63u] : '=';
        text += '=';
    }
    return text;
}

// 0x7a86e30
std::vector<std::uint8_t> ByteUtils::Base64Decode(std::string_view data) {
    // Convert.FromBase64String: whitespace is ignored, the remaining length must be a
    // multiple of four and up to two '=' may pad the end.
    std::string compact;
    compact.reserve(data.size());
    for (char c : data) {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            compact += c;
        }
    }
    if (compact.size() % 4 != 0) {
        throw std::invalid_argument("Base64Decode: invalid length");
    }
    std::size_t padding = 0;
    while (padding < 2 && padding < compact.size() &&
           compact[compact.size() - 1 - padding] == '=') {
        ++padding;
    }

    std::vector<std::uint8_t> bytes;
    bytes.reserve(compact.size() / 4 * 3);
    std::uint32_t accumulator = 0;
    int bits = 0;
    for (std::size_t i = 0; i < compact.size() - padding; ++i) {
        const char* found = std::strchr(Base64Alphabet, compact[i]);
        if (compact[i] == '\0' || found == nullptr) {
            throw std::invalid_argument("Base64Decode: invalid character");
        }
        accumulator = (accumulator << 6) | static_cast<std::uint32_t>(found - Base64Alphabet);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            bytes.push_back(static_cast<std::uint8_t>((accumulator >> bits) & 0xFFu));
        }
    }
    return bytes;
}

// 0x7a86e88
std::vector<std::uint8_t> ByteUtils::GZipCompressBytes(std::span<const std::uint8_t> data) {
    return GZipEncode(data);
}

// 0x7a8700c
void ByteUtils::GZipCompressBytes(std::span<const std::uint8_t> data, int offset, int size,
                                  std::ostream& output) {
    if (offset < 0 || size < 0 ||
        static_cast<std::size_t>(offset) + static_cast<std::size_t>(size) > data.size()) {
        throw std::out_of_range("ByteUtils::GZipCompressBytes: range outside data");
    }
    const auto encoded =
        GZipEncode(data.subspan(static_cast<std::size_t>(offset), static_cast<std::size_t>(size)));
    output.write(reinterpret_cast<const char*>(encoded.data()),
                 static_cast<std::streamsize>(encoded.size()));
}

// 0x7a87170
void ByteUtils::GZipCompressBytes(std::istream& input, std::ostream& output) {
    auto compressor = CreateGZipCompressStream(output);
    CopyTo(input, *compressor);
    compressor->Close();
}

// 0x7a87290
std::unique_ptr<GZipCompressStream> ByteUtils::CreateGZipCompressStream(std::ostream& output) {
    return std::make_unique<GZipCompressStream>(output);
}

// 0x7a872f4
std::vector<std::uint8_t> ByteUtils::GZipDecompressBytes(std::span<const std::uint8_t> data) {
    return GZipDecode(data);
}

// 0x7a87640
std::vector<std::uint8_t> ByteUtils::GZipCompressString(std::string_view data, Encoding encoding) {
    return GZipEncode(EncodingGetBytes(encoding, data));
}

// 0x7a879b4
std::string ByteUtils::GZipDecompressString(std::span<const std::uint8_t> data, Encoding encoding) {
    return EncodingGetString(encoding, GZipDecode(data));
}

// 0x7a86c28
int ByteUtils::WriteBytes(std::int64_t value, std::span<std::uint8_t> array, int offset) {
    StoreLittleEndian(array, offset, value);
    return 8;
}

// 0x7a86bb8
int ByteUtils::WriteBytes(std::int32_t value, std::span<std::uint8_t> array, int offset) {
    StoreLittleEndian(array, offset, value);
    return 4;
}

} // namespace Photon::Deterministic
