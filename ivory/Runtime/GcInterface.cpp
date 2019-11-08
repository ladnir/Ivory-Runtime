#include "GcInterface.h"

using namespace osuCrypto;

std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> garblerProgram(std::array<Party, 2> parties, i64 myInput)
{
	// choose how large the arithmetic should be.
	u64 bitCount = 16;

	std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> data;

	// get the two input variables. If this party is the local party, then 
	// lets use our input value. Otherwise the remote party will provide the value.
	// In addition, the bitCount parameter means a value with that many bits
	// will fit into this secure variable. However, the runtime reserver the right
	// to increase the bits or to use something like a prime feild, in which case
	// the exact wrap around point is undefined. However, the binary circuit base runtimes
	// will always use exactly that many bits.
	auto input0 = parties[0].isLocalParty() ?
		parties[0].input<sInt>(myInput, bitCount) :
		parties[0].input<sInt>(bitCount);

	auto input1 = parties[1].isLocalParty() ?
		parties[1].input<sInt>(myInput, bitCount) :
		parties[1].input<sInt>(bitCount);


    // logical operations
	auto gt = input1 >= input0;

    // assigments
	input0 = input1;


	// reveal this output to party 0.

	if (parties[0].isLocalParty()) {
		parties[0].reveal(gt);
	}
	else {
		parties[1].reveal(gt);
    }


	// The garbler
	if (parties[0].isLocalParty())
	{
		// return circuit and label data
		data = gt.genLabelsCircuit();
	}

	// if (parties[1].isLocalParty())
	// {
	// 	// std::cout << "add       " << add.getValue() << std::endl;
	// 	// std::cout << "sub       " << sub.getValue() << std::endl;
	// 	// std::cout << "mul       " << mul.getValue() << std::endl;
    //     // std::cout << "div       " << div.getValue() << std::endl;
    //     // std::cout << "sign(in1) " << signBit.getValue() << std::endl;
	// 	std::cout << "gt      " << gt.getValueOffline() << std::endl;
	// 	// std::cout << "lt        " << lt.getValue() << std::endl;
	// 	// std::cout << "max       " << max.getValue() << std::endl;
	// }
	return data;
}

i64 evaluatorProgram(std::array<Party, 2> parties, i64 myInput)
{
	// choose how large the arithmetic should be.
	u64 bitCount = 16;

	// get the two input variables. If this party is the local party, then 
	// lets use our input value. Otherwise the remote party will provide the value.
	// In addition, the bitCount parameter means a value with that many bits
	// will fit into this secure variable. However, the runtime reserver the right
	// to increase the bits or to use something like a prime feild, in which case
	// the exact wrap around point is undefined. However, the binary circuit base runtimes
	// will always use exactly that many bits.
	auto input0 = parties[0].isLocalParty() ?
		parties[0].input<sInt>(myInput, bitCount) :
		parties[0].input<sInt>(bitCount);

	auto input1 = parties[1].isLocalParty() ?
		parties[1].input<sInt>(myInput, bitCount) :
		parties[1].input<sInt>(bitCount);


    // logical operations
	auto gt = input1 >= input0;

    // assigments
	input0 = input1;


	// reveal this output to party 0.

	if (parties[0].isLocalParty()) {
		parties[0].reveal(gt);
	}
	else {
		parties[1].reveal(gt);
    }

	return gt.getValueOffline();
}

std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> generateCircuit(u64 reservePrice)
{
	OfflineSocket sharedChannel;
	PRNG prng(OneBlock);
	bool debug = false;

	// set up the runtime, see above for details
	ShGcRuntime rt0;
	rt0.mDebugFlag = debug;
	rt0.init(sharedChannel, prng.get<block>(), ShGcRuntime::Garbler, 0);

	// instantiate the parties
	std::array<Party, 2> parties{
		Party(rt0, 0),
		Party(rt0, 1)
	};

	auto data = garblerProgram(parties, reservePrice);
	return data;
}

i64 evaluateCircuit(std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>>& data)
{
	OfflineSocket sharedChannel;
	bool debug = false;
	// We will need a random number generator. Should pass it a real seed.
	PRNG prng(ZeroBlock);

	// In this example, we will use the semi-honest Garbled Circuit
	// runtime. Once constructed, init should be called. We need to
	// provide the runtime the channel that it will use to communicate 
	// with the other party, a seed, what mode it should run in, and 
	// the local party index. 
	ShGcRuntime rt1;
	rt1.mDebugFlag = debug;

    auto evalLabels = std::get<0>(data);
	auto q_u8 = std::get<1>(data);
	auto q_gate = std::get<2>(data);

    // Set channel to have proper circuit/garbler input values passed from garbler
	sharedChannel.setQGate(q_gate);
	sharedChannel.setQu8(q_u8);

	std::vector<block> blockEvalLabels;
	auto castedEvalLabels = (block*) evalLabels.data();
	for (int i = 0; i < evalLabels.size()/(sizeof(block)); i++) {
		blockEvalLabels.push_back(castedEvalLabels[i]);
	}

	rt1.init(sharedChannel, prng.get<block>(), ShGcRuntime::Evaluator, 1, blockEvalLabels);

	// We can then instantiate the parties that will be running the protocol.
	std::array<Party, 2> parties{
		Party(rt1, 0),
		Party(rt1, 1)
	};
	
	// the prgram take the parties that are participating and the input
	// of the local party, though since we're setting the eval labels directly this
	// value is not actually necessary
	return evaluatorProgram(parties, 0);
}

i64 testPipeline(u64 reservePrice)
{
    auto data = generateCircuit(reservePrice);
    return evaluateCircuit(data);
}