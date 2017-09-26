#pragma once
#include "AlphaCircuit.h"
#include <unordered_map>


namespace osuCrypto
{

	class AlphaLibrary
	{
	public:
		AlphaLibrary();
		~AlphaLibrary();



		std::unordered_map<std::string, AlphaCircuit*> mCirMap;

		AlphaCircuit* int_int_add(u64 a, u64 b, u64 c);
		AlphaCircuit* int_int_subtract(u64 a, u64 b, u64 c);
		AlphaCircuit* int_int_mult(u64 a, u64 b, u64 c);

		void int_int_add_build(AlphaCircuit& cir, AlphaBundle& a, AlphaBundle& b, AlphaBundle& c);
		void int_int_subtract_build(AlphaCircuit& cir, AlphaBundle& a, AlphaBundle& b, AlphaBundle& c);
		void int_int_mult_build(AlphaCircuit& cir, AlphaBundle& a, AlphaBundle& b, AlphaBundle& c);
	};

}
