#include <common/wait_time.h>

namespace common {

void waitInNanosecs(uint64_t waitTimeInNanosecs) {
    waitInNanosecs(getTimestamp() /* start */, waitTimeInNanosecs);
}

void waitInNanosecs(timestamp_t start, uint64_t durationInNanosecs) {
    auto stop = getTimestamp();
	while (stop - start < durationInNanosecs) {
        stop = getTimestamp();
    }
}

} // namespace common
