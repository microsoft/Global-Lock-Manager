#include <processor/processor.h>

#include <common/thread_pin.h>
#include <benchmark/query_generator.h>
#include <benchmark/ycsb/ycsb_query.h>

using namespace benchmark;

namespace processor {

Processor::Processor(bool isGlm, query::SystemAPI& storageAPI, uint32_t numThreads) :
        isGlm{isGlm}, system{storageAPI}, nextTxId{0u}, numThreads{numThreads},
        processQueries{false}, txsCommitted{std::make_unique<uint64_t[]>(numThreads)},
        txsAborted{std::make_unique<uint64_t[]>(numThreads)} {
    auto threadOffset = (isGlm ? 0 : numThreads); /* +
		6 /* 3 threads for transport * 2 * + 2 * storageAPI.
        lockManager.getNumRemoteGetNodeLockHandlerThreads(); */
    threads.reserve(numThreads);
    for (auto threadId = 0u; threadId < numThreads; ++threadId) {
        threads.emplace_back(&Processor::runWorkerThread, this, threadId);
        spdlog::info("thread id = {} & core to pin to = {}", threadId,
            threadId + threadOffset);
        pinThread(threads.back(), threadId + threadOffset /* coreToPinTo */);
    }
}

void Processor::startProcessingQueries() {
    processQueries.store(true);
}

void Processor::stopProcessingQueries() {
    processQueries.store(false);
    system.failInFlightTransactions();

    spdlog::info("query processor threads joining...");
    for (auto& thread : threads) {
        thread.join();
    }
    spdlog::info("query processor threads joined");
}

uint64_t Processor::getNextTxIdStartOffset() {
	auto txIdOffset = nextTxId.load();
    auto newTxIdOffset = txIdOffset + 1000000000000 /* 1T */;
    while(!nextTxId.compare_exchange_weak(txIdOffset, newTxIdOffset)) {
        newTxIdOffset  = txIdOffset + 1000000000000 /* 1T */;
	}
    return txIdOffset;
}

void Processor::runWorkerThread(uint32_t threadId) {
    auto localTxsCommitted = 0u;
    auto localTxsAborted = 0u;

    YcsbReadUpdateQuery query(threadId, 1000000 /* 1M */, 95u);

    query::TransactionContext context{nextTxId};
    query::Transaction tx{query, system, context};

    auto nextTxIdStartOffset = getNextTxIdStartOffset();
    auto nextTxIdEndOffset = nextTxIdStartOffset + 1000000000000u /* 1T */;

    while(!processQueries);

    do {
        spdlog::info("thread-{} has slice IDs [{},{})", threadId,
            nextTxIdStartOffset, nextTxIdEndOffset);
        for (; nextTxIdStartOffset < nextTxIdEndOffset; nextTxIdStartOffset++) {
            query.next();
            context.reset(nextTxIdStartOffset);
            while (!tx.run(threadId)) {
                localTxsAborted++;
                if (!processQueries) {
                    spdlog::info("thread-{} break 1", threadId);
                    break;
                }
            }
            if (!processQueries) {
                spdlog::info("thread-{} break 2", threadId); break; }
            localTxsCommitted++;
        }
        nextTxIdStartOffset = getNextTxIdStartOffset();
        nextTxIdEndOffset = nextTxIdStartOffset + 1000000000000 /* 1T */;
        spdlog::info("thread-{} break 3", threadId);
    } while (processQueries);

    txsCommitted[threadId] = localTxsCommitted;
    txsAborted[threadId] = localTxsAborted;
}

} // namespace processor
