#include <storage/buffer_manager/frame.h>

namespace storage {

void Frame::reset() {
    pageId = INVALID_PAGE;
    pinCount = 0u;
    numUncommitedUpdates = 0u;
    refBit = false;
    needFetchFromRemote = true;
}

} // namespace storage
