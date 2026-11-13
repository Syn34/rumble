// frame.h - Quantum.Frame (quantum.code.dll): the game's simulation frame.
#pragma once

#include "quantum_code/components.h"
#include "quantum_core/frame_base.h"

namespace Quantum {

class Frame : public Core::FrameBase {
  public:
    using Core::FrameBase::FrameBase;
};

} // namespace Quantum
