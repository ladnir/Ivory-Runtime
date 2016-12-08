#pragma once
#include "Runtime/Runtime.h"
#include "Common/Defines.h"
#include <future>

namespace osuCrypto
{

    class Party
    {
    public:
        Party(Runtime& runtime, u64 partyIdx);

        template<typename T>
        T input(typename T::ValueType&, u64 bitCount);

        template<typename T>
        T input(u64 bitCount);

        template<typename T>
        void reveal(const T&);

        u64 getPartyIdx() { return mPartyIdx; }

        bool isLocalParty() { return mPartyIdx == mRuntime.getPartyIdx(); }

        Runtime& getRuntime()
        {
            return mRuntime;
        }

    private:
        Runtime& mRuntime;
        u64 mPartyIdx;
    };



    template<typename T>
    inline T Party::input(typename T::ValueType& value, u64 bitCount)
    {
        T ret(mRuntime, bitCount);
        mRuntime.scheduleInput(ret.mData.get(), ret.valueToBV(value));
        return ret;
    }


    template<typename T>
    inline T Party::input(u64 bitCount)
    {
        T ret(mRuntime, bitCount);
        mRuntime.scheduleInput(ret.mData.get(), mPartyIdx);
        return ret;
    }

    template<typename T>
    inline void Party::reveal(const T& var)
    {
        // cast the const away...
        auto& v = *(T*)&var;

        if (isLocalParty())
        {
            v.mValFut.reset(new std::future<BitVector>());
            mRuntime.scheduleOutput(v.mData.get(), *v.mValFut.get());
        }
        else
        {
            mRuntime.scheduleOutput(v.mData.get(), mPartyIdx);
        }
    }



}
