#pragma once
//#include "Runtime/CrtModulal.h"

#include "Circuit/Circuit.h"

#include "Circuit/BetaCircuit.h"
#include "Runtime/ShGcRuntime.h"

namespace osuCrypto
{
    //template<u32 N>
    class sInt
    {
    public:
        typedef i64 ValueType;

        u64 mBitCount;
        sInt(Runtime& runtime, u64 bitCount);
        sInt(const sInt&);
        sInt(sInt&&);
        ~sInt();

        Runtime& mRuntime;

        sInt& operator=(const sInt&);


        sInt operator+(const sInt&);
        //sInt operator-(const sInt&);
        sInt operator*(const sInt&);
        //sInt operator/(const sInt&);
        
        void operator+=(const sInt&);
        //sInt operator-=(const sInt&);
        //sInt operator*=(const sInt&);
        //sInt operator/=(const sInt&);


        ValueType getValue();


        void reveal(ArrayView<u64> partyIdxs);

        BitVector valueToBV(const ValueType& val);
        ValueType valueFromBV(const BitVector& val);
        



        std::unique_ptr<RuntimeData> mData;
        std::unique_ptr<std::future<BitVector>> mValFut;

        //std::vector<block> mLabels;
        
        //void* mData;

    //private:

        //static void staticInit();
        //static void staticInitAddBetaCir();
        //static void staticBuildAddBetaCir(BetaCircuit& cd,BetaBundle& add0, BetaBundle& add1, BetaBundle& sum, BetaBundle& temp);
        //static void staticInitMultBetaCir();

        //static BetaCircuit mAddBetaCir;
        //static BetaCircuit mMultBetaCir;
    };



    //class sInt32 : public sInt
    //{
    //    static const  u32 N;
    //    sInt32(Runtime& runtime);
    //    sInt32(const sInt&);
    //    sInt32(sInt&&);
    //};

}
