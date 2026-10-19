#pragma once

#include <cstdint>
#include <atomic>
#include <memory>

namespace storage {

class LogManager {
public:
    LogManager() : LogManager(64 /* buffer size MBs */) {};
    void append(uint8_t* record, uint32_t recordSizeInBytes);

private:
    explicit LogManager(uint32_t bufferSizeInMBs) :
        bufferSizeInBytes{bufferSizeInMBs * 1024 * 1024},
        buffer{std::make_unique<uint8_t[]>(bufferSizeInBytes)},
        bufferPtr{buffer.get()}, currLocalOffsetInLog{0u}
        {};

    uint32_t bufferSizeInBytes;
    std::unique_ptr<uint8_t[]> buffer;
    uint8_t* bufferPtr;
    std::atomic<uint32_t> currLocalOffsetInLog;
};

} // namespace storage
