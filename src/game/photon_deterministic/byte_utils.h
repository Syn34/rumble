// byte_utils.h - Photon.Deterministic.ByteUtils: helpers for packing byte blocks,
// raw little-endian value writes, Base64 and GZip.
//
// The original is a static C# class working on byte[] and System.IO.Stream. Here byte
// arrays are std::vector / std::span and streams are std::istream / std::ostream.
#pragma once

#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <span>
#include <streambuf>
#include <string>
#include <string_view>
#include <vector>

namespace Photon::Deterministic {

// Stand-in for the System.Text.Encoding argument some methods take. Strings in this
// codebase are UTF-8 std::string, and the only encoding PhotonDeterministic itself
// passes is Encoding.UTF8, so that is all this supports for now.
// TODO: replace with the System.Text.Encoding reimplementation once mscorlib exists.
enum class Encoding { UTF8 };

std::vector<std::uint8_t> EncodingGetBytes(Encoding encoding, std::string_view text);
std::string EncodingGetString(Encoding encoding, std::span<const std::uint8_t> bytes);

// Write side of a gzip stream (System.IO.Compression.GZipStream in Compress mode with
// leaveOpen = true). Bytes written to it are compressed into `output` when the stream
// is closed or destroyed; `output` itself is left open.
class GZipCompressStream : public std::ostream {
  public:
    explicit GZipCompressStream(std::ostream& output);
    ~GZipCompressStream() override;

    GZipCompressStream(const GZipCompressStream&) = delete;
    GZipCompressStream& operator=(const GZipCompressStream&) = delete;

    // Finishes the gzip member. Further writes are ignored.
    void Close();

  private:
    class Buffer : public std::streambuf {
      public:
        std::vector<std::uint8_t> bytes;
        bool closed = false;

      protected:
        int_type overflow(int_type ch) override;
        std::streamsize xsputn(const char* s, std::streamsize count) override;
    };

    Buffer buffer_;
    std::ostream& output_;
};

class ByteUtils final {
  public:
    ByteUtils() = delete;

    // Concatenates the blocks.
    static std::vector<std::uint8_t>
    MergeByteBlocks(std::span<const std::vector<std::uint8_t>> blocks);

    // A "value block" is a 32-bit size followed by the value's bytes. Each returns
    // the offset just past what it wrote.
    static int AddValueBlock(std::int32_t value, std::span<std::uint8_t> buffer, int offset);
    static int AddValueBlock(std::int64_t value, std::span<std::uint8_t> buffer, int offset);
    static int AddValueBlock(std::uint64_t value, std::span<std::uint8_t> buffer, int offset);

    // Reserve room for a 32-bit block-size header at `offset`; returns where the
    // block's contents start.
    static int BeginByteBlockHeader(std::span<std::uint8_t> buffer, int offset, int& blockStart);
    // Fill in the header reserved by BeginByteBlockHeader; returns the offset past
    // the block.
    static int EndByteBlockHeader(std::span<std::uint8_t> buffer, int blockStart, int bytesWritten);

    // Each block prefixed by its 32-bit length, all concatenated.
    static std::vector<std::uint8_t>
    PackByteBlocks(std::span<const std::vector<std::uint8_t>> blocks);
    // Inverse of PackByteBlocks. The original is a lazy iterator; this returns all
    // blocks at once.
    static std::vector<std::vector<std::uint8_t>>
    ReadByteBlocks(std::span<const std::uint8_t> data);

    // Copies `source` to `destination` in 4 KiB chunks until the source is exhausted.
    static void CopyTo(std::istream& source, std::ostream& destination);

    static std::string Base64Encode(std::span<const std::uint8_t> data);
    // Throws std::invalid_argument on malformed input (FormatException in the original).
    static std::vector<std::uint8_t> Base64Decode(std::string_view data);

    static std::vector<std::uint8_t> GZipCompressBytes(std::span<const std::uint8_t> data);
    static void GZipCompressBytes(std::span<const std::uint8_t> data, int offset, int size,
                                  std::ostream& output);
    static void GZipCompressBytes(std::istream& input, std::ostream& output);
    static std::unique_ptr<GZipCompressStream> CreateGZipCompressStream(std::ostream& output);
    // Throws std::runtime_error on corrupt input (InvalidDataException in the original).
    static std::vector<std::uint8_t> GZipDecompressBytes(std::span<const std::uint8_t> data);
    static std::vector<std::uint8_t> GZipCompressString(std::string_view data, Encoding encoding);
    static std::string GZipDecompressString(std::span<const std::uint8_t> data, Encoding encoding);

    // Raw little-endian store at `offset`; returns the number of bytes written.
    static int WriteBytes(std::int64_t value, std::span<std::uint8_t> array, int offset);
    static int WriteBytes(std::int32_t value, std::span<std::uint8_t> array, int offset);
};

} // namespace Photon::Deterministic
