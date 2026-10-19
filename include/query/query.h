#pragma once

#include <query/system_api.h>
#include <query/transaction/transaction_context.h>
#include <query/database/database_state.h>

namespace query {

class Query {
public:
    virtual bool run(SystemAPI& storageAPI, TransactionContext& context,
                     uint32_t threadId) = 0;

    virtual void next() = 0;

    virtual ~Query() = default;
};

} // namespace query
