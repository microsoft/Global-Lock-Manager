#include <storage/lock_manager/obj_id_to_obj_locks_map.h>

#include <common/config.h>

#include <spdlog/spdlog.h>

using namespace common;

namespace storage {

ObjIdToObjLocksMap::ObjIdToObjLocksMap() : bitmask(NUM_LOCK_MANAGER_SLOTS - 1),
    numAllocs{0u}, numNonAllocs{0u}, numDirectLookup{0u}, numSearchChaining{0u}, hasher{} {
    buckets.reserve(NUM_LOCK_MANAGER_SLOTS);
    for (auto i = 0u; i < NUM_LOCK_MANAGER_SLOTS; i++) {
        buckets.emplace_back();
    }
    for (auto i = 0u; i < NUM_LOCK_MANAGER_SLOTS; i++) {
        if (buckets[i].objId != UINT64_MAX) {
            spdlog::info("buckets[{}].objId = {} != UINT64_MAX ", i, buckets[i].objId);
        }
    }
}

ObjLocks* ObjIdToObjLocksMap::get(obj_id_t objId) {
    auto offset = std::hash<obj_id_t>{}(objId) & bitmask;
    ObjLocks* curr = &buckets[offset];
    curr->spinlock.lock();
    while (curr->objId != objId) {
        curr->next->spinlock.lock();
        curr->spinlock.unlock();
        curr = curr->next.get();
    }
    curr->spinlock.unlock();
    return curr;
}

ObjLocks* ObjIdToObjLocksMap::getElseInsert(obj_id_t objId) {
    auto offset = hasher(objId) & bitmask;
    ObjLocks* curr = &buckets[offset];
    curr->spinlock.lock();
    if (curr->objId == UINT64_MAX) {
        curr->objId = objId;
        curr->spinlock.unlock();
        numNonAllocs++;
    } else {
        if (curr->objId == objId) {
            curr->spinlock.unlock();
            numDirectLookup++;
            return curr;
        }
        while (curr->next != nullptr) {
            curr->next->spinlock.lock();
            curr->spinlock.unlock();
            curr = curr->next.get();
            if (curr->objId == objId) {
                curr->spinlock.unlock();
                numSearchChaining++;
                return curr;
            }
        }
        //spdlog::info("(objId, offset) ({}, {}) required heap allocation!",
        //    objId, offset);
        curr->next = std::make_unique<ObjLocks>(objId);
        curr->spinlock.unlock();
        curr = curr->next.get();
        numAllocs++;
    }
    return curr;
}

} // namespace storage
