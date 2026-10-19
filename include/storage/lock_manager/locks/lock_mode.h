#pragma once

#include <cstdint>

namespace storage {

constexpr uint8_t const NUM_LOCK_MODES_POSSIBLE = 13u;
constexpr uint8_t const NUM_SINGLE_LOCK_MODES_SUPPORTED = 6u;

enum lock_mode_t : uint8_t {
    /* single tx lock mode granted */
     NONE =  0u, /* not owned */
       IS =  1u, /* intention-to-read */
        S =  2u, /* read */
       IX =  3u, /* intention-to-write */
      SIX =  4u, /* read-with-intention-to-write */
        X =  5u, /* write */
        U =  6u, /* update, asymmetrical to S */

    /* pair of tx lock modes granted simultaneously */
     S_IS = (  S << 4u) | IS, /*   S and IS tx locks */
    IX_IS = ( IX << 4u) | IS, /*  IX and IS tx locks */
   SIX_IS = (SIX << 4u) | IS, /* SIX and IS tx locks */
     U_IS = (  U << 4u) | IS, /*   U and IS tx locks */
     U_S  = (  U << 4u) |  S, /*   U and  S tx locks */

    /* three tx lock modes granted simultaneously */
   U_S_IS = (  1u << 7u) | S_IS, /* U, S, and IS tx locks */
};

const char* getLockModeAsStr(lock_mode_t lockMode);

class IsCompatible {
private:
    static constexpr bool const tableForTxs
        [NUM_LOCK_MODES_POSSIBLE         /* held */]
        [NUM_SINGLE_LOCK_MODES_SUPPORTED /* requested */] = {
        // |----------------------------------------------------------|
        // |          |                lock requested                 |
        // |----------------------------------------------------------|
        // |  held    |   IS  ,   S   ,   IX  ,  SIX  ,   X   ,   U   |
        /* |  NONE  */{  true ,  true ,  true ,  true ,  true ,  true },
        /* |    IS  */{  true ,  true ,  true ,  true , false ,  true },
        /* |     S  */{  true ,  true , false , false , false ,  true },
        /* |    IX  */{  true , false ,  true , false , false , false },
        /* |   SIX  */{  true , false , false , false , false , false },
        /* |     X  */{ false , false , false , false , false , false },
        /* |     U  */{  true ,  true , false , false , false , false },
        // ------------------------------------------------------------
        /* |   S_IS */{  true ,  true , false , false , false ,  true },
        /* |  IX_IS */{  true , false ,  true , false , false , false },
        /* | SIX_IS */{  true , false , false , false , false , false },
        /* |   U_IS */{  true ,  true , false , false , false , false },
        /* |    U_S */{  true ,  true , false , false , false , false },
        /* | U_S_IS */{  true ,  true , false , false , false , false },
    };

    static constexpr bool const tableForNodeLockAndTx
        [NUM_SINGLE_LOCK_MODES_SUPPORTED /* held by node */]
        [NUM_SINGLE_LOCK_MODES_SUPPORTED /* requested by a tx */] = {
        // |----------------------------------------------------------|
        // |          |            lock requested                     |
        // |----------------------------------------------------------|
        // |  held    |   IS  ,   S   ,  IX   ,  SIX  ,   X   ,   U   |
        /* |  NONE  */{ false , false , false , false , false , false },
        /* |    IS  */{  true , false , false , false , false , false },
        /* |     S  */{  true ,  true , false , false , false ,  true },
        /* |    IX  */{  true , false ,  true , false , false , false },
        /* |   SIX  */{  true ,  true ,  true ,  true , false ,  true },
        /* |     X  */{  true ,  true ,  true ,  true ,  true ,  true },
    };

public:
    static bool forTxs(lock_mode_t held, lock_mode_t requested) {
        auto idx = requested - 1;
        switch (held) {
        case   S_IS: return tableForTxs[ 7u][idx];
        case  IX_IS: return tableForTxs[ 8u][idx];
        case SIX_IS: return tableForTxs[ 9u][idx];
        case   U_IS: return tableForTxs[10u][idx];
        case    U_S: return tableForTxs[11u][idx];
        case U_S_IS: return tableForTxs[12u][idx];
        default:
            return tableForTxs[held][idx];
        }
    }

    static bool forNodeLockAndTx(lock_mode_t nodeLockHeld,
        lock_mode_t txRequest) {
        return tableForNodeLockAndTx[nodeLockHeld][txRequest - 1];
    }
};

} // namespace storage
