#include "CrtInput.h"


namespace osuCrypto
{


	CrtLocalParty::CrtLocalParty(CrtRuntime & runtime, u64 partyIdx)
		: mRuntime(runtime)
		, mPartyIdx(partyIdx)
	{
	}
	CrtRemoteParty::CrtRemoteParty(CrtRuntime & runtime, u64 partyIdx)
		: mRuntime(runtime)
		, mPartyIdx(partyIdx)
	{
	}
}