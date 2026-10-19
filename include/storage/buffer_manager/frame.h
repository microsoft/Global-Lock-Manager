#pragma once

#include <atomic>
#include <mutex>

#include <common/typedefs.h>

using namespace common;

namespace storage {

constexpr uint64_t INVALID_PAGE = UINT64_MAX;

/**
 * A frame is a page held in-memory.
 */
struct Frame {
public:
    Frame() { reset(); }

    void reset();

    bool canBeEvicted() {
        // We can evict a page if it is not dirty.
        return numUncommitedUpdates == 0 && pinCount == 0;
    }

    std::mutex mtx; /** if pageId == INVALID_PAGE, the frame is empty. */
    page_id_t pageId; /** if pageId == INVALID_PAGE, the frame is empty. */
    uint8_t* buffer; /** the buffer storing the page in-memory. */
    std::atomic<bool> needFetchFromRemote; /** set if the page has to be fetched. */
    std::atomic<uint32_t> pinCount; /** the number of pins by transactions. */
    bool refBit; /** set if the page has been recently accessed. */
    std::atomic<uint32_t> numUncommitedUpdates; /** tracks the number of updated or
                                               inserted records in the page. */
};

} // namespace storage
