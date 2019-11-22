#include "cryptoTools/Network/IOService.h"
#include "cryptoTools/Network/Session.h"

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
#include "ivory/Runtime/GcInterface.h"

using namespace osuCrypto;

std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> program(std::array<Party, 2> parties, i64 myInput)
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

	std::cout << myInput << std::endl;
	// // perform some computation
	// auto add = input1 + input0;
	// auto sub = input1 - input0;
	// auto mul = input1 * input0;
	// auto div = input1 / input0;
    
    // // multiplies input 1 by 2^4
    // auto shift = input1 << 4;

    // logical operations
	auto gt = input1 >= input0;
	// auto lt = input1 < input0;

    // // select a subset of the bits
    // auto signBit = input1.copyBits(bitCount - 1, bitCount);

    // // perform if statements
	// auto max = gteq.ifelse(input1, input0);

    // assigments
	input0 = input1;


	// reveal this output to party 0.
	// parties[0].reveal(add);
	// parties[0].reveal(sub);
	// parties[0].reveal(mul);
    // parties[0].reveal(div);
    // parties[0].reveal(signBit);
	if (parties[0].isLocalParty()) {
		parties[0].reveal(gt);
	}
	else {
		parties[1].reveal(gt);
	}
	// parties[0].reveal(gteq);
	
	
	// parties[0].reveal(lt);
	// parties[0].reveal(max);


	// The garbler
	if (parties[0].isLocalParty())
	{
		// return circuit and label data
		data = gt.genLabelsCircuit();
	}

	if (parties[1].isLocalParty())
	{
		std::cout << "HERE" << std::endl;
		// std::cout << "add       " << add.getValue() << std::endl;
		// std::cout << "sub       " << sub.getValue() << std::endl;
		// std::cout << "mul       " << mul.getValue() << std::endl;
        // std::cout << "div       " << div.getValue() << std::endl;
        // std::cout << "sign(in1) " << signBit.getValue() << std::endl;
		std::cout << "gt      " << gt.getValueOffline() << std::endl;
		// std::cout << "lt        " << lt.getValue() << std::endl;
		// std::cout << "max       " << max.getValue() << std::endl;
	}

	// operations can get queued up in the background. Eventually this call should not
	// be required but in the mean time, if one party does not call getValue(), then
	// processesQueue() should be called.
	// parties[1].getRuntime().processesQueue(); //MAYBE WE STILL NEED THIS

	return data;

}

// std::vector<u8> generateCircuit(u64 reservePrice)
// {
// 	OfflineSocket sharedChannel;
// 	PRNG prng(OneBlock);
// 	bool debug = false;

// 	// set up the runtime, see above for details
// 	ShGcRuntime rt0;
// 	rt0.mDebugFlag = debug;
// 	rt0.init(sharedChannel, prng.get<block>(), ShGcRuntime::Garbler, 0);

// 	// instantiate the parties
// 	std::array<Party, 2> parties{
// 		Party(rt0, 0),
// 		Party(rt0, 1)
// 	};

// 	std::vector<u8> evalLabels = program(parties, reservePrice);
// 	return evalLabels;
// }

// u8 evaluateCircuit(std::vector<u8>& evalLabels)
// {
// 	OfflineSocket sharedChannel;
// 	bool debug = false;
// 	// We will need a random number generator. Should pass it a real seed.
// 	PRNG prng(ZeroBlock);

// 	// In this example, we will use the semi-honest Garbled Circuit
// 	// runtime. Once constructed, init should be called. We need to
// 	// provide the runtime the channel that it will use to communicate 
// 	// with the other party, a seed, what mode it should run in, and 
// 	// the local party index. 
// 	ShGcRuntime rt1;
// 	rt1.mDebugFlag = debug;
// 	std::vector<block> blockEvalLabels;
// 	auto castedEvalLabels = (block*) evalLabels.data();
// 	for (int i = 0; i < evalLabels.size()/(sizeof(block)); i++) {
// 		blockEvalLabels.push_back(castedEvalLabels[i]);
// 		// std::cout << zeros[i] << std::endl;
// 	}

// 	rt1.init(sharedChannel, prng.get<block>(), ShGcRuntime::Evaluator, 1, blockEvalLabels);

// 	// We can then instantiate the parties that will be running the protocol.
// 	std::array<Party, 2> parties{
// 		Party(rt1, 0),
// 		Party(rt1, 1)
// 	};
	
// 	// the prgram take the parties that are participating and the input
// 	// of the local party, though since we're setting the eval labels directly this
// 	// value is not actually necessary
// 	program(parties, 0);
// }

void party1(std::string ip, OfflineSocket& shared_channel, std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>>& data)
{


	u64 tries(1);
	bool debug = false;

	// // IOSerive will perform the networking operations in the background
	// IOService ios;

	// // Session represents one end of a connection. It facilitates the
	// // creation of sockets that all bind to this port. First we pass it the 
	// // IOSerive and then the server's IP:port number. Next we state that 
	// // this Session should act as a server (listens to the provided port).
	// Session ep1(ios, ip, SessionMode::Server);

	// // We can now create a socket. This is done with addChannel. This operation 
	// // is asynchronous. If additional connections are needed between the 
	// // two parties, call addChannel again.
	// Channel chl1 = ep1.addChannel();

	// // this is an optional call that blocks until the socket has successfully 
	// // been set up.
	// chl1.waitForConnection();

	// We will need a random number generator. Should pass it a real seed.
	PRNG prng(ZeroBlock);
	// In this example, we will use the semi-honest Garbled Circuit
	// runtime. Once constructed, init should be called. We need to
	// provide the runtime the channel that it will use to communicate 
	// with the other party, a seed, what mode it should run in, and 
	// the local party index.

	auto evalLabels = std::get<0>(data);
	auto q_u8 = std::get<1>(data);
	auto q_gate = std::get<2>(data);

	shared_channel.setQGate(q_gate);
	shared_channel.setQu8(q_u8);

	ShGcRuntime rt1;
	rt1.mDebugFlag = debug;
	auto zeros = ((block*) evalLabels.data());
	std::cout << "POTENTIAL EVAL LABEL VALS" << std::endl;
	for (int i = 0; i < evalLabels.size()/(sizeof(block)); i++) {
		std::cout << zeros[i] << std::endl;
	}
	std::vector<block> zeroEvalLabels;

	for (int i = evalLabels.size()/(2*sizeof(block)); i < evalLabels.size()/(sizeof(block))-10; i++) {
		zeroEvalLabels.push_back(zeros[i]);
		// std::cout << zeros[i] << std::endl;
	}
	for (int i = evalLabels.size()/(2*sizeof(block))-10; i < evalLabels.size()/(2*sizeof(block)); i++) {
		zeroEvalLabels.push_back(zeros[i]);
		// std::cout << zeros[i] << std::endl;
	}
	// zeroEvalLabels.push_back(zeros[evalLabels.size()/(sizeof(block))-1]);
	rt1.init(shared_channel, prng.get<block>(), ShGcRuntime::Evaluator, 1, zeroEvalLabels);

	// We can then instantiate the parties that will be running the protocol.
	std::array<Party, 2> parties{
		Party(rt1, 0),
		Party(rt1, 1)
	};
	// Next, lets call the main "program" several times.
	for (u64 i = 0; i < tries; ++i)
	{
		// the prgram take the parties that are participating and the input
		// of the local party, in this case its 44.
		program(parties, 44);
	}
}

std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> party0(std::string ip, OfflineSocket& shared_channel)
{

	u64 tries(1);
	PRNG prng(OneBlock);
	bool debug = false;

	// // IOSerive will perform the networking operations in the background
	// IOService ios;

	// // set up networking. See above for details
	// Session ep0(ios, ip, SessionMode::Client);
	// Channel chl0 = ep0.addChannel();

	// set up the runtime, see above for details
	ShGcRuntime rt0;
	rt0.mDebugFlag = debug;
	rt0.init(shared_channel, prng.get<block>(), ShGcRuntime::Garbler, 0);

	// instantiate the parties
	std::array<Party, 2> parties{
		Party(rt0, 0),
		Party(rt0, 1)
	};

	std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> data;
	// run the program serveral time, with time with 23 as the input value
	for (u64 i = 0; i < tries; ++i)
	{
		data = program(parties, 64);
	}
	return data;
}

int main(int argc, char**argv)
{

	// // parse command line options
	// CLP cmd(argc, argv);

	// // Mock socket
	// OfflineSocket shared_channel;
	// OfflineSocket shared_channel_1;
	// std::tuple<std::vector<u8>, std::deque<u8>, std::deque<block>> hold;

	// if (cmd.isSet("r"))
	// {
	// 	// this is the control flow in the event that we want to run the protocol between two programs.

	// 	auto r = cmd.getOr("r", -1);
	// 	auto ip = cmd.getOr("ip", std::string{ "127.0.0.1:1212" });

	// 	if (r == 0)
	// 	{
	// 		party0(ip, shared_channel);
	// 	}
	// 	else if (r == 1)
	// 	{
	// 		party1(ip, shared_channel, hold);
	// 	}
	// 	else
	// 	{
	// 		std::cout << "the -r flag needs to have a value of 0 or 1" << std::endl;
	// 	}
	// }
	// else
	// {
	// 	// here we run both parties in a single program.

	// 	// We need a second thread to run the other party.
	// 	auto data = party0("127.0.0.1:1212", shared_channel);

	// 	// std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	// 	party1("127.0.0.1:1212", shared_channel_1, data);
	// }
	std::cout << "gt " << testPipeline(64) << std::endl;
	return 0;
}

