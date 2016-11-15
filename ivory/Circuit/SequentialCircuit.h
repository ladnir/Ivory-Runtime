#pragma once
#include "Common/Defines.h"
#include <array>
#include <vector>
#include "Circuit/Gate.h"
#include "Common/ArrayView.h"
#include "Common/BitVector.h"


namespace osuCrypto
{
	class VerilogParser;
	class SequentialCircuit
	{

		friend class VerilogParser;
	public:
		SequentialCircuit();
		~SequentialCircuit();

		u64 outputCount();
		u64 inputCount();

		//void setInputSize(u64 size);

		void addOutputWire(u64 outputIdx, u64 wireIdx);
		void addInputWire(u64 inputIdx, u64 wireIdx);
		void addGate(u64 input0, u64 input1, u64 output, GateType gt);

		ArrayView<Gate> gates();



		void evaluate(BitVector& input, BitVector& output);


	private:
		u64 mWireCount, mNonXorGateCount, mOutputCount, mInputCount;



		//std::vector<std::pair<u64,u64>> mInputs;
		std::vector<Gate> mGates;
		std::vector<u64> mOutputs;
	};

}
