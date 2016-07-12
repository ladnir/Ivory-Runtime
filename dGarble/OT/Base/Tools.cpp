#include "Tools.h"
#include "Common/Defines.h"
#include "Crypto/PRNG.h"
#include "Common/Log.h"
#include <wmmintrin.h>

#ifndef _MSC_VER
#include <x86intrin.h>
#endif 

namespace osuCrypto {

	void mul128(block x, block y, block& xy1, block& xy2)
	{
		auto t1 = _mm_clmulepi64_si128(x, y, (int)0x00);
		auto t2 = _mm_clmulepi64_si128(x, y, 0x10);
		auto t3 = _mm_clmulepi64_si128(x, y, 0x01);
		auto t4 = _mm_clmulepi64_si128(x, y, 0x11);

		t2 = _mm_xor_si128(t2, t3);
		t3 = _mm_slli_si128(t2, 8);
		t2 = _mm_srli_si128(t2, 8);
		t1 = _mm_xor_si128(t1, t3);
		t4 = _mm_xor_si128(t4, t2);

		xy1 = t1;
		xy2 = t4;
	}



	void eklundh_transpose128(std::array<block, 128>& inOut)
	{
		const static u64 TRANSPOSE_MASKS128[7][2] = {
			{ 0x0000000000000000, 0xFFFFFFFFFFFFFFFF },
			{ 0x00000000FFFFFFFF, 0x00000000FFFFFFFF },
			{ 0x0000FFFF0000FFFF, 0x0000FFFF0000FFFF },
			{ 0x00FF00FF00FF00FF, 0x00FF00FF00FF00FF },
			{ 0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F },
			{ 0x3333333333333333, 0x3333333333333333 },
			{ 0x5555555555555555, 0x5555555555555555 }
		};

		u32 width = 64;
		u32 logn = 7, nswaps = 1;

#ifdef TRANSPOSE_DEBUG
		stringstream input_ss[128];
		stringstream output_ss[128];
#endif

		// now transpose output in-place
		for (u32 i = 0; i < logn; i++)
		{
			u64 mask1 = TRANSPOSE_MASKS128[i][1], mask2 = TRANSPOSE_MASKS128[i][0];
			u64 inv_mask1 = ~mask1, inv_mask2 = ~mask2;

			// for width >= 64, shift is undefined so treat as x special case
			// (and avoid branching in inner loop)
			if (width < 64)
			{
				for (u32 j = 0; j < nswaps; j++)
				{
					for (u32 k = 0; k < width; k++)
					{
						u32 i1 = k + 2 * width*j;
						u32 i2 = k + width + 2 * width*j;

						// t1 is lower 64 bits, t2 is upper 64 bits
						// (remember we're transposing in little-endian format)
						u64& d1 = ((u64*)&inOut[i1])[0];
						u64& d2 = ((u64*)&inOut[i1])[1];

						u64& dd1 = ((u64*)&inOut[i2])[0];
						u64& dd2 = ((u64*)&inOut[i2])[1];

						u64 t1 = d1;
						u64 t2 = d2;

						u64 tt1 = dd1;
						u64 tt2 = dd2;

						// swap operations due to little endian-ness
						d1 = (t1 & mask1) ^ ((tt1 & mask1) << width);

						d2 = (t2 & mask2) ^
							((tt2 & mask2) << width) ^
							((tt1 & mask1) >> (64 - width));

						dd1 = (tt1 & inv_mask1) ^
							((t1 & inv_mask1) >> width) ^
							((t2 & inv_mask2)) << (64 - width);

						dd2 = (tt2 & inv_mask2) ^
							((t2 & inv_mask2) >> width);
					}
				}
			}
			else
			{
				for (u32 j = 0; j < nswaps; j++)
				{
					for (u32 k = 0; k < width; k++)
					{
						u32 i1 = k + 2 * width*j;
						u32 i2 = k + width + 2 * width*j;

						// t1 is lower 64 bits, t2 is upper 64 bits
						// (remember we're transposing in little-endian format)
						u64& d1 = ((u64*)&inOut[i1])[0];
						u64& d2 = ((u64*)&inOut[i1])[1];

						u64& dd1 = ((u64*)&inOut[i2])[0];
						u64& dd2 = ((u64*)&inOut[i2])[1];

						//u64 t1 = d1;
						u64 t2 = d2;

						//u64 tt1 = dd1;
						//u64 tt2 = dd2;

						d1 &= mask1;
						d2 = (t2 & mask2) ^
							((dd1 & mask1) >> (64 - width));

						dd1 = (dd1 & inv_mask1) ^
							((t2 & inv_mask2)) << (64 - width);

						dd2 &= inv_mask2;
					}
				}
			}
			nswaps *= 2;
			width /= 2;
		}
#ifdef TRANSPOSE_DEBUG
		for (u32 colIdx = 0; colIdx < 128; colIdx++)
		{
			for (u32 blkIdx = 0; blkIdx < 128; blkIdx++)
			{
				output_ss[blkIdx] << inOut[offset + blkIdx].get_bit(colIdx);
			}
		}
		for (u32 colIdx = 0; colIdx < 128; colIdx++)
		{
			if (output_ss[colIdx].str().compare(input_ss[colIdx].str()) != 0)
			{
				cerr << "String " << colIdx << " failed. offset = " << offset << endl;
				exit(1);
			}
		}
		cout << "\ttranspose with offset " << offset << " ok\n";
#endif
	}
}
