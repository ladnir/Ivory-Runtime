#pragma once
#include "ivory/Runtime/Runtime.h"
#include "ivory/Circuit/AlphaLibrary.h"
#include "ivory/Runtime/Thunder/util.h"
#include <queue>
#include <deque>
#include "cryptoTools/Network/Channel.h"
#include "cryptoTools/Crypto/PRNG.h"

namespace osuCrypto
{

	class ThunderRuntime : public Runtime
	{
	public:
		u64 mPartyIdx, mInputByteCount;
		PRNG mPrng;
		Channel mChlNext, mChlPrev;
		std::vector<u8> mBuffer;

		ThunderRuntime(u64 partyIdx, block seed, Channel& next, Channel& prev);
		~ThunderRuntime();

		void init();

		AlphaLibrary mLibrary;

		sInt sIntInput(BitCount bitCount, u64 partyIdx) override;
		sInt sIntInput(sInt::ValueType data, BitCount bitCount) override;

		// processQueue() will ensure that all scheduled operations have been completed
		//    before returning.
		// Assumptions: None
		// Result: Upon return, all operations are either in process or completed.
		void processQueue() override;

		void processInputs();
		void processCircuits();
		void processOutputs();

		// getPartyIdx() returns the index of the local party.
		u64 getPartyIdx() override;


		void enqueue(Thunder::WorkItem&& item);
		void enqueue(Thunder::InputItem&& item);

		//std::queue<Thunder::WorkItem> mWorkQueue;
		//std::vector<
		AlphaStream mCircuitStream;
		std::deque<Thunder::InputItem> mInputQueue;



		std::array<std::vector<block>, 2> mShareBuff;
		u64 mShareIdx;
		std::array<AES, 2> mShareGen;
		u64 mShareGenIdx;

		i64 getShare(u32 bitCount);
		void refillBuffer();

	};

}
