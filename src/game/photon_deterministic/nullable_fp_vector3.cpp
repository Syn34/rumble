#include "nullable_fp_vector3.h"

#include <optional>

namespace Photon::Deterministic {

// 0x7a76950
FPVector3 NullableFPVector3::Value() const {
    if (_hasValue == 0) {
        throw std::bad_optional_access();
    }
    return _value;
}

} // namespace Photon::Deterministic
