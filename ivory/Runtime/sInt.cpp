#include "sInt.h"

namespace osuCrypto
{



    sInt::sInt(Runtime& runtime, u64 bitCount)
        : mRuntime(runtime)
        , mBitCount(bitCount)
    {
        mRuntime.initVar(mData, bitCount);

    }

    sInt::sInt(const sInt & v)
        : mRuntime(v.mRuntime)
        , mBitCount(v.mBitCount)
    {
        mRuntime.copyVar(mData, v.mData.get());
    }

    sInt::sInt(sInt &&v)
        : mRuntime(v.mRuntime)
        , mBitCount(v.mBitCount)
        , mData(std::move(v.mData))
    {
    }

    sInt::~sInt()
    {
    }

    sInt& sInt::operator=(const sInt & copy)
    {
        mRuntime.copyVar(mData, copy.mData.get());
        return *this;
    }

    sInt sInt::operator~()
    {
        sInt ret(mRuntime, mBitCount);
        std::array<RuntimeData*, 2> io{ mData.get(), ret.mData.get() };
        mRuntime.scheduleOp(Op::BitwiseNot, io);

        return ret;
    }

    sInt sInt::operator+(const sInt& in2)
    {
        sInt ret(mRuntime, std::max(mBitCount, in2.mBitCount));
        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(), ret.mData.get()};
        mRuntime.scheduleOp(Op::Add, io);

        return ret;
    }

    sInt sInt::operator-(const sInt & in2)
    {
        sInt ret(mRuntime, std::max(mBitCount, in2.mBitCount));
        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(), ret.mData.get() };
        mRuntime.scheduleOp(Op::Subtract, io);
        return ret;
    }

    sInt sInt::operator>=(const sInt & in2)
    {
        sInt ret(mRuntime, 1);
        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(), ret.mData.get() };
        mRuntime.scheduleOp(Op::GTEq, io);
        return ret;
    }

    sInt sInt::operator>(const sInt &in2)
    {
        sInt ret(mRuntime, 1);
        std::array<RuntimeData*, 3> io{ in2.mData.get(),mData.get(), ret.mData.get() };
        mRuntime.scheduleOp(Op::LT, io);
        return ret;
    }

    sInt sInt::operator<=(const sInt &in2)
    {
        sInt ret(mRuntime, 1);
        std::array<RuntimeData*, 3> io{ in2.mData.get(), mData.get(), ret.mData.get() };
        mRuntime.scheduleOp(Op::GTEq, io);
        return ret;
    }

    sInt sInt::operator<(const sInt & in2)
    {
        sInt ret(mRuntime, 1);
        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(), ret.mData.get() };
        mRuntime.scheduleOp(Op::LT, io);
        return ret;
    }

    sInt sInt::operator&(const sInt &in2)
    {
        sInt ret(mRuntime, std::max(mBitCount, in2.mBitCount));
        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(), ret.mData.get() };
        mRuntime.scheduleOp(Op::BitwiseAnd, io);
        return ret;
    }

    sInt sInt::ifelse(const sInt & ifTrue, const sInt & ifFalse)
    {
        if (mBitCount != 1)
            throw std::runtime_error(LOCATION);

        sInt ret(mRuntime, ifTrue.mBitCount);
        std::array<RuntimeData*, 4> io{ ifTrue.mData.get(), ifFalse.mData.get(), mData.get(), ret.mData.get() };
        mRuntime.scheduleOp(Op::IfElse, io);
        return ret;
    }

    void sInt::operator+=(const sInt& in2)
    {
        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(),  mData.get() };
        mRuntime.scheduleOp(Op::Add, io);
    }

    sInt sInt::operator*(const sInt& in2)
    {
        sInt ret(mRuntime, std::max(mBitCount, in2.mBitCount));
        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(), ret.mData.get() };

        mRuntime.scheduleOp(Op::Multiply, io);

        return std::move(ret);
    }

    sInt sInt::operator/(const sInt & in2)
    {
        sInt ret(mRuntime, std::max(mBitCount, in2.mBitCount));
        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(), ret.mData.get() };

        mRuntime.scheduleOp(Op::Divide, io);

        return std::move(ret);
    }



    sInt::ValueType sInt::getValue()
    {
        mRuntime.processesQueue();
        if (mValFut)
        {
            mVal = std::move(mValFut->get());
            mValFut.reset();
        }
        return valueFromBV(mVal);
    }

    void sInt::reveal(ArrayView<u64> partyIdxs)
    {
        mValFut.reset(new std::future<BitVector>());
        mRuntime.scheduleOutput(mData.get(), *mValFut.get());
    }

    BitVector sInt::valueToBV(const ValueType & val)
    {
        return BitVector((u8*)&val, mBitCount);
    }

    sInt::ValueType sInt::valueFromBV(const BitVector & val)
    {
        if (val.size() != mBitCount)throw std::runtime_error("");

        ValueType ret = 0;
        memcpy(&ret, val.data(), val.sizeBytes());

        return ret;
    }
}