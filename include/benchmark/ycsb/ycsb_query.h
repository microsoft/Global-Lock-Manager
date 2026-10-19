#pragma once

#include <cstdint>

#include <benchmark/ycsb/distribution/zipfian_generator.h>
#include <benchmark/ycsb/distribution/uniform_generator.h>
#include <query/query.h>

using namespace common;
using namespace query;

namespace benchmark {

/**
 * YCSB is the Yahoo! Cloud Serving Benchmark for systems addressing "Cloud
 * OLTP" applications i.e., cloud data serving that differ from traditional
 * serving workloads e.g., TPC-C.
 * There is a table of records, each with F fields. Each record is identified
 * by a primary key, which is a string like 'user234123'. The values of each
 * field are a random string of ASCII characters of length L. In the paper,
 * 1000 byte records are constructed with F = 10 fields, each of which
 * L = 100 bytes. Operations are similar to 'CRUD' operations and can be either:
 *  i)  insert: insert a new record.
 *  ii) update: update a record by replacing the value of one field.
 *  iii)  read: Read a record, either one randomly chosen field or all fields.
 *  iv)   scan: Scan records in order, starting at a randomly chosen record key.
 *              The number of records to scan is randomly chosen.
 *  YCSB uses several distributions: i) Uniform; ii) Zipfian; iii) latest; and
 *  iv) Multinomial. Distributions are assigned to the two main choices we must
 *  make: which operation to perform, and which record to read or write.
 *                       - Benchmarking Cloud Serving Systems with YCSB, SoCC'10
 */
enum YCSB_OPERATION : uint8_t {
    YCSB_INSERT = 0u,
    YCSB_UPDATE = 1u,
    YCSB_READ_SINGLE_FIELD = 2u,
    YCSB_READ_ALL_FIELDS   = 3u,
    YCSB_SCAN = 4u,
};

enum YCSB_WORKLOAD : uint8_t {
    A_UPDATE_HEAVY = 0u, /* Read:  50%, Update: 50% - Zipfian for key selection
      - Session store recording recent actions in a user session */
    B_READ_HEAVY   = 1u, /* Read:  95%, Update:  5% - Zipfian for key selection
      - Photo tagging: add tag is an update but most operations are read tag */
    C_READ_ONLY    = 2u, /* Read: 100% - Zipfian for key selection
      - User profile cache, where profiles are constructed elsewhere */
    D_READ_LATEST  = 3u, /* Read:  95%, Insert:  5% - Latest for key selection
      - User status updates; people want to read the latest updates */
    E_SHORT_RANGES = 4u, /* Scan:  95%, Insert:  5%
      - Zipfian for 1st key in range, uniform for # records to scan
      - Threaded conversations, where each scan is for the posts in a given
        thread (assumed to be clustered by thread id) */

    // More general workloads:
    READ_UPDATE = 5u,
    READ_INSERT = 6u,
    SCAN_INSERT = 7u,
};

constexpr uint32_t F = 5; // fields
constexpr uint32_t L = 4; // bytes each
constexpr uint32_t NUM_RECORDS_PER_PAGE = PAGE_SIZE / (F*L);
constexpr uint64_t INITIAL_NUM_RECORDS = 100000; // 100k records -> 1 KB

class YcsbReadUpdateQuery : public Query {
public:
    YcsbReadUpdateQuery(uint32_t coreId, uint64_t numRecords,
        uint32_t leftOperationPercentage);

    void next() override;

    bool run(SystemAPI& storageAPI, TransactionContext& txContext,
             uint32_t threadId) override;

protected:
    file_id_par_t fileIdPar;
    std::unique_ptr<UniformGenerator> operationGenerator;
    std::unique_ptr<UniformGenerator> keyHotColdGenerator;
    std::unique_ptr<UniformGenerator> hotKeyGenerator;
    std::unique_ptr<UniformGenerator> coldKeyGenerator;
    uint32_t leftOperationPercentage;
    bool isLeftOperation;
    std::uint64_t key;
    page_id_t pageId;
    page_id_t recordId;
};

} // namespace benchmark
