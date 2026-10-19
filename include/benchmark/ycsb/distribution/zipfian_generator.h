#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <random>

#include "benchmark/ycsb/distribution_generator.h"

namespace benchmark {

/**
 * In a Zipfian distribution, we are assigning popularity to N integers in [1,N].
 * For integer k in [1,N], popularity is based on a weight proportional to
 * (1/K)^(theta) where 0 < theta < 1 is the skew.
 * The Zipfian distribution is characterized by the property that the frequency
 * distribution is a straight line when plotted on a log-log graph.
 * Not that this class does not spread the popularity throughout the [1,N] values
 * Use the PermutedZipfianGenerator which permutes the values randomly.
 */
class ZipfianGenerator : public DistributionGenerator<uint64_t> {
public:
    static constexpr double DEFAULT_THETA = 0.99;
    static constexpr uint64_t K_MAX_NUM_ITEMS = (UINT64_MAX >> 24);
                                             /* 1,099,511,627,775 */

    ZipfianGenerator(uint64_t N, double theta = DEFAULT_THETA) :
        ZipfianGenerator(0 /* min */, N - 1 /* max */, theta) {};

    ZipfianGenerator(uint64_t min, uint64_t max, double theta = DEFAULT_THETA);

    uint64_t next() override { return next(N); }
    uint64_t next(uint64_t numItems);

    uint64_t getLast() override {
        std::lock_guard<std::mutex> lock(mtx);
        return lastGenerated;
    }

private:
    /// Compute the zeta constant needed for the distribution.
    /// Remember the number of items, so if it is changed, we can recompute zeta.
    void RaiseZeta(uint64_t num) {
        assert(num >= nForZeta);
        zetaN = Zeta(nForZeta, num, theta, zetaN);
        nForZeta = num;
    }

    [[nodiscard]] double computeEta() const {
        return (1 - std::pow(2.0 / N, 1 - theta)) / (1 - zeta2 / zetaN);
    }

    /// Calculate the zeta constant needed for a distribution.
    /// Do this incrementally from the last_num of items to the cur_num.
    /// Use the zipfian constant as theta. Remember the new number of items
    /// so that, if it is changed, we can recompute zeta.
    static double Zeta(uint64_t last_num, uint64_t cur_num, double theta,
        double prevZeta) {
        double zeta = prevZeta;
        for (uint64_t i = last_num + 1; i <= cur_num; ++i) {
            zeta += 1 / std::pow(i, theta);
        }
        return zeta;
    }

    static double Zeta(uint64_t num, double theta) {
        return Zeta(0, num, theta, 0);
    }

    uint64_t N;
    uint64_t min;

    // Computed params for generating the distribution
    double theta, alpha, zetaN, zeta2, eta;
    uint64_t nForZeta;   // num of items used to compute zetaN
    uint64_t lastGenerated;
    std::mutex mtx;
};

} // namespace benchmark
