#pragma once

#include <atomic>

namespace common {

/**
 * The Spinlock is a lock that causes a thread trying to acquire it to remain
 * active with a busy wait if the lock is unavailable. As such, the thread spins
 * i.e., waits in a loop, repeatedly checking whether the lock is available.
 * The lock and unlock methods of a spinlock are abstractions for the use of
 * test_and_set and clear methods of an atomic_flag, respectively.
 */
class Spinlock {
public:
    void lock()   { while(flag.test_and_set(std::memory_order_acquire)); }
    void unlock() { flag.clear(std::memory_order_release); }

private:
    std::atomic_flag flag ATOMIC_FLAG_INIT;
};

} // namespace common
