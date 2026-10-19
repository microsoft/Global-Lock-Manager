#include <gtest/gtest.h>

#include <common/typedefs.h>

using namespace common;

TEST(ObjIds, ObjIdsAssertions) {
    /* uint64_t objId = 1u;
    for (; objId < 0x10000u; objId = objId << 1) {
        EXPECT_TRUE(isFile(getFileId(objId)));
    }
    EXPECT_TRUE(!isFile(getFileId(objId)));
    EXPECT_TRUE(isPage(getFileId(objId)));

    for (; objId < 0x1000000000000u; objId = objId << 1) {
        EXPECT_TRUE(isPage(objId));
    }
    EXPECT_TRUE(!isFile(objId));
    EXPECT_TRUE(!isPage(objId));
    EXPECT_TRUE(isRecord(objId));

    auto pageId = getPageId(0u /* file *, 0u /* pageIdx *);
    for (auto recordIdx = 0u; recordIdx < 0xFFFu; recordIdx++) {
        auto recordId = getRecordId(pageId, recordIdx);
        EXPECT_TRUE(!isFile(recordId));
        EXPECT_TRUE(!isPage(recordId));
        EXPECT_TRUE(isRecord(recordId));
    } */
}
