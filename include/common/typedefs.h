#pragma once

#include <chrono>
#include <cstdint>

namespace common {

typedef uint64_t tx_id_t;
typedef uint64_t page_lsn_t;
typedef uint64_t frame_idx_t;

typedef uint8_t  column_t;

/**
 * Timestamps are assigned to transactions. A timestamp is the time since epoch
 * in nanoseconds. Our GLM and non-GLM processes run on the same node. As such,
 * using this timestamp is okay for now since it is the same clock, there would
 * be no drift and we would have to change it the moment we start running on two
 * different machines.
 *
 * 18,446,744,073,709,551,615 <- MAX_UINT64_T
 *              1,000,000,000 <- 1  sec
 *             60,000,000,000 <- 1  min
 *            360,000,000,000 <- 1  hr
 *          8,640,000,000,000 <- 1  day
 *  8,640,000,000,000,000,000 <- 1M days (2.7K years)
 */
typedef uint64_t timestamp_t;

inline timestamp_t getTimestamp() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

/** 
 * An 'id' is a unique identifier for an object: 1) file; 2) page;
 * or 3) a record. All ids of an object contained in another rely
 * on the ids of its ancestors in the object hierarchy which is:
 * file -> page -> record.
 *
 * An id is an uint64_t value split as follows:
 *   -----------------------------------------------------------
 *   | obj. type |  6-bits  | 21-bits |    5-bits              |
 *   |-----------|----------|---------|------------------------|
 *   |  file_id  | 0b000000 | file_id | coreId = core % #cores |
 *   -----------------------------------------------------------
 *
 *   -------------------------------------------------------
 *   | obj. type | 21-bits |    32 bits   |    11-bits     |
 *   |-----------|---------|--------------|----------------|
 *   |  page_id  | file_id | page_idx + 1 |         0x0000 |
 *   | record_id | file_id | page_idx + 1 | record_idx + 1 |
 *   -------------------------------------------------------
 *
 * idx indicates the relative pos of a page (or record) in a file (or a page).
 */

typedef uint64_t obj_id_t;
typedef uint64_t file_id_t;
typedef uint32_t file_id_par_t;
typedef uint32_t file_idx_t; /* only lowest 21-bits are considered */
typedef uint64_t page_id_t;
typedef uint32_t page_idx_t;
typedef uint64_t record_id_t;
typedef uint16_t record_idx_t; /* only lowest 11-bits are considered */

inline bool isFile(uint64_t id)   { return (id & 0x0000111111111111) == 0u; }
inline bool isRecord(uint64_t id) { return (id & 0x0000111100000000) >= 1u; }
inline bool isPage(uint64_t id)   { return (id & 0x0000000011111111) >= 1u; }

inline file_id_t getFileId(file_idx_t fidx) {
    // # hardware cores is assumed to be a power of two.
    return (((uint64_t) (fidx)) << 43);
}

inline file_id_par_t getFileIdPartitioned(file_idx_t fidx, uint64_t coreId) {
    // # hardware cores is assumed to be a power of two.
    return (((uint32_t) (fidx)) << 5) | coreId;
}

inline page_id_t getPageId(file_idx_t fidx, page_idx_t pidx) {
    return getFileId(fidx) | (((uint64_t) (pidx + 1)) << 11);
}

inline record_id_t getRecordId(page_id_t pid, record_idx_t ridx) {
    return pid | ((uint64_t) (ridx + 1));
}

} // namespace common
