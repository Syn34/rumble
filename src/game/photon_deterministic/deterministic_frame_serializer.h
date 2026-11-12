// deterministic_frame_serializer.h - Photon.Deterministic.IDeterministicFrameSerializer.
//
// Interface handed to per-type serialisation callbacks (such as RNGSession::Serialize)
// when a simulation frame is written to or read from a snapshot. Only the interface is
// declared here; implementations belong to the frame/snapshot code.
#pragma once

namespace Photon::Deterministic {

class IBitStream;

class IDeterministicFrameSerializer {
  public:
    virtual ~IDeterministicFrameSerializer() = default;

    // Interface slot 0: get_Writing
    virtual bool Writing() const = 0;
    // Interface slot 1: get_Reading
    virtual bool Reading() const = 0;
    // Interface slot 2: get_Stream
    virtual IBitStream& Stream() = 0;
};

} // namespace Photon::Deterministic
