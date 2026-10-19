#pragma once

#include <query/system_api.h>
#include <query/transaction/transaction.h>

namespace processor {

/**
 * The processor executes transactions using a fixed number of threads.
 */
class Processor {
public:

    /**
     * Constructor.
     *
     * @param isGlm specifies if the processor is running on a glm node.
     * @param storageAPI is a reference to a shared system required
     * by transactions to access storage functionality.
     * @param numThreads is the number of threads executing transactions.
     */
    Processor(bool isGlm, query::SystemAPI& system, uint32_t numThreads);

    /**
     * All threads are blocked before the 'execution loop' and only start
     * executing transactions once this method is called.
     */
    void startProcessingQueries();
    
    /**
     * Once all threads are within the 'execution loop', they would break from
     * the loop once this method is called.
     */
    void stopProcessingQueries();

    /**
     * @return in a thread-safe manner the number of txs committed at the time
     * of the call.
     */
    uint64_t getNumTxsCommitted(uint64_t id) { return txsCommitted[id]; }

    uint64_t getNumTxsCommitted() {
        auto numTxsCommitted = 0u;
        for (auto i = 0; i < numThreads; i++) {
            numTxsCommitted += txsCommitted[i];
        }
        return numTxsCommitted;
    }

    /**
     * @return in a thread-safe manner the number of aborts at the time of the
     * call.
     */
    uint64_t getNumTxsAborted(uint64_t id) { return txsAborted[id];   }

private:
    /**
     * The method executed by each transaction thread. The method within a loop
     * obtains a transaction from a workload generator which it then repeatedly
     * executes till it is committed.
     */
    void runWorkerThread(uint32_t threadId);

    /**
     * Used by transaction threads to obtain a slice of 1000000000000 IDs to use.
     * @return the first ID of the slice obtained.
     */
    uint64_t getNextTxIdStartOffset();

private:
    const bool isGlm;
    query::SystemAPI& system;

    std::atomic<uint64_t> nextTxId;
    const uint64_t numThreads;
    std::atomic<bool> processQueries;
    
    std::unique_ptr<uint64_t[]> txsCommitted;
    std::unique_ptr<uint64_t[]> txsAborted;

    std::vector<std::thread> threads;
};

} // namespace processor
