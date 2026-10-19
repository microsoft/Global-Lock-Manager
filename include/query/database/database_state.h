#pragma once

#include <common/datatypes.h>
#include <common/typedefs.h>

namespace common {

/**
 * A database state simulates the metadata information for a stored database.
 */
class DatabaseState {
public:
    virtual ~DatabaseState() = default;

    virtual uint32_t getNumFiles() { throw; };

    virtual uint32_t getNumRecords(file_id_t table) { throw; };
    virtual uint32_t getNumRecordsPerPage(file_id_t table) { throw; };
    virtual uint32_t getRecordSizeInBytes(file_id_t table) { throw; };

    virtual DataType getColumnDataType(column_t column) { throw; };
    virtual uint32_t getColumnSizeInBytes(column_t column) { throw; };
    virtual uint32_t getStringColumnLen(column_t stringColumn) { throw; };

    virtual uint32_t getPageIdx(file_id_t table, uint32_t key) { throw; };
    virtual uint32_t getRecordIdx(file_id_t table, uint32_t key) { throw; };
};

} // namespace common
