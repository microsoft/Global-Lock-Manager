#include <storage/lock_manager/locks/transaction_locks_queue.h>

#include <storage/lock_manager/locks/transaction_locks_queue_state.h>

#include <spdlog/spdlog.h>

namespace storage {

bool TransactionLocksQueue::push(TransactionLock* txLock) {
    mtx.lock();
    txLock->granted = getLock(txLocksState, txLock->mode);
    if (txLock->granted) {
        txLock->next = head->next;
        head->next = txLock;
        mtx.unlock();
    } else {
        auto currNode = head.get()->next;
        // Find the tail TxLockHeader and obtain a lock on it.
        while (currNode->next != nullptr) {
            currNode = currNode->next;
            if (txLock->timestamp > currNode->timestamp) {
                // The comparison implements 'Wait-Die', a non-preemptive
                // technique for deadlock prevention. A timestamp is the number
                // of nanosecs since epoch so an older transaction has a smaller
                // timestamp. The transaction that invoted this push call was
                // not granted the lock directly and is only allowed to wait on
                // txs that are younger and hence have larger timestamp.
                decrementNumWaiters(txLocksState);
                mtx.unlock();
                return false;
            }
        }
        currNode->next = txLock;
        {
            std::unique_lock<std::mutex> lock(txLock->waitMtx);
            mtx.unlock();
            spdlog::info("... sleeeeping ...");
            txLock->waitCond.wait(lock, [txLock] {
                return txLock->granted;
            });
        }
    }
    return true;
}

TransactionLock* TransactionLocksQueue::pop(tx_id_t txId, lock_mode_t mode) {
    std::unique_lock<std::mutex> lock(mtx);
    if (/* grant waiting locks */ releaseLock(txLocksState, mode)) {
        auto nodeToAwaken = head->next->next;
        setLock(txLocksState, nodeToAwaken->mode);
        nodeToAwaken->granted = true;
        nodeToAwaken->waitCond.notify_one();
        while (nodeToAwaken->next != nullptr &&
               getLockForWaitingTx(txLocksState, nodeToAwaken->next->mode)) {
            nodeToAwaken = nodeToAwaken->next;
            nodeToAwaken->granted = true;
            nodeToAwaken->waitCond.notify_one();
        }
    }

    auto currNode = head.get();
    while(currNode->next->txId != txId) {
        currNode = currNode->next;
    }
    auto txLock = currNode->next;
    currNode->next = txLock->next;
    return txLock;
}

} // namespace storage
