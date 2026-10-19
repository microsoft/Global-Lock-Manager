#pragma once

#include <condition_variable>
#include <mutex>

#include "lock_mode.h"
#include <common/typedefs.h>
#include <common/spinlock.h>

using namespace common;

namespace storage {

struct alignas(64) TransactionLock {
    TransactionLock() : txId{UINT64_MAX}, timestamp{UINT64_MAX}, next{NULL},
        mode{NONE}, granted{false}
        {}

    TransactionLock(tx_id_t txId, timestamp_t timestamp, lock_mode_t mode) :
        txId{txId}, timestamp{timestamp}, next{NULL},
        mode{mode}, granted{false}
        {}

    TransactionLock(const TransactionLock& txLock) : TransactionLock() {};

    tx_id_t txId;
    timestamp_t timestamp;
    TransactionLock* next;
    lock_mode_t mode;
    bool granted;

    /* used for waiting */
    std::mutex waitMtx;
    std::condition_variable waitCond;
};

} // namespace storage
