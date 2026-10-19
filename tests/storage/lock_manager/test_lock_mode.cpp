#include <gtest/gtest.h>

#include <storage/lock_manager/locks/transaction_locks_queue_state.h>

using namespace storage;

void lockAndAssert(uint64_t& lockingStateAsInt, lock_mode_t lockMode,
    lock_mode_t expectedLockMode, uint16_t txCount1, uint16_t txCount2,
    uint16_t numWaiters, bool expectedGranted) {
    auto granted = getLock(lockingStateAsInt, lockMode);
    transaction_locks_queue_state_t lockingState;
    lockingState.bits = lockingStateAsInt;
    uint64_t state = lockingState.bits;
    lockingState.bits = state;
    EXPECT_EQ(expectedLockMode, lockingState.fields.lockModes);
    EXPECT_EQ(txCount1, lockingState.fields.txCount1);
    EXPECT_EQ(txCount2, lockingState.fields.txCount2);
    EXPECT_EQ(numWaiters, lockingState.fields.numWaiters);
    EXPECT_EQ(expectedGranted, granted);
}

void releaseAndAssert(uint64_t& lockingStateAsInt, lock_mode_t lockMode,
    lock_mode_t expectedLockMode, uint16_t txCount1, uint16_t txCount2,
    uint16_t numWaiters, bool expectedAwaken) {
    auto awaken = releaseLock(lockingStateAsInt, lockMode);
    transaction_locks_queue_state_t lockingState;
    lockingState.bits = lockingStateAsInt;
    uint64_t state = lockingState.bits;
    lockingState.bits = state;
    EXPECT_EQ(expectedLockMode, lockingState.fields.lockModes);
    EXPECT_EQ(txCount1, lockingState.fields.txCount1);
    EXPECT_EQ(txCount2, lockingState.fields.txCount2);
    EXPECT_EQ(numWaiters, lockingState.fields.numWaiters);
    EXPECT_EQ(expectedAwaken, awaken);
}

TEST(LockingState, Lock_IS_IX_and_Unlock) {
    uint64_t lockingStateAsInt = 0u;

    lockAndAssert(lockingStateAsInt, IS /* lockMode */,
        IS /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, IS /* lockMode */,
        IS /* expectedLockMode */, 2u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, IX /* lockMode */,
        IX_IS /* expectedLockMode */, 2u /* txCount1 */, 1u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, IX /* lockMode */,
        IX_IS /* expectedLockMode */, 2u /* txCount1 */, 2u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    // ~~~~~~~~~~~~~~~~~~~~~~ relase IS then IX ~~~~~~~~~~~~~~~~~~~~~~

    releaseAndAssert(lockingStateAsInt, IS /* lockMode */,
        IX_IS /* expectedLockMode */, 1u /* txCount1 */, 2u /* txCount2 */,
        0u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsInt, IS /* lockMode */,
        IX /* expectedLockMode */, 2u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsInt, IX /* lockMode */,
        IX /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsInt, IX /* lockMode */,
        NONE /* expectedLockMode */, 0u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, false /* expectedAwaken */);
}

TEST(LockingState, Lock_IX_IS_and_Unlock) {
    uint64_t lockingStateAsInt = 0u;

    lockAndAssert(lockingStateAsInt, IX /* lockMode */,
        IX /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, IX /* lockMode */,
        IX /* expectedLockMode */, 2u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, IS /* lockMode */,
        IX_IS /* expectedLockMode */, 1u /* txCount1 */, 2u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, IX /* lockMode */,
        IX_IS /* expectedLockMode */, 1u /* txCount1 */, 3u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    // ~~~~~~~~~~~~~~~~~~~~~~ relase IX then IS ~~~~~~~~~~~~~~~~~~~~~~

    releaseAndAssert(lockingStateAsInt, IX /* lockMode */,
        IX_IS /* expectedLockMode */, 1u /* txCount1 */, 2u /* txCount2 */,
        0u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsInt, IX /* lockMode */,
        IX_IS /* expectedLockMode */, 1u /* txCount1 */, 1u /* txCount2 */,
        0u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsInt, IX /* lockMode */,
        IS /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsInt, IS /* lockMode */,
        NONE /* expectedLockMode */, 0u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, false /* expectedAwaken */);
}

TEST(LockingState, Lock_U_S_IS_then_Unlock) {
    uint64_t lockingStateAsInt = 0u;

    lockAndAssert(lockingStateAsInt, IS /* lockMode */,
        IS /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, IS /* lockMode */,
        IS /* expectedLockMode */, 2u /* txCount1 */, 0u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, S /* lockMode */,
        S_IS /* expectedLockMode */, 2u /* txCount1 */, 1u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, S /* lockMode */,
        S_IS /* expectedLockMode */, 2u /* txCount1 */, 2u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, S /* lockMode */,
        S_IS /* expectedLockMode */, 2u /* txCount1 */, 3u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, U /* lockMode */,
        U_S_IS /* expectedLockMode */, 2u /* txCount1 */, 3u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, S /* lockMode */,
        U_S_IS /* expectedLockMode */, 2u /* txCount1 */, 4u /* txCount2 */,
        0u /* numWaiters */, true /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, X /* lockMode */,
        U_S_IS /* expectedLockMode */, 2u /* txCount1 */, 4u /* txCount2 */,
        1u /* numWaiters */, false /* expectedGranted */);

    lockAndAssert(lockingStateAsInt, U /* lockMode */,
        U_S_IS /* expectedLockMode */, 2u /* txCount1 */, 4u /* txCount2 */,
        2u /* numWaiters */, false /* expectedGranted */);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    auto lockingStateAsIntCopy = lockingStateAsInt;

    // ~~~~~~~~~~~~~~~~~~~~~~~~ relase U then 1 IS ~~~~~~~~~~~~~~~~~~~~~~~~

    releaseAndAssert(lockingStateAsIntCopy, U /* lockMode */,
        S_IS /* expectedLockMode */, 2u /* txCount1 */, 4u /* txCount2 */,
          2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy, IS /* lockMode */,
        S_IS /* expectedLockMode */, 1u /* txCount1 */, 4u /* txCount2 */,
          2u /* numWaiters */, false /* expectedAwaken */);

    auto lockingStateAsIntCopy2 = lockingStateAsIntCopy;

    // ~~~~~~~~~~~~~~~~~~~~~~~~~ release IS then S ~~~~~~~~~~~~~~~~~~~~~~~~~

    releaseAndAssert(lockingStateAsIntCopy, IS /* lockMode */,
         S /* expectedLockMode */, 4u /* txCount1 */, 0u /* txCount2 */,
        2u /* numWaiters */, false /* expectedAwaken */);

    for (auto i = 1u; i <= 3; i++) {
        releaseAndAssert(lockingStateAsIntCopy, S /* lockMode */,
             S /* expectedLockMode */, 4u - i /* txCount1 */, 0u /* txCount2 */,
            2u /* numWaiters */, false /* expectedAwaken */);
    }

    releaseAndAssert(lockingStateAsIntCopy, S /* lockMode */,
        NONE /* expectedLockMode */, 0u /* txCount1 */, 0u /* txCount2 */,
          2u /* numWaiters */, true /* expectedAwaken */);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~ release S then IS ~~~~~~~~~~~~~~~~~~~~~~~~~

    for (auto i = 1u; i <= 3; i++) {
        releaseAndAssert(lockingStateAsIntCopy2, S /* lockMode */,
            S_IS /* expectedLockMode */, 1u /* txCount1 */, 4u - i /* txCount2 */,
              2u /* numWaiters */, false /* expectedAwaken */);
    }

    releaseAndAssert(lockingStateAsIntCopy2, S /* lockMode */,
        IS /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy2, IS /* lockMode */,
        NONE /* expectedLockMode */, 0u /* txCount1 */, 0u /* txCount2 */,
          2u /* numWaiters */, true /* expectedAwaken */);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    lockingStateAsIntCopy = lockingStateAsInt;
    lockingStateAsIntCopy2 = lockingStateAsInt;
    auto lockingStateAsIntCopy3 = lockingStateAsInt;
    auto lockingStateAsIntCopy4 = lockingStateAsInt;

    // ~~~~~~~~~~~~~~~~~~~~~~ relase IS then S then U ~~~~~~~~~~~~~~~~~~~~~~

    releaseAndAssert(lockingStateAsIntCopy, IS /* lockMode */,
        U_S_IS /* expectedLockMode */, 1u /* txCount1 */, 4u /* txCount2 */,
            2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy, IS /* lockMode */,
        U_S /* expectedLockMode */, 4u /* txCount1 */, 1u /* txCount2 */,
         2u /* numWaiters */, false /* expectedAwaken */);

    for (auto i = 1u; i <= 3; i++) {
        releaseAndAssert(lockingStateAsIntCopy, S /* lockMode */,
            U_S /* expectedLockMode */, 4u - i /* txCount1 */, 1u /* txCount2 */,
             2u /* numWaiters */, false /* expectedAwaken */);
    }

    releaseAndAssert(lockingStateAsIntCopy, S /* lockMode */,
         U /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy, U /* lockMode */,
        NONE /* expectedLockMode */, 0u /* txCount1 */, 0u /* txCount2 */,
          2u /* numWaiters */, true /* expectedAwaken */);

    // ~~~~~~~~~~~~~~~~~~~~~~ relase S then IS then U ~~~~~~~~~~~~~~~~~~~~~~

    for (auto i = 1u; i <= 3; i++) {
        releaseAndAssert(lockingStateAsIntCopy2, S /* lockMode */,
            U_S_IS /* expectedLockMode */, 2u /* txCount1 */, 4u - i /* txCount2 */,
                2u /* numWaiters */, false /* expectedAwaken */);
    }

    releaseAndAssert(lockingStateAsIntCopy2, S /* lockMode */,
        U_IS /* expectedLockMode */, 2u /* txCount1 */, 1u /* txCount2 */,
          2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy2, IS /* lockMode */,
        U_IS /* expectedLockMode */, 1u /* txCount1 */, 1u /* txCount2 */,
          2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy2, IS /* lockMode */,
         U /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy2, U /* lockMode */,
        NONE /* expectedLockMode */, 0u /* txCount1 */, 0u /* txCount2 */,
          2u /* numWaiters */, true /* expectedAwaken */);

    // ~~~~~~~~~~~~~~~~~~~~~~ relase S then U then IS ~~~~~~~~~~~~~~~~~~~~~~

    for (auto i = 1u; i <= 3; i++) {
        releaseAndAssert(lockingStateAsIntCopy3, S /* lockMode */,
            U_S_IS /* expectedLockMode */, 2u /* txCount1 */, 4u - i /* txCount2 */,
                2u /* numWaiters */, false /* expectedAwaken */);
    }

    releaseAndAssert(lockingStateAsIntCopy3, S /* lockMode */,
        U_IS /* expectedLockMode */, 2u /* txCount1 */, 1u /* txCount2 */,
          2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy3, U /* lockMode */,
        IS /* expectedLockMode */, 2u /* txCount1 */, 0u /* txCount2 */,
        2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy3, IS /* lockMode */,
        IS /* expectedLockMode */, 1u /* txCount1 */, 0u /* txCount2 */,
        2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy3, IS /* lockMode */,
        NONE /* expectedLockMode */, 0u /* txCount1 */, 0u /* txCount2 */,
          2u /* numWaiters */, true /* expectedAwaken */);

    // ~~~~~~~~~~~~~~~~~~~~~~ relase IS then U then S ~~~~~~~~~~~~~~~~~~~~~~

    releaseAndAssert(lockingStateAsIntCopy4, IS /* lockMode */,
        U_S_IS /* expectedLockMode */, 1u /* txCount1 */, 4u /* txCount2 */,
            2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy4, IS /* lockMode */,
        U_S /* expectedLockMode */, 4u /* txCount1 */, 1u /* txCount2 */,
         2u /* numWaiters */, false /* expectedAwaken */);

    releaseAndAssert(lockingStateAsIntCopy4, U /* lockMode */,
         S /* expectedLockMode */, 4u /* txCount1 */, 0u /* txCount2 */,
        2u /* numWaiters */, false /* expectedAwaken */);

    for (auto i = 1u; i <= 3; i++) {
        releaseAndAssert(lockingStateAsIntCopy4, S /* lockMode */,
             S /* expectedLockMode */, 4u - i /* txCount1 */, 0u /* txCount2 */,
            2u /* numWaiters */, false /* expectedAwaken */);
    }

    releaseAndAssert(lockingStateAsIntCopy4, S /* lockMode */,
        NONE /* expectedLockMode */, 0u /* txCount1 */, 0u /* txCount2 */,
          2u /* numWaiters */, true /* expectedAwaken */);
}
