#pragma once

#include <cstdint>

#include <benchmark/ycsb/distribution/counter_generator.h>
#include <benchmark/ycsb/distribution/zipfian_generator.h>

namespace benchmark {

class SkewedLatestGenerator : public DistributionGenerator<uint64_t> {
public:
    SkewedLatestGenerator(CounterGenerator &counter) :
            basis(counter), zipfian(basis.getLast()) {
        next();
    }

    uint64_t next() override;
    uint64_t getLast() override { return lastGenerated; }

private:
    CounterGenerator &basis;
    ZipfianGenerator zipfian;
    std::atomic<uint64_t> lastGenerated;
};

inline uint64_t SkewedLatestGenerator::next() {
    uint64_t max = basis.getLast();
    return lastGenerated = max - zipfian.next(max);
};

} // namespace benchmark
