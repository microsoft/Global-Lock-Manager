#include "compute_node.h"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        spdlog::info("Five arguments expected: ");
        spdlog::info("1. nodeId hosting the processor;");
        spdlog::info("2. absolute path to the logs dir ending in '/';");
        spdlog::info("3. length of experiment in secs;");
        spdlog::info("4. number query processor threads; and");
        spdlog::info("5. number remote GetNodeLock request handler threads");
        return 0;
    }

    uint8_t nodeId = stoi(std::string(argv[1]));
    auto isGlm = nodeId == 0u || nodeId == 1u;
    auto absolutePathLogsDir = strcmp(argv[2], "NULL") == 0 ? nullptr :
                               argv[2];
    if (absolutePathLogsDir == nullptr) {
        spdlog::info("No file logging");
    }
    uint64_t experimentLenInSecs = stoi(std::string(argv[3]));
    uint32_t numQueryProcessorThreads = stoi(std::string(argv[4]));
    uint32_t numNodeLockRequestHandlers = stoi(std::string(argv[5]));

    spdlog::info("Hardware Concurrency: {}", std::thread::hardware_concurrency());

    auto hwConcurrency = std::thread::hardware_concurrency() * 2;
    hwConcurrency = hwConcurrency > 0 ? hwConcurrency : 2;
    auto numProcThreads = std::min(hwConcurrency, numQueryProcessorThreads);
    spdlog::info("# Query Processor threads: {}", numProcThreads);

    BufferManager bufferMan;
    LogManager logMan;
    LockManager lockManager(isGlm, numProcThreads, numNodeLockRequestHandlers,
        1 /* #files */, absolutePathLogsDir, nodeId >= 1u /* initTransport */);

    SystemAPI system(bufferMan, lockManager, logMan, absolutePathLogsDir);
    Processor processor(isGlm, system, numProcThreads);

	spdlog::info("sleeping for 1 secs pre-query-processing.");
    std::this_thread::sleep_for(std::chrono::seconds(10));

    processor.startProcessingQueries();
    std::this_thread::sleep_for(
        std::chrono::nanoseconds(experimentLenInSecs * 1000000000));

    processor.stopProcessingQueries();

    spdlog::info("sleeping for 10 msecs post-query-processing.");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    lockManager.shutdown();

    auto tpsPerCore = (processor.getNumTxsCommitted() /
       (double) experimentLenInSecs) / numProcThreads;
    spdlog::info("#TPS/core : {}", tpsPerCore);
    for (auto i = 0; i < numProcThreads; i++) {
        spdlog::info("#TPS - tid_{} : {}", i,
            (processor.getNumTxsCommitted(i) / (double) experimentLenInSecs));
    }
    lockManager.log();

    return 0;
}
