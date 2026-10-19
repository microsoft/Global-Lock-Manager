#include <gtest/gtest.h>

#include <atomic>
#include <storage/lock_manager/obj_locks.h>

using namespace storage;

void ASSERT_TXS_BLOCKED(NodeLock& nodeLock, bool expectedIsBlocked,
    uint32_t expectedNumRegisteredTxs) {
    EXPECT_EQ(nodeLock.areTxsBlocked(), expectedIsBlocked);
    node_lock_state_t nodeLockState;
    nodeLockState.bits = nodeLock.state.load();
    EXPECT_EQ(nodeLockState.fields.numRegisteredTxs, expectedNumRegisteredTxs);
}

void ASSERT_NUM_REGISTERED_TXS(NodeLock& nodeLock,
    uint32_t expectedNumRegisteredTxs) {
    node_lock_state_t nodeLockState;
    nodeLockState.bits = nodeLock.state.load();
    EXPECT_EQ(nodeLockState.fields.numRegisteredTxs, expectedNumRegisteredTxs);
}

TEST(RegisteredTxsStateTests, areTxsBlocked) {
    NodeLock nodeLock;

    for (auto i = 0u; i < 16; i++) {
        ASSERT_TXS_BLOCKED(nodeLock, false /* expectedIsBlocked */,
            i + 1 /* expectedNumRegisteredTxs */);
    }

    nodeLock.blockTxs();
    for (auto i = 0u; i < 16; i++) {
        ASSERT_TXS_BLOCKED(nodeLock, true /* expectedIsBlocked */,
            16 /* expectedNumRegisteredTxs */);
    }

    for (auto i = 0u; i < 8; i++) {
        nodeLock.unregisterTx();
        auto expectedNumRegisteredTxs = 16 - 1 - i;
        ASSERT_TXS_BLOCKED(nodeLock, true /* expectedIsBlocked */,
            expectedNumRegisteredTxs);
        ASSERT_NUM_REGISTERED_TXS(nodeLock, expectedNumRegisteredTxs);
    }

    nodeLock.unblockTxs();
    for (auto i = 0u; i < 8; i++) {
        ASSERT_TXS_BLOCKED(nodeLock, false /* expectedIsBlocked */,
            8 + 1 + i /* expectedNumRegisteredTxs */);
    }
}
