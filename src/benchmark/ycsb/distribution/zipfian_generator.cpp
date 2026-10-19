#include <benchmark/ycsb/distribution/zipfian_generator.h>

namespace benchmark {

inline double RandomDouble(double min = 0.0, double max = 1.0) {
    static std::default_random_engine generator;
    static std::uniform_real_distribution<double> uniform(min, max);
    return uniform(generator);
}

ZipfianGenerator::ZipfianGenerator(uint64_t min, uint64_t max, double theta) :
    N{max - min + 1}, min{min}, theta(theta), zetaN(0), nForZeta(0) {
    assert(N >= 2 && N < K_MAX_NUM_ITEMS);
    zeta2 = Zeta(2, theta);
    alpha = 1.0 / (1.0 - theta);
    RaiseZeta(N);
    eta = computeEta();

    next();
}

inline uint64_t ZipfianGenerator::next(uint64_t num) {
    assert(num >= 2 && num < K_MAX_NUM_ITEMS);
    std::lock_guard<std::mutex> lock(mtx);

    if (num > nForZeta) { // Recompute zeta_n and eta.
        RaiseZeta(num);
        eta = computeEta();
    }

    double u = RandomDouble();
    double uz = u * zetaN;

    if (uz < 1.0) {
        return lastGenerated = 0;
    }

    if (uz < 1.0 + std::pow(0.5, theta)) {
        return lastGenerated = 1;
    }

    return lastGenerated = min + num * std::pow(eta * u - eta + 1, alpha);
}

} // namespace benchmark
