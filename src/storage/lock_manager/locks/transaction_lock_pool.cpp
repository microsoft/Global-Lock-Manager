#include <storage/lock_manager/locks/transaction_lock_pool.h>

#include "spdlog/spdlog.h"

namespace storage {

TransactionLockPool::TransactionLockPool(uint32_t capacity) :
    capacity{capacity}, nextPos{capacity - 1},
    queue{std::make_unique<TransactionLock*[]>(capacity)} {
    buffer.reserve(capacity);
    for (auto i = 0u; i < capacity; i++) {
        buffer.emplace_back();
        queue[i] = &(buffer.back());
    }
}

TransactionLock* TransactionLockPool::next() {
    if (nextPos == -1) { // Check for pool overflow
        spdlog::error("Not enough buffers, our BufferPool has {} buffers only",
            capacity);
        throw;
    }
    return queue[nextPos--];
}

TransactionLock* TransactionLockPool::next(tx_id_t txId, timestamp_t timestamp,
    lock_mode_t mode) {
    auto txLock = next();
    txLock->txId = txId;
    txLock->timestamp = timestamp;
    txLock->mode = mode;
    txLock->granted = false;
    return txLock;
}

void TransactionLockPool::release(TransactionLock* buffer) {
    nextPos++;
    queue[nextPos] = buffer;
}

} // namespace storage
