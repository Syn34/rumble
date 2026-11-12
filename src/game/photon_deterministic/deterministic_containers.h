// deterministic_containers.h - small generic helpers of the session runtime:
// QTuple<T0, T1>, NetworkQueue<T>, Dispatcher<T> and RingBuffer<T>.
//
// All four are generic types whose bodies are IL2CPP shared generic code (no address of
// their own in the method list); the addresses below are the instantiations that were
// studied.
#pragma once

#include "log.h"

#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Photon::Deterministic {

// Photon.Deterministic.QTuple<T0, T1> (0x65e5250 ctor, 0x65e5278 Deconstruct).
template <typename T0, typename T1> struct QTuple {
    T0 Item0{};
    T1 Item1{};

    QTuple() = default;
    QTuple(T0 item0, T1 item1) : Item0(std::move(item0)), Item1(std::move(item1)) {}

    void Deconstruct(T0& item0, T1& item1) const {
        item0 = Item0;
        item1 = Item1;
    }
};

// Photon.Deterministic.QTuple.Create (0x5118d5c)
template <typename T0, typename T1> QTuple<T0, T1> MakeQTuple(T0 item0, T1 item1) {
    return QTuple<T0, T1>(std::move(item0), std::move(item1));
}

// Photon.Deterministic.NetworkQueue<T>: a FIFO safe to push from a network thread
// (0x634b3b0 Push, 0x634b494 TryPop, 0x634b5d0 ctor).
template <typename T> class NetworkQueue {
  public:
    void Push(T ev) {
        std::lock_guard<std::mutex> lock(sync_);
        eventsQueue_.push_back(std::move(ev));
    }

    bool TryPop(T& ev) {
        std::lock_guard<std::mutex> lock(sync_);
        if (eventsQueue_.empty()) {
            return false;
        }
        ev = std::move(eventsQueue_.front());
        eventsQueue_.pop_front();
        return true;
    }

  private:
    std::mutex sync_;
    std::deque<T> eventsQueue_;
};

// Photon.Deterministic.Dispatcher<T>: routes messages (shared_ptr<T>) to the callback bound
// for their dynamic type (0x4986f60 Bind, 0x7417da0 DispatchNext, 0x7417e08 Dispatch,
// 0x7418014 ctor).
template <typename T> class Dispatcher {
  public:
    template <typename TMessage> void Bind(std::function<void(TMessage&)> callback) {
        callbacks_[typeid(TMessage)] = [callback = std::move(callback)](T& message) {
            callback(static_cast<TMessage&>(message));
        };
    }

    // Dispatches the oldest queued message, if any.
    void DispatchNext(std::deque<std::shared_ptr<T>>& queue) {
        if (queue.empty()) {
            return;
        }
        std::shared_ptr<T> message = std::move(queue.front());
        queue.pop_front();
        Dispatch(*message);
    }

    // Unbound message types are logged as errors and dropped.
    void Dispatch(T& obj) {
        auto it = callbacks_.find(typeid(obj));
        if (it == callbacks_.end()) {
            Log::Error("Could not dispatch a callback for message type {0}", typeid(obj).name());
            return;
        }
        it->second(obj);
    }

  private:
    std::unordered_map<std::type_index, std::function<void(T&)>> callbacks_;
};

// Photon.Deterministic.RingBuffer<T>: fixed-capacity FIFO; when full, Push either drops the
// oldest item (overwrite mode) or throws. Bodies studied in the RingBuffer<NetworkSend>
// instantiation at 0x68adb4c..0x68ade94.
template <typename T> class RingBuffer {
  public:
    // ctor(int size, bool overwrite)
    RingBuffer(int size, bool overwrite)
        : array_(static_cast<std::size_t>(size)), overwrite_(overwrite) {}

    int Count() const { return count_; }
    bool IsFull() const { return count_ == static_cast<int>(array_.size()); }
    bool IsEmpty() const { return count_ == 0; }

    // get_Item / set_Item: index 0 is the oldest item. Throws std::out_of_range
    // (IndexOutOfRangeException) outside [0, Count).
    const T& operator[](int index) const { return array_[Slot(index)]; }
    T& operator[](int index) { return array_[Slot(index)]; }

    void Push(T item) {
        if (IsFull()) {
            if (!overwrite_) {
                throw std::logic_error("RingBuffer is full"); // InvalidOperationException
            }
            Pop();
        }
        ++version_;
        array_[static_cast<std::size_t>(head_)] = std::move(item);
        head_ = (head_ + 1) % static_cast<int>(array_.size());
        ++count_;
    }

    // Removes and returns the oldest item; throws std::logic_error when empty.
    T Pop() {
        if (count_ == 0) {
            throw std::logic_error("RingBuffer is empty"); // InvalidOperationException
        }
        ++version_;
        T item = std::move(array_[static_cast<std::size_t>(tail_)]);
        array_[static_cast<std::size_t>(tail_)] = T{};
        tail_ = (tail_ + 1) % static_cast<int>(array_.size());
        --count_;
        return item;
    }

    // GetEnumerator: oldest to newest.
    template <typename F> void ForEach(F&& visit) const {
        for (int i = 0; i < count_; ++i) {
            visit((*this)[i]);
        }
    }

  private:
    std::size_t Slot(int index) const {
        if (index < 0 || index >= count_) {
            throw std::out_of_range("RingBuffer index out of range");
        }
        return static_cast<std::size_t>((tail_ + index) % static_cast<int>(array_.size()));
    }

    int head_ = 0;
    int tail_ = 0;
    int count_ = 0;
    int version_ = 0;
    std::vector<T> array_;
    bool overwrite_ = false;
};

} // namespace Photon::Deterministic
