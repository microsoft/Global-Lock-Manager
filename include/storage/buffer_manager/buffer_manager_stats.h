#pragma once

#include <cstdint>

namespace storage {

struct BufferManagerStats {
    uint64_t numPins{0};
    uint64_t numEvicts{0};
    uint64_t numGetPageReqs{0};
    uint64_t numFetchesFromPageServer{0};
    uint64_t numFetchesFromComputeServer{0};

    void reset() {
        numPins = 0;
        numEvicts = 0;
        numGetPageReqs = 0;
        numFetchesFromPageServer = 0;
        numFetchesFromComputeServer = 0;
    }
};

} // namespace storage
