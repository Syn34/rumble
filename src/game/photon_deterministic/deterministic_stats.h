// deterministic_stats.h - Photon.Deterministic.DeterministicStats: per-update session
// statistics for debug overlays, refreshed by DeterministicSession::UpdateStats.
#pragma once

namespace Photon::Deterministic {

class DeterministicStats {
  public:
    // 0x7a8e100
    DeterministicStats() = default;

    // 0x7a8e0b8 / 0x7a8e0c0 - round trip time in ms.
    int Ping() const { return ping_; }
    void SetPing(int value) { ping_ = value; }
    // 0x7a8e0c8 - number of the latest predicted frame.
    int Frame() const { return frame_; }
    void SetFrame(int value) { frame_ = value; }
    // 0x7a8e0d0 / 0x7a8e0d8 - current local input offset in frames.
    int Offset() const { return offset_; }
    void SetOffset(int value) { offset_ = value; }
    // 0x7a8e0e0 - frames predicted during the last update.
    int Predicted() const { return predicted_; }
    void SetPredicted(int value) { predicted_ = value; }
    // 0x7a8e0e8 - never written by the session.
    int ResimulatedFrames() const { return resimulatedFrames_; }
    // 0x7a8e0f0 / 0x7a8e0f8 - seconds spent in the last simulation update.
    double UpdateTime() const { return updateTime_; }
    void SetUpdateTime(double value) { updateTime_ = value; }

  private:
    int ping_ = 0;
    int frame_ = 0;
    int offset_ = 0;
    int predicted_ = 0;
    int resimulatedFrames_ = 0;
    double updateTime_ = 0.0;
};

} // namespace Photon::Deterministic
