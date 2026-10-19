#pragma once

#include <condition_variable>
#include <shared_mutex>
#include <common/spinlock.h>
#include <common/typedefs.h>

#include <storage/lock_manager/locks/lock_mode.h>
#include <storage/lock_manager/locks/node_lock.h>
#include <storage/lock_manager/locks/transaction_lock.h>
#include <storage/lock_manager/locks/transaction_locks_queue.h>

using namespace common;

namespace storage {

/**
 * The ObjLocks represents the set of node and tx requests held on the object.
 */
struct alignas(64) ObjLocks {
    obj_id_t objId;

    // (next, mtx) used within transaction_locks_
    std::unique_ptr<ObjLocks> next;
    Spinlock spinlock;

    NodeLock glmNodeLock;
    NodeLock nonGlmNodeLock;
    TransactionLocksQueue txRequests;

    explicit ObjLocks(obj_id_t objId) : objId{objId}, next() {}
    ObjLocks() : ObjLocks(UINT64_MAX) {}
    ObjLocks(const ObjLocks& objLocks) : ObjLocks() {}

    bool push(TransactionLock* txLock) {
        return txRequests.push(txLock);
    }

    TransactionLock* pop(tx_id_t txId, lock_mode_t lockMode) {
        return txRequests.pop(txId, lockMode);
    }
};

} // namespace storage
