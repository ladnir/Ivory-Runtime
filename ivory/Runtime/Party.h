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
        T input(typename T::ValueType&, u64 bitCount);

        template<typename T>
        T input(typename T::ValueType&);


        template<typename T>
        void reveal(const T&);

        u64 getIdx() { return mPartyIdx; }


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
    inline T LocalParty::input(typename T::ValueType& value, u64 bitCount)
    {
        T ret(mRuntime, bitCount);
        mRuntime.scheduleInput(ret.mData.get(), mPartyIdx, ret.valueToBV(value));
        return ret;
    }

    template<typename T>
    inline T LocalParty::input(typename T::ValueType& value)
    {
        T ret(mRuntime, T::N);
        mRuntime.scheduleInput(ret.mData.get(), mPartyIdx, ret.valueToBV(value));
        return ret;
    }

    template<typename T>
    inline void LocalParty::reveal(const T& var)
    {
        auto& v = *(T*)&var;
        v.mValFut.reset(new std::future<BitVector>());
        mRuntime.scheduleOutput(v.mData.get(), *v.mValFut.get());
    }


    template<typename T>
    inline T RemoteParty::input(u64 bitCount)
    {
        T ret(mRuntime, bitCount);
        mRuntime.scheduleInput(ret.mData.get(), mPartyIdx);
        return ret;
    }

    template<typename T>
    inline T RemoteParty::input()
    {
        T ret(mRuntime, T::N);
        mRuntime.scheduleInput(ret.mData.get(), mPartyIdx);
        return ret;
    }

    template<typename T>
    inline void RemoteParty::reveal(const T& var)
    {
        auto& v = *(T*)&var;
        mRuntime.scheduleOutput(v.mData.get(), mPartyIdx);
    }

}
