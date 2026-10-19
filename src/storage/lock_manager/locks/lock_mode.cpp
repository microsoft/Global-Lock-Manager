#include "storage/lock_manager/locks/lock_mode.h"

namespace storage {

constexpr const char* LockModeAsStr[] = {
    "NONE", "IS", "S", "IX", "SIX", "X", "U"
};

const char* getLockModeAsStr(lock_mode_t lockMode) {
    switch (lockMode)
    {
    case   S_IS: return "S_IS";
    case  IX_IS: return "IX_IS";
    case SIX_IS: return "SIX_IS";
    case   U_IS: return "U_IS";
    case    U_S: return "U_S";
    case U_S_IS: return "U_S_IS";
    default:
        if (lockMode > 6u) throw;
        return LockModeAsStr[lockMode];
    }
}

} // namespace storage
