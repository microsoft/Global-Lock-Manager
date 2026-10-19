#pragma once

#include <cstdint>

#include <benchmark/ycsb/distribution/zipfian_generator.h>

namespace benchmark {

constexpr uint64_t kFNVOffsetBasis64 = 0xCBF29CE484222325;
constexpr uint64_t kFNVPrime64 = 1099511628211;

inline uint64_t FNVHash64(uint64_t val);

class ScrambledZipfianGenerator : public DistributionGenerator<uint64_t> {
public:
    ScrambledZipfianGenerator(uint64_t N, double theta =
        ZipfianGenerator::DEFAULT_THETA) : N{N}, generator{0, theta} {}

    uint64_t next() override { return Scramble(generator.next()); }
    uint64_t getLast() override { return Scramble(generator.getLast()); }

private:
    const uint64_t N;
    ZipfianGenerator generator;

    uint64_t Scramble(uint64_t value) const { return FNVHash64(value) % N; }
};

uint64_t FNVHash64(uint64_t val) {
    uint64_t hash = kFNVOffsetBasis64;

    for (int i = 0; i < 8; i++) {
        uint64_t octet = val & 0x00ff;
        val = val >> 8;

        hash = hash ^ octet;
        hash = hash * kFNVPrime64;
    }
    return hash;
}

} // namespace benchmark
