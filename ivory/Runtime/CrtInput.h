#pragma once
#include "Runtime/CrtRuntime.h"
#include "Common/Defines.h"
#include <future>

namespace osuCrypto
{
    class CrtLocalParty
    {
    public:
        CrtLocalParty(CrtRuntime& runtime, u64 partyIdx);

        template<typename T>
        T input(typename const T::ValueType&);


        template<typename T>
        typename T::ValueType reveal(const T&);

        u64 getIdx() { return mPartyIdx; }

        //template<typename T>
        //std::future<T::ValueType> asyncReveal(const T&);

    private:
        CrtRuntime& mRuntime;
        u64 mPartyIdx;
    };








    class CrtRemoteParty
    {
    public:
        CrtRemoteParty(CrtRuntime& runtime, u64 partyIdx);

        template<typename T>
        T input();


        template<typename T>
        void reveal(const T&);

        u64 getIdx() { return mPartyIdx; }

    private:
        CrtRuntime& mRuntime;
        u64 mPartyIdx;
    };


    template<typename T>
    inline T CrtLocalParty::input(typename const T::ValueType& value)
    {
        T ret(mRuntime);
        mRuntime.scheduleInput(ret.mLabels, ret.valueToBV(value), mPartyIdx);
        return ret;
    }

    template<typename T>
    inline typename  T::ValueType CrtLocalParty::reveal(const T& var)
    {
        auto& v = *(T*)&var;
        BitVector fut;
        mRuntime.scheduleOutput(v.mLabels, fut);
        return T::valueFromBV(fut);
    }

    //template<typename T>
    //inline std::future<T::ValueType> CrtLocalParty::asyncReveal(const T &)
    //{	

    //	return std::future<T::ValueType>();
    //}

    template<typename T>
    inline T CrtRemoteParty::input()
    {
        T ret(mRuntime);
        mRuntime.scheduleInput(ret.mLabels, mPartyIdx);
        return ret;
    }

    template<typename T>
    inline void CrtRemoteParty::reveal(const T& var)
    {
        auto& v = *(T*)&var;
        mRuntime.scheduleOutput(v.mLabels, mPartyIdx);
    }

}