#pragma once

#include <cinttypes>
#include <iomanip>
#include <vector>
#include <sstream>
#include <iostream>
#include <emmintrin.h>
#include <smmintrin.h>
#include "Common/Timer.h"
//#include <mmintrin.h>
//#include <xmmintrin.h>

#ifdef _MSC_VER 
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define TODO(x) __pragma(message (__FILE__ ":"__STR1__(__LINE__) " Warning:TODO - " #x))
#define ALIGNED(__Declaration, __alignment) __declspec(align(__alignment)) __Declaration 
#else
//#if defined(__llvm__)
#define TODO(x) 
//#else
//#define TODO(x) DO_PRAGMA( message ("Warning:TODO - " #x))
//#endif

#define ALIGNED(__Declaration, __alignment) __Declaration __attribute__((aligned (16)))
#endif

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define LOCATION __FILE__ ":" STRINGIZE(__LINE__)


namespace osuCrypto {


	typedef uint64_t u64;
	typedef int64_t i64;
	typedef uint32_t u32;
	typedef int32_t i32;
	typedef uint16_t u16;
	typedef int16_t i16;
	typedef uint8_t u8;
	typedef int8_t i8;

	enum Role
	{
		First = 0,
		Second = 1
	};

	extern Timer gTimer;

	typedef  __m128i block;



#ifdef _MSC_VER
	inline block operator^(const block& lhs, const block& rhs)
	{
		return _mm_xor_si128(lhs, rhs);
	}
	inline block operator&(const block& lhs, const block& rhs)
	{
		return _mm_and_si128(lhs, rhs);
	}

	inline block operator<<(const block& lhs, const u8& rhs)
	{
		return _mm_slli_epi64(lhs, rhs);
	}
	inline block operator>>(const block& lhs, const u8& rhs)
	{
		return _mm_srli_epi64(lhs, rhs);
	}
	inline block operator+(const block& lhs, const block& rhs)
	{
		return _mm_add_epi64(lhs, rhs);
	}
#endif

	extern const block ZeroBlock;
	extern const block OneBlock;
	extern const block AllOneBlock;
	extern const block CCBlock;

	inline u64 roundUpTo(u64 val, u64 step)
	{
		return ((val + step - 1) / step) * step;
	}

	std::ostream& operator<<(std::ostream& out, const block& block);

	class Commit;
	class BitVector;


	void split(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> split(const std::string &s, char delim);

	u64 log2ceil(u64);
	u64 log2floor(u64);
}

inline bool eq(const osuCrypto::block& lhs, const osuCrypto::block& rhs)
{
	osuCrypto::block neq = _mm_xor_si128(lhs, rhs);
	return _mm_test_all_zeros(neq, neq) != 0;
}

inline bool neq(const osuCrypto::block& lhs, const osuCrypto::block& rhs)
{
	osuCrypto::block neq = _mm_xor_si128(lhs, rhs);
	return _mm_test_all_zeros(neq, neq) == 0;
}
