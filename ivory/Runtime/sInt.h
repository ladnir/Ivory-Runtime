#pragma once
#include "cryptoTools/Common/Defines.h"
#include <vector>
#include <tuple>
#include <deque>

namespace osuCrypto
{
    struct BitCount
    {
        u64 mBitCount;
        BitCount(const u64& b) : mBitCount(b) {}
    };



    class Runtime;
    class sIntBase;
    typedef uPtr<sIntBase> sIntBasePtr;
    class sIntBase
    {
    public:

        virtual ~sIntBase() {}

        typedef i64 ValueType;

        enum class Op
        {
            Add,
            Subtract,
            Multiply,
            Divide,
            LT,
            GTEq,
            Mod,
            And,
            Or,
            Not,
            BitwiseAnd,
            BitWiseOr,
            BitwiseNot,
            IfElse
        };

        virtual void copy(sIntBasePtr& b, u64 lowIdx, u64 highIdx, i64 leftShift) = 0;
        virtual sIntBasePtr copy(u64 lowIdx, u64 highIdx, i64 leftShift) = 0;
        virtual u64 bitCount() = 0;
        virtual Runtime& getRuntime() = 0;

        virtual sIntBasePtr add(sIntBasePtr& a, sIntBasePtr& b) = 0;
        virtual sIntBasePtr subtract(sIntBasePtr& a, sIntBasePtr& b) = 0;
        virtual sIntBasePtr multiply(sIntBasePtr& a, sIntBasePtr& b) = 0;
        virtual sIntBasePtr divide(sIntBasePtr& a, sIntBasePtr& b) = 0;

        virtual sIntBasePtr negate() = 0;

        virtual sIntBasePtr neq(sIntBasePtr& a, sIntBasePtr& b) = 0;
        virtual sIntBasePtr eq(sIntBasePtr& a, sIntBasePtr& b) = 0;
        virtual sIntBasePtr gteq(sIntBasePtr& a, sIntBasePtr& b) = 0;
        virtual sIntBasePtr gt(sIntBasePtr& a, sIntBasePtr& b) = 0;

        virtual sIntBasePtr bitwiseInvert() = 0;
        virtual sIntBasePtr bitwiseXor(sIntBasePtr& a, sIntBasePtr& b) = 0;
        virtual sIntBasePtr bitwiseAnd(sIntBasePtr& a, sIntBasePtr& b) = 0;
        virtual sIntBasePtr bitwiseOr(sIntBasePtr& a, sIntBasePtr& b) = 0;

        virtual sIntBasePtr ifelse(sIntBasePtr& selectBit, sIntBasePtr& ifTrue, sIntBasePtr& ifFalse) = 0;
        virtual sIntBasePtr isZero() = 0;


        virtual void reveal(u64 partyIdx) = 0;
        virtual void reveal(span<u64> partyIdxs) = 0;
        virtual ValueType getValue() = 0;
        virtual std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> genLabelsCircuit() = 0;
        virtual ValueType getValueOffline() = 0;
    };



    class sInt
    {
    public:
        typedef sIntBase::ValueType ValueType;

        //sInt(Runtime& rt, const BitCount& bitCount);

        sInt() = default;
        sInt(const sInt&);
        sInt(sInt&&) = default;
        sInt(sIntBasePtr&& data) : mData(std::move(data)) {}

        sInt(const i64& val, u64 bitCount);
        sInt(const i64& val);
        sInt(const i32& val);
        sInt(const i16& val);
        sInt(const i8& val);

        ~sInt();

        sInt& operator=(const sInt&);
        sInt& operator=(sInt&&);


        sInt operator~()const;
        sInt operator|(const sInt&)const;
        sInt operator&(const sInt&)const;
        sInt operator^(const sInt&)const;

        sInt operator+(const sInt&) const;
        sInt operator-(const sInt&) const;
        sInt operator*(const sInt&) const;
        sInt operator/(const sInt&) const;


        //sInt operator+(const i64&);
        //sInt operator-(const i64&);
        //sInt operator*(const i64&);
        //sInt operator/(const i64&);
        //friend sInt operator+(const sInt&, const i64&);
        //friend sInt operator-(const sInt&, const i64&);
        //friend sInt operator*(const sInt&, const i64&);
        //friend sInt operator/(const sInt&, const i64&);

        sInt& operator+=(const sInt&);
        //sInt operator-=(const sInt&);
        //sInt operator*=(const sInt&);
        //sInt operator/=(const sInt&);

        sInt operator!=(const sInt&);
        sInt operator==(const sInt&);
        sInt operator>=(const sInt&);
        sInt operator>(const sInt&);
        sInt operator<=(const sInt&);
        sInt operator<(const sInt&);

        sInt operator>>(int shift);
        sInt operator<<(int shift);

        sInt copyBits(u64 lowIdx, u64 highIdx) const;
        u64 bitCount()const;

        sInt ifelse(const sInt&, const sInt&);

        sInt isZero() const;


        ValueType getValue();

        std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> genLabelsCircuit();
        ValueType getValueOffline();


        void reveal(span<u64> partyIdxs);

        //BitVector valueToBV(const ValueType& val);
        //ValueType valueFromBV(const BitVector& val);
        


        sIntBasePtr mData;
        //Runtime& mRuntime;
        //GUI mGUI;
        //u64 mBitCount;
        //std::unique_ptr<RuntimeData> mData;
        //std::unique_ptr<std::future<BitVector>> mValFut;
        //BitVector mVal;
    };


}
