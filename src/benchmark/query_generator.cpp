#include "benchmark/query_generator.h"

#include "benchmark/ycsb/ycsb_query.h"

using namespace query;
using namespace std;

namespace benchmark {

unique_ptr<Query> QueryGenerator::generateYcsbQuery(uint64_t numRecords,
    double theta) {
    spdlog::info("theta generator = {}", theta);
    switch (workload) {
        case YCSB_A_UPDATE_HEAVY:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 50u, theta);
        case YCSB_B_READ_HEAVY:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 95u, theta);
        case YCSB_C_READ_ONLY:
            return make_unique<YcsbReadUpdateQuery>(numRecords,100u, theta);
        case YCSB_READ_90_UPDATE_10:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 90u, theta);
        case YCSB_READ_85_UPDATE_15:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 85u, theta);
        case YCSB_READ_80_UPDATE_20:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 80u, theta);
        case YCSB_READ_75_UPDATE_25:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 75u, theta);
        case YCSB_READ_70_UPDATE_30:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 70u, theta);
        case YCSB_READ_65_UPDATE_35:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 65u, theta);
        case YCSB_READ_60_UPDATE_40:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 60u, theta);
        case YCSB_READ_55_UPDATE_45:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 55u, theta);
        case YCSB_READ_45_UPDATE_55:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 45u, theta);
        case YCSB_READ_40_UPDATE_60:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 40u, theta);
        case YCSB_READ_35_UPDATE_65:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 35u, theta);
        case YCSB_READ_30_UPDATE_70:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 30u, theta);
        case YCSB_READ_25_UPDATE_75:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 25u, theta);
        case YCSB_READ_20_UPDATE_80:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 20u, theta);
        case YCSB_READ_15_UPDATE_85:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 15u, theta);
        case YCSB_READ_10_UPDATE_90:
            return make_unique<YcsbReadUpdateQuery>(numRecords, 10u, theta);
        case YCSB_READ_05_UPDATE_95:
            return make_unique<YcsbReadUpdateQuery>(numRecords,  5u, theta);
        case YCSB_D_READ_LATEST:
        case YCSB_READ_90_INSERT_10:
        case YCSB_READ_85_INSERT_15:
        case YCSB_READ_80_INSERT_20:
        case YCSB_READ_75_INSERT_25:
        case YCSB_READ_70_INSERT_30:
        case YCSB_READ_65_INSERT_35:
        case YCSB_READ_60_INSERT_40:
        case YCSB_READ_55_INSERT_45:
        case YCSB_READ_50_INSERT_40:
        case YCSB_READ_45_INSERT_55:
        case YCSB_READ_40_INSERT_60:
        case YCSB_READ_35_INSERT_65:
        case YCSB_READ_30_INSERT_70:
        case YCSB_READ_25_INSERT_75:
        case YCSB_READ_20_INSERT_80:
        case YCSB_READ_15_INSERT_85:
        case YCSB_READ_10_INSERT_90:
        case YCSB_READ_05_INSERT_95:
        case TPCC:
        case YCSB_E_SHORT_RANGES:
        case YCSB_SCAN_90_INSERT_10:
        case YCSB_SCAN_85_INSERT_15:
        case YCSB_SCAN_80_INSERT_20:
        case YCSB_SCAN_75_INSERT_25:
        case YCSB_SCAN_70_INSERT_30:
        case YCSB_SCAN_65_INSERT_35:
        case YCSB_SCAN_60_INSERT_40:
        case YCSB_SCAN_55_INSERT_45:
        case YCSB_SCAN_50_INSERT_50:
        case YCSB_SCAN_45_INSERT_55:
        case YCSB_SCAN_40_INSERT_60:
        case YCSB_SCAN_35_INSERT_65:
        case YCSB_SCAN_30_INSERT_70:
        case YCSB_SCAN_25_INSERT_75:
        case YCSB_SCAN_20_INSERT_80:
        case YCSB_SCAN_15_INSERT_85:
        case YCSB_SCAN_10_INSERT_90:
        case YCSB_SCAN_05_INSERT_95:
        case YCSB_UPDATE_100:
        case YCSB_INSERT_100:
        default: throw;
    }
}

} // namespace benchmark
