#pragma once
#include "ivory/Runtime/sInt.h"
#include <future>
#include "ivory/Runtime/Thunder/ThunderRuntime.h"
#include "ivory/Runtime/Thunder/util.h"

namespace osuCrypto
{
	class ThunderInt : public sIntBase
	{
	public:


		Thunder::Memory mData;

		std::shared_future<BitVector> mFutr;
		ThunderRuntime& mRt;

		ThunderInt(ThunderRuntime& rt, Thunder::Memory&& data);
		ThunderInt(ThunderInt&& m) = default;
		ThunderInt(const ThunderInt& s) = default;
		~ThunderInt() override;


		void copy(sIntBasePtr& c)override;
		sIntBasePtr copy()override;
		u64 bitCount()override;
		Runtime& getRuntime()override { return mRt; }

		sIntBasePtr add(sIntBasePtr& a, sIntBasePtr& b)override;
		sIntBasePtr subtract(sIntBasePtr& a, sIntBasePtr& b)override;
		sIntBasePtr multiply(sIntBasePtr& a, sIntBasePtr& b)override;
		sIntBasePtr divide(sIntBasePtr& a, sIntBasePtr& b)override;

		sIntBasePtr negate()override;

		sIntBasePtr gteq(sIntBasePtr& a, sIntBasePtr& b)override;
		sIntBasePtr gt(sIntBasePtr& a, sIntBasePtr& b)override;

		sIntBasePtr bitwiseInvert()override;
		sIntBasePtr bitwiseAnd(sIntBasePtr& a, sIntBasePtr& b)override;
		sIntBasePtr bitwiseOr(sIntBasePtr& a, sIntBasePtr& b)override;
		sIntBasePtr bitwiseXor(sIntBasePtr& a, sIntBasePtr& b)override;

		sIntBasePtr ifelse(sIntBasePtr& a, sIntBasePtr& ifTrue, sIntBasePtr& ifFalse)override;

		void reveal(u64 partyIdx)override;
		void reveal(span<u64> partyIdxs)override;
		ValueType getValue()override;

		Thunder::Memory getMemory(sIntBasePtr& a);
	};

}
