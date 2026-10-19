#include "storage/lock_manager/locks/transaction_locks_queue_state.h"

namespace storage {

void setLock(uint64_t& txLocksState, lock_mode_t txLockMode) {
    transaction_locks_queue_state_t lockingState;
    lockingState.bits = txLocksState;
    lockingState.fields.numWaiters--;
    lockingState.fields.lockModes = txLockMode;
    lockingState.fields.txCount1 = 1;
    txLocksState = lockingState.bits;
}

void updateLockGivenCompatible(transaction_locks_queue_state_t& lockingState,
    lock_mode_t txLockMode) {
    auto currLocks = lockingState.fields.lockModes;
    if (currLocks == NONE || currLocks == txLockMode) {
        lockingState.fields.lockModes = txLockMode;
        lockingState.fields.txCount1++;
    } else if (currLocks <= lock_mode_t::U) {
        /* lock modes: { NONE, IS, S, IX, SIX, X, U } */
        if (currLocks > txLockMode) {
        lockingState.fields.lockModes = (lock_mode_t)
            ((currLocks << 4u) | txLockMode);
            lockingState.fields.txCount2 = lockingState.fields.txCount1;
            lockingState.fields.txCount1 = 1u;
        } else { // currLocks < requestedTxLockMode
            lockingState.fields.lockModes = (lock_mode_t)
                ((txLockMode << 4u) | currLocks);
            lockingState.fields.txCount2 = 1u;
        }
    } else /* lock modes: { S_IS, IX_IS, SIX_IS, U_IS, U_S, U_S_IS } */
        /* if (currLocks <= lock_mode_t::U_S_IS) && */
           if (txLockMode == (currLocks & 0x0F)) {
        lockingState.fields.txCount1++;
    } else if (txLockMode == ((currLocks & 0b01110000) >> 4u)) {
        lockingState.fields.txCount2++;
    } else {
        // Possible (held, requested) pairs:
        // (U_IS, S), (U_S, IS), (S_IS, U) -> U_S_IS
        lockingState.fields.lockModes = U_S_IS;
        if (currLocks == U_S) {
            lockingState.fields.txCount2 = lockingState.fields.txCount1;
            lockingState.fields.txCount1 = 1u;
        }
    }
}

bool getLockForWaitingTx(uint64_t& txLocksState, lock_mode_t txLockMode) {
    transaction_locks_queue_state_t lockingState;
    lockingState.bits = txLocksState;

    auto grant = false;
    auto currLocks = lockingState.fields.lockModes;

    if (IsCompatible::forTxs(currLocks, txLockMode)) {
        grant = true;
        lockingState.fields.numWaiters--;
        updateLockGivenCompatible(lockingState, txLockMode);
    }

    txLocksState = lockingState.bits;
    return grant;
}

bool getLock(uint64_t& txLocksState, lock_mode_t txLockMode) {
    transaction_locks_queue_state_t lockingState;
    lockingState.bits = txLocksState;

    auto grant = false;
    auto currLocks = lockingState.fields.lockModes;

    if (lockingState.fields.numWaiters > 0 ||
        !IsCompatible::forTxs(currLocks, txLockMode)) {
        lockingState.fields.numWaiters++;
    } else { // numWaiters == 0 && IsCompatible::check
        grant = true;
        updateLockGivenCompatible(lockingState, txLockMode);
    }
    // update the shared state and return whether the lock is granted.
    txLocksState = lockingState.bits;
    return grant;
}

// warning: should never worry about releasing U and
// should instead have an upgrade call.
bool releaseLock(uint64_t& txLocksState, lock_mode_t mode) {
    transaction_locks_queue_state_t lockingState;
    lockingState.bits = txLocksState;

    auto currLocks = lockingState.fields.lockModes;
    if (currLocks < lock_mode_t::U) {
        lockingState.fields.txCount1--;
        if (lockingState.fields.txCount1 == 0) {
            lockingState.fields.lockModes = lock_mode_t::NONE;
        }
    } else if (currLocks < U_S_IS) {
        if (mode == (currLocks & 0x0F)) {
            lockingState.fields.txCount1--;
            if (lockingState.fields.txCount1 == 0) {
                lockingState.fields.lockModes = (lock_mode_t)(currLocks >> 4u);
                lockingState.fields.txCount1 = lockingState.fields.txCount2;
                lockingState.fields.txCount2 = 0;
            }
        } else if (mode == ((currLocks & 0xF0) >> 4u)) {
            lockingState.fields.txCount2--;
            if (lockingState.fields.txCount2 == 0) {
                lockingState.fields.lockModes = (lock_mode_t)
                    (((lockingState.fields.lockModes & 0x0F00) >> 4) |
                        (lockingState.fields.lockModes & 0x000F));
            }
        }
    } else { /* U_S_IS */
        if (mode == U) {
            lockingState.fields.lockModes = S_IS;
        } else {
            if (mode == (currLocks & 0x0F)) {
                lockingState.fields.txCount1--;
                if (lockingState.fields.txCount1 == 0) {
                    lockingState.fields.lockModes = U_S;
                    lockingState.fields.txCount1 = lockingState.fields.txCount2;
                    lockingState.fields.txCount2 = 1;
                }
            } else if (mode == ((currLocks & 0b01110000) >> 4u)) {
                lockingState.fields.txCount2--;
                if (lockingState.fields.txCount2 == 0) {
                    lockingState.fields.lockModes = U_IS;
                    lockingState.fields.txCount2 = 1;
                }
            }
        }
    }

    txLocksState = lockingState.bits;
    return lockingState.fields.txCount1 + lockingState.fields.txCount2 == 0 &&
        lockingState.fields.numWaiters > 0;
}

void decrementNumWaiters(uint64_t& txLocksState) {
    transaction_locks_queue_state_t lockingState;
    lockingState.bits = txLocksState;
    lockingState.fields.numWaiters--;
    txLocksState = lockingState.bits;
}

} // namespace storage
