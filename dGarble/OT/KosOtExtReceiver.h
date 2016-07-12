#pragma once
#include "OT/OTExtInterface.h"
#include "Network/Channel.h"
#include <vector>

#ifdef GetMessage
#undef GetMessage
#endif

namespace osuCrypto
{

	class KosOtExtReceiver :
		public OtExtReceiver
	{
	public:
		KosOtExtReceiver()
			:mHasBase(false)
		{}

		bool hasBaseOts() const override
		{
			return mHasBase;
		}

		bool mHasBase;
		std::array<std::array<PRNG, 2>, gOtExtBaseOtCount> mGens;

		void setBaseOts(
			ArrayView<std::array<block, 2>> baseSendOts)override;


		std::unique_ptr<OtExtReceiver> split() override;

		void receive(
			const BitVector& choices,
			ArrayView<block> messages,
			PRNG& prng,
			Channel& chl/*,
			std::atomic<u64>& doneIdx*/)override;


	};

}
