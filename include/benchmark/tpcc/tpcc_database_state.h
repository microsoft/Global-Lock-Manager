#pragma once

#include "query/database/database_state.h"

using namespace common;

namespace benchmark {

class DatabaseStateTPCC : public DatabaseState {
public:
    DatabaseStateTPCC() : DatabaseStateTPCC(100, 100000) {}
    DatabaseStateTPCC(uint32_t numWarehouses, uint32_t numItems) :
        numWarehouses{numWarehouses}, numItems{numItems} {}

    uint32_t getNumFiles() override { return 10; };

    uint32_t getNumRecords(file_id_t table) override;
    uint32_t getNumRecordsPerPage(file_id_t table) override;
    uint32_t getRecordSizeInBytes(file_id_t table) override;

    DataType getColumnDataType(column_t column) override;
    uint32_t getColumnSizeInBytes(column_t column) override;
    uint32_t getStringColumnLen(column_t stringColumn) override;

    uint32_t getPageIdx(file_id_t table, uint32_t primaryKeyValue) override;
    uint32_t getRecordIdx(file_id_t table, uint32_t primaryKeyValue) override;

private:
    uint32_t numWarehouses;
    uint32_t numItems;
};

enum TPCC_Table : file_id_t {
    TAB_WAREHOUSE = 0,
    TAB_ITEM      = 1,
    TAB_STOCK     = 2,
    TAB_DISTRICT  = 3,
    TAB_CUSTOMER  = 4,
    TAB_HISTORY   = 5,
    TAB_OPENORDER = 6,
    TAB_ORDERLINE = 7,
    TAB_NEWORDER  = 8,
};

enum TPCC_Column : column_t {
    /* TAB_WAREHOUSE */
    W_ID           =  0, /* PK */
    W_NAME         =  1,
    W_STREET_1     =  2,
    W_STREET_2     =  3,
    W_CITY         =  4,
    W_STATE        =  5,
    W_ZIP          =  6,
    W_TAX          =  7,
    W_YTD          =  8,
    /* TAB_ITEM */
    I_ID           =  9, /* PK */
    I_IM_ID        = 10,
    I_NAME         = 11,
    I_PRICE        = 12,
    I_DATA         = 13,
    /* TAB_STOCK */
    S_I_ID         = 14, /* PK, FK: I_ID */
    S_W_ID         = 15, /* PK, FK: W_ID */
    S_QUANTITY     = 16,
    S_DIST_01      = 17,
    S_DIST_02      = 18,
    S_DIST_03      = 19,
    S_DIST_04      = 20,
    S_DIST_05      = 21,
    S_DIST_06      = 22,
    S_DIST_07      = 23,
    S_DIST_08      = 24,
    S_DIST_09      = 25,
    S_DIST_10      = 26,
    S_YTD          = 27,
    S_ORDER_CNT    = 28,
    S_REMOTE_CNT   = 29,
    S_DATA         = 30,
    /* TAB_DISTRICT */
    D_ID           = 31, /* PK */
    D_W_ID         = 32, /* PK, FK: W_ID */
    D_NAME         = 33,
    D_STREET_1     = 34,
    D_STREET_2     = 35,
    D_CITY         = 36,
    D_STATE        = 37,
    D_ZIP          = 38,
    D_TAX          = 39,
    D_YTD          = 40,
    D_NEXT_O_ID    = 41,
    /* TAB_CUSTOMER */
    C_ID           = 42, /* PK */
    C_D_ID         = 43, /* PK, FK: D_ID */
    C_W_ID         = 44, /* PK, FK: D_W_ID */
    C_FIRST        = 45,
    C_MIDDLE       = 46,
    C_LAST         = 47,
    C_STREET_1     = 48,
    C_STREET_2     = 49,
    C_CITY         = 50,
    C_STATE        = 51,
    C_ZIP          = 52,
    C_PHONE        = 53,
    C_SINCE        = 54,
    C_CREDIT       = 55,
    C_CREDIT_LIM   = 56,
    C_DISCOUNT     = 57,
    C_BALANCE      = 58,
    C_YTD_PAYMENT  = 59,
    C_PAYMENT_CNT  = 60,
    C_DELIVERY_CNT = 61,
    C_DATA         = 62,
    /* TAB_HISTORY */
    H_C_ID         = 63, /* FK: C_ID   */
    H_C_D_ID       = 64, /* FK: C_D_ID */
    H_C_W_ID       = 65, /* FK: C_W_ID */
    H_D_ID         = 66, /* FK: D_ID   */
    H_W_ID         = 67, /* FK: D_W_ID */
    H_DATE         = 68,
    H_AMOUNT       = 69,
    H_DATA         = 70,
    /* TAB_OPENORDER */
    O_ID           = 71, /* PK */
    O_C_ID         = 72, /*     FK: C_ID   */
    O_D_ID         = 73, /* PK, FK: C_D_ID */
    O_W_ID         = 74, /* PK, FK: C_W_ID */
    O_ENTRY_D      = 75,
    O_CARRIER_ID   = 76,
    O_OL_CNT       = 77,
    O_ALL_LOCAL    = 78,
    /* TAB_ORDERLINE */
    OL_O_ID        = 79, /* PK, FK: O_ID   */
    OL_D_ID        = 80, /* PK, FK: O_D_ID */
    OL_W_ID        = 81, /* PK, FK: O_W_ID */
    OL_NUMBER      = 82, /* PK */
    OL_I_ID        = 83, /*     FK: S_I_ID */
    OL_SUPPLY_W_ID = 84, /*     FK: S_W_ID */
    OL_DELIVERY_D  = 85,
    OL_QUANTITY    = 86,
    OL_AMOUNT      = 87,
    OL_DIST_INFO   = 88,
    /* TAB_NEWORDER */
    NO_O_ID        = 89, /* FK: O_ID   */
    NO_D_ID        = 90, /* FK: O_D_ID */
    NO_W_ID        = 91, /* FK: O_W_ID */
};

} // namespace benchmark
