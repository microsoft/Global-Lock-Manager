#include <gtest/gtest.h>

#include <storage/buffer_manager/buffer_manager.h>

using namespace storage;

TEST(BufferManagerTests, test1) {
    BufferManager bufferManager;

    auto frame = bufferManager.pinPageAndGetFrame(0 /* pageId */);

}
