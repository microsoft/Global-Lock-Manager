#include <common/datatypes.h>

namespace common {

uint32_t getDataTypeSizeInBytes(DataType datatype) {
    switch (datatype)
    {
    case INT32_DT:
    case FLOAT_DT:
        return 4u;
    case INT64_DT:
    case DOUBLE_DT:
        return 8u;
    default: throw;
    }
}

} // namespace common
