#pragma once

#include <cstdint>

namespace benchmark {

template <typename T>
class DistributionGenerator {
public:
    virtual T next() = 0;
    virtual T getLast() = 0;
    virtual ~DistributionGenerator() = default;
};

} // namespace benchmark
