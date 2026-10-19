#pragma once

#include "common/config.h"
#include "query/database/database_state.h"
#include <benchmark/ycsb/ycsb_query.h>

using namespace common;

namespace query {

class DatabaseStateYCSB : public DatabaseState {
public:
    DatabaseStateYCSB() : numRecordsPerPage{PAGE_SIZE / (F * L)} {}

    page_id_t getPageId(uint32_t pkValue) {
        return common::getPageId(0 /* fileId */,
            (pkValue / numRecordsPerPage) /* recordIdx */);
    }

    page_id_t getRecordId(uint32_t pkValue) {
        return common::getPageId(0 /* fileId */,
            (pkValue % numRecordsPerPage) /* recordIdx */);
    }

private:
    uint32_t numRecordsPerPage;
};

} // namespace query
