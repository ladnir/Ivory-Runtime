#include "BetaCircuit.h"
#include <vector>
#include <unordered_map>

namespace osuCrypto
{

	BetaCircuit::BetaCircuit()
	{
	}



	BetaCircuit::~BetaCircuit()
	{
	}

	void BetaCircuit::addTempWireBundle(BetaBundle & in)
	{
		for (auto i = 0; i < in.mWires.size(); ++i)
		{
			in.mWires[i] = mWireCount++;
		}
	}

	void BetaCircuit::addInputBundle(BetaBundle & in)
	{
		for (auto i = 0; i < in.mWires.size(); ++i)
		{
			in.mWires[i] = mWireCount++;
		}

	}
	

	void BetaCircuit::addOutputBundle(BetaBundle & in)
	{
		for (auto i = 0; i < in.mWires.size(); ++i)
		{
			in.mWires[i] = mWireCount;
		}
	}


	void BetaCircuit::addGate(
		BetaWire in0, 
		BetaWire in1, 
		GateType gt, 
		BetaWire out)
	{
		if (gt == GateType::a ||
			gt == GateType::b ||
			gt == GateType::na ||
			gt == GateType::nb ||
			gt == GateType::One ||
			gt == GateType::Zero)
			throw std::runtime_error("");

		if (gt != GateType::Xor && gt != GateType::Nxor) ++mNonXorGateCount;
		mGates.emplace_back(in0, in1, gt, out);

	}


	void BetaCircuit::levelize()
	{
		mLevelGates.clear();
		mLevelGates.emplace_back();


		std::unordered_map<u64, u64> levelMap;


		for (u64 i = 0; i < mGates.size(); ++i)
		{
			u64 level = 0;


			static_assert(sizeof(BetaWire) == sizeof(u32), "");

			auto idx = mGates[i].mInput[0];
			auto iter = levelMap.find(idx);

			if (iter != levelMap.end())
			{
				level = iter->second + 1;
			}

			idx = mGates[i].mInput[1];
			iter = levelMap.find(idx);

			if (iter != levelMap.end())
			{
				level = std::max(iter->second + 1, level);
			}

			idx = mGates[i].mOutput;
			levelMap[idx] = level;


			if (level == mLevelGates.size())
				mLevelGates.emplace_back();

			if (mGates[i].mType == GateType::Xor || mGates[i].mType == GateType::Nxor)
			{
				mLevelGates[level].mXorGates.push_back(mGates[i]);
			}
			else
			{
				mLevelGates[level].mAndGates.push_back(mGates[i]);
			}
		}
	}
}