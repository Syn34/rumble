#include "deterministic_simulator.h"

#include "deterministic_command.h"
#include "deterministic_interfaces.h"
#include "deterministic_session.h"
#include "deterministic_simulator_utils.h"
#include "deterministic_tick_input.h"

#include <algorithm>
#include <stdexcept>

namespace Photon::Deterministic {

namespace {

constexpr std::uint64_t PlayerBit(int player) {
    return std::uint64_t{1} << player;
}

IDeterministicGame& RequireGame(const DeterministicSession& session) {
    IDeterministicGame* game = session.Game();
    if (game == nullptr) {
        throw std::logic_error("DeterministicSimulator: the session has no game");
    }
    return *game;
}

const DeterministicSessionConfig& RequireConfig(const DeterministicSession& session) {
    const auto& config = session.SessionConfig();
    if (!config) {
        throw std::logic_error("DeterministicSimulator: the session has no session config");
    }
    return *config;
}

} // namespace

// --- InputFrame -----------------------------------------------------------------------

DeterministicSimulator::InputFrame::InputFrame(int playerCount, int inputSize,
                                               Native::Allocator* allocator)
    : Input(std::make_unique<DeterministicFrameInput>(playerCount, inputSize, allocator)) {}

void DeterministicSimulator::InputFrame::DeserializeCommands(
    DeterministicCommandSerializer& serializer) {
    Input->DeserializeCommands(serializer);
}

std::string DeterministicSimulator::InputFrame::ToString() const {
    return "InputFrame " + std::to_string(Number);
}

// --- DeterministicSimulator -----------------------------------------------------------

DeterministicSimulator::DeterministicSimulator(DeterministicSession& session) : session_(session) {}

DeterministicSimulator::~DeterministicSimulator() = default;

int DeterministicSimulator::FramesRemaining() const {
    return detail::DoubleToInt32(timeAccumulated_ / DeltaTime());
}

double DeterministicSimulator::DeltaTime() const {
    return 1.0 / static_cast<double>(RequireConfig(session_).UpdateFPS);
}

std::shared_ptr<DeterministicFrame> DeterministicSimulator::GetVerifiedFrame(int tick) {
    return RequireGame(session_).GetVerifiedFrame(tick);
}

ByteArray DeterministicSimulator::GetExtraErrorFrameDumpData(DeterministicFrame& frame) {
    return RequireGame(session_).GetExtraErrorFrameDumpData(frame);
}

bool DeterministicSimulator::IsFrameVerified(int frame) const {
    return inputFrameHead_ != nullptr && inputFrameHead_->Number >= frame;
}

void DeterministicSimulator::AccumulateTime(double dt) {
    timeAccumulated_ += deltaScale_ * dt;
}

void DeterministicSimulator::AdjustClock(double dt) {
    timeAccumulated_ += dt;
}

bool DeterministicSimulator::StepFrameNumber(int& frame) {
    const bool haveTime = timeAccumulated_ >= DeltaTime();
    const bool stalledAhead =
        isStalling_ && stateVerified_->Number + minFramesToVerify_ <= inputFrameCounter_;
    if (!haveTime || clockPaused_ || stalledAhead) {
        frame = -1;
        return false;
    }
    timeAccumulated_ -= DeltaTime();
    frame = ++inputFrameCounter_;
    return true;
}

bool DeterministicSimulator::StepFrameNumberDown(int count) {
    const bool possible = inputFrameCounter_ >= count;
    if (possible) {
        const double dt = DeltaTime();
        inputFrameCounter_ -= count;
        timeAccumulated_ += dt * static_cast<double>(count);
    }
    return possible;
}

std::shared_ptr<DeterministicFrame>
DeterministicSimulator::CreateStateFrame(const std::shared_ptr<IDisposable>& context, int number) {
    IDeterministicGame& game = RequireGame(session_);
    std::shared_ptr<DeterministicFrame> frame = game.CreateFrame(context);
    if (!frame) {
        throw std::logic_error("DeterministicSimulator: the game returned no frame");
    }
    frame->Number = number;
    frame->Verified = true;
    const int playerCount = session_.SessionConfig() ? session_.SessionConfig()->PlayerCount : 0;
    frame->Init(playerCount, game.GetInputInMemorySize(), session_.PlatformInfo().Allocator);
    return frame;
}

void DeterministicSimulator::InsertInput(const DeterministicFrameInputTemp& input) {
    if (input.Frame <= stateVerified_->Number) {
        return; // too old to matter
    }
    const std::uint64_t bit = PlayerBit(input.Player);
    InputFrame& f = GetInputFrame(input.Frame);
    if (input.IsVerified) {
        if (f.Verified & bit) {
            return;
        }
        f.Verified |= bit;
    } else {
        if ((f.Predicted & bit) || (f.Verified & bit)) {
            return;
        }
        f.Predicted |= bit;
    }
    f.Input->Assign(input, RequireGame(session_));
}

void DeterministicSimulator::CheckIsStalling(int maxVerifiedTicks) {
    const int verifiedNumber = stateVerified_ ? stateVerified_->Number : 0;
    const int targetFrame =
        detail::DoubleToInt32(timeAccumulated_ / DeltaTime()) + inputFrameCounter_;

    // Count the complete ticks right after the verified state.
    minFramesToVerify_ = 0;
    if (verifiedNumber < targetFrame && maxVerifiedTicks >= 1) {
        for (int frame = verifiedNumber + 1;; ++frame) {
            if ((playerMask_ & ~GetInputFrame(frame).Verified) != 0) {
                break;
            }
            ++minFramesToVerify_;
            if (frame >= targetFrame || minFramesToVerify_ >= maxVerifiedTicks) {
                break;
            }
        }
    }

    const DeterministicGameMode mode = session_.GameMode();
    if (mode == DeterministicGameMode::Spectating || mode == DeterministicGameMode::Multiplayer) {
        // Online: stall when even after verifying, the target is beyond the rollback window.
        const int current = stateVerified_ ? stateVerified_->Number : 0;
        isStalling_ =
            minFramesToVerify_ + current + RequireConfig(session_).RollbackWindow < targetFrame;
    } else {
        isStalling_ = minFramesToVerify_ >= maxVerifiedTicks;
    }
}

void DeterministicSimulator::CopyPreviousUpdatePredicted(
    const std::shared_ptr<DeterministicFrame>& fromState) {
    if (session_.IsLockstep() || !session_.IsInterpolatable()) {
        return;
    }
    const std::shared_ptr<DeterministicFrame>& source = fromState ? fromState : statePredicted_;
    statePreviousUpdatePredicted_->CopyFrom(*source);
}

void DeterministicSimulator::Simulate(int maxVerifiedTicks) {
    predictedFramesLastUpdate_ = 0;
    const int previousVerified = stateVerified_->Number;
    const bool verifiedAdvanced = SimulateVerified(maxVerifiedTicks);
    if (session_.IsLockstep()) {
        return;
    }

    const int previousPredicted = statePredicted_->Number;
    const int rollbackWindow = session_.RollbackWindow();
    if (previousPredicted - stateVerified_->Number >= rollbackWindow) {
        return;
    }

    if (verifiedAdvanced) {
        // Roll the prediction back to the new verified state. If the predicted state was
        // still sitting on the old verified tick it will not be re-simulated past, so keep
        // it as the previous update's prediction now.
        if (previousPredicted == previousVerified) {
            CopyPreviousUpdatePredicted(nullptr);
        }
        statePredicted_->CopyFrom(*stateVerified_);
    } else {
        CopyPreviousUpdatePredicted(nullptr);
    }

    if (isStalling_ || statePredicted_->Number >= inputFrameCounter_) {
        return;
    }
    for (int frame = statePredicted_->Number + 1;; ++frame) {
        if (frame - stateVerified_->Number >= session_.RollbackWindow()) {
            return;
        }
        Simulate(GetInputFrame(frame), *statePredicted_, true);
        ++predictedFramesLastUpdate_;
        if (frame == previousPredicted) {
            CopyPreviousUpdatePredicted(nullptr);
        }
        if (frame >= inputFrameCounter_) {
            return;
        }
    }
}

void DeterministicSimulator::OnGameStart() {
    if (!session_.IsLockstep()) {
        statePredicted_->CopyFrom(*stateVerified_);
    }
    if (session_.IsInterpolatable()) {
        statePredictedPrevious_->CopyFrom(*stateVerified_);
    }
    CopyPreviousUpdatePredicted(nullptr);

    if (session_.GameMode() != DeterministicGameMode::Local) {
        return;
    }
    // Offline, the ticks covered by the input delay never get polled input; tell the game
    // they are confirmed as empty (ResetInputFrameHead marked them complete).
    const DeterministicSessionConfig& config = RequireConfig(session_);
    for (int delay = 1; delay <= config.InputDelayMin; ++delay) {
        for (int player = 0; player < config.PlayerCount; ++player) {
            DeterministicFrameInputTemp empty;
            empty.Frame = stateVerified_->Number + delay;
            empty.Player = player;
            empty.IsVerified = true;
            RequireGame(session_).OnInputConfirmed(empty);
        }
    }
}

void DeterministicSimulator::Initialize(const std::shared_ptr<IDisposable>& context) {
    inputFrameLookup_.clear();
    inputFramePool_.clear();

    const DeterministicSessionConfig& config = RequireConfig(session_);
    const int firstFrame = config.RollbackWindow - 1;
    stateVerified_ = CreateStateFrame(context, firstFrame);
    statePredicted_ =
        session_.IsLockstep() ? stateVerified_ : CreateStateFrame(context, firstFrame);
    statePredictedPrevious_ =
        session_.IsInterpolatable() ? CreateStateFrame(context, firstFrame) : stateVerified_;
    statePreviousUpdatePredicted_ = (session_.IsLockstep() || !session_.IsInterpolatable())
                                        ? stateVerified_
                                        : CreateStateFrame(context, firstFrame);

    for (int player = 0; player < config.PlayerCount; ++player) {
        playerMask_ |= PlayerBit(player);
    }
    for (int player : session_.LocalPlayerIndices()) {
        playerMaskLocal_ |= PlayerBit(player);
    }
    playerMaskRemote_ = playerMask_ & ~playerMaskLocal_;

    predictInput_.assign(static_cast<std::size_t>(config.PlayerCount), nullptr);
    predictFlags_.assign(static_cast<std::size_t>(config.PlayerCount),
                         DeterministicInputFlags::None);
    predictPlayers_.clear();

    ResetInputFrameHead(firstFrame, true);
}

void DeterministicSimulator::Reset(const DeterministicFrame& frame, bool resetInputFrame) {
    ResetStateFrame(*stateVerified_, frame);
    ResetFrames(resetInputFrame);
}

void DeterministicSimulator::Reset(std::span<const std::uint8_t> frameData,
                                   std::optional<int> frameNumber, bool resetInputFrame) {
    stateVerified_->Deserialize(frameData);
    if (frameNumber) {
        stateVerified_->Number = *frameNumber;
    }
    ResetFrames(resetInputFrame);
}

void DeterministicSimulator::ResetFrames(bool resetInputFrame) {
    if (!session_.IsLockstep()) {
        ResetStateFrame(*statePredicted_, *stateVerified_);
    }
    if (session_.IsInterpolatable()) {
        ResetStateFrame(*statePredictedPrevious_, *stateVerified_);
    }
    ResetInputFrameHead(stateVerified_->Number, resetInputFrame);
    CopyPreviousUpdatePredicted(nullptr);
}

void DeterministicSimulator::Simulate(InputFrame& f, DeterministicFrame& state, bool predicted) {
    if (f.Number == inputFrameCounter_ && session_.IsInterpolatable()) {
        // About to simulate the newest tick: keep the state before it for interpolation.
        statePredictedPrevious_->CopyFrom(state);
    }
    if (predicted) {
        PredictInput(f);
    }
    f.Input->DeserializeCommands(session_.CommandSerializer());
    ApplyInputToStateFrame(f, state, predicted);
    IDeterministicGame& game = RequireGame(session_);
    DeterministicSimulatorUtils::CallSimulate(state, game);
    DeterministicSimulatorUtils::CallSimulateFinished(state, game);
}

void DeterministicSimulator::PredictInput(InputFrame& f) {
    const std::uint64_t missing = playerMaskRemote_ & ~f.Verified;
    if (missing == 0) {
        return;
    }
    std::fill(predictInput_.begin(), predictInput_.end(), nullptr);
    std::fill(predictFlags_.begin(), predictFlags_.end(), DeterministicInputFlags::None);

    predictPlayers_.clear();
    const DeterministicSessionConfig& config = RequireConfig(session_);
    for (int player = 0; player < config.PlayerCount; ++player) {
        if (missing & PlayerBit(player)) {
            predictPlayers_.push_back(player);
        }
    }

    // Walk the older frames in order, so the newest usable input wins.
    for (InputFrame* node = inputFrameHead_; node != nullptr && node != &f; node = node->Next) {
        if (f.Number - node->Number > config.InputRepeatMaxDistance ||
            (node->Verified & missing) == 0) {
            continue;
        }
        for (int player : predictPlayers_) {
            const std::uint64_t bit = PlayerBit(player);
            if ((bit & node->Verified) != 0 &&
                HasFlag(node->Input->Flags.at(static_cast<std::size_t>(player)),
                        DeterministicInputFlags::Repeatable)) {
                f.Input->CopyForPrediction(*node->Input, player);
                f.Predicted |= bit;
            }
        }
    }
}

void DeterministicSimulator::ResetInputFrameHead(int frame, bool resetInputFrame) {
    DestroyAllInputFrames();

    std::unique_ptr<InputFrame> head = AcquireInputFrame(frame);
    head->Verified = playerMask_;
    head->Predicted = playerMask_;
    inputFrameHead_ = head.get();
    inputFrameLookup_.emplace(frame, std::move(head));

    if (!resetInputFrame) {
        return;
    }
    inputFrameCounter_ = frame;
    if (session_.GameMode() == DeterministicGameMode::Local) {
        // No input is ever polled for the input-delay ticks: treat them as complete.
        const int inputDelay = RequireConfig(session_).InputDelayMin;
        for (int delay = 1; delay <= inputDelay; ++delay) {
            InputFrame& delayed = GetInputFrame(frame + delay);
            delayed.Verified = playerMask_;
            delayed.Predicted = playerMask_;
        }
    }
}

bool DeterministicSimulator::SimulateVerified(int maxTicks) {
    if (inputFrameHead_ == nullptr) {
        throw std::logic_error("DeterministicSimulator: no input frame list");
    }
    bool advanced = false;
    for (int remaining = maxTicks; remaining >= 1; --remaining) {
        InputFrame* previous = inputFrameHead_;
        InputFrame* next = previous->Next;
        if (next == nullptr || next->Number != stateVerified_->Number + 1 ||
            (playerMask_ & ~next->Verified) != 0 || next->Number > inputFrameCounter_) {
            break;
        }

        --minFramesToVerify_;
        previous->Next = nullptr;
        inputFrameHead_ = next;
        ReleaseInputFrame(*previous);

        Simulate(*inputFrameHead_, *stateVerified_, false);
        if (inputFrameHead_->Number == statePredicted_->Number) {
            CopyPreviousUpdatePredicted(stateVerified_);
        }

        // Checksums are taken on the configured interval and reported to the network (a
        // no-op offline) and the game, for the tick just simulated.
        const int interval = RequireConfig(session_).ChecksumInterval;
        const int tick = inputFrameHead_->Number;
        if (interval >= 1 && tick % interval == 0) {
            const std::uint64_t checksum =
                DeterministicSimulatorUtils::CallChecksum(*stateVerified_);
            if (!session_.network_) {
                throw std::logic_error("DeterministicSimulator: the session has no network");
            }
            session_.network_->SendLocalChecksum(tick, checksum);
            DeterministicSimulatorUtils::CallChecksumComputed(RequireGame(session_), tick,
                                                              checksum);
        }
        advanced = true;
    }
    return advanced;
}

void DeterministicSimulator::ApplyInputToStateFrame(InputFrame& inputFrame,
                                                    DeterministicFrame& state, bool predicted) {
    state.Number = inputFrame.Number;
    state.Verified = !predicted && (playerMask_ & ~inputFrame.Verified) == 0;
    state.Input->CopyFrom(*inputFrame.Input);
}

void DeterministicSimulator::ResetStateFrame(DeterministicFrame& target,
                                             const DeterministicFrame& source) {
    target.Copy(source);
    target.Verified = true;
    target.Number = source.Number;
    const int playerCount = session_.SessionConfig() ? session_.SessionConfig()->PlayerCount : 0;
    target.Init(playerCount, RequireGame(session_).GetInputInMemorySize(),
                session_.PlatformInfo().Allocator);
}

DeterministicSimulator::InputFrame& DeterministicSimulator::GetInputFrame(int frame) {
    if (auto it = inputFrameLookup_.find(frame); it != inputFrameLookup_.end()) {
        return *it->second;
    }
    if (inputFrameHead_ == nullptr) {
        throw std::logic_error("DeterministicSimulator: no input frame list");
    }

    std::unique_ptr<InputFrame> created = AcquireInputFrame(frame);
    InputFrame* f = created.get();
    inputFrameLookup_.emplace(frame, std::move(created));

    // Link into the list, which is sorted by tick.
    if (inputFrameHead_->Number > f->Number) {
        f->Next = inputFrameHead_;
        inputFrameHead_ = f;
        return *f;
    }
    InputFrame* node = inputFrameHead_;
    while (node->Next != nullptr && node->Next->Number <= f->Number) {
        node = node->Next;
    }
    f->Next = node->Next;
    node->Next = f;
    return *f;
}

std::unique_ptr<DeterministicSimulator::InputFrame>
DeterministicSimulator::AcquireInputFrame(int frame) {
    std::unique_ptr<InputFrame> f;
    if (!inputFramePool_.empty()) {
        f = std::move(inputFramePool_.back());
        inputFramePool_.pop_back();
    } else {
        const int playerCount =
            session_.SessionConfig() ? session_.SessionConfig()->PlayerCount : 0;
        f = std::make_unique<InputFrame>(playerCount, RequireGame(session_).GetInputInMemorySize(),
                                         session_.PlatformInfo().Allocator);
    }
    f->Number = frame;
    return f;
}

void DeterministicSimulator::ReleaseInputFrame(InputFrame& f) {
    auto it = inputFrameLookup_.find(f.Number);
    if (it == inputFrameLookup_.end() || it->second.get() != &f) {
        throw std::logic_error("DeterministicSimulator: releasing an input frame it does not own");
    }
    std::unique_ptr<InputFrame> owned = std::move(it->second);
    inputFrameLookup_.erase(it);
    owned->Next = nullptr;
    owned->Number = 0;
    owned->Verified = 0;
    owned->Predicted = 0;
    owned->Input->Clear();
    inputFramePool_.push_back(std::move(owned));
}

void DeterministicSimulator::DestroyAllInputFrames() {
    Native::Allocator* allocator = session_.PlatformInfo().Allocator;
    for (auto& pooled : inputFramePool_) {
        pooled->Input->Free(allocator);
    }
    inputFramePool_.clear();

    for (InputFrame* node = inputFrameHead_; node != nullptr; node = node->Next) {
        node->Input->Free(allocator);
    }
    inputFrameHead_ = nullptr;

    for (auto& [number, frame] : inputFrameLookup_) {
        frame->Input->Free(allocator); // a no-op for frames already freed above
    }
    inputFrameLookup_.clear();
}

void DeterministicSimulator::Destroy() {
    DestroyAllInputFrames();
    Native::Allocator* allocator = session_.PlatformInfo().Allocator;

    auto release = [&](std::shared_ptr<DeterministicFrame>& frame, bool isVerified) {
        if (!frame || (!isVerified && frame == stateVerified_)) {
            return;
        }
        frame->Free();
        frame->Input->Free(allocator);
    };
    // The predicted states may alias the verified one; those are freed once.
    release(stateVerified_, true);
    release(statePredicted_, false);
    release(statePredictedPrevious_, false);
    release(statePreviousUpdatePredicted_, false);
}

void DeterministicSimulator::ChecksumError(const DeterministicTickChecksumError& error) {
    const std::shared_ptr<DeterministicFrame> frames[] = {stateVerified_};
    DeterministicSimulatorUtils::CallChecksumError(RequireGame(session_), error, frames);
}

} // namespace Photon::Deterministic
