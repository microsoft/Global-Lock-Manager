#pragma once

#include <cstdint>

#include <common/typedefs.h>

using namespace common;

namespace transport {

/**
 * An enum representing the responses to a request.
 */
enum response_status_t : uint8_t {
    SUCCESS = 0u,
    FAILURE = 1u,
    WAITING = 2u,
};

struct request_msg_t {
    obj_id_t objId;
    timestamp_t timestamp; // of the transaction requesting
    uint8_t payload;

    request_msg_t(obj_id_t objId, timestamp_t timestamp, uint8_t byte) :
        objId{objId}, timestamp{timestamp}, payload{byte} {}

    request_msg_t() :
        objId{UINT64_MAX}, timestamp{UINT64_MAX}, payload{UINT8_MAX} {}

    [[nodiscard]] bool signalsBreak() const {
        return payload == UINT8_MAX;
    }
};

struct response_msg_t {
    obj_id_t objId;
    uint8_t payload;

    response_msg_t(obj_id_t objId, timestamp_t timestamp, uint8_t byte) :
        objId{objId}, payload{byte} {}

    response_msg_t() :
        objId{UINT64_MAX}, payload{UINT8_MAX} {}

    [[nodiscard]] bool signalsBreak() const {
        return payload == UINT8_MAX;
    }
};

} // namespace transport
