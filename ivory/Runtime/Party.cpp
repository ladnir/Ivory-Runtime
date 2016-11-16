#include "Party.h"


namespace osuCrypto
{


	LocalParty::LocalParty(Runtime & runtime, u64 partyIdx)
		: mRuntime(runtime)
		, mPartyIdx(partyIdx)
	{
	}
	RemoteParty::RemoteParty(Runtime & runtime, u64 partyIdx)
		: mRuntime(runtime)
		, mPartyIdx(partyIdx)
	{
	}
}