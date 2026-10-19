#pragma once

#include <cstdint>

#include <common/typedefs.h>
#include <storage/lock_manager/locks/lock_mode.h>

namespace common {

/* Generic */
constexpr uint32_t CACHE_LINE = 64; /* or 128 */

/* Compute */
constexpr uint32_t NUM_LOCK_MANAGER_SLOTS = 1u << 24; /* 16,777,216
                                            larger prime 16,777,259 */

/* Storage */
constexpr uint64_t PAGE_SIZE =    4 /* KBs   */ *
                               1024 /* bytes */;
constexpr uint64_t BUFFER_POOL_SIZE =    1 /* GB */ *
                                      1024 /* MB */ *
                                      1024 /* KB */ *
                                      1024 /* bytes */;
constexpr uint32_t PAGE_FETCH_TIME_IN_MS = 2 /* msec */;

/* Transport */
constexpr int GLM_PORT     = 27015;
constexpr int NON_GLM_PORT = 27016;

constexpr uint32_t NODE_LOCK_REQUEST_MIN_LATENCY_IN_MICROSEC    =  10u;
constexpr uint32_t NODE_LOCK_HELD_DURATION_DEAD_TIME_MULTIPLIER = 100u;
constexpr uint32_t REQUEST_MSG_SIZE  = /* 17 bytes */
    sizeof(obj_id_t) + sizeof(timestamp_t) + sizeof(uint8_t);
constexpr uint32_t RESPONSE_MSG_SIZE = /*  9 bytes */
    sizeof(obj_id_t) + sizeof(uint8_t);

} // namespace common
