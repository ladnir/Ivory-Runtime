#pragma once
//#include "Runtime/CrtModulal.h"

#include "Circuit/Circuit.h"

#include "Circuit/BetaCircuit.h"
#include "Runtime/ShGcRuntime.h"

namespace osuCrypto
{

    class sInt
    {
    public:
        typedef i64 ValueType;

        sInt(Runtime& runtime, u64 bitCount);
        sInt(const sInt&);
        sInt(sInt&&);
        ~sInt();


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
        

        Runtime& mRuntime;
        u64 mBitCount;
        std::unique_ptr<RuntimeData> mData;
        std::unique_ptr<std::future<BitVector>> mValFut;
        BitVector mVal;
    };


}
