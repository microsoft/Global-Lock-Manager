#pragma once

#include <cstdint>
#include <thread>
#include <memory>

namespace common {

/**
 * Pins the thread to be scheduled on a specific physical core.
 * @param thread is a reference to the thread object to pin.
 * @param coreToPinTo specifies the core to pin to as a single bit set within
 * the uint32_t.
 */
void pinThread(std::thread& thread, uint32_t coreToPinTo);

/**
 * Gets the core the thread is pinned to.
 * @param thread is a reference to the thread object to pin.
 * @return the the core the thread is pinned to.
 */
uint32_t getCoreThreadPinnedTo(std::thread& thread);

} // namespace common
