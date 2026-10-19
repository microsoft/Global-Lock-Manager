#pragma once

#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <mutex>

#include <common/typedefs.h>
#include <storage/lock_manager/locks/lock_mode.h>

using namespace common;

namespace storage {

/**
 * Represents the state of a node lock. For each object, there is only one
 * single node lock.
 */
typedef union {
    struct {
        uint32_t areTxsBlocked : 2; /* specifies whether transctions can use the
          node lock. This field is set and unset by a remote GetNodeLock handler
          threads to 'drain' the transaction locks queue */
        uint32_t numRegisteredTxs : 12; /* is the number of transactions that
          is currently using the node lock. For all registered transactions,
          the node lock cannot be released and can only be downgraded to the
          most restrictive held transaction lock */
        uint32_t numTxsIntendingToRegister : 12; /* is the number of transactions
          intending to register and could not. This field is only larger than 0
          if the areTxsBlocked field is set */
        uint32_t heldLockMode : 3; /* is the current held lock mode */
        uint32_t requestedLockMode : 3; /* is the currently requested lock mode.
          The requested lock mode is always more restrictive than the held one */
    } fields;
    uint32_t bits;
} node_lock_state_t;

/**
 * A node lock represents the intent of the node with a file or a page. 
 */
struct alignas(64) NodeLock {
    NodeLock() : state{0u}, pageLSN {UINT64_MAX}, requestTimestamp{UINT64_MAX},
        obtainTimestamp{0u}, durationInNanosecs{0u}
        {}

    bool isHeld();

    lock_mode_t getHeldLockMode();
    void setHeldLockMode(lock_mode_t lockMode);
    bool setRequestedLockMode(lock_mode_t lockMode);

    bool areTxsBlocked();
    void blockTxs();
    void unblockTxs();
    void unregisterTx();
    bool isEmpty();
    uint32_t getNumRegisteredTxs();

    void log();

    std::atomic<uint32_t> state;
    /* Incremented /w each record update */
    std::atomic<page_lsn_t> pageLSN;
    /* Timestamp when sending the node lock request */
    std::atomic<timestamp_t> requestTimestamp;
    timestamp_t obtainTimestamp;
    timestamp_t durationInNanosecs;

    /* for blocking tx thread by remote GetNodeLock requests */
    std::mutex blockMtx;
    std::condition_variable blockCondVar;
    /* for blocking tx thread by local GetNodeLock requests */
    std::mutex localRequestMtx;
    std::condition_variable localRequestCondVar;
    /* for blocking tx thread by remote GetNodeLock requests */
    std::mutex remoteRequestMtx;
};

} // namespace storage
