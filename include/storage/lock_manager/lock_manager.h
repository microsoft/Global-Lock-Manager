#pragma once

#include <map>
#include <vector>

#include <common/typedefs.h>

#include <transport/transport.h>
#include <transport/message.h>

#include "locks/transaction_lock_pool.h"
#include "obj_id_to_obj_locks_map.h"

#include <unistd.h>

using namespace common;
using namespace transport;

namespace storage {

/**
 *
 */
enum GET_NODE_LOCK_RESPONSE : uint8_t {
    SUCCESS = 0u,
    LOCAL_REQ_SENT = 1u,
    REMOTE_REQ_FAILED = 2u,
};

/**
 * The lock manager (LM) manages the node and transaction locks. An LM instance
 * exists on each compute node and only one of the LMs acts as a global LM (GLM)
 * i.e., has a global view of all node locks. The other non-GLMs keep track of
 * only their own node locks.  In our implementation, we only consider two
 * compute nodes. One  running as a GLM and the other as an LLM. LMs keep track
 * of transaction locks locally and these locks are not accessible to other LMs.
 *
 * An LM uses a lock table that is a map from an object ID to an ObjLocks
 * instance that contains both node and tx locks of the object with the ID.
 *
 * An LM is setup as a server and as a client by extending the Transport class.
 * The server (or client) handles (or sends) remote GetNodeLock requests by
 * (or to) other LMs. A GLM makes a remote GetNodeLock request to resolve a node
 * lock conflict with a non-GLM while a non-GLM makes a remote GetNodeLock
 * request to the GLM to request the node lock (non-GLM requests might include
 * conflict resolution executed by the GLM but the non-GLM is unaware of such
 * details as it does not have a global view of all node locks).
 */
class LockManager : public transport::Transport<request_msg_t> {
public:
    /**
     * A constructor.
     * @param isGlm indicates if the LM acts as a GLM.
     * @param numTxThreads is the number of transaction worker threads.
     * @param numNodeLockRequestHandlers is the number of threads local
     * to the LockManager to handle remoteGetNodeLock requests.
     * @param numFIles is the number of files in the database.
     * @param absolutePathToLogDir is the aboluste path to the log directory.
     * @param initTransport Flag used for unit testing, set to false in tests
     * to not setup server/client transport. By default, true.
     * @return a lock manager object.
     */
    LockManager(bool isGlm, uint32_t numTxThreads,
        uint32_t numNodeLockRequestHandlers, uint32_t numFiles,
        const char* absolutePathToLogDir, bool initTransport = true);

    /**
     * A deconstructor.
     */
    void shutdown();

    /**
     * @return true if the LM acts as GLM. False, otherwise.
     */
    [[nodiscard]] bool isGLM() const { return isGlm; }

    bool lockFile(file_idx_t fidx, lock_mode_t txMode, tx_id_t txId,
        timestamp_t timestamp, uint32_t threadId) {
        return fileIdToFileLocksMap[fidx].push(
            transactionLockPools[threadId].next(txId, timestamp, txMode));
    }

    void unlockFile(file_idx_t fidx, tx_id_t txId, lock_mode_t txMode,
        uint32_t threadId) {
        transactionLockPools[threadId].release(
            fileIdToFileLocksMap[fidx].pop(txId, txMode));
    }

    bool lock(record_id_t objId, lock_mode_t txMode,
        tx_id_t txId, timestamp_t timestamp, uint32_t threadId) {
        auto txLock = transactionLockPools[threadId].next(
             txId, timestamp, txMode);
        auto success = objIdToObjLocksMap.getElseInsert(objId)->push(txLock);
        if (!success) {
            transactionLockPools[threadId].release(txLock);
            spdlog::info("thread {} - released tid {} | objId {} | txMode {}",
                threadId, txId, objId, txMode);
        }
        return success;
    }

    void unlock(record_id_t objId, tx_id_t txId, lock_mode_t txMode,
        uint32_t threadId) {
        transactionLockPools[threadId].release(
            objIdToObjLocksMap.get(objId)->pop(txId, txMode));
    }

    void cleanupFailedLock(record_id_t objId, tx_id_t txId, lock_mode_t txMode,
        uint32_t threadId) {
        // What to do here?!
//        transactionLockPools[threadId].release(/* objIdToObjLocksMap.get(objId) */);
    }

    bool getFileNodeLock(file_id_t fileId, lock_mode_t nodeMode,
        timestamp_t txTimestamp);

    bool getPageNodeLock(page_id_t pageId, lock_mode_t nodeMode,
        timestamp_t txTimestamp);

    void releaseFileNodeLockByTx(file_id_t fileId) {
        if (isGlm) {
            fileIdToFileLocksMap[fileId].glmNodeLock.unregisterTx();
        } else {
            fileIdToFileLocksMap[fileId].nonGlmNodeLock.unregisterTx();
        }
    }

    void releasePageNodeLockByTx(page_id_t pageId) {
        if (isGlm) {
            objIdToObjLocksMap.get(pageId)->glmNodeLock.unregisterTx();
        } else {
            objIdToObjLocksMap.get(pageId)->nonGlmNodeLock.unregisterTx();
        }
    }

    uint32_t getNumRemoteGetNodeLockHandlerThreads() {
        return remoteGetNodeLockHandlerThreads.size();
    }

    void log() {
        objIdToObjLocksMap.log();
    }

protected:
    bool getNodeLock(obj_id_t objId, ObjLocks* objLocks,
        lock_mode_t nodeMode, timestamp_t timestamp);

    GET_NODE_LOCK_RESPONSE localGetNodeLock(obj_id_t objId, ObjLocks* objLocks,
        lock_mode_t nodeModeReq, timestamp_t nodeLockRequestTimestamp);

    virtual GET_NODE_LOCK_RESPONSE remoteGetNodeLock(ObjLocks* objLocks,
        lock_mode_t nodeMode, timestamp_t nodeLockRequestTimestamp);

    /**
     * The function executed by the threads local to the LockManager to handle
     * incoming remote GetNodeLock requests from other nodes.
     */
    void remoteGetNodeLockHandlerThreadFunc();

    /**
     * Maps a transaction lock mode of a file to the appropriate node lock mode.
     * The node lock is the same as the transaction lock unless it is a U-lock
     * The requested node mode lock is upgraded to SIX if the held and requested
     * locks are S (or IX) and IX (or S), respectively. A transaction U-lock is
     * mapped to an S-lock.
     *
     * @param heldModeByNode is the node lock mode held.
     * @param requestedModeByTx is the transaction lock mode.
     * @return the appropriate node lock mode givne the transaction lock mode.
     */
    lock_mode_t mapFileModeIfNecessary(lock_mode_t heldModeByNode,
        lock_mode_t requestedModeByTx);

    /**
     * Maps a transaction lock mode of a page to the appropriate node lock mode
     * as either: a) S-lock if the transaction lock mode requested is an IS-,
     * S-, or U-lock; and b) X-lock, otherwise.
     * @param requestedModeByTx is the transaction lock mode.
     * @return the appropriate node lock mode givne the transaction lock mode.
     */
    lock_mode_t mapPageModeIfNecessary(lock_mode_t requestedModeByTx) {
        return pageTxModeToNodeModeMap[requestedModeByTx - 1];
    }

    /* transport::Server */
    void deserializeRequestFromRecvBuffer(request_msg_t& request) override;
    void serializeResponseToSendBuffer(request_msg_t& response) override;

private:
    const lock_mode_t pageTxModeToNodeModeMap[6u] = {
    /* IS, S, IX, SIX, X, U */
        S, S,  X,   X, X, S
    };

    const bool isGlm;
    ObjIdToObjLocksMap objIdToObjLocksMap;
    const bool enableLogging;

    std::thread recvServerThread;
    std::thread sendServerThread;

    std::vector<std::thread> remoteGetNodeLockHandlerThreads;
    std::vector<TransactionLockPool> transactionLockPools;
    std::vector<ObjLocks> fileIdToFileLocksMap;

    std::shared_ptr<spdlog::logger> logger;
};

} // namespace storage
