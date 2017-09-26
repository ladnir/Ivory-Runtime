#include "ThunderInt.h"


#define BINARY_OP_PREAMBLE 		\
	auto aSize = a->bitCount(); \
	auto bSize = b->bitCount(); \
	auto cSize = std::max(aSize, bSize); \
	auto aa = getMemory(a); \
	auto bb = getMemory(b); \
	Thunder::Memory mm(std::make_shared<Thunder::TMemory>(cSize));\
	auto ret(new ThunderInt(mRt, std::move(mm))); \
	Thunder::WorkItem workItem; \
	workItem.mInputBundleCount = 2; \
	workItem.mMemory = { aa, bb, ret->mData };

namespace osuCrypto
{

	ThunderInt::ThunderInt(ThunderRuntime & rt, Thunder::Memory&& data)
		: mRt(rt)
		, mData(data)
	{ }

	ThunderInt::~ThunderInt()
	{ }

	void ThunderInt::copy(sIntBasePtr & c)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}

	sIntBasePtr ThunderInt::copy()
	{
		throw std::runtime_error("not impl. " LOCATION);
	}

	u64 ThunderInt::bitCount()
	{
		return mData->mBitCount;
	}

	sIntBasePtr ThunderInt::add(sIntBasePtr & a, sIntBasePtr & b)
	{
		BINARY_OP_PREAMBLE;
		workItem.mCircuit = mRt.mLibrary.int_int_add(aSize, bSize, cSize);
		mRt.enqueue(std::move(workItem));
		return sIntBasePtr(ret);
	}
	sIntBasePtr ThunderInt::subtract(sIntBasePtr & a, sIntBasePtr & b)
	{
		BINARY_OP_PREAMBLE;
		workItem.mCircuit = mRt.mLibrary.int_int_subtract(aSize, bSize, cSize);
		mRt.enqueue(std::move(workItem));
		return sIntBasePtr(ret);
	}
	sIntBasePtr ThunderInt::multiply(sIntBasePtr & a, sIntBasePtr & b)
	{
		BINARY_OP_PREAMBLE;
		workItem.mCircuit = mRt.mLibrary.int_int_mult(aSize, bSize, cSize);
		mRt.enqueue(std::move(workItem));
		return sIntBasePtr(ret);
	}
	sIntBasePtr ThunderInt::divide(sIntBasePtr & a, sIntBasePtr & b)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	sIntBasePtr ThunderInt::negate()
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	sIntBasePtr ThunderInt::gteq(sIntBasePtr & a, sIntBasePtr & b)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	sIntBasePtr ThunderInt::gt(sIntBasePtr & a, sIntBasePtr & b)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	sIntBasePtr ThunderInt::bitwiseInvert()
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	sIntBasePtr ThunderInt::bitwiseAnd(sIntBasePtr & a, sIntBasePtr & b)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	sIntBasePtr ThunderInt::bitwiseOr(sIntBasePtr & a, sIntBasePtr & b)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	sIntBasePtr ThunderInt::bitwiseXor(sIntBasePtr & a, sIntBasePtr & b)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	sIntBasePtr ThunderInt::ifelse(sIntBasePtr & a, sIntBasePtr & ifTrue, sIntBasePtr & ifFalse)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	void ThunderInt::reveal(u64 partyIdx)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	void ThunderInt::reveal(span<u64> partyIdxs)
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	ThunderInt::ValueType ThunderInt::getValue()
	{
		throw std::runtime_error("not impl. " LOCATION);
	}
	Thunder::Memory ThunderInt::getMemory(sIntBasePtr & a)
	{
		return ((ThunderInt&)a).mData;
	}
}
