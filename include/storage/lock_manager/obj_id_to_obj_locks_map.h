#pragma once

#include <vector>

#include "obj_locks.h"

#include <spdlog/spdlog.h>

namespace storage {

//template <typename key_t>
class ObjIdToObjLocksMap {
public:
    ObjIdToObjLocksMap();

    ObjLocks* get(obj_id_t objId);

    ObjLocks* getElseInsert(obj_id_t objId);

    void remove(obj_id_t objId) { /* TODO: implement */ }

    void log() {
        // Print the type of research!
        spdlog::info("- numAllocs: {}", numAllocs);
        spdlog::info("- numNonAllocs: {}", numNonAllocs);
        spdlog::info("- numDirectLookup: {}", numDirectLookup);
        spdlog::info("- numSearchChaining: {}", numSearchChaining);
    }

private:
    key_t bitmask;
    std::atomic<uint64_t> numAllocs;
    std::atomic<uint64_t> numNonAllocs;
    std::atomic<uint64_t> numDirectLookup;
    std::atomic<uint64_t> numSearchChaining;
    std::hash<key_t> hasher;
    std::vector<ObjLocks> buckets;
};

//template class IdToObjLocksMap<obj_id_t>;
//template class IdToObjLocksMap<file_id_par_t>;

} // namespace storage
