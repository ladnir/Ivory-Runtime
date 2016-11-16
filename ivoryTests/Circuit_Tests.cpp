#include "Circuit_Tests.h"


#include "Circuit/Circuit.h"
//#include "Circuit/DagCircuit.h"
//#include "MyAssert.h"
#include <fstream>
#include "Common.h"
#include "Common/Log.h"
#include "Common/BitVector.h"
#include "Crypto/AES.h"
#include "Crypto/PRNG.h"
#include "DebugCircuits.h"

using namespace osuCrypto;


#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif


void Circuit_BrisRead_SHA_Test_Impl()
{
	// SHA test vectors
	// in = 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
	//		 
	// in = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f
	// out = fc99a2df88f42a7a7bb9d18033cdc6a20256755f9d5b9a5044a9cc315abe84a7
	// 
	// in = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
	// out = ef0c748df4da50a8d6c43c013edc3ce76c9d9fa9a1458ade56eb86c0a64492d2
	// 
	// in = 243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c89452821e638d01377be5466cf34e90c6cc0ac29b7c97c50dd3f84d5b5b5470917
	// out = cf0ae4eb67d38ffeb94068984b22abde4e92bc548d14585e48dca8882d7b09ce



	//BitVector input(512);
	//Circuit cir;
	//std::fstream in;
	//in.open("../../circuits/sha-256.txt");

	//if (in.is_open() == false)
	//	throw UnitTestFail();

	//cir.readBris(in, false);

	//std::fstream out;
	//out.open("../../circuits/sha-256.out");
	////cir.writeBris(out);
	//BitVector output;

	//cir.evaluate(input);
	//cir.translate(input, output);


	//u8 data[4 * 8]{
	//	0xda, 0x56, 0x98 , 0xbe, 0x17, 0xb9, 0xb4, 0x69,
	//	0x62, 0x33, 0x57 , 0x99, 0x77, 0x9f, 0xbe, 0xca,
	//	0x8c, 0xe5, 0xd4 , 0x91, 0xc0, 0xd2, 0x62, 0x43,
	//	0xba, 0xfe, 0xf9 , 0xea, 0x18, 0x37, 0xa9, 0xd8
	//};

	//BitVector expectedOut((u8*)data, 256);

	//if (expectedOut != output)
	//{
	//	//Log::out << "in   " << input.hex() << Log::endl;
	//	//Log::out << "out  " << output.hex() << Log::endl;
	//	//Log::out << "outx " << expectedOut.hex() << Log::endl;
	//	throw UnitTestFail();
	//}
}

void Circuit_BrisRead_AES_Test_Impl()
{

	//Circuit cir;
	//std::fstream in;
	//in.open("../../circuits/AES-non-expanded.txt");

	//if (in.is_open() == false)
	//	throw UnitTestFail();

	//cir.readBris(in, true);

	//block key = ZeroBlock;
	//block data = ZeroBlock;
	//block enc;

	//AES keyShed(key);
	//keyShed.ecbEncBlock(data, enc);

	//BitVector labels, output;
	//labels.reserve(cir.WireCount());
	//labels.resize(256);

	//cir.evaluate(labels);
	//cir.translate(labels, output);

	//BitVector expected;
	//expected.assign(enc);


	//if (expected != output)
	//{

	//	//Log::out << "out  " << output.hex() << Log::endl;
	//	//Log::out << "outx " << expected.hex() << Log::endl;		
	//	throw UnitTestFail();
	//}
}



void Circuit_Gen_Adder32_Test_Impl()
{
	Log::setThreadName("CP_Test_Thread");

	Circuit cir = AdderCircuit(32);

	BitVector labels, output;
	labels.resize(cir.WireCount());
	labels[2] = 1;
	labels[32 + 2] = 1;

	//Log::out << "in " << labels << Log::endl;

	cir.evaluate(labels);

	//Log::out << "ev " << labels << Log::endl;

	cir.translate(labels, output);

	BitVector expected(33);
	expected[3] = 1;

	for (u64 i = 0; i < 33; ++i)
	{
		if (expected[i] != output[i])
		{
			//Log::out << "ex " << expected << Log::endl;
			//Log::out << "ac " << output << Log::endl;
			throw UnitTestFail();
			
		}
	}
}

void Circuit_BrisRead_Adder32_Test_Impl()
{

	//Log::setThreadName("CP_Test_Thread");

	//Circuit cir;
	//std::fstream in;
	//in.open(testData + "/circuits/adder_32bit.txt");

	//if (in.is_open() == false)
	//	throw UnitTestFail("failed to open file: " + testData + "/circuits/adder_32bit.txt");

	//cir.readBris(in);


	//BitVector labels, output;
	//labels.reserve(cir.WireCount());
	//labels.resize(64);
	//((u32*)labels.data())[0] = (u32)-1;
	//((u32*)labels.data())[1] = (u32)1;

	////Log::out << "in " << labels << Log::endl;

	//cir.evaluate(labels);


	////Log::out << "ev " << labels << Log::endl;


	//cir.translate(labels, output);

	//BitVector expected(33);
	//expected[32] = 1;

	//for (u64 i = 0; i < 33; ++i)
	//{
	//	if (expected[i] != output[i])
	//	{
	//		//Log::out << "ex " << expected << Log::endl;
	//		//Log::out << "ac " << output << Log::endl;
	//		throw UnitTestFail("output doesnt match expected");

	//	}
	//}
}



void DagCircuit_BrisRead_Adder32_Test_Impl()
{


	//DagCircuit dag;
	//std::fstream in;
	//in.open(testData + "/circuits/adder_32bit.txt");

	//if (in.is_open() == false)
	//	throw UnitTestFail("failed to open file: " + testData + "/circuits/adder_32bit.txt");
	// 
	//dag.readBris(in);

	//Circuit c0;
	//dag.toCircuit(c0);

	//dag.removeInvertGates();

	//Circuit c1;
	//dag.toCircuit(c1);
	//PRNG prng(ZeroBlock);

	//BitVector labels, output0, output1;
	//labels.reserve(c0.WireCount());

	//for (u64 i = 0; i < 100; ++i)
	//{
	//	u32 input0 = prng.get<u32>();
	//	u32 input1 = prng.get<u32>();


	//	labels.resize(64);
	//	((u32*)labels.data())[0] = input0;
	//	((u32*)labels.data())[1] = input1;


	//	c0.evaluate(labels);
	//	c0.translate(labels, output0);

	//	labels.resize(64);
	//	c1.evaluate(labels);
	//	c1.translate(labels, output1);

	//	if (output0 != output1)
	//		throw UnitTestFail();


	//}
	//	std::fstream out;
	//	out.open("../../circuits/adder_32bit_out.txt",std::ios::trunc | std::ios::out);

	//	dag.writeBris(out);
}



void DagCircuit_RandomReduce_Test_Impl()
{/*


	u64 inputSize = 10;
	u64 numGates = 1000;
	u64 outputSize = 100;

	PRNG prng(ZeroBlock);

	DagCircuit dag;
	dag.mWireCount = inputSize + numGates;

	dag.mInputCounts[0] = inputSize / 2;
	dag.mInputCounts[1] = inputSize - dag.mInputCounts[0];

	dag.mInputs.resize(inputSize);
	for (u64 i = 0; i < inputSize; ++i)
	{
		dag.mInputs[i].mWireIdx = i;
		dag.mNodes.push_back(&dag.mInputs[i]);
	}
	dag.mNonInvertGateCount = 0;
	dag.mGates.resize(numGates);
	for (u64 i = 0; i < numGates; ++i)
	{
		auto& gate = dag.mGates[i];


		u64 input0 = prng.get<u64>() % dag.mNodes.size();
		u64 input1 = prng.get<u64>() % dag.mNodes.size();

		if (input1 == input0)
			input1 = (input1 + 1) % dag.mNodes.size();

		gate.mParents.push_back(dag.mNodes[input0]);
		u8 gt = prng.get<u8>() % ((gate.mParents[0]->isInvert()) ? 3 : 2);

		if (gt == 0)
		{
			dag.mNonInvertGateCount++;
			gate.mType = GateType::And;
			gate.mParents.push_back(dag.mNodes[input1]);
		}
		else if (gt == 1)
		{
			dag.mNonInvertGateCount++;
			gate.mType = GateType::Xor;
			gate.mParents.push_back(dag.mNodes[input1]);
		}
		else
		{
			gate.mType = GateType::na;
		}

		for (auto parent : gate.mParents)
		{
			parent->mChildren.push_back(&gate);
		}

		dag.mNodes.push_back(&gate);

		gate.mWireIdx = i + inputSize;
	}

	for (auto iter = dag.mNodes.end() - outputSize; iter != dag.mNodes.end(); ++iter)
	{
		dag.mOutputs.emplace_back();
		dag.mOutputs.back().mParent = *iter;
	}

	Circuit nonReduce;
	dag.toCircuit(nonReduce);

	if (nonReduce.Gates().size() != dag.mGates.size())
		throw UnitTestFail();

	dag.removeInvertGates();

	Circuit reduced;
	dag.toCircuit(reduced);


	if (reduced.Gates().size() != dag.mNonInvertGateCount)
		throw UnitTestFail();

	for (u64 i = 0; i < 15; ++i)
	{
		BitVector input(inputSize);
		BitVector out0, out1;
		input.randomize(prng);

		nonReduce.evaluate(input);
		nonReduce.translate(input, out0);

		reduced.evaluate(input);
		reduced.translate(input, out1);

		if (out0 != out1)
			throw UnitTestFail();
	}
*/
}