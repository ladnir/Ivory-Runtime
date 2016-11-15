#include "KProbe_Tests.h"

#include "Common.h"
#include "Common/Defines.h"
#include "GarbledCircuit/KProbeResistant.h"
#include "Common/Log.h"
#include <fstream>

#include "Circuit/Gate.h"

#include <array>

using namespace osuCrypto;

void KProbe_Build_Test_Impl()
{
	block seed = _mm_set_epi64x(2134562354, 6754325436543);
	PRNG prng0(seed);
	u64 inputSize = 4;
	u64 secParam = 40;

	KProbeMatrix kprobe0(inputSize, secParam, prng0);

	PRNG prng1(seed);
	KProbeMatrix kprobe1(inputSize, secParam, prng1);

	if (kprobe0.mMtx[0] != kprobe1.mMtx[0])
		throw UnitTestFail();

}


void KProbe_XORTransitive_Test_Impl()
{
	block seed = _mm_set_epi64x(2134562354, 6754325436543);
	PRNG prng(seed);
	u64 inputSize = 40;
	u64 secParam = 40;

	KProbeMatrix kprobe(inputSize, secParam, prng);

	BitVector encoding0(kprobe.encodingSize());
	BitVector encoding1(kprobe.encodingSize());

	encoding0.randomize(prng);
	encoding1.randomize(prng);

	BitVector deltaEcoding(encoding0);
	deltaEcoding ^= encoding1;

	BitVector decoding0(inputSize);
	BitVector decoding1(inputSize);
	BitVector deltaDecoding(inputSize);

	kprobe.decode(encoding0, decoding0);
	kprobe.decode(encoding1, decoding1);
	kprobe.decode(deltaEcoding, deltaDecoding);

	//Log::out << encoding0 << "  " << encoding0 << Log::endl;
	//Log::out << encoding1 << "  " << encoding1 << Log::endl;

	BitVector decodingXor(decoding0);
	decodingXor ^= decoding1;

	if (decodingXor != deltaDecoding)
		throw UnitTestFail();
}



void KProbe_BlockBitVector_Test_Impl()
{
	block seed = _mm_set_epi64x(2134562354, 6754325436543);
	PRNG prng(seed);
	u64 inputSize = 40;
	u64 secParam = 40;

	KProbeMatrix kprobe(inputSize, secParam, prng);

	BitVector encoding0(kprobe.encodingSize());

	std::vector<block> blocks(kprobe.encodingSize()), decodedBlocks;
	std::array<block, 2> zeroOne{ {ZeroBlock, OneBlock} };

	for (u64 i = 0; i < encoding0.size(); ++i)
	{
		blocks[i] = prng.get<block>() ;
		encoding0[i] = PermuteBit(blocks[i]);
	}

	BitVector decoding;
	kprobe.decode(encoding0, decoding);

	kprobe.decode(blocks, decodedBlocks);

	for (u64 i = 0; i < decodedBlocks.size(); ++i)
	{ 
		if(decoding[i] != PermuteBit(decodedBlocks[i]))
			throw UnitTestFail();
	}
}


void KProbe_SaveLoad_Test_Impl()
{
	Log::setThreadName("KProbe");
	//InitDebugPrinting("..\\test.out");

	PRNG prng(_mm_set_epi64x(222, 22));
	u64 inputSize = 128;
	u64 secParam = 35;

	std::fstream strm;
	strm.open("kProbe_test", std::ios::binary | std::ios::trunc | std::ios::out);
	if (!strm.is_open())
		throw UnitTestFail();

	KProbeMatrix kProbe(inputSize, secParam, prng);
	kProbe.save(strm);
	strm.close();

	strm.open("kProbe_test", std::ios::binary | std::ios::in);
	if (!strm.is_open())
		throw UnitTestFail();

	KProbeMatrix kpLoad;
	kpLoad.load(strm);

	if (kProbe.mMtx.size() != kpLoad.mMtx.size())
		throw UnitTestFail();

	for (u64 i = 0; i < kProbe.mMtx.size(); ++i)
	{
		if (kProbe.mMtx[i] != kpLoad.mMtx[i])
			throw UnitTestFail();
	}

}


#ifdef ENCODABLE_KPROBE
void KProbe_BitVector_Test_Impl()
{
	Log::setThreadName("KProbe");
	//InitDebugPrinting("..\\test.out");

	PRNG prng(_mm_set_epi64x(222, 22));
	u64 inputSize = 28;
	u64 secParam = 40;

	BitVector input(inputSize);
	input.randomize(prng);

	KProbeMatrix kProbe(inputSize, secParam, prng);

	BitVector encoding;
	kProbe.encode(input, encoding, prng);

	BitVector decodedInput;
	kProbe.decode(encoding, decodedInput);

	if (input != decodedInput)
	{
		Log::out << "Failed " << Log::endl
			<< input << Log::endl
			<< decodedInput << Log::endl;
		throw UnitTestFail();
	}
	else
	{
		//Log::out << "n=" << inputSize << Log::endl
		//	<< "k=" << secParam << Log::endl;

		//Log::out << "m=" << encoding.size() << Log::endl;

		//Log::out << "input/encoding" << Log::endl
		//	<< input << Log::endl
		//	<< encoding << Log::endl;
	}
}


void KProbe_ZeroLabels_Test_Impl()
{
	Log::setThreadName("KProbe");
	//InitDebugPrinting("..\\test.out");

	PRNG prng(_mm_set_epi64x(222, 22));
	u64 inputSize = 38;
	u64 secParam = 40;

	// compute the zero and one wire label values
	std::vector<block> zeroLabels(inputSize);
	for (auto& b : zeroLabels) b = prng.get<block>();

	// generate the k probe matrix
	KProbeMatrix kProbe(inputSize, secParam, prng);

	// encode the zero and one wire labels 
	std::vector<block> zeroEncoding;
	kProbe.encode(zeroLabels.data(), zeroEncoding, prng);

	// decode the zero encoded wire labels
	std::vector<block> decodedInput;
	kProbe.decode(zeroEncoding, decodedInput);

	// compare
	for (u64 i = 0; i < inputSize; ++i)
	{
		if (neq(zeroLabels[i], decodedInput[i]))
		{
			Log::out << "Failed[" << i << "] " << Log::endl
				<< zeroLabels[i] << Log::endl
				<< decodedInput[i] << Log::endl;
			throw UnitTestFail();
		}
	}
}

void KProbe_Labels_Test_Impl()
{
	Log::setThreadName("KProbe");
	//InitDebugPrinting("..\\test.out");

	PRNG prng(_mm_set_epi64x(222, 22));
	u64 inputSize = 128;
	u64 secParam = 35;

	// set receivers input value
	BitVector inputBits(inputSize);
	inputBits.randomize(prng);

	// compute the zero and one wire label values
	block xorOffset = prng.get<block>();
	std::vector<block> zeroLabels(inputSize);
	for (auto& b : zeroLabels) b = prng.get<block>();

	// generate the k probe matrix
	KProbeMatrix kProbe(inputSize, secParam, prng);

	// encode the zero and one wire labels 
	std::vector<block> zeroEncoding;
	kProbe.encode(zeroLabels.data(), zeroEncoding, prng);

	// encode the receiver's input bits
	BitVector encodedInputBits;
	kProbe.encode(inputBits, encodedInputBits, prng);

	// compute the receiver's encoded input wire labels
	std::vector<block> inputLabels(zeroEncoding.size());
	for (u64 i = 0; i < zeroEncoding.size(); ++i)
	{
		if (encodedInputBits[i])
			inputLabels[i] = zeroEncoding[i] ^ xorOffset;
		else
			inputLabels[i] = zeroEncoding[i];
	}

	// compute the receiver's expected decoded input wire labels
	std::vector<block> expectedInputLabels(inputSize);
	for (u64 i = 0; i < inputSize; ++i)
	{
		if (inputBits[i])
			expectedInputLabels[i] = zeroLabels[i] ^ xorOffset;
		else
			expectedInputLabels[i] = zeroLabels[i];
	}

	// decode the receiver's encoded wire labels
	std::vector<block> decodedInput;
	kProbe.decode(inputLabels, decodedInput);

	// compare
	for (u64 i = 0; i < inputSize; ++i)
	{
		if (neq(expectedInputLabels[i], decodedInput[i]))
		{
			Log::out << "Failed[" << i << "] " << Log::endl
				<< expectedInputLabels[i] << Log::endl
				<< decodedInput[i] << Log::endl;
			throw UnitTestFail();
		}
	}
}

#endif
