#include <common/thread_pin.h>

#include <spdlog/spdlog.h>

namespace common {

void pinThread(std::thread& thread, uint32_t coreToPinTo) {
    // cpu_set_t represents a set of physical cores
    cpu_set_t cpuSet;
    // initially, cpu_set does not contain any cores.
    CPU_ZERO(&cpuSet);
    // we add the input core to the CPU set.
    CPU_SET(coreToPinTo, &cpuSet);

    auto rc = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t),
        &cpuSet);
    if (rc != 0) {
        spdlog::error("Error calling pthread_setaffinity_np: ={}", rc);
    }
}

uint32_t getCoreThreadPinnedTo(std::thread& thread) {
    cpu_set_t cpuSet;
    auto rc = pthread_getaffinity_np(thread.native_handle(), sizeof(cpu_set_t),
        &cpuSet);
    if (rc != 0) {
        spdlog::error("Error calling pthread_getaffinity_np: ={}", rc);
        return -1;
    }

    auto processorCount = std::thread::hardware_concurrency();
    for (auto core = 0u; core < processorCount; core++) {
        if (CPU_ISSET(core, &cpuSet)) {
            return core;
        }
    }
    throw;
}

} // namespace common
