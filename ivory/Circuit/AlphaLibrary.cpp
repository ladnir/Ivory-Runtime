#include "AlphaLibrary.h"


namespace osuCrypto
{

	AlphaLibrary::AlphaLibrary()
	{
	}


	AlphaLibrary::~AlphaLibrary()
	{
		for (auto cir : mCirMap)
		{
			delete cir.second;
		}
	}
	AlphaCircuit * AlphaLibrary::int_int_add(u64 aSize, u64 bSize, u64 cSize)
	{
		auto key = "add" + ToString(aSize) + "x" + ToString(bSize) + "x" + ToString(cSize);
		auto iter = mCirMap.find(key);

		if (iter != mCirMap.end())
		{
			return iter->second;
		}
		else
		{
			auto* cd = new AlphaCircuit;

			AlphaBundle a(aSize);
			AlphaBundle b(bSize);
			AlphaBundle c(cSize);

			cd->addInputBundle(a);
			cd->addInputBundle(b);
			cd->addOutputBundle(c);

			int_int_add_build(*cd, a, b, c);

			return cd;
		}
	}
	AlphaCircuit * AlphaLibrary::int_int_subtract(u64 aSize, u64 bSize, u64 cSize)
	{
		auto key = "subtract" + ToString(aSize) + "x" + ToString(bSize) + "x" + ToString(cSize);
		auto iter = mCirMap.find(key);

		if (iter != mCirMap.end())
		{
			return iter->second;
		}
		else
		{
			auto* cd = new AlphaCircuit;

			AlphaBundle a(aSize);
			AlphaBundle b(bSize);
			AlphaBundle c(cSize);

			cd->addInputBundle(a);
			cd->addInputBundle(b);
			cd->addOutputBundle(c);

			int_int_subtract_build(*cd, a, b, c);

			return cd;
		}
	}
	AlphaCircuit * AlphaLibrary::int_int_mult(u64 aSize, u64 bSize, u64 cSize)
	{
		auto key = "mult" + ToString(aSize) + "x" + ToString(bSize) + "x" + ToString(cSize);
		auto iter = mCirMap.find(key);

		if (iter != mCirMap.end())
		{
			return iter->second;
		}
		else
		{
			auto* cd = new AlphaCircuit;

			AlphaBundle a(aSize);
			AlphaBundle b(bSize);
			AlphaBundle c(cSize);

			cd->addInputBundle(a);
			cd->addInputBundle(b);
			cd->addOutputBundle(c);

			int_int_mult_build(*cd, a, b, c);

			return cd;
		}
	}

	void AlphaLibrary::int_int_add_build(AlphaCircuit & cir, AlphaBundle & a, AlphaBundle & b, AlphaBundle & c)
	{
		Expects(a.mBitCount == b.mBitCount && a.mBitCount == c.mBitCount);
		cir.addGate(a.mWire, b.mWire, AlphaGateType::Add, c.mWire);
	}
	void AlphaLibrary::int_int_subtract_build(AlphaCircuit & cir, AlphaBundle & a, AlphaBundle & b, AlphaBundle & c)
	{
		Expects(a.mBitCount == b.mBitCount && a.mBitCount == c.mBitCount);
		cir.addGate(a.mWire, b.mWire, AlphaGateType::Subtract, c.mWire);
	}
	void AlphaLibrary::int_int_mult_build(AlphaCircuit & cir, AlphaBundle & a, AlphaBundle & b, AlphaBundle & c)
	{
		Expects(a.mBitCount == b.mBitCount && a.mBitCount == c.mBitCount);
		cir.addGate(a.mWire, b.mWire, AlphaGateType::Multiply, c.mWire);
	}

}
