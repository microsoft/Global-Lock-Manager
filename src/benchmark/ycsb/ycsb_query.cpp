#include <benchmark/ycsb/ycsb_query.h>

namespace benchmark {

YcsbReadUpdateQuery::YcsbReadUpdateQuery(uint32_t coreId, uint64_t numRecords,
    uint32_t leftOperationPercentage) :
    fileIdPar{getFileIdPartitioned(0u /* fidx */, coreId)},
    operationGenerator{std::make_unique<UniformGenerator>(1, 100)},
    keyHotColdGenerator{std::make_unique<UniformGenerator>(1, 100)},
    hotKeyGenerator{std::make_unique<UniformGenerator>(
            /*  5% : */  1, (uint64_t) (numRecords * 0.05))},
    coldKeyGenerator{std::make_unique<UniformGenerator>(
            /* 95% : */  (uint64_t) (numRecords * 0.05) + 1, numRecords)},
    leftOperationPercentage{leftOperationPercentage}, isLeftOperation{true} {
    next();
}

void YcsbReadUpdateQuery::next() {
    auto isHot = keyHotColdGenerator->next() <= 50;
    isLeftOperation = operationGenerator->next() <= leftOperationPercentage;
    key = isHot ? hotKeyGenerator->next() : coldKeyGenerator->next();
    pageId = getPageId(0u /* fidx */, key / NUM_RECORDS_PER_PAGE /* pageIdx */);
    recordId = getRecordId(pageId, key % NUM_RECORDS_PER_PAGE /* recordIdx */);
}

bool readRecord(SystemAPI& systemAPI, TransactionContext& context,
    uint32_t threadId, file_id_par_t fileIdPar, page_id_t pageId,
    record_id_t recordId);

bool updateRecord(SystemAPI& system, TransactionContext& context,
    uint32_t threadId, file_id_par_t fileIdPar, page_id_t pageId,
    record_id_t recordId);

bool YcsbReadUpdateQuery::run(SystemAPI& system, TransactionContext& context,
    uint32_t threadId) {
    return isLeftOperation ?
        readRecord(system, context, threadId, fileIdPar, pageId, recordId) :
        updateRecord(system, context, threadId, fileIdPar, pageId, recordId);
}

bool readRecord(SystemAPI& system, TransactionContext& context,
    uint32_t threadId, file_id_par_t fileIdPar, page_id_t pageId,
    record_id_t recordId) {
    auto success = system.readRecord(context, fileIdPar, pageId, recordId, threadId);
    if (success) {
        system.lockManager.unlock(recordId, context.ID, S, threadId);
    }
    system.lockManager.unlock(fileIdPar, context.ID, IS, threadId);
    return success;
}

bool updateRecord(SystemAPI& system, TransactionContext& context,
    uint32_t threadId, file_id_par_t fileIdPar, page_id_t pageId,
    record_id_t recordId) {
    auto success = system.updateRecord(context, fileIdPar, pageId, recordId, threadId);
    if (success) {
        system.lockManager.unlock(recordId, context.ID, X, threadId);
    }
    system.lockManager.unlock(fileIdPar, context.ID, IX, threadId);
    return success;
}

} // namespace benchmark
