#pragma once

#include <set>
#include <memory>

#include <common/typedefs.h>
#include <query/query.h>
#include <query/system_api.h>

namespace query {

class Transaction {
public:
    Transaction(Query& query, SystemAPI& storageAPI, TransactionContext& context) :
        query{query}, storageAPI{storageAPI}, context{context}
        {}

    bool run(uint32_t threadId) {
        return query.run(storageAPI, context, threadId);
    }

    [[nodiscard]] tx_id_t getTransactionId() const { return context.ID; }

    Query& query;
    SystemAPI& storageAPI;
    TransactionContext& context;
};

} // namespace query
