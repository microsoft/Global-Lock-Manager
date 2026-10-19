#pragma once

#include "lock_mode.h"

#include <storage/lock_manager/locks/transaction_lock.h>

using namespace common;

namespace storage {

/**
 * Queue of granted and waiting lock requests by active txs
 */
class TransactionLocksQueue {
public:
    TransactionLocksQueue() : head{std::make_unique<TransactionLock>()},
        txLocksState{0u}
        {}

    bool push(TransactionLock* txLock);

    TransactionLock* pop(tx_id_t txId, lock_mode_t mode);

    std::unique_ptr<TransactionLock> head;
    uint64_t txLocksState;
    std::mutex mtx;
};

} // namespace storage
