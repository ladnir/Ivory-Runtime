#include <fstream>
#include <iostream>
#include "cryptoTools/Common/Log.h"
#include "cryptoTools/Common/Timer.h"
#include "ivory/Runtime/ShGc/ShGcRuntime.h"
//#include "ivory/Runtime/ClearRuntime.h"
#include "ivory/Runtime/sInt.h"
#include "ivory/Runtime/Party.h"

#include <string>
#include "cryptoTools/Crypto/PRNG.h"
#include "cryptoTools/Common/CLP.h"

using namespace osuCrypto;


std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> garblerProgram(std::array<Party, 2> parties, i64 myInput);

i64 evaluatorProgram(std::array<Party, 2> parties, i64 myInput);

std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> generateCircuit(u64 reservePrice);

i64 evaluateCircuit(std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>>& data);

i64 testPipeline(u64 reserveprice);
