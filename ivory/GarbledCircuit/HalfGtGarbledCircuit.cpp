#include "HalfGtGarbledCircuit.h"
#include "cryptopp/randpool.h"
#include "Network/Channel.h"
#include "Common/ByteStream.h"
#include "Common/Log.h"
#include "cryptopp/sha.h"
#include <cassert>

namespace osuCrypto 
{

	const AES HalfGtGarbledCircuit::mAesFixedKey(_mm_set_epi8(36, -100, 50, -22, 92, -26, 49, 9, -82, -86, -51, -96, 98, -20, 29, -13));


	HalfGtGarbledCircuit::HalfGtGarbledCircuit(HalfGtGarbledCircuit&& src)
		:
		mGlobalOffset(src.mGlobalOffset),
		mInputWires(std::move(src.mInputWires)),
		mOutputWires(std::move(src.mOutputWires)),
		mGates(std::move(src.mGates)),
		mTranslationTable(std::move(src.mTranslationTable))
	{

	}


	HalfGtGarbledCircuit::~HalfGtGarbledCircuit()
	{
	}

	void HalfGtGarbledCircuit::garbleStream(
		CircuitStream & cd, 
		const block & seed, 
		Channel & chl, 
		std::vector<block>& wires,
		std::function<void(std::vector<std::array<block, 2>>)> sendInputsCallback)
	{

		wires.resize(cd.getInternalWireBuffSize());

		mOutputWires.clear();

		AES aesSeedKey(seed);

		//create the delta for the free Xor. Encrypt zero twice. We get a good enough random delta by encrypting twice
		aesSeedKey.ecbEncBlock(ZeroBlock, mGlobalOffset);
		aesSeedKey.ecbEncBlock(mGlobalOffset, mGlobalOffset);
		
		// make sure the bottom bit is a 1 for point-n-permute
		*(u8*)&(mGlobalOffset) |= 1; 

		block temp[4], hash[4], tweaks[2]{ ZeroBlock, _mm_set_epi64x(1, 0) };
		std::array<block, 2> zeroAndGlobalOffset{ { ZeroBlock, mGlobalOffset } };

		u8 aPermuteBit, bPermuteBit, bAlphaBPermute, cPermuteBit;
		u64 inputIndex(0);

		while (cd.hasMoreGates())
		{
			auto buff = std::unique_ptr<ByteStream>(new ByteStream());
			buff->resize(sizeof(GarbledGate<2>) * cd.getNonXorGateCount());
			auto gateIter = reinterpret_cast<GarbledGate<2>*>(buff->data());

			auto gates = cd.getMoreGates();
			auto outputIdxs = cd.getOutputIndices();
			auto inputIdxs = cd.getInputIndices();

			std::vector<std::array<block, 2>> inputs(inputIdxs.size());

			// generate the needed input wires. they will be AES counter mode encryptions with start value
			// inputIndex and will be stored in the wires vector at the locations specified by inputIdxs
			mInputWires.resize(inputIdxs.size());
			aesSeedKey.ecbEncCounterMode(inputIndex, inputIdxs.size(), mInputWires.data());
			for (u64 i = 0; i < inputIdxs.size(); ++i, ++inputIndex)
			{
				inputs[i][0] = wires[inputIdxs[i]] = mInputWires[inputIndex];
				inputs[i][1] = mInputWires[inputIndex] ^ mGlobalOffset;
			}

			sendInputsCallback(std::move(inputs));


			for (const auto& gate : gates)
			{

				auto& c = wires[gate.mWireIdx];
				auto& a = wires[gate.mInput[0]];
				auto& b = wires[gate.mInput[1]];
				auto gt = gate.Type();


				if (gt == GateType::Xor || gt == GateType::Nxor) 
				{
					c = a ^ b ^ zeroAndGlobalOffset[(u8)gt & 1];
				}
				else 
				{
					// compute the gate modifier variables
					auto& aAlpha = gate.AAlpha();
					auto& bAlpha = gate.BAlpha();
					auto& cAlpha = gate.CAlpha();

					//signal bits of wire 0 of input0 and wire 0 of input1
					aPermuteBit = PermuteBit(a);
					bPermuteBit = PermuteBit(b);
					bAlphaBPermute = bAlpha ^ bPermuteBit;
					cPermuteBit = ((aPermuteBit ^ aAlpha) && (bAlphaBPermute)) ^ cAlpha;

					// compute the hashs of the wires as H(x) = AES_f( x * 2 ^ tweak) ^ (x * 2 ^ tweak)    
					hash[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
					hash[1] = _mm_slli_epi64((a ^ mGlobalOffset), 1) ^ tweaks[0];
					hash[2] = _mm_slli_epi64(b, 1) ^ tweaks[1];
					hash[3] = _mm_slli_epi64((b ^ mGlobalOffset), 1) ^ tweaks[1];
					mAesFixedKey.ecbEncFourBlocks(hash, temp);
					hash[0] = hash[0] ^ temp[0]; // H( a0 )
					hash[1] = hash[1] ^ temp[1]; // H( a1 )
					hash[2] = hash[2] ^ temp[2]; // H( b0 )
					hash[3] = hash[3] ^ temp[3]; // H( b1 )

					// increment the tweaks
					tweaks[0] = tweaks[0] + OneBlock;
					tweaks[1] = tweaks[1] + OneBlock;

					// generate the garbled table
					auto& garbledTable = gateIter++->mGarbledTable;

					// compute the table entries
					garbledTable[0] = hash[0] ^ hash[1] ^ zeroAndGlobalOffset[bAlphaBPermute];
					garbledTable[1] = hash[2] ^ hash[3] ^ a ^ zeroAndGlobalOffset[aAlpha];

					// compute the out wire
					c = hash[aPermuteBit] ^
						hash[2 ^ bPermuteBit] ^
						zeroAndGlobalOffset[cPermuteBit];
				}
			}

			chl.asyncSend(std::move(buff));

			mOutputWires.resize(mOutputWires.size() + outputIdxs.size());
			mTranslationTable.resize(mTranslationTable.size() + outputIdxs.size());
			for (u64 i = 0; i < outputIdxs.size(); ++i)
			{
				mOutputWires.push_back(wires[outputIdxs[i]]);
				mTranslationTable[i] = PermuteBit(wires[outputIdxs[i]]);
			}

		}

		chl.asyncSendCopy(mTranslationTable);

#ifdef STRONGEVAL
		mInternalWires = wires;
#endif

	}



	void HalfGtGarbledCircuit::evaluateStream(
		CircuitStream& cd,
		Channel& chl,
		std::vector<block>& wires,
		std::function<ArrayView<block>(u64)> receiveInputCallback)
	{
		wires.resize(cd.getInternalWireBuffSize());
		mOutputWires.clear();


		block temp[2], hashs[2], tweaks[2]{ ZeroBlock, _mm_set_epi64x(1,0) }, zeroAndGarbledTable[2][2]{ { ZeroBlock,ZeroBlock}, {ZeroBlock,ZeroBlock} };
		ArrayView<block> inputs;
		ByteStream buff;

		while (cd.hasMoreGates())
		{
			chl.recv(buff);
			auto gateTables = buff.getArrayView<GarbledGate<2>>();
			auto garbledGateIter = gateTables.begin();


			auto gates = cd.getMoreGates();
			auto outputIdxs = cd.getOutputIndices();
			auto inputIdxs = cd.getInputIndices();

			if (gateTables.size() != cd.getNonXorGateCount())
				throw std::runtime_error("");

			auto inputs = receiveInputCallback(inputIdxs.size());

			for (auto i = 0; i < inputIdxs.size(); ++i)
				wires[inputIdxs[i]] = inputs[i];


			for (const auto& gate : gates) 
			{
				auto& a = wires[gate.mInput[0]];
				auto& b = wires[gate.mInput[1]];
				auto& c = wires[gate.mWireIdx];
				auto& gt = gate.Type();

				if (gt == GateType::Xor || gt == GateType::Nxor) 
				{
					c = a ^ b;
				}
				else 
				{
					// compute the hashs
					hashs[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
					hashs[1] = _mm_slli_epi64(b, 1) ^ tweaks[1];
					mAesFixedKey.ecbEncTwoBlocks(hashs, temp);
					hashs[0] = temp[0] ^ hashs[0]; // a
					hashs[1] = temp[1] ^ hashs[1]; // b

					// increment the tweaks
					tweaks[0] = tweaks[0] + OneBlock;
					tweaks[1] = tweaks[1] + OneBlock;


					auto& garbledTable = garbledGateIter++->mGarbledTable;

					zeroAndGarbledTable[1][0] = garbledTable[0];
					zeroAndGarbledTable[1][1] = garbledTable[1] ^ a;

					// compute the output wire label
					c = hashs[0] ^ 
						hashs[1] ^ 
						zeroAndGarbledTable[PermuteBit(a)][0] ^ 
						zeroAndGarbledTable[PermuteBit(b)][1];
				}
			}


			u64 i = mOutputWires.size();
			mOutputWires.resize(mOutputWires.size() + outputIdxs.size());

			for (auto idx : outputIdxs)
				mOutputWires[i++] = wires[idx];
		}

		mTranslationTable.resize(mOutputWires.size());
		chl.recv(mTranslationTable);

	}




	void HalfGtGarbledCircuit::GarbleSend(
		const Circuit & cd, 
		const block & seed, 
		Channel & chl,
		std::vector<block>& wires
#ifdef ADAPTIVE_SECURE 
		, std::vector<block> tableMasks
#endif
		)
	{
		//mGates.reserve(cd.NonXorGateCount());

		mInputWires.clear();
		mInputWires.resize(cd.InputWireCount());

		mOutputWires.clear();
		//mGates.clear();

		AES aesSeedKey(seed);

		//create the delta for the free Xor. Encrypt zero twice. We get a good enough random delta by encrypting twice
		aesSeedKey.ecbEncBlock(ZeroBlock, mGlobalOffset);
		aesSeedKey.ecbEncBlock(mGlobalOffset, mGlobalOffset);
		*(u8*)&(mGlobalOffset) |= 1; // make sure the bottom bit is a 1 for point-n-permute

		// Compute the input labels as AES permutations on mIndexArray
		mInputWires.resize(cd.InputWireCount());
		aesSeedKey.ecbEncCounterMode(0, mInputWires.size(), (block*)mInputWires.data());
		std::copy(mInputWires.begin(), mInputWires.end(), wires.begin());

		block temp[4], hash[4], tweaks[2]{ ZeroBlock, _mm_set_epi64x(0, cd.Gates().size()) };
		u8 aPermuteBit, bPermuteBit;

		auto buff = std::unique_ptr<ByteStream>(new ByteStream(sizeof(GarbledGate<2>) * cd.NonXorGateCount()));
		buff->setp(sizeof(GarbledGate<2>) * cd.NonXorGateCount());
		std::array<block, 2> zeroAndGlobalOffset{ { ZeroBlock, mGlobalOffset } };


		//Log::out << "cir size " << buff->size() << Log::endl;
		auto gateIter = reinterpret_cast<GarbledGate<2>*>(buff->data());
#ifdef ADAPTIVE_SECURE 
		auto maskIter = tableMasks.begin();
#endif

		for (const auto& gate : cd.Gates())
		{
			//mInputWires.emplace_back();
			auto& c = wires[gate.mWireIdx];
			auto& a = wires[gate.mInput[0]];
			auto& b = wires[gate.mInput[1]];
			auto gt = gate.Type();

//#ifndef NDEBUG
//			if (gt == GateType::Zero || gt == GateType::One || gt == GateType::na || gt == GateType::nb || gt == GateType::a || gt == GateType::b)
//				throw std::runtime_error("Constant/unary gates not supported");
//#endif
			if (gt == GateType::Xor || gt == GateType::Nxor) {
				c = a ^ b ^ zeroAndGlobalOffset[(u8)gt & 1];
			}
			else {
				// generate the garbled table
				auto& garbledTable = gateIter++->mGarbledTable;


				// compute the gate modifier variables
				auto& aAlpha = gate.AAlpha();
				auto& bAlpha = gate.BAlpha();
				auto& cAlpha = gate.CAlpha();

				//signal bits of wire 0 of input0 and wire 0 of input1
				aPermuteBit = PermuteBit(a);
				bPermuteBit = PermuteBit(b);

				// compute the hashs of the wires as H(x) = AES_f( x * 2 ^ tweak) ^ (x * 2 ^ tweak)    
				hash[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
				hash[1] = _mm_slli_epi64((a ^ mGlobalOffset), 1) ^ tweaks[0];
				hash[2] = _mm_slli_epi64(b, 1) ^ tweaks[1];
				hash[3] = _mm_slli_epi64((b ^ mGlobalOffset), 1) ^ tweaks[1];
				mAesFixedKey.ecbEncFourBlocks(hash, temp);
				hash[0] = hash[0] ^ temp[0]; // H( a0 )
				hash[1] = hash[1] ^ temp[1]; // H( a1 )
				hash[2] = hash[2] ^ temp[2]; // H( b0 )
				hash[3] = hash[3] ^ temp[3]; // H( b1 )

											 // increment the tweaks
				tweaks[0] = tweaks[0] + OneBlock;
				tweaks[1] = tweaks[1] + OneBlock;

				// compute the table entries
				garbledTable[0] = hash[0] ^ hash[1] ^ zeroAndGlobalOffset[bAlpha ^ bPermuteBit];
				garbledTable[1] = hash[2] ^ hash[3] ^ a ^ zeroAndGlobalOffset[aAlpha];

				c = hash[aPermuteBit] ^
					hash[2 ^ bPermuteBit] ^
					zeroAndGlobalOffset[((aPermuteBit ^ aAlpha) && (bPermuteBit ^ bAlpha)) ^ cAlpha];


#ifdef ADAPTIVE_SECURE 
				garbledTable[0] = garbledTable[0] ^ *maskIter++;
				garbledTable[1] = garbledTable[1] ^ *maskIter++;
#endif
			}
		}

		chl.asyncSend(std::move(buff));

		mOutputWires.reserve(cd.Outputs().size());
		mTranslationTable.reset(cd.OutputCount());
		for (u64 i = 0; i < cd.Outputs().size(); ++i)
		{
			mOutputWires.push_back(wires[cd.Outputs()[i]]);
			mTranslationTable[i] = PermuteBit(wires[cd.Outputs()[i]]);
		}
		chl.asyncSendCopy(mTranslationTable);

#ifdef STRONGEVAL
		mInternalWires = wires;
#endif
	}



	void HalfGtGarbledCircuit::Garble(const Circuit& cd, const block& seed
#ifdef ADAPTIVE_SECURE 
		, std::vector<block> tableMasks
#endif
		)
	{

		//assert(indexArray.size() >= mInputWires.size());
		//assert(eq(indexArray[0], ZeroBlock));

		mGates.reserve(cd.NonXorGateCount());

		mInputWires.clear();
		mInputWires.reserve(cd.WireCount());

		mOutputWires.clear();
		mGates.clear();

		AES aesSeedKey(seed);

		//create the delta for the free Xor. Encrypt zero twice. We get a good enough random delta by encrypting twice
		aesSeedKey.ecbEncBlock(ZeroBlock, mGlobalOffset);
		aesSeedKey.ecbEncBlock(mGlobalOffset, mGlobalOffset);
		*(u8*)&(mGlobalOffset) |= 1; // make sure the bottom bit is a 1 for point-n-permute

		// Compute the input labels as AES permutations on mIndexArray
		mInputWires.resize(cd.InputWireCount());
		aesSeedKey.ecbEncCounterMode(0, mInputWires.size(), (block*)mInputWires.data());

		block temp[4], hash[4], tweaks[2]{ ZeroBlock, _mm_set_epi64x(0, cd.Gates().size()) };
		u8 aPermuteBit, bPermuteBit;

		std::array<block, 2> zeroAndGlobalOffset{ {ZeroBlock, mGlobalOffset} };

#ifdef ADAPTIVE_SECURE 
		auto maskIter = tableMasks.begin();
#endif

		for (const auto& gate : cd.Gates())
		{
			mInputWires.emplace_back();
			auto& c = mInputWires.back();
			auto& a = mInputWires[gate.mInput[0]];
			auto& b = mInputWires[gate.mInput[1]];
			auto gt = gate.Type();

#ifndef NDEBUG
			//if (gt == GateType::Zero || gt == GateType::One || gt == GateType::na || gt == GateType::nb || gt == GateType::a || gt == GateType::b )
			//	throw std::runtime_error("Constant/unary gates not supported");
#endif
			if (gt == GateType::Xor || gt == GateType::Nxor) {
				c = a ^ b ^ zeroAndGlobalOffset[ (u8)gt & 1];
			}
			else {
				// generate the garbled table
				mGates.emplace_back();
				auto& garbledTable = mGates.back().mGarbledTable;

				// compute the gate modifier variables
				auto& aAlpha = gate.AAlpha();
				auto& bAlpha = gate.BAlpha();
				auto& cAlpha = gate.CAlpha();

				//signal bits of wire 0 of input0 and wire 0 of input1
				aPermuteBit = PermuteBit(a);
				bPermuteBit = PermuteBit(b);

				// compute the hashs of the wires as H(x) = AES_f( x * 2 ^ tweak) ^ (x * 2 ^ tweak)    
				hash[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
				hash[1] = _mm_slli_epi64((a ^ mGlobalOffset), 1) ^ tweaks[0];
				hash[2] = _mm_slli_epi64(b, 1) ^ tweaks[1];
				hash[3] = _mm_slli_epi64((b ^ mGlobalOffset), 1) ^ tweaks[1];
				mAesFixedKey.ecbEncFourBlocks(hash, temp);
				hash[0] = hash[0] ^ temp[0]; // H( a0 )
				hash[1] = hash[1] ^ temp[1]; // H( a1 )
				hash[2] = hash[2] ^ temp[2]; // H( b0 )
				hash[3] = hash[3] ^ temp[3]; // H( b1 )

				// increment the tweaks
				tweaks[0] = tweaks[0] + OneBlock;
				tweaks[1] = tweaks[1] + OneBlock;

				// compute the table entries
				garbledTable[0] = hash[0] ^ hash[1] ^ zeroAndGlobalOffset[bAlpha ^ bPermuteBit];
				garbledTable[1] = hash[2] ^ hash[3] ^ a ^ zeroAndGlobalOffset[aAlpha];

				c = hash[aPermuteBit] ^
					hash[2 ^ bPermuteBit] ^
					zeroAndGlobalOffset[((aPermuteBit ^ aAlpha) && (bPermuteBit ^ bAlpha)) ^ cAlpha];
#ifdef ADAPTIVE_SECURE 
				garbledTable[0] = garbledTable[0] ^ *maskIter++;
				garbledTable[1] = garbledTable[1] ^ *maskIter++;
#endif
			}
		}

		mOutputWires.reserve(cd.Outputs().size());
		mTranslationTable.reset(cd.OutputCount());
		for (u64 i = 0; i < cd.Outputs().size(); ++i)
		{
			mOutputWires.push_back(mInputWires[cd.Outputs()[i]]);
			mTranslationTable[i] = PermuteBit(mInputWires[cd.Outputs()[i]]);
		}


#ifdef STRONGEVAL
		mInternalWires = mInputWires;
#endif
		mInputWires.resize(cd.InputWireCount());
		mInputWires.shrink_to_fit();
	}

	void HalfGtGarbledCircuit::evaluate(const Circuit& cd, std::vector<block>& labels
#ifdef ADAPTIVE_SECURE 
		, std::vector<block> tableMasks
#endif
		)
	{
#ifndef NDEBUG
		if (labels.size() != cd.WireCount())
			throw std::runtime_error("");
#endif

#ifdef STRONGEVAL
		BitVector values(cd.WireCount());
		for (u64 i = 0; i < labels.size(); ++i)
		{
			if (labels[i] == mInternalWires[i].mZeroLabel)
				values[i] = 0;
			else if (labels[i] == (mInternalWires[i].mZeroLabel ^ mGlobalOffset))
				values[i] = 1;
			else throw std::runtime_error("");
		}
#endif
		auto garbledGateIter = mGates.begin(); 
#ifdef ADAPTIVE_SECURE 
		auto maskIter = tableMasks.begin();
#endif
		block temp[2], hashs[2], tweaks[2]{ ZeroBlock, _mm_set_epi64x(0, cd.Gates().size()) };

		for (const auto& gate : cd.Gates()) {
			auto& a = labels[gate.mInput[0]];
			auto& b = labels[gate.mInput[1]];
			auto& c = labels[gate.mWireIdx];
			auto& gt = gate.Type();

			if (gt == GateType::Xor || gt == GateType::Nxor) {
				c = a ^ b;
			}
			else {
#ifdef ADAPTIVE_SECURE 
				auto& maskedGT = garbledGateIter++->mGarbledTable;
				block* garbledTable = &(*maskIter);
				maskIter += 2;

				garbledTable[0] = garbledTable[0] ^ maskedGT[0];
				garbledTable[1] = garbledTable[1] ^ maskedGT[1];
#else 
				auto& garbledTable = garbledGateIter++->mGarbledTable;
#endif

				// compute the hashs
				hashs[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
				hashs[1] = _mm_slli_epi64(b, 1) ^ tweaks[1];
				mAesFixedKey.ecbEncTwoBlocks(hashs, temp);
				hashs[0] = temp[0] ^ hashs[0]; // a
				hashs[1] = temp[1] ^ hashs[1]; // b

				// increment the tweaks
				tweaks[0] = tweaks[0] + OneBlock;
				tweaks[1] = tweaks[1] + OneBlock;

				// compute the output wire label
				c = hashs[0] ^ hashs[1];
				if (PermuteBit(a)) c = c ^ garbledTable[0];
				if (PermuteBit(b)) c = c ^ garbledTable[1] ^ a;
			}

#ifdef STRONGEVAL
			u8 aVal = values[gate.mInput[0]] ? 1 : 0;
			u8 bVal = values[gate.mInput[1]] ? 2 : 0;
			values[gate.mWireIdx] = gate.mLogicTable[aVal | bVal];

			if (values[gate.mWireIdx])
			{
				if (c != mInternalWires[gate.mWireIdx]^(mGlobalOffset))
					throw std::runtime_error("");
			}
			else if (c != mInternalWires[gate.mWireIdx].mZeroLabel)
				throw std::runtime_error("");
#endif
		}
	}

	bool HalfGtGarbledCircuit::Validate(const Circuit& cd, const block& seed
#ifdef ADAPTIVE_SECURE 
		, std::vector<block> tableMasks
#endif
		)
	{

		
		AES aesSeedKey(seed);

		mInputWires.clear();
		mInputWires.reserve(cd.WireCount());


		//create the delta for the free Xor. Encrypt zero twice. We get a good enough random delta by encrypting twice
		aesSeedKey.ecbEncBlock(ZeroBlock, mGlobalOffset);
		aesSeedKey.ecbEncBlock(mGlobalOffset, mGlobalOffset);
		*(u8*)&(mGlobalOffset) |= 1; // make sure the bottom bit is a 1 for point-n-permute

		// Compute the input labels as AES permutations on mIndexArray
		mInputWires.resize(cd.InputWireCount());
		aesSeedKey.ecbEncCounterMode(0, mInputWires.size(), (block*)mInputWires.data());

		block temp[4], hash[4], tweaks[2]{ ZeroBlock, _mm_set_epi64x(0, cd.Gates().size()) };
		u8 aPermuteBit, bPermuteBit;
		GarbledGate<2> garbledGate;

		// generate the garbled table
		auto& garbledTable = garbledGate.mGarbledTable;

		std::vector<GarbledGate<2>>::const_iterator gateIter = mGates.begin();
#ifdef ADAPTIVE_SECURE 
		auto maskIter = tableMasks.begin();
#endif
		std::array<block, 2> zeroAndGlobalOffset{ { ZeroBlock, mGlobalOffset } };


		for (const auto& gate : cd.Gates())
		{
			mInputWires.emplace_back();
			auto& c = mInputWires.back();
			auto& a = mInputWires[gate.mInput[0]];
			auto& b = mInputWires[gate.mInput[1]];
			auto gt = gate.Type();

#ifndef NDEBUG
			if (gt == GateType::Zero || gt == GateType::One || gt == GateType::na || gt == GateType::nb || gt == GateType::a || gt == GateType::b)
				throw std::runtime_error("Constant/unary gates not supported");
#endif
			if (gt == GateType::Xor || gt == GateType::Nxor) {
				c = a ^ b ^ zeroAndGlobalOffset[(u8)gt & 1];
			}
			else {

				// compute the gate modifier variables
				auto& aAlpha = gate.AAlpha();
				auto& bAlpha = gate.BAlpha();
				auto& cAlpha = gate.CAlpha();

				//signal bits of wire 0 of input0 and wire 0 of input1
				aPermuteBit = PermuteBit(a);
				bPermuteBit = PermuteBit(b);

				// compute the hashs of the wires ROUGHLY (<< op loses bit shift 64<-63)
				// as    H(x) = AES_f( x * 2 ^ tweak) ^ (x * 2 ^ tweak)    
				hash[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
				hash[1] = _mm_slli_epi64((a ^ mGlobalOffset), 1) ^ tweaks[0];
				hash[2] = _mm_slli_epi64(b, 1) ^ tweaks[1];
				hash[3] = _mm_slli_epi64((b ^ mGlobalOffset), 1) ^ tweaks[1];
				mAesFixedKey.ecbEncFourBlocks(hash, temp);
				hash[0] = hash[0] ^ temp[0]; // H( a0 )
				hash[1] = hash[1] ^ temp[1]; // H( a1 )
				hash[2] = hash[2] ^ temp[2]; // H( b0 )
				hash[3] = hash[3] ^ temp[3]; // H( b1 )

				// increment the tweaks
				tweaks[0] = tweaks[0] + OneBlock;
				tweaks[1] = tweaks[1] + OneBlock;

				// compute the table entries
				garbledTable[0] = hash[0] ^ hash[1] ^ zeroAndGlobalOffset[bAlpha ^ bPermuteBit];
				garbledTable[1] = hash[2] ^ hash[3] ^ a ^ zeroAndGlobalOffset[aAlpha];

				c = hash[aPermuteBit] ^
					hash[2 ^ bPermuteBit] ^
					zeroAndGlobalOffset[((aPermuteBit ^ aAlpha) && (bPermuteBit ^ bAlpha)) ^ cAlpha];


#ifdef ADAPTIVE_SECURE 
				garbledTable[0] = garbledTable[0] ^ *maskIter++;
				garbledTable[1] = garbledTable[1] ^ *maskIter++;
#endif

				if (neq(garbledTable[0], gateIter->mGarbledTable[0]))
					throw std::runtime_error("GC check failed");
				if (neq(garbledTable[1], gateIter->mGarbledTable[1]))
					throw std::runtime_error("GC check failed");

				++gateIter;
			}
		}

		mOutputWires.clear();
		mOutputWires.reserve(cd.Outputs().size());
		for (u64 i = 0; i < cd.Outputs().size(); ++i)
		{
			mOutputWires.push_back(mInputWires[cd.Outputs()[i]]);
			if (mTranslationTable[i] != PermuteBit(mInputWires[cd.Outputs()[i]]))
				throw std::runtime_error("GC check failed");
		}

		mInputWires.resize(cd.InputWireCount());
		mInputWires.shrink_to_fit();

		return true;
	}
	//#define STRONG_TRANSLATE
	void HalfGtGarbledCircuit::translate(const Circuit& cd, std::vector<block>&  labels, BitVector& output)
	{
		output.reset(mTranslationTable.size());

		for (u64 i = 0; i < cd.OutputCount(); ++i)
		{
#ifdef STRONG_TRANSLATE
			if (labels[wire] == mInputWires[wire].mZeroLabel)
			{
				output.push_back(false);
			}
			else  if (labels[wire] == mInputWires[wire]^(mGlobalOffset))
			{
				output.push_back(true);
			}
			else
			{
				throw std::runtime_error("Wire didnt match");
			}
#else
			output[i] = (mTranslationTable[i] ^ PermuteBit(labels[cd.Outputs()[i]]));
#endif
		}
	}

	void HalfGtGarbledCircuit::translate(const Circuit& cd, BitVector& output)
	{
		output.reset(mTranslationTable.size());

		for (u64 i = 0; i < cd.OutputCount(); ++i)
		{
#ifdef STRONG_TRANSLATE
			if (labels[wire] == mInputWires[wire].mZeroLabel)
			{
				output.push_back(false);
			}
			else  if (labels[wire] == mInputWires[wire] ^ (mGlobalOffset))
			{
				output.push_back(true);
			}
			else
			{
				throw std::runtime_error("Wire didnt match");
			}
#else
			output[i] = (mTranslationTable[i] ^ PermuteBit(mOutputWires[i]));
#endif
		}
	}



	void HalfGtGarbledCircuit::SendToEvaluator(Channel & channel)
	{
		auto buff = std::unique_ptr<ByteStream>(new ByteStream());

		for (u64 i = 0; i < mGates.size(); ++i)
		{
			buff->append((u8*)&(mGates[i].mGarbledTable[0]), sizeof(block));
			buff->append((u8*)&(mGates[i].mGarbledTable[1]), sizeof(block));
		}

		channel.asyncSend(std::move(buff));
		
		//mTranslationTable.pack(*buff);
		channel.send(mTranslationTable);
	}

	void HalfGtGarbledCircuit::ReceiveFromGarbler(const Circuit& cd, Channel & channel)
	{
		mGates.resize(cd.NonXorGateCount());
		channel.recv(mGates.data(), mGates.size() * sizeof(GarbledGate<2>));

		mTranslationTable.reset(cd.OutputCount());
		channel.recv(mTranslationTable.data(), mTranslationTable.sizeBytes());
	}

}
