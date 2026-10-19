#include <gtest/gtest.h>

#include <common/thread_pin.h>

using namespace common;

TEST(ObjIds, PinThreads) {
    auto processorCount = std::thread::hardware_concurrency();

    std::atomic<bool> stopServer{false};
    std::vector<std::thread> threads;
    threads.reserve(processorCount);
    for (auto core = 0u; core < processorCount; core++) {
        threads.emplace_back([&stopServer] { while (!stopServer); });
        pinThread(threads[core], core);
    }

    for (auto i = 0u; i < processorCount; i++) {
        ASSERT_EQ(i /* core */, getCoreThreadPinnedTo(threads[i]));
    }

    stopServer.store(true);
    for (auto i = 0u; i < processorCount; i++) {
        threads[i].join();
    }
}
