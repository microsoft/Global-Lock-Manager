#pragma once

#include <cstdint>

namespace benchmark {

/**
 * From "https://www.tpc.org/tpcc/":
 * TPC Benchmark C is an on-line transaction processing (OLTP) benchmark. TPC-C
 * involves a mix of five concurrent transactions of different types and
 * complexity either executed on-line or queued for deferred execution.
 * The database is comprised of nine types of tables with a wide range of record
 * and population sizes. TPC-C is measured in transactions per min (tpmC). The
 * benchmark portrays the activity of a wholesale supplier.
 * 
 * TPC-C queries can be found in tpcc_queries.txt and are grouped as follows: */
enum TPCC_QUERY : uint8_t {
    TPCC_DELIVERY     = 0u,
    TPCC_NEW_ORDER    = 1u,
    TPCC_ORDER_STATUS = 2u,
    TPCC_PAYMENT      = 3u,
    TPCC_STOCK_LEVEL  = 4u,
};

} // namespace benchmark
