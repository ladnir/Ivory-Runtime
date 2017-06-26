#pragma once
#include <ivory/Runtime/sInt.h>

namespace osuCrypto
{
    class ClearRuntime;

    namespace clear
    {
        i64 signExtend(i64 v, u64 bitIdx);
    }

    class ClearInt 
        : public sIntBase
    {
    public:
        sIntBase::ValueType mValue;
        u64 mBitCount;
        ClearRuntime& mRt;

        ClearInt(ClearRuntime& rt, u64 mBitCount);
        ~ClearInt();

        ~ClearInt() override {}

        void copy(sIntBasePtr& c)override;
        u64 bitCount()override;
        Runtime& getRuntime()override;

        sIntBasePtr add(sIntBasePtr& b)override;
        sIntBasePtr subtract(sIntBasePtr& b)override;
        sIntBasePtr multiply(sIntBasePtr& b)override;
        sIntBasePtr divide(sIntBasePtr& b)override;

        sIntBasePtr negate()override;

        sIntBasePtr gteq(sIntBasePtr& b)override;
        sIntBasePtr gt(sIntBasePtr& b)override;

        sIntBasePtr bitwiseInvert()override;
        sIntBasePtr bitwiseAnd(sIntBasePtr& b)override;
        sIntBasePtr bitwiseOr(sIntBasePtr& b)override;

        sIntBasePtr ifelse(sIntBasePtr& ifTrue, sIntBasePtr& ifFalse)override;

        void reveal(u64 partyIdx)override;
        void reveal(span<u64> partyIdxs)override;
        ValueType getValue()override;

    };

}
