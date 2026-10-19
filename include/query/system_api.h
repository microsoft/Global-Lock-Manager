#pragma once

#include <storage/buffer_manager/buffer_manager.h>
#include <storage/lock_manager/lock_manager.h>
#include <storage/log_manager/log_manager.h>

#include <query/database/database_state.h>
#include <query/transaction/transaction_context.h>

using namespace storage;

namespace query {

/**
 * The storage API provides generic facilities to read and update records while
 * abstracting the details of the storage i.e., buffer manager, lock manager, 
 * and log manager.
 */
class SystemAPI {
public:
    SystemAPI(BufferManager& bufferManager, LockManager& lockManager,
              LogManager& logManager, const char* absolutePathToLogDir = nullptr);

    bool readRecord(TransactionContext& context, file_id_par_t fileId,
        page_id_t pageId, record_id_t recordId, uint32_t threadId) {
        return getFilePageAndSingleRecord(context, fileId, pageId, recordId, IS,
            S, threadId);
    }

    bool updateRecord(TransactionContext& context, file_id_par_t fileId,
        page_id_t pageId, record_id_t recordId, uint32_t threadId) {
        return getFilePageAndSingleRecord(context, fileId, pageId, recordId, IX,
            X, threadId);
    }

    void fetchPage(Frame* frame, page_lsn_t pageLSN = UINT64_MAX);
    void commit(TransactionContext& txContext, uint32_t txThreadId);
    void abort(TransactionContext& txContext, uint32_t txThreadId);

    void failInFlightTransactions() {
        // lockManager.denyRequestsForTearDown();
    }

private:
    bool getFilePageAndSingleRecord(TransactionContext& context,
        file_id_par_t fileIdPar, page_id_t pageId, record_id_t recordId,
        lock_mode_t fileLockMode, lock_mode_t recordLockMode, uint32_t tid);

public:
    BufferManager& bufferManager;
    LockManager& lockManager;
    LogManager& logManager;

    const bool enableLogging;
    std::shared_ptr<spdlog::logger> logger;
};

} // namespace query
