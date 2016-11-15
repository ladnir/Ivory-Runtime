#pragma once
//#include "Runtime/CrtModulal.h"

#include "Circuit/Circuit.h"

#include "Runtime/BetaCircuit.h"
#include "Runtime/CrtRuntime.h"

namespace osuCrypto
{

	class CrtInt32 //: public CrtModulal
	{
	public:
		typedef i32 ValueType;
		static u64 sBitCount;
		CrtInt32(CrtRuntime& runtime);
		~CrtInt32();

        CrtRuntime& mRuntime;
		CrtInt32 operator+(const CrtInt32&);
		//CrtInt32 operator-(const CrtInt32&);
		CrtInt32 operator*(const CrtInt32&);
		//CrtInt32 operator/(const CrtInt32&);
		
		void operator+=(const CrtInt32&);
		//CrtInt32 operator-=(const CrtInt32&);
		//CrtInt32 operator*=(const CrtInt32&);
		//CrtInt32 operator/=(const CrtInt32&);

		static void staticInit();
		static BitVector valueToBV(const ValueType& val);
		static ValueType valueFromBV(const BitVector& val);
		
		std::vector<block> mLabels;

	private:

		static void staticInitAddBetaCir();
		static void staticBuildAddBetaCir(BetaCircuit& cd,BetaBundle& add0, BetaBundle& add1, BetaBundle& sum, BetaBundle& temp);
		static void staticInitMultBetaCir();

		static BetaCircuit mAddBetaCir;
		static BetaCircuit mMultBetaCir;
	};

}
