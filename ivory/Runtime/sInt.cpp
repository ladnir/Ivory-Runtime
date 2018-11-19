#include "sInt.h"
#include <ivory/Runtime/Runtime.h>
namespace osuCrypto
{

    sInt::sInt(const sInt & val)
        : mData(std::move(val.copyBits(0,-1).mData))
    { }

    sInt::sInt(const i64 & val, u64 bitCount)
        : mData(Runtime::getPublicInt(val, bitCount))
    { }

    sInt::sInt(const i64 & val)
        : mData(Runtime::getPublicInt(val, 64))
    { }

    sInt::sInt(const i32 & val)
        : mData(Runtime::getPublicInt(val, 32))
    { }

    sInt::sInt(const i16 & val)
        : mData(Runtime::getPublicInt(val, 16))
    { }

    sInt::sInt(const i8 & val)
        : mData(Runtime::getPublicInt(val, 8))
    { }

    sInt::~sInt()
    { }

    sInt& sInt::operator=(const sInt & c)
    {
        sIntBasePtr& s = (sIntBasePtr&)c.mData;
        mData->copy(s, 0, -1, 0);
        return *this;
    }

    sInt & sInt::operator=(sInt && mv)
    {
        mData = std::move(mv.mData);
        return *this;
    }

    sInt sInt::operator~() const
    {
        return mData->bitwiseInvert();
    }
    sInt sInt::operator|(const sInt& in2) const
    {
        return mData->bitwiseOr((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }
    sInt sInt::operator&(const sInt& in2) const
    {
        return mData->bitwiseAnd((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator^(const sInt & in2) const
    {
        return mData->bitwiseXor((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator+(const sInt& in2) const
    {
        return mData->add((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator-(const sInt & in2) const
    {
        return mData->subtract((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator!=(const sInt & in2)
    {
        return mData->neq((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator==(const sInt & in2)
    {
        return mData->eq((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator>=(const sInt & in2)
    {
        return mData->gteq((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator>(const sInt &in2)
    {
        return mData->gt((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator<=(const sInt &in2)
    {
        return in2.mData->gteq((sIntBasePtr&)in2.mData, (sIntBasePtr&)mData);
    }

    sInt sInt::operator<(const sInt & in2)
    {
        return in2.mData->gt((sIntBasePtr&)in2.mData, mData);
    }


    //sInt sInt::operator<<(int s)
    //{
    //    return mData->leftShift(s);
    //}
    sInt sInt::operator<<(int s)
    {
        auto bc = bitCount();
        if (s < 0 || s > bc)
            throw std::runtime_error("bad shift value " LOCATION);

        // copy the bits. drop the top bc bits.
        return mData->copy(0, bc, s);
    }

    sInt sInt::operator>>(int s)
    {
        auto bc = bitCount();
        if (s < 0 || s > bc)
            throw std::runtime_error("bad shift value " LOCATION);

        // copy the bits. drop the top bc bits.
        return mData->copy(0, bc, -s);
    }


    sInt sInt::copyBits(u64 lowIdx, u64 highIdx) const
    {
        return mData->copy(lowIdx, highIdx, 0);
    }
    u64 sInt::bitCount() const
    {
        return mData->bitCount();
    }

    sInt sInt::ifelse(const sInt & ifTrue, const sInt & ifFalse)
    {
        return mData->ifelse((sIntBasePtr&)mData, (sIntBasePtr&)ifTrue.mData, (sIntBasePtr&)ifFalse.mData);
    }

    sInt sInt::isZero() const
    {
        return mData->isZero();
    }


    sInt& sInt::operator+=(const sInt& in2)
    {
        mData = mData->add((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
        return *this;
    }

    sInt sInt::operator*(const sInt& in2) const
    {
        return mData->multiply((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt sInt::operator/(const sInt & in2) const
    {
        return mData->divide((sIntBasePtr&)mData, (sIntBasePtr&)in2.mData);
    }

    sInt::ValueType sInt::getValue()
    {
        return mData->getValue();
    }

    void sInt::reveal(span<u64> partyIdxs)
    {
        mData->reveal(partyIdxs);
    }

}