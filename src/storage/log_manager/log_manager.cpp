#include <storage/log_manager/log_manager.h>

#include <chrono>
#include <thread>

#include <common/config.h>

using namespace common;

namespace storage {

void LogManager::append(uint8_t* record, uint32_t recordSizeInBytes) {
    auto localOffsetInLog = currLocalOffsetInLog.fetch_add(recordSizeInBytes);
    auto writeOffset = localOffsetInLog % bufferSizeInBytes;
    if (/* writing record exceeds buffer size: */
            (writeOffset + recordSizeInBytes) > bufferSizeInBytes) {
        // TODO: this_thread::sleep_for(chrono::milliseconds(LOG_APPEND_TIME_IN_MS));
        writeOffset = 0;
    }
    // TODO: memcpy(bufferPtr + writeOffset, record, recordSizeInBytes);
}

} // namespace storage
