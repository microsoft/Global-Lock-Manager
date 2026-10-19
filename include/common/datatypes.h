#pragma once

#include <cstdint>

namespace common {

/* data types of a particular column */
enum DataType {
    INT32_DT,
    INT64_DT,
    FLOAT_DT,
    DOUBLE_DT,
    STRING_DT
};

uint32_t getDataTypeSizeInBytes(DataType datatype);

} // namespace common
