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
    {
        mBitCount = v.mBitCount;
        mRuntime.copyVar(mData, v.mData.get());
    }

    sInt::sInt(sInt &&v)
        : mRuntime(v.mRuntime)
        , mData(std::move(v.mData))
    {
        mBitCount = v.mBitCount;
    }

    sInt::~sInt()
    {
    }

    sInt& sInt::operator=(const sInt & copy)
    {
        mRuntime.copyVar(mData, copy.mData.get());
        return *this;
    }

    sInt sInt::operator+(const sInt& in2)
    {

        sInt ret(mRuntime, std::max(mBitCount, in2.mBitCount));

        std::array<RuntimeData*, 3> io{ mData.get(), in2.mData.get(), ret.mData.get()};

        mRuntime.scheduleOp(Op::Add, io);


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

    sInt::ValueType sInt::getValue()
    {
        return valueFromBV(mValFut->get());
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