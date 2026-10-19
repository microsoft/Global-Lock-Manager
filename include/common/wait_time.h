#pragma once

#include <chrono>
#include <cstdint>

#include <common/typedefs.h>

namespace common {

/**
 * Causes a thread to remain active with a busy wait for the given wait time.
 * @param waitTimeInNanosecs the given wait time for the busy wait.
 */
void waitInNanosecs(uint64_t waitTimeInNanosecs);

/**
 * Causes a thread to remain active with a busy wait to ensure the given
 * duration happened since the given start time.
 * @param obtainTimestamp the start time of the duration as a timestamp_t.
 * @param durationInNanosecs duration that has to elapse since obtainTimestamp.
 */
void waitInNanosecs(timestamp_t obtainTimestamp, uint64_t durationInNanosecs);

} // namespace common
