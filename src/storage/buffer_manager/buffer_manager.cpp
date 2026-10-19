#include <storage/buffer_manager/buffer_manager.h>

// TODO: add update buffer manager stats.

namespace storage {

BufferManager::BufferManager(uint64_t pageSize, uint64_t bufferPoolSize) :
    numFrames{bufferPoolSize / pageSize}, clockHand{0},
    pageIdToFrameIdxMap{numFrames} {
    for (auto i = 0u; i < numFrames; i++) {
        framePool.emplace_back(std::make_unique<Frame>());
        // In order to avoid isEmpty checks, the first turn around the clock
        // evicts "unnecessarily".
        framePool[i]->refBit = true;
    }
}

void BufferManager::flagPinnedPageAsDirty(page_id_t pageId) {
    auto frame = framePool[pageIdToFrameIdxMap.get(pageId, UINT64_MAX)].get();
    frame->numUncommitedUpdates++;
}

void BufferManager::setChangesAsCommitted(page_id_t pageId) {
    auto frame = framePool[pageIdToFrameIdxMap.get(pageId, UINT64_MAX)].get();
    frame->numUncommitedUpdates--;
}

Frame* BufferManager::pinPageAndGetFrame(const page_id_t pageId) {
    auto frameIdxWithLock = pageIdToFrameIdxMap.getAndLock(pageId, UINT64_MAX);
    if (frameIdxWithLock.value != UINT64_MAX) { // is cached.
        framePool[frameIdxWithLock.value]->pinCount++;
        return framePool[frameIdxWithLock.value].get();
    } else { // not cached.
        auto currFrameIdx = clockHand.load() % numFrames;
        Frame* frame = nullptr;
        while (true) {
            std::unique_lock<std::mutex> frameLock(framePool[currFrameIdx]->mtx,
                std::try_to_lock);
            if (frameLock.owns_lock()) {
                auto canBeEvicted = framePool[currFrameIdx]->canBeEvicted();
                if (canBeEvicted) {
                    frame = framePool[currFrameIdx].get();
                    if (!frame->refBit) {
                        // Give it a second chance by not evicting yet.
                        frame->refBit = true;
                    } else {            
                        // TODO: eviction requires releasing node lock.
                        clockHand++;
                        auto prevPageId = frame->pageId;
                        if (prevPageId != INVALID_PAGE) {
                            // Frame empty on initial round around clock.
                            pageIdToFrameIdxMap.remove(prevPageId);
                        }
                        frame->reset(); /* eviction */
                        frame->pageId = pageId;
                        frame->pinCount++;
                        // lock is held in the frameIdxWithLock struct instance.
                        pageIdToFrameIdxMap.put(pageId, currFrameIdx,
                            std::move(frameIdxWithLock.lock));
                        return frame;
                    }
                }
            }
            currFrameIdx = clockHand++ % numFrames;
        }
    }
}

} // namespace storage
