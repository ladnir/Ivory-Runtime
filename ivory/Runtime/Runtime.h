#pragma once
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/Defines.h"
#include <future>
#include <memory>
#include <ivory/Runtime/sInt.h>

namespace osuCrypto
{


    class Runtime
    {
    public:
        Runtime();
        ~Runtime();

        static sIntBasePtr getPublicInt(i64 v, u64 size);

        virtual sInt sIntInput(BitCount bitCount, u64 partyIdx) = 0;
        virtual sInt sIntInput(sInt::ValueType data, BitCount bitCount) = 0;

        // processQueue() will ensure that all scheduled operations have been completed
        //    before returning.
        // Assumptions: None
        // Result: Upon return, all operations are either in process or completed.
        virtual void processQueue() = 0;


        // getPartyIdx() returns the index of the local party.
        virtual u64 getPartyIdx() = 0;
    };

}
