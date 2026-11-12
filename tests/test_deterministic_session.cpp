#include "test_framework.h"

#include "photon_deterministic/deterministic_command.h"
#include "photon_deterministic/deterministic_containers.h"
#include "photon_deterministic/deterministic_frame.h"
#include "photon_deterministic/deterministic_frame_ring_buffer.h"
#include "photon_deterministic/deterministic_interfaces.h"
#include "photon_deterministic/deterministic_session.h"
#include "photon_deterministic/deterministic_session_config.h"
#include "photon_deterministic/log.h"
#include "photon_deterministic/native.h"

#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

using namespace Photon::Deterministic;

namespace {

// --- platform services --------------------------------------------------------------

class TestAllocator final : public Native::Allocator {
  public:
    void Free(void* ptr) override {
        live_.erase(ptr);
        std::free(ptr);
    }
    void* Alloc(int count) override { return Alloc(count, 8); }
    void* Alloc(int count, int) override {
        void* block = std::malloc(static_cast<std::size_t>(count > 0 ? count : 1));
        live_.insert(block);
        return block;
    }
    void Clear(void* dest, int count) override {
        std::memset(dest, 0, static_cast<std::size_t>(count));
    }
    std::size_t Live() const { return live_.size(); }

  private:
    std::set<void*> live_;
};

class TestUtility final : public Native::Utility {
  public:
    Native::ObjectHandle HandleAcquire(void*) override { throw std::logic_error("unused"); }
    void HandleRelease(Native::ObjectHandle) override {}
    void* GetObjectForHandle(Native::ObjectHandle) override { return nullptr; }
    void Copy(void* dest, const void* src, int count) override {
        std::memcpy(dest, src, static_cast<std::size_t>(count));
    }
    void Clear(void* dest, int count) override {
        std::memset(dest, 0, static_cast<std::size_t>(count));
    }
    void Move(void* dest, const void* src, int count) override {
        std::memmove(dest, src, static_cast<std::size_t>(count));
    }
    int Compare(const void* a, const void* b, int count) override {
        return std::memcmp(a, b, static_cast<std::size_t>(count));
    }
};

// --- a trivial game -------------------------------------------------------------------

// The whole "game state": a running sum of every player's input value.
class SumFrame final : public DeterministicFrame {
  public:
    std::int64_t Sum = 0;

    std::string DumpFrame(int) override { return std::to_string(Sum); }
    std::uint64_t CalculateChecksum() override { return static_cast<std::uint64_t>(Sum); }
    void Free() override {}
    std::vector<std::uint8_t> Serialize(DeterministicFrameSerializeMode) override {
        std::vector<std::uint8_t> out(sizeof(Sum));
        std::memcpy(out.data(), &Sum, sizeof(Sum));
        return out;
    }
    void Deserialize(std::span<const std::uint8_t> data) override {
        std::memcpy(&Sum, data.data(), sizeof(Sum));
    }
    void Copy(const DeterministicFrame& frame) override {
        Sum = static_cast<const SumFrame&>(frame).Sum;
    }
};

class AddCommand final : public DeterministicCommand {
  public:
    std::int32_t Amount = 0;
    void Serialize(BitStream& stream) override { stream.Serialize(Amount, 32); }

  protected:
    std::shared_ptr<DeterministicCommand> CreateInstance() const override {
        return std::make_shared<AddCommand>();
    }
};

constexpr int InputSize = 4;

class SumGame final : public IDeterministicGame {
  public:
    DeterministicSession* session = nullptr;
    std::vector<int> verifiedFrames; // ticks simulated on the verified state, in order
    std::map<int, std::uint64_t> checksums;
    int confirmedInputs = 0;
    int commandsSeen = 0;
    int begins = 0, ends = 0, updatesDone = 0;
    bool destroyed = false;

    static std::int32_t InputValue(int frame, int player) { return frame * 10 + player; }

    DeterministicSession* Session() const override { return session; }
    int GetInputSerializedFixedSize() override { return InputSize; }
    int GetInputInMemorySize() override { return InputSize; }
    void DeserializeInputInto(int, const ByteArray& data, std::uint8_t* buffer) override {
        std::memset(buffer, 0, InputSize);
        if (data && data->size() >= static_cast<std::size_t>(InputSize)) {
            std::memcpy(buffer, data->data(), InputSize);
        }
    }
    DeterministicFrameInputTemp OnLocalInput(int frame, int player) override {
        const std::int32_t value = InputValue(frame, player);
        auto data = MakeByteArray(std::vector<std::uint8_t>(InputSize));
        std::memcpy(data->data(), &value, InputSize);
        return DeterministicFrameInputTemp::Predicted(frame, player, data, InputSize,
                                                      DeterministicInputFlags::Repeatable);
    }
    std::shared_ptr<IDisposable> CreateFrameContext() override { return nullptr; }
    std::shared_ptr<DeterministicFrame> CreateFrame(const std::shared_ptr<IDisposable>&) override {
        return std::make_shared<SumFrame>();
    }
    std::shared_ptr<DeterministicFrame> GetVerifiedFrame(int) override { return nullptr; }
    ByteArray GetExtraErrorFrameDumpData(DeterministicFrame&) override { return nullptr; }
    void OnDestroy() override { destroyed = true; }
    void AssignSession(DeterministicSession* s) override { session = s; }
    void OnGameStart(DeterministicFrame&) override {}
    void OnGameResync() override {}
    void OnSimulate(DeterministicFrame& state) override {
        auto& frame = static_cast<SumFrame&>(state);
        for (int player = 0; player < frame.Input->PlayerCount(); ++player) {
            std::int32_t value = 0;
            std::memcpy(&value, frame.GetRawInput(player), InputSize);
            frame.Sum += value;
            if (auto cmd = std::dynamic_pointer_cast<AddCommand>(frame.GetPlayerCommand(player))) {
                frame.Sum += cmd->Amount;
                if (frame.IsVerified()) {
                    ++commandsSeen;
                }
            }
        }
        if (frame.IsVerified()) {
            verifiedFrames.push_back(frame.Number);
        }
    }
    void OnSimulateFinished(DeterministicFrame&) override {}
    void OnUpdateDone() override { ++updatesDone; }
    void OnChecksumError(const DeterministicTickChecksumError&,
                         std::span<const std::shared_ptr<DeterministicFrame>>) override {}
    void OnChecksumErrorFrameDump(int, int, const std::shared_ptr<DeterministicSessionConfig>&,
                                  const ByteArray&, const ByteArray&, const ByteArray&) override {}
    void OnInputConfirmed(const DeterministicFrameInputTemp&) override { ++confirmedInputs; }
    void OnChecksumComputed(int frame, std::uint64_t checksum) override {
        checksums[frame] = checksum;
    }
    void OnSimulationEnd() override { ++ends; }
    void OnSimulationBegin() override { ++begins; }
    void OnPluginDisconnect(const std::string&) override {}
};

struct Platform {
    TestAllocator allocator;
    TestUtility utility;
    Platform() { Native::Utils = &utility; }
    ~Platform() { Native::Utils = nullptr; }
};

DeterministicSessionArgs LocalArgs(SumGame& game, Platform& platform, int inputDelay) {
    auto config = std::make_shared<DeterministicSessionConfig>();
    config->UpdateFPS = 60;
    config->RollbackWindow = 8;
    config->ChecksumInterval = 10;
    config->InputDelayMin = inputDelay;
    DeterministicSessionArgs args;
    args.Mode = DeterministicGameMode::Local;
    args.SessionConfig = config;
    args.Game = &game;
    args.PlatformInfo.Allocator = &platform.allocator;
    return args;
}

} // namespace

TEST_CASE("DeterministicSession: local session joins, starts and steps the game") {
    Platform platform;
    SumGame game;
    constexpr int InputDelay = 2;
    DeterministicSession session(LocalArgs(game, platform, InputDelay));
    CHECK(game.session == &session);
    CHECK(session.State() == DeterministicSessionState::Idle);

    session.Join("local", 2);
    session.Update(0.0); // the local "server" handles the join request
    CHECK(session.State() == DeterministicSessionState::Idle);
    session.Update(0.0); // Joined + SimulationStart are delivered
    CHECK(session.State() == DeterministicSessionState::Running);
    CHECK(session.PlayerCount() == 2);
    CHECK(session.LocalPlayerIndices() == std::vector<int>({0, 1}));
    CHECK(session.IsLocalPlayer(1));
    // The simulation starts at RollbackWindow - 1; the input-delay ticks are confirmed empty.
    const int first = session.RollbackWindow() - 1;
    CHECK(session.FrameVerified()->Number == first);
    CHECK(game.confirmedInputs == InputDelay * 2);

    constexpr int Ticks = 40;
    const double dt = 1.0 / 60.0;
    for (int i = 0; i < Ticks; ++i) {
        session.Update(dt);
    }

    // Offline input is confirmed when polled, so every tick is verified immediately.
    const int last = first + Ticks;
    CHECK(session.FrameVerified()->Number == last);
    CHECK(session.FramePredicted()->Number == last);
    CHECK(static_cast<int>(game.verifiedFrames.size()) == Ticks);
    CHECK(game.verifiedFrames.front() == first + 1);
    CHECK(game.verifiedFrames.back() == last);
    CHECK(game.begins == game.ends);
    CHECK(game.updatesDone >= Ticks);

    // Ticks within the input delay ran with empty input; tick f later ran with the input
    // polled for it, 10f + player for each of the two players.
    std::int64_t expected = 0;
    for (int f = first + 1 + InputDelay; f <= last; ++f) {
        expected += SumGame::InputValue(f, 0) + SumGame::InputValue(f, 1);
    }
    const auto& verified = static_cast<const SumFrame&>(*session.FrameVerified());
    CHECK(verified.Sum == expected);
    CHECK(session.Stats().Frame() == last);
    CHECK(session.Stats().Offset() == InputDelay);

    // Checksums every 10 verified ticks.
    CHECK(game.checksums.size() == 4u);
    CHECK(game.checksums.count(10) == 1u && game.checksums.count(40) == 1u);

    // A command for player 1 arrives InputDelay ticks later, exactly once.
    auto prototype = std::make_shared<AddCommand>();
    session.CommandSerializer().RegisterFactory(prototype);
    AddCommand command;
    command.Amount = 1000;
    session.SendCommand(1, command);
    const std::int64_t before = static_cast<const SumFrame&>(*session.FrameVerified()).Sum;
    for (int i = 0; i < InputDelay + 2; ++i) {
        session.Update(dt);
    }
    CHECK(game.commandsSeen == 1);
    std::int64_t expectedAfter = before;
    for (int f = last + 1; f <= last + InputDelay + 2; ++f) {
        expectedAfter += SumGame::InputValue(f, 0) + SumGame::InputValue(f, 1);
    }
    CHECK(static_cast<const SumFrame&>(*session.FrameVerified()).Sum == expectedAfter + 1000);

    // Without elapsed time nothing advances.
    const int frozen = session.FrameVerified()->Number;
    session.Update(0.0);
    CHECK(session.FrameVerified()->Number == frozen);

    session.Destroy();
    CHECK(game.destroyed);
    CHECK(session.State() == DeterministicSessionState::Destroyed);
    CHECK(platform.allocator.Live() == 0u);
}

TEST_CASE("DeterministicSession: Resync restarts from serialized frame data") {
    Platform platform;
    SumGame game;
    DeterministicSession session(LocalArgs(game, platform, 0));
    session.Join("local", 1);
    session.Update(0.0);
    session.Update(0.0);
    for (int i = 0; i < 5; ++i) {
        session.Update(1.0 / 60.0);
    }
    SumFrame snapshot;
    snapshot.Sum = 12345;
    const auto data = snapshot.Serialize(DeterministicFrameSerializeMode::Serialize);
    session.Resync(data, 100);
    CHECK(session.FrameVerified()->Number == 100);
    CHECK(static_cast<const SumFrame&>(*session.FramePredicted()).Sum == 12345);
    session.Update(1.0 / 60.0);
    // With resetInputFrame = false the input counter keeps going, so the next tick (101)
    // only verifies once local input reaches it; the state itself must not regress.
    CHECK(session.FrameVerified()->Number >= 100);
    session.Destroy();
    CHECK(platform.allocator.Live() == 0u);
}

TEST_CASE("DeterministicSessionConfig: byte array round trip") {
    auto config = std::make_shared<DeterministicSessionConfig>();
    config->PlayerCount = 4;
    config->UpdateFPS = 30;
    config->RollbackWindow = 12;
    config->InputDelayMin = 3;
    config->LockstepSimulation = true;
    config->InputFixedSizeEnabled = true;
    config->InputFixedSize = 17;
    const auto bytes = DeterministicSessionConfig::ToByteArray(config);
    const auto copy = DeterministicSessionConfig::FromByteArray(bytes);
    CHECK(copy != nullptr);
    CHECK(copy->PlayerCount == 4);
    CHECK(copy->UpdateFPS == 30);
    CHECK(copy->RollbackWindow == 12);
    CHECK(copy->InputDelayMin == 3);
    CHECK(copy->LockstepSimulation);
    CHECK(copy->InputFixedSize == 17);
    CHECK(copy->TimeScalePingMax == 300);
    CHECK(DeterministicSessionConfig::FromByteArray(
              DeterministicSessionConfig::ToByteArray(nullptr)) == nullptr);
}

TEST_CASE("DeterministicCommandSerializer: pack and read back") {
    DeterministicCommandSerializer serializer;
    serializer.RegisterFactory(std::make_shared<AddCommand>());
    AddCommand cmd;
    cmd.Amount = -77;
    BitStream& out = serializer.CommandSerializerStreamWrite;
    out.Reset();
    out.SetWriting(true);
    CHECK(serializer.PackNext(out, cmd));
    std::vector<std::uint8_t> bytes = out.ToArray();
    BitStream in(bytes);
    std::shared_ptr<DeterministicCommand> read;
    CHECK(serializer.ReadNext(in, read));
    auto typed = std::dynamic_pointer_cast<AddCommand>(read);
    CHECK(typed && typed->Amount == -77);
    std::shared_ptr<DeterministicCommand> none;
    CHECK(!serializer.ReadNext(in, none) && none == nullptr);
}

TEST_CASE("DeterministicCommandPool: recycles released commands") {
    DeterministicCommandPool<AddCommand> pool;
    auto first = pool.Acquire();
    CHECK(first->Pool == &pool && !first->Pooled);
    first->Dispose();
    CHECK(first->Pooled);
    CHECK(!pool.Release(first)); // already pooled
    auto again = pool.Acquire();
    CHECK(again == first);
}

TEST_CASE("DeterministicFrameRingBuffer: push, peek and find") {
    DeterministicFrameRingBuffer buffer(3);
    auto create = [] { return std::make_shared<SumFrame>(); };
    for (int n = 1; n <= 5; ++n) {
        SumFrame f;
        f.Number = n * 10;
        f.Sum = n;
        buffer.PushBack(f, create);
    }
    CHECK(buffer.Count() == 3);
    CHECK(buffer.PeekBack(0)->Number == 50);
    CHECK(buffer.PeekBack(2)->Number == 30);
    CHECK(buffer.Find(40, DeterministicFrameSnapshotBufferFindMode::Equal)->Number == 40);
    CHECK(buffer.Find(45, DeterministicFrameSnapshotBufferFindMode::Equal) == nullptr);
    CHECK(
        buffer.Find(45, DeterministicFrameSnapshotBufferFindMode::ClosestLessThanOrEqual)->Number ==
        40);
    CHECK(buffer.Find(100, DeterministicFrameSnapshotBufferFindMode::Closest)->Number == 50);
    int window = 0, sampling = 0;
    CHECK(
        DeterministicFrameRingBuffer::TryGetCommonSamplingPattern(10, 4, 20, 2, window, sampling));
    CHECK(window == 20 && sampling == 2);
    CHECK(DeterministicFrameRingBuffer::GetSize(20, 2) == 11);
    buffer.Clear();
    CHECK(buffer.Count() == 0);
}

TEST_CASE("RingBuffer: overwrite and order") {
    RingBuffer<int> ring(3, true);
    for (int i = 1; i <= 5; ++i) {
        ring.Push(i);
    }
    CHECK(ring.IsFull() && ring[0] == 3 && ring[2] == 5);
    CHECK(ring.Pop() == 3);
    CHECK(ring.Count() == 2);
    RingBuffer<int> strict(1, false);
    strict.Push(1);
    bool threw = false;
    try {
        strict.Push(2);
    } catch (const std::logic_error&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("Log: composite formatting and callbacks") {
    CHECK(Log::FormatComposite("{0} + {1} = {{{2}}}", {"1", "2", "3"}) == "1 + 2 = {3}");
    std::string last;
    Log::Init(nullptr, nullptr, [&](const std::string& m) { last = m; }, nullptr);
    Log::Error("tick {0}", 42);
    CHECK(last == "tick 42");
    Log::Init(nullptr, nullptr, nullptr, nullptr);
}
