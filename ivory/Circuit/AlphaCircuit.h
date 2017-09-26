#pragma once
#include "ivory/Circuit/AlphaCircuit.h"
#include "cryptoTools/Common/Defines.h"
#include <unordered_map>

namespace osuCrypto
{

	enum class AlphaGateType
	{
		Add,
		Subtract,
		Multiply
	};

	typedef u32 AlphaWire;


	struct AlphaGate
	{
		std::array<AlphaWire, 2> mInput;
		AlphaWire mOutput;
		AlphaGateType mType;
	};

	struct AlphaBundle
	{
		AlphaBundle(u32 bitCount)
			: mBitCount(bitCount)
			, mWire(-1)
		{}

		AlphaWire mWire;
		u32 mBitCount;
	};

	class AlphaCircuit
	{
	public:
		AlphaCircuit();
		~AlphaCircuit();


		void addInputBundle(AlphaBundle& a);
		void addTempBundle(AlphaBundle& a);
		void addOutputBundle(AlphaBundle& a);

		void addGate(AlphaWire a, AlphaWire b, AlphaGateType gt, AlphaWire c);

	};



	class AlphaStream
	{
		//struct AlphaLevel
		//{
		//};

		std::vector<AlphaGate> mLevel;

		//std::vector<



	};
}
