#include "SequentialCircuit.h"
#include "Common/Log.h"

namespace osuCrypto
{

	SequentialCircuit::SequentialCircuit()
		:
		mInputCount(0),
		mOutputCount(0)
	{
	}


	SequentialCircuit::~SequentialCircuit()
	{
	}

	u64 SequentialCircuit::outputCount()
	{
		return mOutputCount;
	}

	u64 SequentialCircuit::inputCount()
	{
		return  mInputCount;
	}

	//void SequentialCircuit::setInputSize(u64 size)
	//{
	//	mInputs.resize(size);
	//	mInputCount = size;
	//}

	void SequentialCircuit::addOutputWire(u64 outputIdx, u64 wireIdx)
	{
		mOutputCount++;
		mGates.emplace_back( wireIdx, outputIdx, -1, GateType::One);

	}

	void SequentialCircuit::addInputWire(u64 inputIdx, u64 wireIdx)
	{
		//mInputs[inputIdx] = std::make_pair(inputIdx, wireIdx);
		mInputCount++;
		mGates.emplace_back( inputIdx, wireIdx, -1, GateType::Zero);

	}

	void SequentialCircuit::addGate(u64 input0, u64 input1, u64 output, GateType gt)
	{
		if (gt == GateType::a ||
			gt == GateType::b ||
			gt == GateType::nb ||
			gt == GateType::One ||
			gt == GateType::Zero
			)
			throw std::runtime_error("Invalid gate Type");

		mGates.emplace_back(input0, input1, output, gt);
	}

	ArrayView<Gate> SequentialCircuit::gates()
	{
		return mGates;
	}


	u8 evaluate(u8 i, GateType mType)
	{
		return ((u8)mType & (1 << i)) ? 1 : 0;
	}


	void SequentialCircuit::evaluate(BitVector & input, BitVector & output)
	{

		BitVector label(mWireCount);
		output.reset(outputCount());
		u64 inputIdx = 0;

		for (auto& gate : mGates)
		{
			if (gate.Type() == GateType::Zero)
			{

				auto srcIdx = gate.mInput[0];
				auto destIdx = gate.mInput[1];


				label[destIdx] = input[srcIdx];
			}
			else if (gate.Type() == GateType::na)
			{

				auto a = label[gate.mInput[0]];
				auto outputIdx = gate.mWireIdx;
				label[outputIdx] = !a;

				Log::out << (u32)a << " " << gateToString(gate.Type()) << "  =  " << (u32)label[outputIdx] << Log::endl;


			}
			else if (gate.Type() == GateType::One)
			{
				auto srcIdx = gate.mInput[0];
				auto destIdx = gate.mInput[1];

				output[destIdx] = label[srcIdx];// = input[srcIdx];
				Log::out << "out[" << destIdx << "] = " << (u32)output[destIdx] << Log::endl;
			}
			else
			{
				auto outputIdx = gate.mWireIdx;
				u8 a = label[gate.mInput[0]];
				u8 b = label[gate.mInput[1]] << 1;
				
				label[outputIdx] = gate.eval(a | b);

				Log::out << (u32)a << " " << gateToString(gate.Type()) << "  " << (u32)b << "  =  " << (u32)label[outputIdx] << Log::endl;


			}
		}
	}

}
