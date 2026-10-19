#pragma once

#include <cstdint>

#include <benchmark/ycsb/distribution_generator.h>

namespace benchmark {

class ConstantGenerator : public DistributionGenerator<uint64_t> {
public:
    ConstantGenerator(uint64_t constant) : constant(constant) {}

    inline uint64_t next() override { return constant; }
    inline uint64_t getLast() override { return constant; }

private:
    uint64_t constant;
};

} // namespace benchmark
