#pragma once

#include <iostream>
#include <vector>
#include "Gate.h"
#include "Common/Defines.h"
#include "Common/BitVector.h"

namespace osuCrypto {

	class DagCircuit;

	class Circuit
	{
	public:
		friend class DagCircuit;

		Circuit();
		Circuit(std::array<u64, 2> inputs);
		~Circuit();

		void readBris(std::istream& in, bool reduce = true);

		void evaluate(BitVector& input);
		void translate(BitVector& labels, BitVector& output);

		//void evaluate(std::vector<bool>& input);
		//void translate(std::vector<bool>& labels, std::vector<bool>& output);

		void init();
		 
		u64 AddGate(u64 input0, u64 input1, GateType gt);


		inline void AddOutputWire(u64 i)
		{
			if (i >= mWireCount)
				throw std::runtime_error("");
			mOutputs.push_back(i);
			++mOutputCount;
		}

		inline const u64 InputWireCount() const
		{
			return mInputs[0] + mInputs[1];
		}
		inline const u64& WireCount()const
		{
			return mWireCount;
		}
		inline const u64& NonXorGateCount()const
		{
			return mNonXorGateCount;
		}
		inline const u64& OutputCount()const
		{
			return mOutputCount;
		}

		inline const std::array<u64, 2>& Inputs() const
		{
			return mInputs;
		}
		inline const  std::vector<Gate>& Gates() const
		{
			return mGates;
		}
		inline const std::vector<u64>& Outputs() const
		{
			return mOutputs;
		}
		
		void xorShareInputs();

	private:
		u64 mWireCount, mNonXorGateCount, mOutputCount;
		std::array<u64, 2> mInputs;
		std::vector<Gate> mGates;
		std::vector<u64> mOutputs;
	};

}