#pragma once
#include "Common/Defines.h"
#include "Crypto/AES.h"
//#include "Crypto/sha1.h"
#include <vector>

#define SEED_SIZE   AES_BLK_SIZE
#define RAND_SIZE   AES_BLK_SIZE


namespace osuCrypto
{

	class PRNG
	{
	public:

		block mSeed;
		std::vector<block> mBuffer, mIndexArray;

		AES mAes;
		u64 mBytesIdx, mBlockIdx, mBufferByteCapacity;

		void refillBuffer();

		PRNG();
		PRNG(const block& seed);
		PRNG(const PRNG&) = delete;
		
		// Set seed from array
		void SetSeed(const block& b);

		block get_block();
		double get_double();
		u8 get_uchar();
		u32 get_u32();
		u8 get_bit() { return get_uchar() & 1; }
		//bigint randomBnd(const bigint& B);
		//modp get_modp(const Zp_Data& ZpD);
		u64 get_u64()
		{
			u64 a;
			get_u8s((u8*)&a, sizeof(a));
			return a;
		}
		//void get_ByteStream(ByteStream& ans, u64 len);
		void get_u8s(u8* ans, u64 len);

		const block get_seed() const
		{
			return mSeed;
		}


		typedef u64 result_type;
		static u64 min() { return 0; }
		static u64 max() { return (u64)-1; }
		u64 operator()() {
			return get_u64();
		}
		u64 operator()(u64 mod) {
			return get_u64() % mod;
		}
	};
}
