#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "transaction_lock.h"

namespace storage {

// Warning: not thread-safe
class alignas(64) TransactionLockPool {
public:
    TransactionLockPool() : TransactionLockPool(100u) {}
    TransactionLockPool(uint32_t capacity);

    TransactionLock* next(tx_id_t txId, timestamp_t timestamp, lock_mode_t mode);
    void release(TransactionLock* buffer);

private:
    TransactionLock* next();

private:
    const uint32_t capacity;
    uint32_t nextPos;
    // TODO: change buffer to std::unique_ptr<TxLock[]>
    std::unique_ptr<TransactionLock*[]> queue;
    std::vector<TransactionLock> buffer;
};

} // namespace storage
