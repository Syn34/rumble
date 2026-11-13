#include "quantum_core/mailbox_set8.h"

#include <algorithm>

namespace Quantum {

// 0x7d307b8
MailboxSet8::MailboxSet8(int capacity)
    : set_(static_cast<std::size_t>(std::max(capacity, 0)), 0), capacity_(capacity) {}

// 0x7d30834
void MailboxSet8::SetCapacity(int capacity) {
    if (capacity_ == capacity) {
        return;
    }
    capacity_ = capacity;
    set_.assign(static_cast<std::size_t>(std::max(capacity, 0)), 0);
}

// 0x7d30804
void MailboxSet8::Dispose() {
    set_.clear();
    set_.shrink_to_fit();
}

// 0x7d308a4
void MailboxSet8::StartSearch() {
    if (key_ == 0xFF) {
        std::fill(set_.begin(), set_.end(), std::uint8_t{0});
        key_ = 1;
    } else {
        ++key_;
    }
}

// 0x7d30928
bool MailboxSet8::AlreadyChecked(int index) {
    std::uint8_t& stamp = set_.at(static_cast<std::size_t>(index));
    if (stamp == key_) {
        return true;
    }
    stamp = key_;
    return false;
}

// 0x7d3094c
bool MailboxSet8::ShouldCheck(int index) {
    return !AlreadyChecked(index);
}

} // namespace Quantum
