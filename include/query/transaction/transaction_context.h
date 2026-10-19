#pragma once

#include <common/typedefs.h>

namespace query {

class TransactionContext {
public:
    TransactionContext(tx_id_t txId, timestamp_t timestamp) :
        ID{txId}, timestamp{timestamp}
        {}

    explicit TransactionContext(tx_id_t txId) :
        ID{txId}, timestamp{getTimestamp()}
        {}

    void reset(tx_id_t ID, timestamp_t timestamp) {
        this->ID = ID;
        this->timestamp = timestamp;
    }

    void reset(tx_id_t ID) {
        this->ID = ID;
        this->timestamp = getTimestamp();
    }

public:
    tx_id_t ID;
    timestamp_t timestamp;
};

} // namespace query
