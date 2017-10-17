#pragma once
#include "libOTe/TwoChooseOne/OTExtInterface.h"
#include "cryptoTools/Common/Defines.h"
#include <unordered_map> 
#include "cryptoTools/Crypto/PRNG.h"

using namespace osuCrypto;

#ifdef GetMessage
#undef GetMessage
#endif

class OTOracleSender :
	public OtExtSender
{
public:
	OTOracleSender(const block& seed);
	~OTOracleSender();
	PRNG mPrng;
	bool hasBaseOts() const override { return true; }

	void setBaseOts(
		span<block> baseRecvOts,
		const BitVector& choices) override {};

	std::unique_ptr<OtExtSender> split() override
	{
		std::unique_ptr<OtExtSender> ret(new OTOracleSender(mPrng.get<block>()));
		return std::move(ret);
	}

	void send(
		span<std::array<block,2>> messages,
		PRNG& prng,
		Channel& chl) override;
};
