#include <query/system_api.h>

#include <spdlog/sinks/basic_file_sink.h>

namespace query {

SystemAPI::SystemAPI(BufferManager& bufferManager, LockManager& lockManager,
    LogManager& logManager, const char* absolutePathToLogDir) :
    bufferManager{bufferManager}, lockManager{lockManager}, logManager{logManager},
    enableLogging{absolutePathToLogDir != nullptr} {
    if (enableLogging) {
        auto filename = lockManager.isGLM() ? "storage_api_glm.log" :
                                              "storage_api_nonGlm.log";
        auto absolutePathToLogFile = new char[strlen(absolutePathToLogDir) +
            strlen(filename) + 1];
        strcpy(absolutePathToLogFile, absolutePathToLogDir);
        strcat(absolutePathToLogFile, filename);
        logger = spdlog::basic_logger_mt("storage_api-logger",
            absolutePathToLogFile);
    }
}

bool SystemAPI::getFilePageAndSingleRecord(TransactionContext& context,
    file_id_par_t fileId, page_id_t pageId, record_id_t recordId,
    lock_mode_t fileLockMode, lock_mode_t recordLockMode, uint32_t tid) {
    if (!lockManager.lock(fileId, fileLockMode, context.ID, context.timestamp, tid)) {
        return false;
    }
    return lockManager.lock(recordId, recordLockMode, context.ID, context.timestamp, tid);
}

void SystemAPI::fetchPage(Frame* frame, page_lsn_t pageLSN) {
    std::this_thread::sleep_for(std::chrono::milliseconds(PAGE_FETCH_TIME_IN_MS));
    frame->needFetchFromRemote = false;
}

void SystemAPI::commit(TransactionContext& context, uint32_t txThreadId) {
    uint8_t* commitRecord = nullptr;
    logManager.append(commitRecord, sizeof(uint32_t) /* recordSizeInBytes */);
}

void SystemAPI::abort(TransactionContext& context, uint32_t txThreadId) {
    uint8_t* abortRecord = nullptr;
    logManager.append(abortRecord, sizeof(uint32_t) /* recordSizeInBytes */);
}

} // namespace query
