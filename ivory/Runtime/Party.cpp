#include "Party.h"


namespace osuCrypto
{


    Party::Party(Runtime & runtime, u64 partyIdx)
        : mRuntime(runtime)
        , mPartyIdx(partyIdx)
    {
    }
}