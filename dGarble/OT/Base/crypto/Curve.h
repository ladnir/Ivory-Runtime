#pragma once


#include "miracl_gmt/include/miracl.h"
#include "Common/Defines.h"
#include "Crypto/PRNG.h"
#include <memory>

namespace osuCrypto
{

	struct eccParams
	{
		//eccParams(u32 ba, u32 bb, char* x, char* y, u32 m, u32 a, u32 b, u32 c)
		//{
		//	convert()
		//}
		u32 bitCount;
		u32 BA;
		u32 BB;
		char* X;
		char* Y;
		u32 m;
		u32 a;
		u32 b;
		u32 c;
	};


	// NIST Koblitz Curves Parameters 
	// https://en.wikisource.org/wiki/NIST_Koblitz_Curves_Parameters
	// FIPS 186-2 

	const eccParams k163 =
	{
		163,
		1,  // BA
		1,  // BB 
		"2fe13c0537bbc11acaa07d793de4e6d5e5c94eee8", // X
		"289070fb05d38ff58321f2e800536d538ccdaa3d9", // Y
		163, // m
		7,   // a
		6,   // b
		3    // c
	};

	const eccParams k233 =
	{
		233,
		0,  // BA
		1,  // BB 
		"17232ba853a7e731af129f22ff4149563a419c26bf50a4c9d6eefad6126", // X
		"1db537dece819b7f70f555a67c427a8cd9bf18aeb9b56e0c11056fae6a3", // Y
		233, // m
		74,  // a
		0,   // b
		0    // c
	};

	const eccParams k283 =
	{
		283,
		0,  // BA
		1,  // BB 
		"503213f78ca44883f1a3b8162f188e553cd265f23c1567a16876913b0c2ac2458492836", // X
		"1ccda380f1c9e318d90f95d07e5426fe87e45c0e8184698e45962364e34116177dd2259", // Y
		283, // m
		12,  // a
		7,   // b
		5    // c
	};

	class EllipticCurve;
	class EllipticCurvePoint;
	class EllipticCurveBrick;


	class EllipticCurveNumber
	{
	public:

		EllipticCurveNumber(const EllipticCurveNumber& num);
		EllipticCurveNumber(EllipticCurveNumber&& num);
		EllipticCurveNumber(EllipticCurve& curve);
		EllipticCurveNumber(EllipticCurve& curve, const EllipticCurveNumber& copy);
		EllipticCurveNumber(EllipticCurve& curve, PRNG& prng);
		EllipticCurveNumber(EllipticCurve& curve, const i32& val);

		~EllipticCurveNumber();

		EllipticCurveNumber& operator=(const i32& copy);
		EllipticCurveNumber& operator=(const EllipticCurveNumber& copy);
		EllipticCurveNumber& operator+=(const EllipticCurveNumber& addIn);
		EllipticCurveNumber& operator-=(const EllipticCurveNumber& subtractIn);
		EllipticCurveNumber& operator*=(const EllipticCurveNumber& multIn);

		EllipticCurveNumber operator+(const EllipticCurveNumber& addIn);
		EllipticCurveNumber operator-(const EllipticCurveNumber& subtractIn);
		EllipticCurveNumber operator*(const EllipticCurveNumber& multIn);


		u64 sizeBytes() const;
		void toBytes(u8* dest) const;
		void fromBytes(u8* src);
		void fromHex(char* src);

		void randomize(PRNG& prng);


	private:

		void init();

		big mVal;
		EllipticCurve& mCurve;

		friend EllipticCurveBrick;
		friend EllipticCurvePoint;
		friend std::ostream& operator<<(std::ostream& out, const EllipticCurveNumber& val);
	};
	std::ostream& operator<<(std::ostream& out, const EllipticCurveNumber& val);


	class EllipticCurvePoint
	{
	public:


		EllipticCurvePoint(EllipticCurve& curve);
		EllipticCurvePoint(EllipticCurve& curve, const EllipticCurvePoint& copy);
		EllipticCurvePoint(const EllipticCurvePoint& copy);
		EllipticCurvePoint(EllipticCurvePoint&& move);

		~EllipticCurvePoint();
		
		EllipticCurvePoint& operator=(const EllipticCurvePoint& copy);
		EllipticCurvePoint& operator+=(const EllipticCurvePoint& addIn);
		EllipticCurvePoint& operator-=(const EllipticCurvePoint& subtractIn);
		EllipticCurvePoint& operator*=(const EllipticCurveNumber& multIn);
		

		EllipticCurvePoint operator+(const EllipticCurvePoint& addIn) const;
		EllipticCurvePoint operator-(const EllipticCurvePoint& subtractIn) const;
		EllipticCurvePoint operator*(const EllipticCurveNumber& multIn) const;

		u64 sizeBytes() const;
		void toBytes(u8* dest);
		void fromBytes(u8* src);
		void fromHex(char* x, char* y);


		void randomize(PRNG& prng);

	private:

		void init();
		char* mMem;
		epoint* mVal;
		EllipticCurve& mCurve;

		friend EllipticCurveBrick;
		friend EllipticCurveNumber;
		friend std::ostream& operator<<(std::ostream& out, const EllipticCurvePoint& val);
	};

	std::ostream& operator<<(std::ostream& out, const EllipticCurvePoint& val);

	class EllipticCurveBrick
	{
	public:
		EllipticCurveBrick(const EllipticCurvePoint& copy);
		EllipticCurveBrick(EllipticCurveBrick&& copy);

		EllipticCurvePoint operator*(const EllipticCurveNumber& multIn) const;

		void multiply(const EllipticCurveNumber& multIn, EllipticCurvePoint& result) const;

	private:

		ebrick2 mBrick;
		EllipticCurve& mCurve;

	};

	class EllipticCurve
	{
	public:
		typedef EllipticCurvePoint Point;


			
		EllipticCurve(const eccParams& params, const block& seed);
		EllipticCurve() = delete;
		~EllipticCurve();


		void setParameters(const eccParams& params);
		void setParameters(const eccParams& params, const block& seed);
		void setPrng(const block& seed);

		const Point& getGenerator() const; 

	private:
		// A **non-thread safe** member variable which acts as a memory pool and 
		// determines the byte/bit size of the variables within this curve.
		miracl* mMiracl;

		PRNG mPrng;
		csprng mMrPrng;
		eccParams mParams;
		big BB, BA;
		std::unique_ptr<Point> mG;



		friend Point;
		friend EllipticCurveNumber;
		friend EllipticCurveBrick;
		friend std::ostream& operator<<(std::ostream & out, const EllipticCurvePoint & val);
		friend std::ostream& operator<<(std::ostream& out, const EllipticCurveNumber& val);
	};

}
