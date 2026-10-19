#include <storage/lock_manager/locks/node_lock.h>

#include <spdlog/spdlog.h>

namespace storage {

bool NodeLock::isHeld() {
    node_lock_state_t nodeLockState;
    nodeLockState.bits = this->state.load();
    return nodeLockState.fields.heldLockMode != NONE;
}

lock_mode_t NodeLock::getHeldLockMode() {
    node_lock_state_t nodeLockState;
    nodeLockState.bits = state.load();
    return (lock_mode_t) nodeLockState.fields.heldLockMode;
}

void NodeLock::setHeldLockMode(lock_mode_t lockMode) {
    node_lock_state_t old_;
    node_lock_state_t new_;

    old_.bits = state.load();
    do {
        new_.bits = old_.bits;
        new_.fields.heldLockMode = lockMode;
    } while (!state.compare_exchange_weak(old_.bits, new_.bits));
}

bool NodeLock::setRequestedLockMode(lock_mode_t lockMode) {
    node_lock_state_t old_;
    node_lock_state_t new_;

    old_.bits = state.load();
    if (lockMode != NONE) {
        while (old_.fields.requestedLockMode == NONE /* no req */) {
            new_.bits = old_.bits;
            new_.fields.requestedLockMode = lockMode;
            if (state.compare_exchange_weak(old_.bits, new_.bits)) {
                return true;
            }
        }
        return false;
    } else {
        do {
            new_.bits = old_.bits;
            new_.fields.requestedLockMode = NONE;
        } while (!state.compare_exchange_weak(old_.bits, new_.bits));
    }
    return true;
}

bool NodeLock::areTxsBlocked() {
    node_lock_state_t old_;
    node_lock_state_t new_;

    old_.bits = state.load();
    while (/* not blocked */ old_.fields.areTxsBlocked == 0) {
        new_.bits = old_.bits;
        new_.fields.numRegisteredTxs++;
        if (state.compare_exchange_weak(old_.bits, new_.bits)) {
            return false;
        }
    }
    return true;
}

void NodeLock::log() {
    node_lock_state_t nodeLockState;
    nodeLockState.bits = state.load();
    spdlog::info("(blocked: {}, #registeredTxs: {})",
        nodeLockState.fields.areTxsBlocked == 1,
        (uint32_t) nodeLockState.fields.numRegisteredTxs);
}

void blockOrUnblock(std::atomic<uint32_t>& nodeLockState,
    uint8_t areTxsBlocked) {
    node_lock_state_t old_;
    node_lock_state_t new_;

    old_.bits = nodeLockState.load();
    do {
        new_.bits = old_.bits;
        new_.fields.areTxsBlocked = areTxsBlocked;
    } while (!nodeLockState.compare_exchange_weak(old_.bits, new_.bits));
}

void NodeLock::blockTxs() {
    blockOrUnblock(state, 1u /* block */);
}

void NodeLock::unblockTxs() {
    blockOrUnblock(state, 0u /* block */);
}

void NodeLock::unregisterTx() {
    node_lock_state_t old_;
    node_lock_state_t new_;

    old_.bits = state.load();
    do {
        new_.bits = old_.bits;
        new_.fields.numRegisteredTxs--;
    } while (!state.compare_exchange_weak(old_.bits, new_.bits));
}

bool NodeLock::isEmpty() {
    node_lock_state_t nodeLockState;
    nodeLockState.bits = state.load();
    return nodeLockState.fields.numRegisteredTxs == 0;
}

uint32_t NodeLock::getNumRegisteredTxs() {
    node_lock_state_t nodeLockState;
    nodeLockState.bits = state.load();
    return nodeLockState.fields.numRegisteredTxs;
}

} // namespace storage
