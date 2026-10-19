#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <random>
#include <vector>

#include <benchmark/ycsb/distribution_generator.h>

namespace benchmark {

template <typename T>
class DiscreteGenerator : public DistributionGenerator<T> {
public:
    DiscreteGenerator() : sum(0) { }
    void addValue(T value, double weight);

    T next();
    T getLast() { return lastGenerated; }

private:
    std::vector<std::pair<T, double>> values;
    double sum;
    std::atomic<T> lastGenerated;
    std::mutex mtx;
};

template <typename T>
inline void DiscreteGenerator<T>::addValue(T value, double weight) {
    if (values.empty()) {
        lastGenerated = value;
    }
    values.push_back(std::make_pair(value, weight));
    sum += weight;
}

template <typename T>
inline T DiscreteGenerator<T>::next() {
    mtx.lock();
    static std::default_random_engine generator;
    static std::uniform_real_distribution<double> uniform(0.0 /* min */, 1.0 /* max */);
    double chooser = uniform(generator);
    mtx.unlock();

    for (auto p = values.cbegin(); p != values.cend(); ++p) {
        if (chooser < p->second / sum) {
            return lastGenerated = p->first;
        }
        chooser -= p->second / sum;
    }

    throw;
}

} // namespace benchmark
