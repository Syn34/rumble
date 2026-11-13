// events.h - Quantum.EventBase, Quantum.EventList, Quantum.EventKey, Quantum.EventFlags:
// simulation events raised for the presentation side.
//
// Events are objects of game-defined subclasses of EventBase, pooled per event id by the
// frame context (FrameContext::AcquireEvent / ReleaseEvent). Raised events sit in an
// intrusive circular doubly linked list (EventList) in raising order until the session
// dispatches them. EventKey identifies an event across predicted and verified frames.
#pragma once

#include <cstdint>
#include <string>

namespace Photon::Deterministic {
class IDeterministicGame;
}

namespace Quantum {

enum class EventFlags : int {
    None = 0,
    // Raised again when the frame is verified rather than on prediction.
    Synced = 1,
    Server = 2,
    Client = 4,
};

struct EventKey {
    int Tick = 0;
    int Id = 0;
    int Hash = 0;

    EventKey() = default;
    // 0x7cf1db0
    EventKey(int tick, int id, int hash) : Tick(tick), Id(id), Hash(hash) {}

    // 0x7cf2068 (Equals(EventKey)); 0x7cf2018 is the boxed Equals(object).
    bool Equals(const EventKey& other) const {
        return Tick == other.Tick && Id == other.Id && Hash == other.Hash;
    }
    friend bool operator==(const EventKey& a, const EventKey& b) { return a.Equals(b); }

    // 0x7cf204c - ((Tick * 33) ^ Id) * 33 ^ Hash.
    int GetHashCode() const {
        std::uint32_t hash = static_cast<std::uint32_t>(Tick);
        hash = (hash * 33u) ^ static_cast<std::uint32_t>(Id);
        hash = (hash * 33u) ^ static_cast<std::uint32_t>(Hash);
        return static_cast<int>(hash);
    }

    // 0x7cf1dbc - Debug text (wording is this project's own).
    std::string ToString() const;
};

class EventList;

class EventBase {
  public:
    // 0x7cf2330
    EventBase(int id, EventFlags flags) : Id(id), _flags(flags) {}
    virtual ~EventBase() = default;

    EventBase(const EventBase&) = delete;
    EventBase& operator=(const EventBase&) = delete;

    // 0x7cf2310, 0x7cf2318
    Photon::Deterministic::IDeterministicGame* get_Game() const { return _game; }
    void set_Game(Photon::Deterministic::IDeterministicGame* value) { _game = value; }
    // 0x7cf2320
    bool get_Synced() const { return (static_cast<int>(_flags) & 1) != 0; }
    EventFlags Flags() const { return _flags; }

    // 0x7cf232c - Called when the event goes back to its pool; nothing to do by default.
    virtual void OnReleased() {}

    // List links (owned by the EventList the event is in, null otherwise).
    EventBase* Prev = nullptr;
    EventBase* Next = nullptr;
    EventList* List = nullptr;
    int Tick = 0;
    const int Id;

  private:
    Photon::Deterministic::IDeterministicGame* _game = nullptr;
    EventFlags _flags;
};

class EventList {
  public:
    // 0x7cf2308
    EventList() = default;

    EventList(const EventList&) = delete;
    EventList& operator=(const EventList&) = delete;

    // 0x7cf2100
    int get_Count() const { return _count; }
    int get_Version() const { return _version; }
    EventBase* Head() const { return _head; }

    // 0x7cf2108 - Unlinks and returns the oldest event (null-unsafe: the list must not be
    // empty).
    EventBase* PopHead();
    // 0x7cf21f8
    void AddLast(EventBase* node);
    // 0x7cf2124 - Unlinks `node`, which must be in this list.
    void Remove(EventBase* node);

  private:
    // 0x7cf2208 - First node of an empty list.
    void AddEmpty(EventBase* node);
    // 0x7cf2278 - Links `node` in front of `before` (at the tail when `before` is head).
    void AddBefore(EventBase* node, EventBase* before);

    int _count = 0;
    int _version = 0;
    EventBase* _head = nullptr;
};

} // namespace Quantum
