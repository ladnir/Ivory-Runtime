#pragma once
#include "GarbledCircuit.h"
#include <unordered_map>
#include "Common/Defines.h"
#include "Crypto/AES.h"
#include "Common/BitVector.h"

#define ADAPTIVE_SECURE
//#define STRONGEVAL

namespace osuCrypto {
	class HalfGtGarbledCircuit //:
	   //public GarbledCircuit
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

#ifdef ADAPTIVE_SECURE 
		void GarbleSend(const Circuit& cd, const block& seed, Channel& chl, std::vector<block>& wiresBuff, const std::vector<block>& indexArray, std::vector<block> tableMasks);
		void Garble(const Circuit& cd, const block& seed, const std::vector<block>& indexArray, std::vector<block> tableMasks);
		void evaluate(const Circuit& cd, std::vector<block>& labels, std::vector<block> tableMasks);
		bool Validate(const Circuit& cd, const block&, const std::vector<block>& indexArray, std::vector<block> tableMasks);
#else
		void GarbleSend(const Circuit& cd, const block& seed, Channel& chl, std::vector<block>& wiresBuff, const std::vector<block>& indexArray);
		void Garble(const Circuit& cd, const block& seed, const std::vector<block>& indexArray);
		void evaluate(const Circuit& cd, std::vector<block>& labels);
		bool Validate(const Circuit& cd, const block&, const std::vector<block>& indexArray);
#endif
		void translate(const Circuit& cd, std::vector<block>&  labels, BitVector& output);// override; 

		void SendToEvaluator(Channel& channel);
		void ReceiveFromGarbler(const Circuit& cd, Channel& channel);

	};

}
