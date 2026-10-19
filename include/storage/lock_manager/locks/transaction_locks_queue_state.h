#pragma once

#include <cstdint>
#include "lock_mode.h"

namespace storage {

typedef union {
    struct {
        lock_mode_t lockModes;
        uint16_t txCount1;
        uint16_t txCount2;
        uint16_t numWaiters;
    } fields;
    uint64_t bits;
} transaction_locks_queue_state_t;

void setLock(uint64_t& txLocksState, lock_mode_t txLockMode);
bool getLock(uint64_t& txLocksState, lock_mode_t txLockMode);
bool getLockForWaitingTx(uint64_t& txLocksState, lock_mode_t txLockMode);
bool releaseLock(uint64_t& txLocksState, lock_mode_t txLockMode);
void decrementNumWaiters(uint64_t& txLocksState);

} // namespace storage
