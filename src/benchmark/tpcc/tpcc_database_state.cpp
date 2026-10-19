#include <benchmark/tpcc/tpcc_database_state.h>

#include "common/config.h"

using namespace common;

namespace benchmark {

uint32_t DatabaseStateTPCC::getPageIdx(file_id_t table, uint32_t key) {
    return key / getNumRecordsPerPage(table);
}

uint32_t DatabaseStateTPCC::getRecordIdx(file_id_t table, uint32_t key) {
    return key % getNumRecordsPerPage(table);
}

uint32_t DatabaseStateTPCC::getNumRecords(file_id_t table) {
    switch (table)
    {
    case TAB_WAREHOUSE:
        return numWarehouses;
    case TAB_ITEM:
        return numItems;
    case TAB_STOCK:
        // Each warehouse corresponds to the stock data of 100,000 items.
        return numWarehouses * 100000;
    case TAB_DISTRICT:
        // Each warehouse provides services to 10 districts.
        return numWarehouses * 10;
    case TAB_CUSTOMER:
        // Each district has 3,000 customers.
        return numWarehouses * 10 * 3000;
    case TAB_HISTORY:
        // Each customer has one transaction history.
        return numWarehouses * 10 * 3000;
    case TAB_OPENORDER:
        // Each district has 3,000 orders.
        return numWarehouses * 10 * 3000;
    case TAB_ORDERLINE:
        // Each order randomly generates 5 ~ 15 ORDER-LINE records.
        return numWarehouses * 10 * 3000 * /* 5 ~ 15 midpoint */ 10;
    case TAB_NEWORDER:
        // The last 900 orders generated are added as new orders.
        return 900;
    default: throw;
    }
}

uint32_t DatabaseStateTPCC::getNumRecordsPerPage(file_id_t table) {
    return PAGE_SIZE / getRecordSizeInBytes(table);
}

uint32_t DatabaseStateTPCC::getRecordSizeInBytes(file_id_t table) {
    switch (table)
    {
    case TAB_WAREHOUSE:
        return 105;
    case TAB_ITEM:
        return 98;
    case TAB_STOCK:
        return 338;
    case TAB_DISTRICT:
        return 121;
    case TAB_CUSTOMER:
        return 703;
    case TAB_HISTORY:
        return 80;
    case TAB_OPENORDER:
        return 64;
    case TAB_ORDERLINE:
        return 80;
    case TAB_NEWORDER:
        return 24;
    default: throw;
    }
}

DataType DatabaseStateTPCC::getColumnDataType(column_t column) {
    switch (column)
    {
    case W_ID:
    case D_ID:
    case D_W_ID:
    case D_NEXT_O_ID:
    case C_ID:
    case C_D_ID:
    case C_W_ID:
    case C_SINCE:
    case C_CREDIT_LIM:
    case C_DISCOUNT:
    case C_PAYMENT_CNT:
    case C_DELIVERY_CNT:
    case H_C_ID:
    case H_C_D_ID:
    case H_C_W_ID:
    case H_D_ID:
    case H_W_ID:
    case H_DATE:
    case NO_O_ID:
    case NO_D_ID:
    case NO_W_ID:
    case O_ID:
    case O_C_ID:
    case O_D_ID:
    case O_W_ID:
    case O_ENTRY_D:
    case O_CARRIER_ID:
    case O_OL_CNT:
    case O_ALL_LOCAL:
    case OL_O_ID:
    case OL_D_ID:
    case OL_W_ID:
    case OL_NUMBER:
    case OL_I_ID:
    case OL_SUPPLY_W_ID:
    case OL_DELIVERY_D:
    case OL_QUANTITY:
    case OL_DIST_INFO:
    case I_ID:
    case I_IM_ID:
    case I_PRICE:
    case S_I_ID:
    case S_W_ID:
    case S_QUANTITY:
    case S_YTD:
    case S_ORDER_CNT:
    case S_REMOTE_CNT:
        return INT64_DT;

    case W_TAX:
    case W_YTD:
    case D_TAX:
    case D_YTD:
    case C_BALANCE:
    case C_YTD_PAYMENT:
    case H_AMOUNT:
    case OL_AMOUNT:
        return DOUBLE_DT;

    case W_NAME:
    case W_STREET_1:
    case W_STREET_2:
    case W_CITY:
    case W_STATE:
    case W_ZIP:
    case D_NAME:
    case D_STREET_1:
    case D_STREET_2:
    case D_CITY:
    case D_STATE:
    case D_ZIP:
    case C_FIRST:
    case C_MIDDLE:
    case C_LAST:
    case C_STREET_1:
    case C_STREET_2:
    case C_CITY:
    case C_STATE:
    case C_ZIP:
    case C_PHONE:
    case C_CREDIT:
    case C_DATA:
    case H_DATA:
    case I_NAME:
    case I_DATA:
    case S_DIST_01:
    case S_DIST_02:
    case S_DIST_03:
    case S_DIST_04:
    case S_DIST_05:
    case S_DIST_06:
    case S_DIST_07:
    case S_DIST_08:
    case S_DIST_09:
    case S_DIST_10:
    case S_DATA:
        return STRING_DT;
    }
    throw;
}

uint32_t DatabaseStateTPCC::getColumnSizeInBytes(column_t column) {
    auto datatype = getColumnDataType(column);
    if (datatype == STRING_DT) {
        return getStringColumnLen(column);
    }
    return getDataTypeSizeInBytes(datatype);
}

uint32_t DatabaseStateTPCC::getStringColumnLen(column_t stringColumn) {
    switch (stringColumn)
    {
    case W_STATE:
    case D_STATE:
    case C_MIDDLE:
    case C_STATE:
    case C_CREDIT:
        return 2;
    case W_ZIP:
    case D_ZIP:
    case C_ZIP:
        return 9;
    case W_NAME:
    case D_NAME:
        return 10;
    case C_FIRST:
    case C_LAST:
    case C_PHONE:
        return 16;
    case W_STREET_1:
    case W_STREET_2:
    case W_CITY:
    case D_STREET_1:
    case D_STREET_2:
    case D_CITY:
    case C_STREET_1:
    case C_STREET_2:
    case C_CITY:
        return 20;
    case H_DATA:
    case I_NAME:
    case S_DIST_01:
    case S_DIST_02:
    case S_DIST_03:
    case S_DIST_04:
    case S_DIST_05:
    case S_DIST_06:
    case S_DIST_07:
    case S_DIST_08:
    case S_DIST_09:
    case S_DIST_10:
        return 24;
    case I_DATA:
    case S_DATA:
        return 50;
    case C_DATA:
        return 500;
    }
    throw;
}

} // namespace benchmark
