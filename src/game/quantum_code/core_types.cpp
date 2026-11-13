// core_types.cpp - QStringN, EncodedDirection3, BTDataValue and BlackboardValue.
#include "quantum_code/core_types.h"

#include "photon_deterministic/fp_math.h"
#include "quantum_core/q_string.h"

#include <cstring>
#include <stdexcept>

namespace Quantum {

namespace {

// QString.ConstructFrom for any QStringN layout.
template <class Q> Q MakeQString(std::u16string_view str) {
    if (!QString::CanHold(str, Q::MaxByteCount)) {
        throw std::invalid_argument("QString: string too long for this QString type");
    }
    Q result{};
    if (!str.empty()) {
        result.ByteCount = static_cast<std::uint16_t>(QString::GetByteCount(str));
        std::memcpy(result.Bytes.data(), str.data(), result.ByteCount);
    }
    return result;
}

template <class Q> std::u16string QStringText(const Q& q) {
    std::u16string text(static_cast<std::size_t>(q.ByteCount >> 1), u'\0');
    std::memcpy(text.data(), q.Bytes.data(), text.size() * sizeof(char16_t));
    return text;
}

// Hash of the byte count, then of the used bytes only.
template <class Q> std::int32_t QStringHash(std::uint32_t seed, const Q& q) {
    const std::int32_t bytesHash = HashCodeUtils::GetArrayHashCode(
        q.Bytes.data(), q.ByteCount, [](std::uint8_t b) { return FieldHash(b); });
    return HashBuilder(seed).Add(q.ByteCount).Add(bytesHash).Value();
}

// The count must fit the buffer (the original asserts it), then the used bytes go as one
// buffer.
template <class Q> void SerializeQString(Q* q, FrameSerializer* serializer) {
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    stream.Serialize(&q->ByteCount);
    if (q->ByteCount > Q::MaxByteCount) {
        throw std::logic_error("QString: stored byte count exceeds the capacity");
    }
    stream.SerializeBuffer(q->Bytes.data(), q->ByteCount);
}

// Reads one member of a union's shared storage as another type would see it.
template <class T> T ReadStorage(const void* storage) {
    T value{};
    std::memcpy(static_cast<void*>(&value), storage, sizeof(T));
    return value;
}

// Switches a union to `field`, clearing `size` bytes of the storage if another member
// was active.
void SelectUnionField(std::int32_t& fieldUsed, int field, void* storage, std::size_t size) {
    if (fieldUsed != field) {
        std::memset(storage, 0, size);
        fieldUsed = field;
    }
}

} // namespace

// --- QString34 / 50 / 64 ---------------------------------------------------------------

QString34 QString34::FromString(std::u16string_view str) { return MakeQString<QString34>(str); }
std::u16string QString34::ToString() const { return QStringText(*this); }
int QString34::CompareOrdinal(const std::uint8_t* bytes, std::uint16_t byteCount) const {
    return QString::CompareOrdinal(Bytes.data(), ByteCount, bytes, byteCount);
}
bool QString34::Equals(std::u16string_view other) const {
    return QString::CompareOrdinal(Bytes.data(), ByteCount, other) == 0;
}
std::int32_t QString34::GetHashCode() const { return QStringHash(73u, *this); }
void QString34::Serialize(void* ptr, FrameSerializer* serializer) {
    SerializeQString(static_cast<QString34*>(ptr), serializer);
}

QString50 QString50::FromString(std::u16string_view str) { return MakeQString<QString50>(str); }
std::u16string QString50::ToString() const { return QStringText(*this); }
int QString50::CompareOrdinal(const std::uint8_t* bytes, std::uint16_t byteCount) const {
    return QString::CompareOrdinal(Bytes.data(), ByteCount, bytes, byteCount);
}
bool QString50::Equals(std::u16string_view other) const {
    return QString::CompareOrdinal(Bytes.data(), ByteCount, other) == 0;
}
std::int32_t QString50::GetHashCode() const { return QStringHash(79u, *this); }
void QString50::Serialize(void* ptr, FrameSerializer* serializer) {
    SerializeQString(static_cast<QString50*>(ptr), serializer);
}

QString64 QString64::FromString(std::u16string_view str) { return MakeQString<QString64>(str); }
std::u16string QString64::ToString() const { return QStringText(*this); }
int QString64::CompareOrdinal(const std::uint8_t* bytes, std::uint16_t byteCount) const {
    return QString::CompareOrdinal(Bytes.data(), ByteCount, bytes, byteCount);
}
bool QString64::Equals(std::u16string_view other) const {
    return QString::CompareOrdinal(Bytes.data(), ByteCount, other) == 0;
}
std::int32_t QString64::GetHashCode() const { return QStringHash(83u, *this); }
void QString64::Serialize(void* ptr, FrameSerializer* serializer) {
    SerializeQString(static_cast<QString64*>(ptr), serializer);
}

// --- EncodedDirection3 -----------------------------------------------------------------

namespace {

constexpr std::int64_t HalfTurnRaw = std::int64_t{180} << 16;
constexpr std::int64_t QuarterTurnRaw = std::int64_t{90} << 16;

std::int16_t EncodeYaw(FP yaw) {
    // C# % keeps the dividend's sign, like C++ %. The scaled value is computed in 32 bits
    // and its upper half kept, as the original does.
    const auto wrapped = static_cast<std::int32_t>(yaw.RawValue() % HalfTurnRaw);
    return static_cast<std::int16_t>((wrapped * EncodedDirection3::YAW_MULT) >> 16);
}

std::int16_t EncodePitch(FP pitch) {
    const FP clamped = Photon::Deterministic::FPMath::Clamp(pitch, FP::FromRaw(-QuarterTurnRaw),
                                                             FP::FromRaw(QuarterTurnRaw));
    const auto raw = static_cast<std::int32_t>(clamped.RawValue());
    return static_cast<std::int16_t>((raw * EncodedDirection3::PITCH_MULT) >> 16);
}

} // namespace

void EncodedDirection3::set_Yaw(FP value) { encodedYaw = EncodeYaw(value); }

void EncodedDirection3::set_Pitch(FP value) { encodedPitch = EncodePitch(value); }

FPQuaternion EncodedDirection3::get_Rotation() const {
    return FPQuaternion::Euler(get_Pitch(), get_Yaw(), FP{});
}

void EncodedDirection3::set_Rotation(const FPQuaternion& value) {
    const FPVector3 euler = value.AsEuler();
    encodedPitch = EncodePitch(euler.X);
    encodedYaw = EncodeYaw(euler.Y);
}

// --- BTDataValue -----------------------------------------------------------------------

FP* BTDataValue::get_FPValue() {
    SelectUnionField(_field_used_, FPVALUE, &_FPValue, sizeof(FP));
    return &_FPValue;
}

std::int32_t* BTDataValue::get_IntValue() {
    SelectUnionField(_field_used_, INTVALUE, &_IntValue, sizeof(std::int32_t));
    return &_IntValue;
}

std::int32_t BTDataValue::GetHashCode() const {
    return HashBuilder(2027u)
        .Add(ReadStorage<FP>(&_FPValue))
        .Add(ReadStorage<std::int32_t>(&_FPValue))
        .Add(_field_used_)
        .Value();
}

void BTDataValue::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BTDataValue*>(ptr);
    SerializeField(&p->_field_used_, serializer);
    if (p->_field_used_ == FPVALUE) {
        SerializeField(&p->_FPValue, serializer);
    }
    if (p->_field_used_ == INTVALUE) {
        SerializeField(&p->_IntValue, serializer);
    }
}

// --- BlackboardValue -------------------------------------------------------------------

AssetRef* BlackboardValue::get_AssetRefValue() {
    SelectUnionField(_field_used_, ASSETREFVALUE, &_AssetRefValue, sizeof(AssetRef));
    return &_AssetRefValue;
}
QBoolean* BlackboardValue::get_BooleanValue() {
    SelectUnionField(_field_used_, BOOLEANVALUE, &_AssetRefValue, sizeof(QBoolean));
    return &_BooleanValue;
}
std::uint8_t* BlackboardValue::get_ByteValue() {
    SelectUnionField(_field_used_, BYTEVALUE, &_AssetRefValue, sizeof(std::uint8_t));
    return &_ByteValue;
}
EntityRef* BlackboardValue::get_EntityRefValue() {
    SelectUnionField(_field_used_, ENTITYREFVALUE, &_AssetRefValue, sizeof(EntityRef));
    return &_EntityRefValue;
}
FP* BlackboardValue::get_FPValue() {
    SelectUnionField(_field_used_, FPVALUE, &_AssetRefValue, sizeof(FP));
    return &_FPValue;
}
FPVector2* BlackboardValue::get_FPVector2Value() {
    SelectUnionField(_field_used_, FPVECTOR2VALUE, &_AssetRefValue, sizeof(FPVector2));
    return &_FPVector2Value;
}
FPVector3* BlackboardValue::get_FPVector3Value() {
    SelectUnionField(_field_used_, FPVECTOR3VALUE, &_AssetRefValue, sizeof(FPVector3));
    return &_FPVector3Value;
}
std::int32_t* BlackboardValue::get_IntegerValue() {
    SelectUnionField(_field_used_, INTEGERVALUE, &_AssetRefValue, sizeof(std::int32_t));
    return &_IntegerValue;
}

std::int32_t BlackboardValue::GetHashCode() const {
    const void* storage = &_AssetRefValue;
    return HashBuilder(2029u)
        .Add(ReadStorage<AssetRef>(storage))
        .Add(ReadStorage<QBoolean>(storage))
        .Add(ReadStorage<std::uint8_t>(storage))
        .Add(ReadStorage<EntityRef>(storage))
        .Add(ReadStorage<FP>(storage))
        .Add(ReadStorage<FPVector2>(storage))
        .Add(ReadStorage<FPVector3>(storage))
        .Add(ReadStorage<std::int32_t>(storage))
        .Add(_field_used_)
        .Value();
}

void BlackboardValue::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlackboardValue*>(ptr);
    SerializeField(&p->_field_used_, serializer);
    // Members in declaration order, each only when active.
    if (p->_field_used_ == ASSETREFVALUE) {
        SerializeField(&p->_AssetRefValue, serializer);
    }
    if (p->_field_used_ == BOOLEANVALUE) {
        SerializeField(&p->_BooleanValue, serializer);
    }
    if (p->_field_used_ == BYTEVALUE) {
        SerializeField(&p->_ByteValue, serializer);
    }
    if (p->_field_used_ == ENTITYREFVALUE) {
        SerializeField(&p->_EntityRefValue, serializer);
    }
    if (p->_field_used_ == FPVALUE) {
        SerializeField(&p->_FPValue, serializer);
    }
    if (p->_field_used_ == FPVECTOR2VALUE) {
        SerializeField(&p->_FPVector2Value, serializer);
    }
    if (p->_field_used_ == FPVECTOR3VALUE) {
        SerializeField(&p->_FPVector3Value, serializer);
    }
    if (p->_field_used_ == INTEGERVALUE) {
        SerializeField(&p->_IntegerValue, serializer);
    }
}

} // namespace Quantum
