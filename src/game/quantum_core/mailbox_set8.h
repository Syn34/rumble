// mailbox_set8.h - Quantum.MailboxSet8 (quantum.core.dll).
//
// "Already visited?" marks for spatial queries that may meet the same item in several
// grid cells. Each item has a byte stamp; a search bumps the current key, and an item
// counts as visited when its stamp equals the key. After 255 searches the stamps are
// cleared and the key restarts at 1, so clearing costs nothing for most searches.
//
// The original allocates the stamp buffer from a native allocator passed to each call;
// here the buffer is owned by the object, so the allocator parameters are gone.
#pragma once

#include <cstdint>
#include <vector>

namespace Quantum {

class MailboxSet8 {
public:
    // 0x7d307b8 - `capacity` stamps, all clear; key 0.
    explicit MailboxSet8(int capacity);

    // 0x7d307b0
    int Capacity() const { return capacity_; }

    // 0x7d30834 SetCapactiy (sic) - reallocates (cleared) when the size changes. The key
    // is kept.
    void SetCapacity(int capacity);

    // 0x7d30804 - releases the stamp buffer.
    void Dispose();

    // 0x7d308a4 - begins a new search.
    void StartSearch();

    // 0x7d30928 - true if `index` was already met in this search; marks it otherwise.
    bool AlreadyChecked(int index);

    // 0x7d3094c - the opposite of AlreadyChecked (and marks the same way).
    bool ShouldCheck(int index);

private:
    std::uint8_t key_ = 0;          // _key
    std::vector<std::uint8_t> set_; // _set
    int capacity_ = 0;              // _capacity
};

} // namespace Quantum
