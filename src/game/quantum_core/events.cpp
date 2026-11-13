#include "events.h"

namespace Quantum {

std::string EventKey::ToString() const {
    return "[EventKey tick " + std::to_string(Tick) + ", id " + std::to_string(Id) +
           ", hash " + std::to_string(Hash) + "]";
}

EventBase* EventList::PopHead() {
    EventBase* head = _head;
    Remove(head);
    return head;
}

void EventList::AddLast(EventBase* node) {
    if (_head != nullptr) {
        AddBefore(node, _head);
    } else {
        AddEmpty(node);
    }
}

void EventList::Remove(EventBase* node) {
    if (_count == 1) {
        _head = nullptr;
    } else {
        node->Prev->Next = node->Next;
        node->Next->Prev = node->Prev;
        if (_head == node) {
            _head = node->Next;
        }
    }
    node->Next = nullptr;
    node->Prev = nullptr;
    node->List = nullptr;
    --_count;
    ++_version;
}

void EventList::AddEmpty(EventBase* node) {
    node->Next = node;
    node->Prev = node;
    node->List = this;
    _head = node;
    _count = 1;
    ++_version;
}

void EventList::AddBefore(EventBase* node, EventBase* before) {
    node->Next = before;
    node->Prev = before->Prev;
    before->Prev->Next = node;
    before->Prev = node;
    node->List = this;
    ++_count;
    ++_version;
}

} // namespace Quantum
