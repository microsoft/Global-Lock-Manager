#pragma once

#include <query/query.h>

namespace query {

constexpr uint32_t NUM_QUERY_TYPES = 2;

enum QUERY_TYPE : uint8_t {
    READ_RECORD = 0u,
    UPDATE_RECORD = 1u,
};

/* uint64_t CDFLikelihoodPerQueryType[2] = {
     READ_RECORD:   [ 0, 49] * 50,
     UPDATE_RECORD: [50, 99] * 100
};*/

class WorkloadGenerator {
public:
    virtual Query& getQuery() = 0;
};

} // namespace query
