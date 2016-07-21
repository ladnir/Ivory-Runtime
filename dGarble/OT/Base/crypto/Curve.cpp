#include "Curve.h"


namespace osuCrypto
{
	EllipticCurve::EllipticCurve(const eccParams & params, const block& seed)
		:
		mMiracl(nullptr),
		mG(nullptr)
	{
		setParameters(params);
		setPrng(seed);
	}

	EllipticCurve::~EllipticCurve()
	{
		if (mMiracl)
		{
			mirexit(mMiracl);

			mirkill(BA);
			mirkill(BB);
		}
	}

	void EllipticCurve::setParameters(const eccParams & params, const block& seed)
	{
		setPrng(seed);
		setParameters(params);
	}

	void EllipticCurve::setPrng(const block & seed)
	{
		mPrng.SetSeed(seed);
		irand(mMiracl, (int)mPrng.get_u32());
	}

	void EllipticCurve::setParameters(const eccParams & params)
	{
		mParams = params;

		if (mMiracl) mirexit(mMiracl);

		mMiracl = mirsys(mParams.bitCount, 2);
		mMiracl->IOBASE = 16;
		
		mirkill(BA);
		mirkill(BB);

		BA = mirvar(mMiracl, 0);
		BB = mirvar(mMiracl, 0);

		convert(mMiracl, mParams.BA, BA);
		convert(mMiracl, mParams.BB, BB);

		ecurve2_init(
			mMiracl,
			mParams.m,
			mParams.a,
			mParams.b,
			mParams.c,
			BA,
			BB,
			false,
			MR_PROJECTIVE);

		mG.reset(new EllipticCurvePoint(*this));
		mG->fromHex(mParams.X, mParams.Y);
	}

	const EllipticCurve::Point & EllipticCurve::getGenerator() const
	{
		return *mG;
	}



	EllipticCurvePoint::EllipticCurvePoint(
		EllipticCurve & curve)
		:
		mMem(nullptr),
		mVal(nullptr),
		mCurve(curve)

	{
		init();
	}

	EllipticCurvePoint::EllipticCurvePoint(
		EllipticCurve & curve, 
		const EllipticCurvePoint & copy)
		:
		mMem(nullptr),
		mVal(nullptr),
		mCurve(curve)
	{
		init();

		*this = copy;
	}

	EllipticCurvePoint::EllipticCurvePoint(
		const EllipticCurvePoint & copy)
		:
		mMem(nullptr),
		mVal(nullptr),
		mCurve(copy.mCurve)
	{

		init();

		*this = copy;
	}

	EllipticCurvePoint::EllipticCurvePoint(EllipticCurvePoint && move)
		: 
		mMem(move.mMem),
		mVal(move.mVal),
		mCurve(move.mCurve)
	{
		move.mVal = nullptr;
		move.mMem = nullptr;
	}

	EllipticCurvePoint::~EllipticCurvePoint()
	{
		if(mMem)
			ecp_memkill(mCurve.mMiracl, mMem, 0);
	}

	EllipticCurvePoint & EllipticCurvePoint::operator=(
		 const EllipticCurvePoint & copy)
	{
		epoint2_copy((epoint*)copy.mVal, mVal);

		return *this;
	}

	EllipticCurvePoint & EllipticCurvePoint::operator+=(
		const EllipticCurvePoint & addIn)
	{
#ifndef NDEBUG
		if (&mCurve != &addIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		ecurve2_add(mCurve.mMiracl, (epoint*)addIn.mVal, mVal);

		return *this;
	}

	EllipticCurvePoint & EllipticCurvePoint::operator-=(
		const EllipticCurvePoint & subtractIn)
	{
#ifndef NDEBUG
		if (&mCurve != &subtractIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		ecurve2_sub(mCurve.mMiracl, (epoint*)subtractIn.mVal, mVal);

		return *this;
	}

	EllipticCurvePoint & EllipticCurvePoint::operator*=(
		const EllipticCurveNumber & multIn)
	{
#ifndef NDEBUG
		if (&mCurve != &multIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		ecurve2_mult(mCurve.mMiracl, multIn.mVal, mVal, mVal);

		return *this;
	}

	EllipticCurvePoint EllipticCurvePoint::operator+(
		const EllipticCurvePoint & addIn) const
	{
#ifndef NDEBUG
		if (&mCurve != &addIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		EllipticCurvePoint temp(*this);

		temp += addIn;

		return temp;
	}

	EllipticCurvePoint EllipticCurvePoint::operator-(
		const EllipticCurvePoint & subtractIn) const
	{
#ifndef NDEBUG
		if (&mCurve != &subtractIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif
		EllipticCurvePoint temp(*this);

		temp -= subtractIn;

		return temp;
	}

	EllipticCurvePoint EllipticCurvePoint::operator*(
		const EllipticCurveNumber & multIn) const
	{
#ifndef NDEBUG
		if (&mCurve != &multIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		EllipticCurvePoint temp(*this);

		temp *= multIn;

		return temp;
	}

	u64 EllipticCurvePoint::sizeBytes() const
	{
		return (mCurve.mParams.bitCount + 7) / 8 + 1;
	}

	void EllipticCurvePoint::toBytes(u8 * dest)
	{
		big varX = mirvar(mCurve.mMiracl, 0);

		// convert the point into compressed format where dest[0] holds
		// the y bit and varX holds the x data.
		dest[0] = epoint2_get(mCurve.mMiracl, mVal, varX, varX) & 1;

		// copy the bits of varX into the buffer
		big_to_bytes(mCurve.mMiracl, (int)sizeBytes() - 1, varX, (char*)dest + 1, true);

		mirkill(varX);
	}

	void EllipticCurvePoint::fromBytes(u8 * src)
	{
		big varX = mirvar(mCurve.mMiracl, 0);

		bytes_to_big(mCurve.mMiracl, (int)sizeBytes() - 1, (char*)src + 1, varX);

		epoint2_set(mCurve.mMiracl, varX, varX, src[0], mVal);

		mirkill(varX);
	}

	void EllipticCurvePoint::fromHex(char * x, char * y)
	{
		big X = mirvar(mCurve.mMiracl, 0),
			Y = mirvar(mCurve.mMiracl, 0);

		cinstr(mCurve.mMiracl, X, x);
		cinstr(mCurve.mMiracl, Y, y);

		epoint2_set(mCurve.mMiracl, X, Y, 0, mVal);

		mirkill(X);
		mirkill(Y);
	}

	void EllipticCurvePoint::randomize(PRNG& orng)
	{

		big var = mirvar(mCurve.mMiracl, 0);
		u8 bit = mCurve.mPrng.get_bit();
		do
		{
			TODO("replace bigdig with our PRNG");

			bigdig(mCurve.mMiracl, mCurve.mParams.bitCount, 2, var);
			epoint2_set(mCurve.mMiracl, var, var, bit, mVal);
		} while (point_at_infinity(mVal));

		mirkill(var);
	}

	void EllipticCurvePoint::init()
	{
		mMem = (char *)ecp_memalloc(mCurve.mMiracl, 1);
		mVal = (epoint *)epoint_init_mem(mCurve.mMiracl, mMem, 0);
	}

	EllipticCurveNumber::EllipticCurveNumber(const EllipticCurveNumber & num)
		:mVal(nullptr)
		,mCurve(num.mCurve)
	{
		init();

		*this = num;
	}

	EllipticCurveNumber::EllipticCurveNumber(EllipticCurveNumber && num)
		: mVal(num.mVal)
		, mCurve(num.mCurve)
	{
		num.mVal = nullptr;
	}

	EllipticCurveNumber::EllipticCurveNumber(
		EllipticCurve & curve)
		:
		mVal(nullptr),
		mCurve(curve)
	{
		init();
	}

	EllipticCurveNumber::EllipticCurveNumber(
		EllipticCurve & curve, 
		const EllipticCurveNumber& copy)
		:
		mVal(nullptr),
		mCurve(curve)
	{
		init();

		*this = copy;
	}

	EllipticCurveNumber::EllipticCurveNumber(EllipticCurve & curve, PRNG & prng)
		:
		mVal(nullptr),
		mCurve(curve)
	{
		init();
		randomize(prng);
	}

	EllipticCurveNumber::EllipticCurveNumber(
		EllipticCurve & curve, 
		const i32 & val)
		: 
		mVal(nullptr),
		mCurve(curve)
	{
		init();
		*this = val;
	}

	EllipticCurveNumber::~EllipticCurveNumber()
	{
		if (mVal)
			mirkill(mVal);
	}

	EllipticCurveNumber & EllipticCurveNumber::operator=(
		const i32 & copy)
	{
		convert(mCurve.mMiracl, copy, mVal);

		return *this;
	}

	EllipticCurveNumber & EllipticCurveNumber::operator=(
		const EllipticCurveNumber & val)
	{
		copy(val.mVal, mVal);
		return *this;
	}

	EllipticCurveNumber & EllipticCurveNumber::operator+=(
		const EllipticCurveNumber & addIn)
	{
#ifndef NDEBUG
		if (&mCurve != &addIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif
		add(mCurve.mMiracl, mVal, addIn.mVal, mVal);
		return *this;
	}

	EllipticCurveNumber & EllipticCurveNumber::operator-=(
		const EllipticCurveNumber & subtractIn)
	{
#ifndef NDEBUG
		if (&mCurve != &subtractIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		subtract(mCurve.mMiracl, mVal, subtractIn.mVal, mVal);
		return *this;
	}

	EllipticCurveNumber & EllipticCurveNumber::operator*=(
		const EllipticCurveNumber & multIn)
	{
#ifndef NDEBUG
		if (&mCurve != &multIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		multiply(mCurve.mMiracl, mVal, multIn.mVal, mVal);
		return *this;
	}

	EllipticCurveNumber EllipticCurveNumber::operator+(
		const EllipticCurveNumber & addIn)
	{
#ifndef NDEBUG
		if (&mCurve != &addIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		EllipticCurveNumber temp(mCurve);

		add(mCurve.mMiracl, mVal, addIn.mVal, temp.mVal);

		return mCurve;
	}

	EllipticCurveNumber EllipticCurveNumber::operator-(
		const EllipticCurveNumber & subtractIn)
	{
#ifndef NDEBUG
		if (&mCurve != &subtractIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		EllipticCurveNumber temp(mCurve);

		subtract(mCurve.mMiracl, mVal, subtractIn.mVal, temp.mVal);

		return mCurve;
	}

	EllipticCurveNumber EllipticCurveNumber::operator*(
		const EllipticCurveNumber & multIn)
	{
#ifndef NDEBUG
		if (&mCurve != &multIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		EllipticCurveNumber temp(mCurve);

		subtract(mCurve.mMiracl, mVal, multIn.mVal, temp.mVal);

		return mCurve;
	}

	u64 EllipticCurveNumber::sizeBytes() const
	{
		return (mCurve.mParams.bitCount + 7) / 8;
	}

	void EllipticCurveNumber::toBytes(u8 * dest) const
	{
		big_to_bytes(mCurve.mMiracl, (int)sizeBytes(), mVal, (char*)dest, true);
	}

	void EllipticCurveNumber::fromBytes(u8 * src)
	{
		bytes_to_big(mCurve.mMiracl, (int)sizeBytes(), (char*)src, mVal);
	}

	void EllipticCurveNumber::fromHex(char * src)
	{
		cinstr(mCurve.mMiracl, mVal, src);
	}

	void EllipticCurveNumber::randomize(PRNG & prng)
	{
		rand2(mCurve.mMiracl, mVal);
	}

	void EllipticCurveNumber::init()
	{
		mVal = mirvar(mCurve.mMiracl, 0);

	}

	EllipticCurveBrick::EllipticCurveBrick(const EllipticCurvePoint & copy)
		:mCurve(copy.mCurve)
	{

		//big x, y;

		
		//fe2ec2(point)->getxy(x, y);
		ebrick2_init(mCurve.mMiracl,&mBrick, copy.mVal->X, copy.mVal->Y, mCurve.BA, mCurve.BB,
			mCurve.mParams.m, mCurve.mParams.a, mCurve.mParams.b, mCurve.mParams.c, 8, mCurve.mParams.bitCount);

	}

	EllipticCurveBrick::EllipticCurveBrick(EllipticCurveBrick && copy)
		:
		mBrick(copy.mBrick),
		mCurve(copy.mCurve)
	{

	}

	EllipticCurvePoint EllipticCurveBrick::operator*(const EllipticCurveNumber & multIn) const
	{
#ifndef NDEBUG
		if (&mCurve != &multIn.mCurve) throw std::runtime_error("curves instances must match.");
#endif
		EllipticCurvePoint ret(mCurve);

		multiply(multIn, ret);
		
		return ret;
	}

	void EllipticCurveBrick::multiply(const EllipticCurveNumber & multIn, EllipticCurvePoint & result) const
	{
#ifndef NDEBUG
		if (&mCurve != &multIn.mCurve) throw std::runtime_error("curves instances must match.");
		if (&mCurve != &result.mCurve) throw std::runtime_error("curves instances must match.");
#endif

		big x, y;

		x = mirvar(mCurve.mMiracl, 0);
		y = mirvar(mCurve.mMiracl, 0);

		mul2_brick(mCurve.mMiracl, (ebrick2*)&mBrick, multIn.mVal, x, y);
		epoint2_set(mCurve.mMiracl, x, y, 0, result.mVal);

		mirkill(x);
		mirkill(y);
	}

	std::ostream & operator<<(std::ostream & out, const EllipticCurveNumber & val)
	{
		cotstr(val.mCurve.mMiracl, val.mVal, val.mCurve.mMiracl->IOBUFF);
		out << val.mCurve.mMiracl->IOBUFF;

		return out;
	}

	std::ostream & operator<<(std::ostream & out, const EllipticCurvePoint & val)
	{

		if (val.mVal->marker == MR_EPOINT_INFINITY)
		{
			out << "(Infinity)";
		}
		else
		{
#if defined(MR_SIMPLE_BASE) || defined(MR_SIMPLE_IO)
			otstr(val.mCurve.mMiracl, val.mVal->X, val.mCurve.mMiracl->IOBUFF);
			out << val.mCurve.mMiracl->IOBUFF;
			otstr(val.mCurve.mMiracl, val.mVal->Y, val.mCurve.mMiracl->IOBUFF);
			out << val.mCurve.mMiracl->IOBUFF;
#else
			cotstr(val.mCurve.mMiracl, val.mVal->X, val.mCurve.mMiracl->IOBUFF);
			out << val.mCurve.mMiracl->IOBUFF;
			cotstr(val.mCurve.mMiracl, val.mVal->Y, val.mCurve.mMiracl->IOBUFF);
			out << val.mCurve.mMiracl->IOBUFF;
#endif
		}

		return out;
	}

}