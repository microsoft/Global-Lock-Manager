#include <storage/lock_manager/lock_manager.h>

#include <chrono>

#include <spdlog/sinks/basic_file_sink.h>

#include <common/thread_pin.h>
#include <common/wait_time.h>

namespace storage {

LockManager::LockManager(bool isGlm, uint32_t numTxThreads,
    uint32_t numNodeLockRequestHandlers, uint32_t numFiles,
    const char* absolutePathToLogDir, bool initTransport /* = true */) :
    Transport((int) (isGlm ? GLM_PORT : NON_GLM_PORT) /* port */,
              REQUEST_MSG_SIZE  /* recvServerBufSize   */,
              RESPONSE_MSG_SIZE /* sendServerBufSize   */,
              numTxThreads      /* maxNumRequests      */,
              "127.0.0.1"       /* run locally for now */,
        isGlm ? NON_GLM_PORT : GLM_PORT /* serverPort */,
              numTxThreads      /* numClientSendThreads */,
              REQUEST_MSG_SIZE  /* sendClientBufferSize */,
              RESPONSE_MSG_SIZE /* recvClientBufferSize */,
              isGlm /* initializeServerThenClient */,
              absolutePathToLogDir, initTransport),
    isGlm{isGlm}, objIdToObjLocksMap{},
    enableLogging{absolutePathToLogDir != nullptr},
    recvServerThread{&LockManager::recvThreadFunc, this},
    sendServerThread{&LockManager::sendThreadFunc, this} {
    pinThread(recvServerThread, isGlm ? 2 : 3 /* coreToPinTo */);
    pinThread(sendServerThread, isGlm ? 4 : 5 /* coreToPinTo */);
    if (initTransport) {
        auto offset = isGlm ? 6 : 6 + numNodeLockRequestHandlers;
        for (auto i = 0u; i < numNodeLockRequestHandlers; i++) {
            remoteGetNodeLockHandlerThreads.emplace_back(
                &LockManager::remoteGetNodeLockHandlerThreadFunc, this);
            pinThread(remoteGetNodeLockHandlerThreads.back(),
                i + offset /* coreToPinTo */);
        }
    }

    transactionLockPools.reserve(numTxThreads);
    for (auto i = 0u; i < numTxThreads; i++) {
        transactionLockPools.emplace_back(10u /* capacity per buffer */);
    }

    fileIdToFileLocksMap.reserve(numFiles + 35);
    for (file_id_t fileId = 0u; fileId < numFiles + 35u; fileId++) {
        fileIdToFileLocksMap.emplace_back(fileId);
    }

    if (enableLogging) {
        auto filename = isGlm ? "glm.log" : "nonGlm.log" ;
        auto absolutePathToLogFile = new char[strlen(absolutePathToLogDir) +
            strlen(filename) + 1];
        strcpy(absolutePathToLogFile, absolutePathToLogDir);
        strcat(absolutePathToLogFile, filename);
        logger = spdlog::basic_logger_mt("lock-manager-logger",
            absolutePathToLogFile);
    }
}

void LockManager::shutdown() {
    Server::stopServer.store(true);
    spdlog::info("shutting down {} server...", (isGlm ? "glm" : "non-glm"));
    spdlog::info("recv server thread joining...");
    recvServerThread.join();
    spdlog::info("recv server thread joined");
    spdlog::info("send server thread joining...");
    sendServerThread.join();
    spdlog::info("send server thread joined");

    Client::stopClient.store(true);
    spdlog::info("shutting down {} client...", (isGlm ? "glm" : "non-glm"));
    shutdownClient();
    spdlog::info("receiving client thread joining...");
    receivingThread.join();
    spdlog::info("receiving client thread joined");

    for (auto i = 0u; i < remoteGetNodeLockHandlerThreads.size(); i++) {
        requestsQueue.push({});
    }
    spdlog::info("remote GetNodeLock handler threads joining...");
    for (auto& t : remoteGetNodeLockHandlerThreads) {
        t.join();
    }
    spdlog::info("remote GetNodeLock handler threads joined");

    if (enableLogging) {
        logger->flush();
    }
}

bool LockManager::getFileNodeLock(file_id_t fileId,
    lock_mode_t nodeMode, timestamp_t timestamp) {
    return getNodeLock(fileId , &fileIdToFileLocksMap[fileId],
        nodeMode, timestamp);
}

bool LockManager::getPageNodeLock(page_id_t pageId,
    lock_mode_t nodeMode, timestamp_t timestamp) {
    return getNodeLock(pageId, objIdToObjLocksMap.getElseInsert(pageId),
        nodeMode, timestamp);
}

bool LockManager::getNodeLock(obj_id_t objId, ObjLocks* objLocks,
    lock_mode_t nodeMode, timestamp_t timestamp) {
    auto nodeLock = isGlm ? &(objLocks->glmNodeLock) :
                            &(objLocks->nonGlmNodeLock);
    /* if (nodeLock->areTxsBlocked()) {
        std::unique_lock<std::mutex> lock(nodeLock->blockMtx);
        if (nodeLock->areTxsBlocked()) {
            nodeLock->blockCondVar.wait(lock, [&nodeLock] {
                return !nodeLock->areTxsBlocked();
            });
        }
    } */
    while (nodeLock->areTxsBlocked()); // Remove the sleeping in between.

    auto getNodeLockResponse = localGetNodeLock(objId, objLocks, nodeMode, timestamp);
    while (getNodeLockResponse == LOCAL_REQ_SENT) {
        std::unique_lock<std::mutex> lock(nodeLock->localRequestMtx);
        getNodeLockResponse = localGetNodeLock(objId, objLocks, nodeMode, timestamp);
        if (getNodeLockResponse == LOCAL_REQ_SENT) {
            nodeLock->localRequestCondVar.wait(lock,
                [&nodeLock] { return nodeLock->isHeld(); });
        }
    }

    auto isSuccessful = getNodeLockResponse == SUCCESS;
    if (!isSuccessful) {
        nodeLock->unregisterTx();
    }
    return isSuccessful;
}

std::atomic<uint32_t> txsExecutedSinceLastReq = 0u;
std::atomic<uint32_t> numRequests = 0u;

GET_NODE_LOCK_RESPONSE LockManager::localGetNodeLock(obj_id_t objId,
    ObjLocks* objLocks, lock_mode_t nodeModeReq, timestamp_t txTimestamp) {
    auto glmLock = &(objLocks->glmNodeLock);
    auto nonGlmLock = &(objLocks->nonGlmNodeLock);
    auto nodeLockGranted = SUCCESS;
    // While the implementation below makes the protocol clear, we repeat the
    // node-level locking protol in plain english. Obtaining a node lock is
    // dependent on whether the LM is a GLM or not.
    if (isGlm) {
        // If the GLM does not hold a node lock on the object or if it owns a
        // less restrictive node lock, we need to actually get the node lock.
        // When getting the node lock, if the other node does not hold the lock,
        // the GLM is directly granted or upgraded to the nodeLockModeReq.
        // Otherwise, if the non-GLM holds a node lock that is not compatible
        // with the nodeLockModeReq, a remote GetNodeLock is sent to the
        // non-GLM to either release its node lock or downgrade it such that it
        // becomes compatible with the nodeLockModeReq.
        if (!IsCompatible::forNodeLockAndTx(glmLock->getHeldLockMode(), nodeModeReq)) {
            /*if (enableLogging) {
                logger->info("Requesting node lock for obj {} after {} txs executed : {}---{}",
                    objId, txsExecutedSinceLastReq, LockModeAsStr[glmLock->getHeldLockMode()],
                        LockModeAsStr[nodeModeReq]);
                txsExecutedSinceLastReq = 0u;
            }*/
            if (!glmLock->setRequestedLockMode(nodeModeReq)) {
                // Another thread is already making the request.
                nodeLockGranted = LOCAL_REQ_SENT;
                if (enableLogging) {
                    logger->info("Request failed a local req is already sent");
                }
            } else {
                std::unique_lock<std::mutex> lock(glmLock->remoteRequestMtx);
                glmLock->requestTimestamp = getTimestamp();
                if (!glmLock->isHeld() || !IsCompatible::forNodeLockAndTx(
                    nonGlmLock->getHeldLockMode(), nodeModeReq)) {
                    if (nonGlmLock->isHeld() &&
                        !IsCompatible::forTxs(nonGlmLock->getHeldLockMode(), nodeModeReq) &&
                        !remoteGetNodeLock(objLocks, nodeModeReq, txTimestamp)) {
                        nodeLockGranted = REMOTE_REQ_FAILED;
                    } else if (!nonGlmLock->isHeld()) {
                        glmLock->setHeldLockMode(nodeModeReq);
                    }
                }
                glmLock->requestTimestamp = UINT64_MAX;
                glmLock->setRequestedLockMode(NONE);
            }
			//txsExecutedSinceLastReq = 0u;
        } /* else {
            txsExecutedSinceLastReq++;
        } */
        /* else if (enableLogging) {
            logger->info("Node lock for obj {} already obtained", objId);
        } */
    } else if /* non-GLM && */ (
        !IsCompatible::forNodeLockAndTx(nonGlmLock->getHeldLockMode(), nodeModeReq)) {
        /* if (enableLogging) {
            logger->info("Requesting node lock for obj {} after {} txs  executed",
                objId, txsExecutedSinceLastReq); */
            // txsExecutedSinceLastReq = 0u;
        // }
        // If the non-GLM does not hold a node lock on the object or if it owns
        // a less restrictive node lock, a remote GetNodeLock is sent to the GLM
        // unless another thread already initiated a request regardless of the
        // requested mode.
        if (!nonGlmLock->setRequestedLockMode(nodeModeReq)) {
            nodeLockGranted = LOCAL_REQ_SENT;
        } else {
            std::unique_lock<std::mutex> lock(nonGlmLock->remoteRequestMtx);
            nonGlmLock->requestTimestamp = getTimestamp();
            nodeLockGranted = remoteGetNodeLock(objLocks, nodeModeReq, txTimestamp);
            nonGlmLock->requestTimestamp = UINT64_MAX;
            nonGlmLock->setRequestedLockMode(NONE);
        }
	//	txsExecutedSinceLastReq = 0u;
    } /*else {
        txsExecutedSinceLastReq++;
    }*/
	txsExecutedSinceLastReq++;
    /* else if (enableLogging) {
        logger->info("Node lock for obj {} already obtained", objId);
    } */
    return nodeLockGranted;
}

GET_NODE_LOCK_RESPONSE LockManager::remoteGetNodeLock(ObjLocks* objLocks,
    lock_mode_t nodeModeReq, timestamp_t requestTimestamp) {

    auto slotIdx = getRequestSlotIdx(objLocks->objId);
    /* if (enableLogging) {
        logger->info("({}, {}) - assigned slot: {}", objLocks->objId,
            LockModeAsStr[nodeModeReq], slotIdx);
    } */
    auto requestSlot = requestSlots[slotIdx].get();

    auto sendBuffer = (uint64_t*) requestSlot->sendBuffer.get();
    sendBuffer[0u] = objLocks->objId;
    sendBuffer[1u] = requestTimestamp;
    requestSlot->sendBuffer[16u] = nodeModeReq;

    if (enableLogging) {
        logger->info("Remote node lock request SENT (objId: {}, LockMode: {}) "
            "- {} txs executed and {} num sent requests",
            objLocks->objId, getLockModeAsStr(nodeModeReq),
            txsExecutedSinceLastReq, numRequests++);
    }
    sendRequest(requestSlot, slotIdx);

    // We save the response status before releasing the slot.
    auto responseStatus = requestSlot->responseStatus;
	waitInNanosecs(NODE_LOCK_REQUEST_MIN_LATENCY_IN_MICROSEC * 1000);
    if (enableLogging) {
        logger->info("Remote node lock request BACK (objId: {}, LockMode: {}) "
            "- {} txs executed", objLocks->objId, getLockModeAsStr(nodeModeReq),
            txsExecutedSinceLastReq);
    }

    if (responseStatus == response_status_t::SUCCESS) {
        // TODO: add pageLSN info to the responsePayload +
        //       set objLocks.glmNodeLock->pageLSN &
        //           objLocks.nonGlmNodeLock->pageLSN.
        auto obtainTimestamp = getTimestamp();
        auto durationInNanosecs = NODE_LOCK_HELD_DURATION_DEAD_TIME_MULTIPLIER *
            (obtainTimestamp - requestSlot->responseServerTimestamp);
        if (isGlm) {
            objLocks->glmNodeLock.setHeldLockMode(nodeModeReq);
            objLocks->glmNodeLock.obtainTimestamp = obtainTimestamp;
            objLocks->glmNodeLock.durationInNanosecs = durationInNanosecs;
            objLocks->nonGlmNodeLock.setHeldLockMode(NONE);
        } else {
            objLocks->nonGlmNodeLock.setHeldLockMode(nodeModeReq);
            objLocks->nonGlmNodeLock.obtainTimestamp = obtainTimestamp;
            objLocks->nonGlmNodeLock.durationInNanosecs = durationInNanosecs;
        }
        logger->info("(obtainTimestamp: {}, durationInNanosecs: {})",
            obtainTimestamp, durationInNanosecs);

        /*if (enableLogging) {
            logNodeLocksHeld(objLocks, true - isClient *);
        }*/
    }
    releaseRequestSlot(requestSlot, slotIdx);

    auto nodeLock = isGlm ? &(objLocks->glmNodeLock) : &(objLocks->nonGlmNodeLock);
    std::unique_lock<std::mutex> lock(nodeLock->localRequestMtx);
    nodeLock->localRequestCondVar.notify_all();

    return responseStatus == response_status_t::SUCCESS ?
        GET_NODE_LOCK_RESPONSE::SUCCESS :
        GET_NODE_LOCK_RESPONSE::REMOTE_REQ_FAILED;
}

void LockManager::remoteGetNodeLockHandlerThreadFunc() {
    request_msg_t msg;
    do { 
        while (!requestsQueue.pop(msg));
        if (msg.objId == UINT64_MAX) {
            spdlog::info("remoteGetNodeLockHandler signalled to stop.");
            break;
        }

        // We sleep to simulate the network delay between the nodes.
        waitInNanosecs(NODE_LOCK_REQUEST_MIN_LATENCY_IN_MICROSEC * 1000);

        auto nodeModeReq = (lock_mode_t) msg.payload;
        if (enableLogging) {
            logger->info("Handling (objId:{}, LockMode:{}) ...", msg.objId,
                getLockModeAsStr(nodeModeReq));
        }

        auto objLocks = isFile(msg.objId) ? &fileIdToFileLocksMap[msg.objId] :
            objIdToObjLocksMap.getElseInsert(msg.objId);
        auto nodeLock = isGlm ? &(objLocks->glmNodeLock) :
            &(objLocks->nonGlmNodeLock);

        std::unique_lock<std::mutex> remoteRequestlock
            { nodeLock->remoteRequestMtx, std::defer_lock };

        bool handleReq = true;
        while (!remoteRequestlock.try_lock()) {
            auto timestampByNode = nodeLock->requestTimestamp.load();
            if (timestampByNode != UINT64_MAX &&
                    timestampByNode < msg.timestamp) {
                if (enableLogging) {
                    logger->info("(objId:{}, Timestamp: {} > {}, LockMode:{})"
                        " failed due to wait-die", msg.objId, msg.timestamp,
                        timestampByNode, getLockModeAsStr(nodeModeReq));
                }
                handleReq = false;
                msg.payload = FAILURE;
                while (!responsesQueue.push(msg));
                break;
            }
        }

        if (handleReq && isGlm && IsCompatible::forTxs(
                objLocks->glmNodeLock.getHeldLockMode(), nodeModeReq)) {
            if (enableLogging) {
                logger->info("(objId:{}, LockMode:{}) is granted as compatible",
                    msg.objId, getLockModeAsStr(nodeModeReq));
            }
            objLocks->nonGlmNodeLock.setHeldLockMode(nodeModeReq);
            handleReq = false;
            msg.payload = SUCCESS;
            while (!responsesQueue.push(msg));
        }

        if (handleReq) {
            waitInNanosecs(nodeLock->obtainTimestamp,
                nodeLock->durationInNanosecs);

            // First, we block all transactions from progressing with
            // lock requests for object with ID objId.
            nodeLock->blockTxs();
            /*if (enableLogging) {
                logger->info("(objId:{}, LockMode:{}) just blocked txs {}",
                    msg.objId, LockModeAsStr[nodeModeReq], txsExecutedSinceLastReq);
            }*/
            // If the request is by the GLM to a non-GLM then it is requesting
            // the non-GLM to release its node lock. However, if the request
            // is by the non-GLM to the GLM, it is either granted the node lock
            // as the GLM does not hold it or holds a compatible node lock, we
            // grant the node lock directly. Otherwise, we have to resolve the
            // conflict as follows. If no active transactions hold a lock on
            // the object, we directly grant the lock (this can only happen for
            // a non-GLM. A GLM would only send a remote GetNodeLock if the 
            // non-GLM held a non-compatible lock.
            while (!nodeLock->isEmpty());

            if (isGlm) {
                objLocks->glmNodeLock.setHeldLockMode(NONE);
                objLocks->nonGlmNodeLock.setHeldLockMode(nodeModeReq);
            } else {
                objLocks->nonGlmNodeLock.setHeldLockMode(NONE);
            }

            // std::unique_lock<std::mutex> lock(nodeLock->blockMtx);
            msg.payload = SUCCESS;
            msg.timestamp = getTimestamp();
            while (!responsesQueue.push(msg));
            nodeLock->unblockTxs();
            if (enableLogging) {
                logger->info("(objId:{}, LockMode:{}) is granted after blocking and {} txs executed",
                    msg.objId, getLockModeAsStr(nodeModeReq), txsExecutedSinceLastReq);
                // logNodeLocksHeld(objLocks, false /* isClient */);
            }
            // nodeLock->blockCondVar.notify_all();
        }
    } while (!stopServer);
}

void LockManager::deserializeRequestFromRecvBuffer(request_msg_t& response) {
    response.objId = ((uint64_t*) recvBuffer.get())[0u];
    response.timestamp = ((uint64_t*) recvBuffer.get())[1u];
    response.payload = recvBuffer[16u];
}

void LockManager::serializeResponseToSendBuffer(request_msg_t& response) {
    ((uint64_t*) sendBuffer.get())[0u] = response.objId;
    ((uint64_t*) sendBuffer.get())[1u] = response.timestamp;
    sendBuffer[16u] = response.payload;
}

lock_mode_t mapFileModeIfNecessary(lock_mode_t heldModeByNode,
    lock_mode_t requestedModeByTx) {
    if (requestedModeByTx == U) {
        return S;
    } else if ((heldModeByNode == S && requestedModeByTx == IX) ||
        (heldModeByNode == IX && requestedModeByTx == S)) {
        return SIX;
    }
    return requestedModeByTx;
}

} // namespace storage
