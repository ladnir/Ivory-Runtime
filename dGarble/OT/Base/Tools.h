#pragma once
 
#include "Crypto/Commit.h"
#include "Crypto/PRNG.h"
#include "Common/Defines.h"
#include "Network/Channel.h"

namespace osuCrypto {



	void mul128(block x, block y, block &xy1 , block &xy2);

	void eklundh_transpose128(std::array<block, 128>& inOut);


}
