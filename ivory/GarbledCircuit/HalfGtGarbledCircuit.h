#pragma once
#include "GarbledCircuit.h"
#include <unordered_map>
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Crypto/AES.h"
#include "cryptoTools/Common/BitVector.h"
#include "ivory/Circuit/Circuit.h"
//#include "Runtime/AlphaCircuit.h"


//#define ADAPTIVE_SECURE
//#define STRONGEVAL

namespace osuCrypto {
	class HalfGtGarbledCircuit
	{
	public:
		HalfGtGarbledCircuit(HalfGtGarbledCircuit&&);

		HalfGtGarbledCircuit(const HalfGtGarbledCircuit&) = delete;
		HalfGtGarbledCircuit() {};

		~HalfGtGarbledCircuit();
		block mGlobalOffset;
		std::vector<block> mInputWires, mOutputWires;
		std::vector<GarbledGate<2>> mGates;
		BitVector mTranslationTable;
		block mAddativeSecureMaskSeed;

#ifdef STRONGEVAL
		std::vector<GarbledWire> mInternalWires;
#endif

		static const AES mAesFixedKey;

		void Clear()
		{
			mInputWires.clear(); mInputWires.shrink_to_fit();
			mOutputWires.clear(); mOutputWires.shrink_to_fit();
			mGates.clear(); mGates.shrink_to_fit();
			mTranslationTable.reset();
		}

		void garbleStream(
			Circuit& cd, 
			const block& seed, 
			Channel& chl, 
			std::vector<block>& wiresBuff,
			std::function<void(std::vector<std::array<block, 2>>)> sendInputsCallback);

		void evaluateStream(
			Circuit& cd,
			Channel& chl,
			std::vector<block>& wiresBuff,
			std::function<ArrayView<block>(u64)> receiveInputCallback);


		void translate(const Circuit& cd, BitVector& output);


#ifdef ADAPTIVE_SECURE 
		void GarbleSend(const Circuit& cd, const block& seed, Channel& chl, std::vector<block>& wiresBuff, std::vector<block> tableMasks);
		void Garble(const Circuit& cd, const block& seed, std::vector<block> tableMasks);
		void evaluate(const Circuit& cd, std::vector<block>& labels, std::vector<block> tableMasks);
		bool Validate(const Circuit& cd, const block&, std::vector<block> tableMasks);
#else
		void GarbleSend(const Circuit& cd, const block& seed, Channel& chl, std::vector<block>& wiresBuff);
		void Garble(const Circuit& cd, const block& seed);
		void evaluate(const Circuit& cd, std::vector<block>& labels);
		bool Validate(const Circuit& cd, const block&);
#endif
		void translate(const Circuit& cd, std::vector<block>&  labels, BitVector& output);

		void SendToEvaluator(Channel& channel);
		void ReceiveFromGarbler(const Circuit& cd, Channel& channel);

	};

}
