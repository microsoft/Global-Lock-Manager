#pragma once

#include <memory>
#include <vector>

#include <common/config.h>
#include <common/map.h>
#include <storage/buffer_manager/buffer_manager_stats.h>
#include <storage/buffer_manager/frame.h>

using namespace common;

namespace storage {

/**
 * The buffer manager (BM) has a fixed-size vector of frames, each of which
 * holds a page that was fetched with a GetPage request.
 *
 * A BM's GetPage checks if the page is cached and if not, the BM fetches the
 * page from a Page Server or another compute server. The BM either holds the
 * newly fetched page in an empty frame by possibly freeing a frame i.e., ev-
 * icting a page.
 * 
 * The BM uses the clock replacement algorithm. The frames are logically in a
 * circular list and each frame has a refBit flag indicating if the frame was
 * recently used that is set to true each time the page it holds is accessed.
 * The clock advances from 0 to the numFrames - 1 in a clockwise fashion.
 */
class BufferManager {
public:
    BufferManager() : BufferManager(PAGE_SIZE, BUFFER_POOL_SIZE) {}
    BufferManager(uint64_t pageSize, uint64_t bufferPoolSize);

    Frame* pinPageAndGetFrame(page_id_t pageId);
    // Warning: flagPinnedPageAsDirty & unpinPage are expected to be called on
    //          pinned pages only. The behaviour is otherwise undefined.
    void unpinPage(const page_id_t pageId) {
        framePool[pageIdToFrameIdxMap.get(pageId, UINT64_MAX)]->pinCount--;
    }
    // Warning: flagPinnedPageAsDirty & setChangesAsCommitted should happen before unpinPage.
    void flagPinnedPageAsDirty(page_id_t pageId);
    void setChangesAsCommitted(page_id_t pageId);

    BufferManagerStats& getBufferManagerStats() { return bufferManagerStats; }
    void resetBufferManagerStats() { bufferManagerStats.reset(); }

public: /* for testing */
    frame_idx_t getClockHand() { return clockHand.load(); }
    frame_idx_t getFrameIdx(page_id_t pageId) {
        return pageIdToFrameIdxMap.get(pageId, UINT64_MAX);
    }

private:
    uint64_t numFrames;
    std::atomic<frame_idx_t> clockHand;
    std::vector<std::unique_ptr<Frame>> framePool;
    Map<page_id_t, frame_idx_t> pageIdToFrameIdxMap;
    BufferManagerStats bufferManagerStats;
};

} // namespace storage
