#pragma once

#include <atomic>
#include <cstdint>

#include <benchmark/ycsb/distribution_generator.h>

namespace benchmark {

class CounterGenerator : public DistributionGenerator<uint64_t> {
public:
    CounterGenerator(uint64_t start) : counter{start} {}

    inline uint64_t next() override { return counter.fetch_add(1); }
    inline uint64_t getLast() override { return counter.load() - 1; }
    void set(uint64_t start) { counter.store(start); }

private:
    std::atomic<uint64_t> counter;
};

} // namespace benchmark
