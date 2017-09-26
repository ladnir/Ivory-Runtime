#include "ThunderRuntime.h"
#include "ThunderInt.h"
namespace osuCrypto
{
#define SHARE_BUFFER_SIZE 1024


	ThunderRuntime::ThunderRuntime(u64 partyIdx, block seed, Channel& next, Channel& prev)
		: mPartyIdx(partyIdx)
		, mInputByteCount(0)
		, mPrng(seed)
		, mChlNext(next)
		, mChlPrev(prev)
		, mShareIdx(0)
		, mShareGenIdx(0)
	{
		mShareBuff[0].resize(SHARE_BUFFER_SIZE);
		mShareBuff[1].resize(SHARE_BUFFER_SIZE);

		mShareGen[0].setKey(mPrng.get<block>());
	}

	ThunderRuntime::~ThunderRuntime()
	{
	}

	void ThunderRuntime::init()
	{
		block s = mShareGen[0].mRoundKey[0];

		mChlPrev.asyncSend((u8*)&s, sizeof(block));
		mChlNext.recv((u8*)&s, sizeof(block));

		mShareGen[1].setKey(s);
	}

	sInt ThunderRuntime::sIntInput(BitCount bc, u64 partyIdx)
	{
		Expects(bc.mBitCount <= 64);

		mInputByteCount += (bc.mBitCount + 7) / 8;

		Thunder::InputItem ii;
		ii.mMem = std::make_shared<Thunder::TMemory>();
		ii.mMem->mBitCount = bc.mBitCount;
		auto c = ii.mMem;
		auto ret = new ThunderInt(*this, std::move(c));
		enqueue(std::move(ii));
		return sIntBasePtr(ret);
	}

	sInt ThunderRuntime::sIntInput(sInt::ValueType data, BitCount bc)
	{
		Expects(bc.mBitCount <= 64);

		mInputByteCount += (bc.mBitCount + 7) / 8;

		Thunder::InputItem ii;
		ii.mMem = std::make_shared<Thunder::TMemory>();
		ii.mMem->mBitCount = bc.mBitCount;
		ii.mMem->mData[1] = data;
		auto c = ii.mMem;
		auto ret = new ThunderInt(*this, std::move(c));
		enqueue(std::move(ii));
		return sIntBasePtr(ret);
	}

	//i64 getWord(u8*& data, u64 bitCount)
	//{
	//	i64 ret = 0;
	//	auto size = (bitCount + 7) / 8;
	//	memcpy(&ret, data, size);
	//	data += size;
	//}


	void ThunderRuntime::processQueue()
	{
		processInputs();
		processCircuits();
		processOutputs();


	}

	void ThunderRuntime::processInputs()
	{
		if (mInputByteCount)
		{
			mBuffer.resize(mInputByteCount);
			//mShareGen

			auto iter = mBuffer.data();

			for (auto& item : mInputQueue)
			{
				auto ss = (item.mMem->mBitCount + 7) / 8;
				i64 share = getShare(item.mMem->mBitCount);
				memcpy(iter, &share, ss);
				iter += ss;
				item.mMem->mData[1] ^= share;
			}
			mChlNext.asyncSend(std::move(mBuffer));


			mBuffer.resize(mInputByteCount);
			mChlPrev.recv(mBuffer.data(), mBuffer.size());
			iter = mBuffer.data();
			while (mInputQueue.size())
			{
				auto& item = mInputQueue.front();
				auto ss = (item.mMem->mBitCount + 7) / 8;

				memcpy(&item.mMem->mData[0], iter, ss);
				iter += ss;
			}

			mInputByteCount = 0;
		}
	}

	void ThunderRuntime::processCircuits()
	{
		while (mWorkQueue.size())
		{
			auto& item = mWorkQueue.front();

			//if (item.mCircuit)
			//{
			//}

		}
	}

	void ThunderRuntime::processOutputs()
	{
	}

	u64 ThunderRuntime::getPartyIdx()
	{
		return mPartyIdx;
	}

	void ThunderRuntime::enqueue(Thunder::WorkItem && item)
	{

	}

	void ThunderRuntime::enqueue(Thunder::InputItem && item)
	{
		mInputQueue.emplace_back(item);
	}

	i64 ThunderRuntime::getShare(u32 bitCount)
	{
		auto size = (bitCount += 7) / 8;
		if (mShareIdx + size > mShareBuff[0].size())
		{
			refillBuffer();
		}

		i64 ret
			= *(u64*)((u8*)mShareBuff[0].data() + mShareIdx)
			+ *(u64*)((u8*)mShareBuff[0].data() + mShareIdx);

		mShareIdx += size;

		return ret & (u64(-1) >> (64 - bitCount));

	}

	void ThunderRuntime::refillBuffer()
	{
		mShareGen[0].ecbEncCounterMode(mShareGenIdx, mShareBuff[0].size(), mShareBuff[0].data());
		mShareGen[1].ecbEncCounterMode(mShareGenIdx, mShareBuff[1].size(), mShareBuff[1].data());
		mShareGenIdx += mShareBuff[0].size();
		mShareIdx = 0;
	}


}
