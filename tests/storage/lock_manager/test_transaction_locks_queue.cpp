#include <gtest/gtest.h>

#include <thread>
#include <iostream>

#include <storage/lock_manager/obj_locks.h>
#include "storage/lock_manager/locks/transaction_locks_queue_state.h"

using namespace common;

using namespace storage;

void ASSERT_LOCKING_STATE(uint64_t state, lock_mode_t expectedLockingMode,
    uint16_t expectedTxCount2, uint16_t expectedTxCount1,
    uint16_t expectedNumWaiters) {
    transaction_locks_queue_state_t lockingState;
    lockingState.bits = state;
    EXPECT_EQ(lockingState.fields.lockModes, expectedLockingMode);
    EXPECT_EQ(lockingState.fields.txCount1, expectedTxCount1);
    EXPECT_EQ(lockingState.fields.txCount2, expectedTxCount2);
    EXPECT_EQ(lockingState.fields.numWaiters, expectedNumWaiters);
}

void ASSERT_TX_LOCK(TransactionLock* txLock, tx_id_t expectedTxId,
    lock_mode_t expectedMode, bool expectedGranted) {
    EXPECT_EQ(txLock->txId, expectedTxId);
    EXPECT_EQ(txLock->mode, expectedMode);
    EXPECT_EQ(txLock->granted, expectedGranted);
}

TEST(TransactionLocksQueueTests, WaitDieAndBlocking) {
    TransactionLocksQueue queue;

    // Queue is empty, we push 3 S- and 2 IS-lock requests which are granted automatically.
    auto NUM_S_LOCK_REQUESTS  = 2u;
    auto NUM_IS_LOCK_REQUESTS = 1u;
    for (auto i = 0u; i < NUM_S_LOCK_REQUESTS; i++) {
        queue.push(new TransactionLock(i /* txId */, getTimestamp(), S));
    }
    for (auto i = 0u; i < NUM_IS_LOCK_REQUESTS; i++) {
        auto txId = NUM_S_LOCK_REQUESTS + i;
        queue.push(new TransactionLock(txId, getTimestamp(), IS));
    }

    ASSERT_LOCKING_STATE(queue.txLocksState, lock_mode_t::S_IS, 2, 1, 0);
    /** Once the 2 S- and 1 IS-lock requests are pushed, the queue of tx lock
     * requests is as follows:
     *
     * | head:  => {          => {          => {
     * | (dummy) |  txId: 2    |  txId: 1    |  txId: 0    |
     * |         |  mode: IS   |  mode: S    |  mode: S    |
     * |         |  granted: T |  granted: T |  granted: T |
     * |         | }           | }           | }           |
     */
    auto currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 2u /* txId */, IS, true /* granted */);
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 1u /* txId */, S, true /* granted */);
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 0u /* txId */, S, true /* granted */);

    auto newXLockReq = new TransactionLock(3u /* txId */, getTimestamp(), X);
    EXPECT_FALSE(queue.push(newXLockReq)); // not pushed, younger tx aborted.
    ASSERT_LOCKING_STATE(queue.txLocksState, lock_mode_t::S_IS, 2, 1, 0);

    newXLockReq->timestamp = 100u;
    std::thread t(&TransactionLocksQueue::push, &queue, newXLockReq);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    /** Once the X-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head => {          => {          => {          => {
     * | dummy |  txId: 2    |  txId: 1    |  txId: 0    |  txId: 3    |
     * |       |  mode: IS   |  mode: S    |  mode: S    |  mode: X    |
     * |       |  granted: T |  granted: T |  granted: T |  granted: F |
     * |       | }           | }           | }           | }           |
     */
    ASSERT_LOCKING_STATE(queue.txLocksState, lock_mode_t::S_IS, 2, 1, 1);
    ASSERT_EQ(currNode->next, newXLockReq);
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 3u /* txId */, X, false /* granted */);

    delete queue.pop(1u /* txId */, S);
    ASSERT_LOCKING_STATE(queue.txLocksState, lock_mode_t::S_IS, 1, 1, 1);
    /** Once the lock of tx 1 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head => {          => {          => {
     * | dummy |  txId: 2    |  txId: 0    |  txId: 3    |
     * |       |  mode: IS   |  mode: S    |  mode: X    |
     * |       |  granted: T |  granted: T |  granted: F |
     * |       | }           | }           | }           |
     */
    currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 2u /* txId */, IS, true /* granted */);
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 0u /* txId */, S, true /* granted */);
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 3u /* txId */, X, false /* granted */);

    delete queue.pop(0u /* txId */, S);
    ASSERT_LOCKING_STATE(queue.txLocksState, lock_mode_t::IS, 0, 1, 1);
    /** Once the lock of tx 0 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head => {          => {
     * | dummy |  txId: 2    |  txId: 3    |
     * |       |  mode: IS   |  mode: X    |
     * |       |  granted: T |  granted: F |
     * |       | }           | }           |
     */
    currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 2u /* txId */, IS, true /* granted */);
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 3u /* txId */, X, false /* granted */);

    delete queue.pop(2u /* txId */, IS);
    ASSERT_LOCKING_STATE(queue.txLocksState, lock_mode_t::X, 0, 1, 0);
    /** Once the lock of tx 2 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {
     * | dummy |  txId: 3    |
     * |       |  mode: X    |
     * |       |  granted: F |
     * |       | }           |
     */
    currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 3u /* txId */, X, true /* granted */);

    t.join();

    delete queue.pop(3u, X);
}

TEST(TransactionLocksQueueTests, BlockingAlternating) {
    TransactionLocksQueue queue;

    EXPECT_TRUE(queue.push(new TransactionLock(0u /* txId */, 10 /* timestamp */, X)));
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 0);
    /** Once the X-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {
     * | dummy |  txId: 0    |
     * |       |  mode: X    |
     * |       |  granted: T |
     * |       | }           |
     */
    auto currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 0u /* txId */, X, true /* granted */);

    // We next request S-, X-, IS-, S-, X-, X- locks.

    std::thread t1(&TransactionLocksQueue::push, &queue,
        new TransactionLock(1u /* txId */, 9, S));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 1);
    /** Once the S-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {
     * | dummy |  txId: 0    |  txId: 1    |
     * |       |  mode: X    |  mode: S    |
     * |       |  granted: T |  granted: F |
     * |       | }           | }           |
     */
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 1u /* txId */, S, false /* granted */);

    std::thread t2(&TransactionLocksQueue::push, &queue,
        new TransactionLock(2u /* txId */, 8, X));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 2);
    /** Once the X-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {          => {
     * | dummy |  txId: 0    |  txId: 1    |  txId: 2    |
     * |       |  mode: X    |  mode: S    |  mode: X    |
     * |       |  granted: T |  granted: F |  granted: F |
     * |       | }           | }           | }           |
     */
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 2u /* txId */, X, false /* granted */);

    std::thread t3(&TransactionLocksQueue::push, &queue,
        new TransactionLock(3u /* txId */, 7, IS));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 3);
    /** Once the IS-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {          => {          => {
     * | dummy |  txId: 0    |  txId: 1    |  txId: 2    |  txId: 3    |
     * |       |  mode: X    |  mode: S    |  mode: X    |  mode: IS   |
     * |       |  granted: T |  granted: F |  granted: F |  granted: F |
     * |       | }           | }           | }           | }           |
     */
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 3u /* txId */, IS, false /* granted */);

    std::thread t4(&TransactionLocksQueue::push, &queue,
        new TransactionLock(4u /* txId */, 6, S));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 4);
    /** Once the S-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {          => {          => {          =>
     * | dummy |  txId: 0    |  txId: 1    |  txId: 2    |  txId: 3    |
     * |       |  mode: X    |  mode: S    |  mode: X    |  mode: IS   |
     * |       |  granted: T |  granted: F |  granted: F |  granted: F |
     * |       | }           | }           | }           | }           |
     *
     * => {
     * |  txId: 4    |
     * |  mode: S    |
     * |  granted: F |
     * | }           |
     */
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 4u /* txId */, S, false /* granted */);

    std::thread t5(&TransactionLocksQueue::push, &queue,
        new TransactionLock(5u /* txId */, 5, X));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 5);
    /** Once the X-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {          => {          => {          =>
     * | dummy |  txId: 0    |  txId: 1    |  txId: 2    |  txId: 3    |
     * |       |  mode: X    |  mode: S    |  mode: X    |  mode: IS   |
     * |       |  granted: T |  granted: F |  granted: F |  granted: F |
     * |       | }           | }           | }           | }           |
     *
     * => {          => {
     * |  txId: 4    |  txId: 5    |
     * |  mode: S    |  mode: X    |
     * |  granted: F |  granted: F |
     * | }           | }           |
     */
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 5u /* txId */, X, false /* granted */);

    std::thread t6(&TransactionLocksQueue::push, &queue,
        new TransactionLock(6u /* txId */, 4, X));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 6);
    /** Once the X-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {          => {          => {          =>
     * | dummy |  txId: 0    |  txId: 1    |  txId: 2    |  txId: 3    |
     * |       |  mode: X    |  mode: S    |  mode: X    |  mode: IS   |
     * |       |  granted: T |  granted: F |  granted: F |  granted: F |
     * |       | }           | }           | }           | }           |
     *
     * => {          => {          => {
     * |  txId: 4    |  txId: 5    |  txId: 6    |
     * |  mode: S    |  mode: X    |  mode: X    |
     * |  granted: F |  granted: F |  granted: F |
     * | }           | }           | }           |
     */
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 6u /* txId */, X, false /* granted */);

    delete queue.pop(0, X);
    t1.join();
    /** Once the lock of tx 0 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {          => {          => {
     * | dummy |  txId: 1    |  txId: 2    |  txId: 3    |  txId: 4    |
     * |       |  mode: S    |  mode: X    |  mode: IS   |  mode: S    |
     * |       |  granted: T |  granted: F |  granted: F |  granted: F |
     * |       | }           | }           | }           | }           |
     *
     * => {          => {          
     * |  txId: 5    |  txId: 6    |
     * |  mode: X    |  mode: X    |
     * |  granted: F |  granted: F |
     * | }           | }           |
     */
    ASSERT_LOCKING_STATE(queue.txLocksState, S, 0, 1, 5);
    currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 1u /* txId */, S, true /* granted */);

    delete queue.pop(1, X);
    t2.join();
    /** Once the lock of tx 1 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {          => {          => {
     * | dummy |  txId: 2    |  txId: 3    |  txId: 4    |  txId: 5    |
     * |       |  mode: X    |  mode: IS   |  mode: S    |  mode: X    |
     * |       |  granted: T |  granted: F |  granted: F |  granted: F |
     * |       | }           | }           | }           | }           |
     *
     * => {          
     * |  txId: 6    |
     * |  mode: X    |
     * |  granted: F |
     * | }           |
     */
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 4);
    currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 2u /* txId */, X, true /* granted */);

    delete queue.pop(2, X);
    t3.join();
    t4.join();
    /** Once the lock of tx 2 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {          => {          => {
     * | dummy |  txId: 3    |  txId: 4    |  txId: 5    |  txId: 6    |
     * |       |  mode: IS   |  mode: S    |  mode: X    |  mode: X    |
     * |       |  granted: T |  granted: T |  granted: F |  granted: F |
     * |       | }           | }           | }           | }           |
     */
    ASSERT_LOCKING_STATE(queue.txLocksState, S_IS, 1, 1, 2);
    currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 3u /* txId */, IS, true /* granted */);
    currNode = currNode->next;
    ASSERT_TX_LOCK(currNode, 4u /* txId */, S, true /* granted */);

    delete queue.pop(4u, S);
    ASSERT_LOCKING_STATE(queue.txLocksState, IS, 0, 1, 2);
    delete queue.pop(3u, IS);
    t5.join();
    /** Once the lock of tx 2 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {          => {
     * | dummy |  txId: 5    |  txId: 6    |
     * |       |  mode: X    |  mode: X    |
     * |       |  granted: F |  granted: F |
     * |       | }           | }           |
     */
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 1);
    currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 5u /* txId */, X, true /* granted */);

    delete queue.pop(5u, X);
    t6.join();
    /** Once the lock of tx 2 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {
     * | dummy |  txId: 6    |
     * |       |  mode: X    |
     * |       |  granted: F |
     * |       | }           |
     */
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 0);
    currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 6u /* txId */, X, true /* granted */);

    delete queue.pop(6u, X);
    /** Once the lock of tx 2 is released, the queue of tx lock requests is as
     * follows:
     *
     * | head
     * | dummy |
     * |       |
     * |       |
     * |       |
     */
    ASSERT_TRUE(queue.head->next == nullptr);
}

TEST(TransactionLocksQueueTests, waitDieTests) {
    TransactionLocksQueue queue;

    EXPECT_TRUE(queue.push(new TransactionLock(0u /* txId */, 10 /* timestamp */, X)));
    ASSERT_LOCKING_STATE(queue.txLocksState, X, 0, 1, 0);
    /** Once the X-lock request is pushed, the queue of tx lock requests is as
     * follows:
     *
     * | head  => {
     * | dummy |  txId: 0    |
     * |       |  mode: X    |
     * |       |  granted: T |
     * |       | }           |
     */
    auto currNode = queue.head->next;
    ASSERT_TX_LOCK(currNode, 0u /* txId */, X, true /* granted */);

    // We next request S- with a younger tx.

    auto txLock = new TransactionLock(0u /* txId */, 10 /* timestamp */, X);
    for (auto i = 0u; i < 3; i++) {

    }
}