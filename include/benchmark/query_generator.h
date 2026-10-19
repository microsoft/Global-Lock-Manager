#pragma once

#include <cstdint>
#include <query/query.h>
#include <benchmark/ycsb/distribution/zipfian_generator.h>

namespace benchmark {

enum WORKLOAD : uint8_t {
    TPCC                   = 0u,
    YCSB_A_UPDATE_HEAVY    = 1u, /* (Read:  50%, Update: 50%)
        - Zipfian for key selection
        - Session store recording recent actions in a user session */
    YCSB_B_READ_HEAVY      = 2u, /* (Read:  95%, Update:  5%)
        - Zipfian for key selection
        - Photo tagging: add tag as update but most operations are read tag */
    YCSB_C_READ_ONLY       = 3u, /* (Read: 100%, Update:  0%)
        - Zipfian for key selection
        - User profile cache, where profiles are constructed elsewhere */
    /* Other (read, update) ratios */
    YCSB_READ_90_UPDATE_10 = 4u,
    YCSB_READ_85_UPDATE_15 = 5u,
    YCSB_READ_80_UPDATE_20 = 6u,
    YCSB_READ_75_UPDATE_25 = 7u,
    YCSB_READ_70_UPDATE_30 = 8u,
    YCSB_READ_65_UPDATE_35 = 9u,
    YCSB_READ_60_UPDATE_40 = 10u,
    YCSB_READ_55_UPDATE_45 = 11u,
    YCSB_READ_45_UPDATE_55 = 12u,
    YCSB_READ_40_UPDATE_60 = 13u,
    YCSB_READ_35_UPDATE_65 = 14u,
    YCSB_READ_30_UPDATE_70 = 15u,
    YCSB_READ_25_UPDATE_75 = 16u,
    YCSB_READ_20_UPDATE_80 = 17u,
    YCSB_READ_15_UPDATE_85 = 18u,
    YCSB_READ_10_UPDATE_90 = 19u,
    YCSB_READ_05_UPDATE_95 = 20u,

    YCSB_D_READ_LATEST     = 21u, /* (Read: 95%, Insert: 5%)
        - Latest for key selection
        - User status updates; people want to read the latest updates */
    /* Other (read, insert) ratios */
    YCSB_READ_90_INSERT_10 = 22u,
    YCSB_READ_85_INSERT_15 = 23u,
    YCSB_READ_80_INSERT_20 = 24u,
    YCSB_READ_75_INSERT_25 = 25u,
    YCSB_READ_70_INSERT_30 = 26u,
    YCSB_READ_65_INSERT_35 = 27u,
    YCSB_READ_60_INSERT_40 = 28u,
    YCSB_READ_55_INSERT_45 = 29u,
    YCSB_READ_50_INSERT_40 = 30u,
    YCSB_READ_45_INSERT_55 = 31u,
    YCSB_READ_40_INSERT_60 = 32u,
    YCSB_READ_35_INSERT_65 = 33u,
    YCSB_READ_30_INSERT_70 = 34u,
    YCSB_READ_25_INSERT_75 = 35u,
    YCSB_READ_20_INSERT_80 = 36u,
    YCSB_READ_15_INSERT_85 = 37u,
    YCSB_READ_10_INSERT_90 = 38u,
    YCSB_READ_05_INSERT_95 = 39u,

    YCSB_E_SHORT_RANGES    = 40u, /* (Scan: 95%, Insert: 5%)
        - Zipfian for 1st key in range, uniform for # records to scan
        - Threaded conversations, where each scan is for the posts in a given
          thread (assumed to be clustered by thread id) */

    /* Other (scan, insert) ratios */
    YCSB_SCAN_90_INSERT_10 = 41u,
    YCSB_SCAN_85_INSERT_15 = 42u,
    YCSB_SCAN_80_INSERT_20 = 43u,
    YCSB_SCAN_75_INSERT_25 = 44u,
    YCSB_SCAN_70_INSERT_30 = 45u,
    YCSB_SCAN_65_INSERT_35 = 46u,
    YCSB_SCAN_60_INSERT_40 = 47u,
    YCSB_SCAN_55_INSERT_45 = 48u,
    YCSB_SCAN_50_INSERT_50 = 49u,
    YCSB_SCAN_45_INSERT_55 = 50u,
    YCSB_SCAN_40_INSERT_60 = 51u,
    YCSB_SCAN_35_INSERT_65 = 52u,
    YCSB_SCAN_30_INSERT_70 = 53u,
    YCSB_SCAN_25_INSERT_75 = 54u,
    YCSB_SCAN_20_INSERT_80 = 55u,
    YCSB_SCAN_15_INSERT_85 = 56u,
    YCSB_SCAN_10_INSERT_90 = 57u,
    YCSB_SCAN_05_INSERT_95 = 58u,

    YCSB_UPDATE_100        = 59u,
    YCSB_INSERT_100        = 60u,
};

/**
 * The query generator.
 */
class QueryGenerator {
public:
    explicit QueryGenerator(WORKLOAD workload) : workload{workload} {}

    std::unique_ptr<query::Query> generateYcsbQuery(uint64_t numRecords,
        double theta = ZipfianGenerator::DEFAULT_THETA);

private:
    WORKLOAD workload;
};

} // namespace benchmark
