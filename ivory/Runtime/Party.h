#pragma once
#include "Runtime/Runtime.h"
#include "Common/Defines.h"
#include <future>

namespace osuCrypto
{

    class LocalParty
    {
    public:
        LocalParty(Runtime& runtime, u64 partyIdx);

        template<typename T>
        T input(typename const T::ValueType&, u64 bitCount);

        template<typename T>
        T input(typename const T::ValueType&);


        template<typename T>
        typename T::ValueType reveal(const T&);

        u64 getIdx() { return mPartyIdx; }

        //template<typename T>
        //std::future<T::ValueType> asyncReveal(const T&);

    private:
        Runtime& mRuntime;
        u64 mPartyIdx;
    };








    class RemoteParty
    {
    public:
        RemoteParty(Runtime& runtime, u64 partyIdx);

        template<typename T>
        T input(u64 bitCount);

        template<typename T>
        T input();

        template<typename T>
        void reveal(const T&);

        u64 getIdx() { return mPartyIdx; }

    private:
        Runtime& mRuntime;
        u64 mPartyIdx;
    };


    template<typename T>
    inline T LocalParty::input(typename const T::ValueType& value, u64 bitCount)
    {
        T ret(mRuntime, bitCount);
        mRuntime.scheduleInput(ret.mLabels, mPartyIdx, ret.valueToBV(value));
        return ret;
    }

    template<typename T>
    inline T LocalParty::input(typename const T::ValueType& value)
    {
        T ret(mRuntime, T::N);
        mRuntime.scheduleInput(ret.mLabels, mPartyIdx, ret.valueToBV(value));
        return ret;
    }

    template<typename T>
    inline typename  T::ValueType LocalParty::reveal(const T& var)
    {
        auto& v = *(T*)&var;
        BitVector fut;
        mRuntime.scheduleOutput(v.mLabels, fut);
        return v.valueFromBV(fut);
    }


    template<typename T>
    inline T RemoteParty::input(u64 bitCount)
    {
        T ret(mRuntime, bitCount);
        mRuntime.scheduleInput(ret.mLabels, mPartyIdx);
        return ret;
    }

    template<typename T>
    inline T RemoteParty::input()
    {
        T ret(mRuntime, T::N);
        mRuntime.scheduleInput(ret.mLabels, mPartyIdx);
        return ret;
    }

    template<typename T>
    inline void RemoteParty::reveal(const T& var)
    {
        auto& v = *(T*)&var;
        mRuntime.scheduleOutput(v.mLabels, mPartyIdx);
    }

}