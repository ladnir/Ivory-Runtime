#pragma once
#include "ivory/Runtime/sInt.h"
#include <cryptoTools/Common/Defines.h>
#include <vector>
#include <tuple>
#include <deque>



namespace osuCrypto
{

    class PublicInt
        : public sIntBase
    {
    public:
        sInt::ValueType mValue = 0;
        u64 mBitCount = 0;

		PublicInt()  {}
		PublicInt(sInt::ValueType v, u64 bits) : mValue(v), mBitCount(bits) {}
        ~PublicInt() override {}

        void copy(sIntBasePtr& c, u64 lowIdx, u64 highIdx, i64 shift)override;
        sIntBasePtr copy(u64 lowIdx, u64 highIdx, i64 shift)override;
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

        void reveal(u64 partyIdx)override { throw std::runtime_error(" cant reveal public value" LOCATION); }
        void reveal(span<u64> partyIdxs)override { throw std::runtime_error(" cant reveal public value" LOCATION); }
        ValueType getValue()override { return mValue; }
        ValueType getValueOffline()override { return mValue; }
        std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> genLabelsCircuit()override { throw std::runtime_error(" cant gen labels" LOCATION); }

    };

}
