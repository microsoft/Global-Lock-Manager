#pragma once

#include <cstdint>
#include <random>

#include <benchmark/ycsb/distribution_generator.h>

namespace benchmark {

// Both min and max are inclusive
class UniformGenerator : public DistributionGenerator<uint64_t> {
public:
    UniformGenerator(uint64_t min, uint64_t max) : distribution{min, max}
        {}

    inline uint64_t next() override {
        return lastGenerated = distribution(generator);
    }

    inline uint64_t getLast() override { return lastGenerated; }

private:
    std::uniform_int_distribution<uint64_t> distribution;
    std::mt19937 generator;
    uint64_t lastGenerated;
};

} // namespace benchmark
