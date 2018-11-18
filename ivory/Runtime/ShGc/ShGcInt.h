#pragma once
#include "ivory/Runtime/sInt.h"
#include "ivory/Runtime/ShGc/utils.h"

namespace osuCrypto
{
    class ShGcRuntime;

    class ShGcInt :
        public sIntBase
    {
    public:

        ShGc::GarbledMem mLabels;
        std::shared_future<BitVector> mFutr;
        ShGcRuntime& mRt;

        ShGcInt(ShGcRuntime& rt, u64 bitCount);
        ShGcInt(ShGcInt&& m) = default;
        ShGcInt(const ShGcInt& s) = default;

        ~ShGcInt() override;

        void copy(sIntBasePtr& c, u64 lowIdx, u64 highIdx, i64 leftShift)override;
        void copy(ShGcInt& c, u64 lowIdx, u64 highIdx, i64 leftShift);
        sIntBasePtr copy(u64 lowIdx, u64 highIdx, i64 leftShift)override;
        u64 bitCount()override;
        Runtime& getRuntime()override;

        sIntBasePtr add(sIntBasePtr& a, sIntBasePtr& b)override;
        sIntBasePtr subtract(sIntBasePtr& a, sIntBasePtr& b)override;
        sIntBasePtr multiply(sIntBasePtr& a, sIntBasePtr& b)override;
        sIntBasePtr divide(sIntBasePtr& a, sIntBasePtr& b)override;

        sIntBasePtr negate()override;

        sIntBasePtr neq(sIntBasePtr& a, sIntBasePtr& b)override;
        sIntBasePtr eq(sIntBasePtr& a, sIntBasePtr& b)override;
        sIntBasePtr gteq(sIntBasePtr& a, sIntBasePtr& b)override;
        sIntBasePtr gt(sIntBasePtr& a, sIntBasePtr& b)override;

        sIntBasePtr bitwiseInvert()override;
        sIntBasePtr bitwiseXor(sIntBasePtr& a, sIntBasePtr& b)override;
        sIntBasePtr bitwiseAnd(sIntBasePtr& a, sIntBasePtr& b)override;
        sIntBasePtr bitwiseOr(sIntBasePtr& a, sIntBasePtr& b)override;

        sIntBasePtr ifelse(sIntBasePtr& a, sIntBasePtr& ifTrue, sIntBasePtr& ifFalse)override;
        sIntBasePtr isZero()override;

        void reveal(u64 partyIdx)override;
        void reveal(span<u64> partyIdxs)override;
        ValueType getValue()override;

        //sIntBasePtr getBits(u64 lowIdx, u64 highIdx) override;

        ShGc::GarbledMem getMemory(sIntBasePtr& a);

    };

}
