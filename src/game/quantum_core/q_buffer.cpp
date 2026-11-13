#include "q_buffer.h"

#include <array>
#include <stdexcept>
#include <string>

namespace Quantum::Collections {

namespace {

// Hash table capacities: 3, 7, 17, 29, then the widely published list of primes that
// roughly double while staying far from powers of two (53, 97, 193, ... 1610612741).
// The original keeps the same 30 values in an int table and a uint table.
constexpr std::array<std::uint32_t, 30> kPrimes = {
    3u,        7u,         17u,        29u,        53u,        97u,        193u,      389u,
    769u,      1543u,      3079u,      6151u,      12289u,     24593u,     49157u,    98317u,
    196613u,   393241u,    786433u,    1572869u,   3145739u,   6291469u,   12582917u, 25165843u,
    50331653u, 100663319u, 201326611u, 402653189u, 805306457u, 1610612741u};

[[noreturn]] void ThrowNoPrime() {
    throw std::overflow_error("QCollectionsUtils: capacity above the largest supported prime " +
                              std::to_string(kPrimes.back()));
}

} // namespace

int QBuffer::GetHashCode() const {
    std::uint32_t hash = 17u;
    hash = hash * 31u + static_cast<std::uint32_t>(Length);
    hash = hash * 31u + static_cast<std::uint32_t>(Stride);
    hash = hash * 31u + static_cast<std::uint32_t>(Ptr.GetHashCode());
    return static_cast<int>(hash);
}

int QCollectionsUtils::GetNextPrime(int value) {
    for (std::uint32_t prime : kPrimes) {
        if (static_cast<int>(prime) > value) {
            return static_cast<int>(prime);
        }
    }
    ThrowNoPrime();
}

std::uint32_t QCollectionsUtils::GetNextPrime(std::uint32_t value) {
    for (std::uint32_t prime : kPrimes) {
        if (prime > value) {
            return prime;
        }
    }
    ThrowNoPrime();
}

int QCollectionsUtils::GetNextPrimeGreaterThanOrEqual(int value) {
    for (std::uint32_t prime : kPrimes) {
        if (static_cast<int>(prime) >= value) {
            return static_cast<int>(prime);
        }
    }
    ThrowNoPrime();
}

std::uint32_t QCollectionsUtils::GetNextPrimeGreaterThanOrEqual(std::uint32_t value) {
    for (std::uint32_t prime : kPrimes) {
        if (prime >= value) {
            return prime;
        }
    }
    ThrowNoPrime();
}

void QBuffer::SerializeMeta(QBuffer* buffer, int stride, FrameSerializer* serializer) {
    serializer->Stream().Serialize(buffer->Length);
    if (serializer->Reading()) {
        buffer->Stride = stride;
    }
}

bool QBuffer::Serialize(QBuffer* buffer, int stride, FrameSerializer* serializer,
                        FrameSerializer::PtrKey& key) {
    SerializeMeta(buffer, stride, serializer);
    return serializer->PointerSerialize(&buffer->Ptr, key);
}

void QBuffer::Serialize(QBuffer* buffer, int stride, FrameSerializer* serializer, int count,
                        ItemSerializer itemSerializer) {
    SerializeMeta(buffer, stride, serializer);
    FrameSerializer::PtrKey key;
    if (!serializer->PointerSerialize(&buffer->Ptr, key)) {
        return;
    }
    auto* items = static_cast<std::byte*>(serializer->PointerResolve(key, buffer->Length * stride));
    for (int i = 0; i < count; ++i) {
        itemSerializer(items + static_cast<std::ptrdiff_t>(i) * stride, serializer);
    }
}

} // namespace Quantum::Collections
