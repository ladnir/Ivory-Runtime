#include "Common/BitVector.h"
#include "KProbeResistant.h"
#include <fstream>
#include "Common/Log.h"

#include "NTL/GF2.h"
#include "NTL/GF2X.h"
#include "NTL/GF2E.h"
#include "NTL/GF2XFactoring.h"
#include "NTL/GF2EX.h"
#include <array>

namespace osuCrypto
{

	static std::array<block, 2> sBlockMasks{ { ZeroBlock, _mm_set_epi64x((u64)-1, (u64)-1) } };

	NTL::GF2E int_to_GF2E(int num) {
		NTL::GF2X num_as_gf2x; // initially zero
		int bits = num;
		int i = 0;

		// for each bit in num, if the bit is 1, turn on the proper coeff in the gf2x polynomial
		while (bits) {
			if (bits & 1) {
				NTL::SetCoeff(num_as_gf2x, i);
			}
			i++;
			bits >>= 1;
		}

		return NTL::to_GF2E(num_as_gf2x);
	}


	KProbeMatrix::KProbeMatrix(u64 inputSize, u64 secParam, PRNG & prng)
	{
		loadOrBuild(inputSize, secParam, prng);
	}



	KProbeMatrix::KProbeMatrix(std::string filename)
	{
		std::fstream file;
		file.open(filename);

		load(file);
	}

	KProbeMatrix::~KProbeMatrix()
	{

	}


	std::mutex loadOrBuildMtx;
	void KProbeMatrix::loadOrBuild(u64 inputSize, u64 secParam, PRNG& prng)
	{

		std::lock_guard<std::mutex> lock(loadOrBuildMtx);

		std::stringstream ss;
		ss << "./kProbe_" << inputSize << "_" << secParam << "_" << prng.get_block();
		std::string filename = ss.str();

		//Log::out << "kprobe " << prng.get_block() << Log::endl;

		std::fstream file;
		file.open(filename, std::ios::binary | std::ios::in);

		if (file.is_open())
			load(file);
		else
		{
			build(inputSize, secParam, prng);

			file.open(filename, std::ios::binary | std::ios::trunc | std::ios::out);
			save(file);
		}

		if (mMtx[0].size() >= 64)
			mSignature = *(u64*)mMtx[0].data();
		else if (mMtx[0].size() >= 16)
			mSignature = *(u16*)mMtx[0].data();
		else
		{
			throw std::runtime_error("");
		}
	}


	std::mutex buildMtx;
	void KProbeMatrix::build(
		u64 inputSize,
		u64 secParam,
		PRNG& prng)
	{
		// NTL is stupid and made theit init function not thread safe. so wrap this whole area of code so that NTL 
		// code is all sequential

		if (inputSize)
		{

			std::lock_guard<std::mutex> lock(buildMtx);

			mMtx.clear();
			mMtx.resize(inputSize);
			//mBlockMaskMtx.resize(inputSize);

			u64 t = static_cast<u64>(
				std::ceil(
					std::max(
						std::log2(4 * inputSize),
						std::log2(4 * secParam)
						)));


			u64 T = (1 << (t - 1));
			double h = secParam + (std::log2(inputSize) + inputSize + secParam);
			// find the minimum t such that	2 ^ t >= k + (lg(n) + n + k	) = t
			while (T > h / (t - 1))
			{
				--t;
				T >>= 1;
			}

			u64 K = static_cast<u64>(
				std::ceil(
					(std::log2(inputSize) + inputSize + secParam) / (double)t));

			// N is roughly 2 ^ t
			u64 N = K + secParam - 1;
			u64 m = N*t;
#ifdef ENCODABLE_KPROBE
			mEncodingSize = m + inputSize;
#else
			mEncodingSize = m ;
#endif
			// initialize the GF2 extension with an irreducible polynomial of size t as modulus.
			// essentially we are creating F_{2^t}
			NTL::GF2X gf2e_modulus = NTL::BuildIrred_GF2X(static_cast<long>(t));
			NTL::GF2E::init(gf2e_modulus);
			 

			for (u64 i = 0; i < inputSize; ++i)
			{
				mMtx[i].resize(m);
				//mBlockMaskMtx[i].resize(m);

				u64 idx = 0;
				NTL::GF2EX p(static_cast<long>(K - 1));

				// gets a random polynomial in F_{2^t}[x] of degree K-1
				for (u64 i = 0; i < K - 1; ++i)
					SetCoeff(p, static_cast<long>(i), prng.get_bit());

				// we calculate P(1)_2, ..., P(N)_2
				for (u64 x = 1; x <= N; x++)
				{
					NTL::GF2E x_element = int_to_GF2E(static_cast<long>(x));

					// v = p(x)
					NTL::GF2X v = rep(eval(p, x_element));

					for (u64 j = 0; j < t; ++j, ++idx) {
						mMtx[i][idx] = static_cast<u8>(IsOne(coeff(v, static_cast<long>(j))));

						//if (mMtx[i][idx])
							//mBlockMaskMtx[i][idx] = allOnesBlock;
					}
				}

			}
		}
	}

#ifdef ENCODABLE_KPROBE
	void KProbeMatrix::encode(const BitVector & input, BitVector & encoding, PRNG& prng) const
	{
		// find x' such that  Mx' = x 
		// M has n rows and m columns
		u64 n = mMtx.size();
		u64 m = mEncodingSize;
		u64 h = m - n;

		if (input.size() != n)
			throw std::runtime_error("");

		encoding.reset(m);
		std::vector<u8> isSet(m);

		encoding.randomize(prng);
		for (u64 i = 0; i < n; ++i)
		{
			BitVector projected(encoding.data(), h);

			projected &= mMtx[i];

			if (projected.parity() != input[i])
			{
				encoding[h + i] = 1;
			}
			else
			{
				encoding[h + i] = 0;
			}
		}
	}


	void KProbeMatrix::encode(
		const block* originalLabels,
		std::vector<block>& encodedLabels,
		PRNG& prng)const
	{
		u64 n = mMtx.size();
		u64 m = mEncodingSize;;
		u64 h = m - n;

		//if (originalLabels.size() != n)
		//	throw std::runtime_error("");

		encodedLabels.clear();
		encodedLabels.resize(m);

		prng.get_u8s((u8*)encodedLabels.data(), encodedLabels.size() * sizeof(block));

		for (u64 i = 0; i < mMtx.size(); i++)
		{
			encodedLabels[h + i] = originalLabels[i];

			for (u64 j = 0; j < h; ++j)
			{
				encodedLabels[h + i] = encodedLabels[h + i] ^ (encodedLabels[j] & sBlockMasks[mMtx[i][j]]);
			}
		}
	}
#endif

	void KProbeMatrix::decode(const BitVector & encoding, BitVector & input) const
	{
		u64 n = mMtx.size();
		u64 m = mEncodingSize;

#ifdef ENCODABLE_KPROBE 
		u64 h = m - n;
#else
		u64 h = m;
#endif

		if (encoding.size() != m)
			throw std::runtime_error("");

		input.reset(n);

		for (u64 i = 0; i < n; ++i)
		{

#ifdef ENCODABLE_KPROBE
			input[i] = encoding[h + i];
#endif

			for (u64 j = 0; j < h; ++j)
			{
				input[i] = input[i] ^ (mMtx[i][j] & encoding[j]);
			}
		}

	}

	void KProbeMatrix::decode(
		const std::vector<block>& encoding,
		std::vector<block>& input)const
	{
		u64 n = mMtx.size();

		input.clear();
		input.resize(n);

		decode(encoding, input.begin());

	}

	void KProbeMatrix::decode(
		const std::vector<block>& encoding,
		std::vector<block>::iterator input)const
	{
		u64 n = mMtx.size();
		u64 m = mEncodingSize;
#ifdef ENCODABLE_KPROBE
		u64 h = m - n;
#else
		u64 h = m;
#endif

		// will throw if there isn't enough space.
	/*	input += (n-1);
		block* ptr = &*input;*/

		if (encoding.size() != m)
			throw std::runtime_error("");

		for (u64 i = 0; i < n; ++i, ++input)
		{
#ifdef ENCODABLE_KPROBE
			*input = encoding[h + i];
#endif
			for (u64 j = 0; j < h; ++j)
			{
				*input = *input ^ (encoding[j] & sBlockMasks[mMtx[i][j]]);
			}
		}
	}

	void KProbeMatrix::decode(
		const std::vector<std::array<block, 2>>& encoding,
		std::vector<std::array<block, 2>>& input)const
	{
		u64 n = mMtx.size();
		u64 m = mEncodingSize;

#ifdef ENCODABLE_KPROBE
		u64 h = m - n;
#else
		u64 h = m;
#endif

		if (encoding.size() != m)
			throw std::runtime_error("");

		input.clear();
		input.resize(n);

		for (u64 i = 0; i < n; ++i)
		{
#ifdef ENCODABLE_KPROBE
			input[i][0] = encoding[h + i][0];
			input[i][1] = encoding[h + i][1];
#endif

			for (u64 j = 0; j < h; ++j)
			{
				input[i][0] = input[i][0] ^ (encoding[j][0] & sBlockMasks[mMtx[i][j]]);
				input[i][1] = input[i][1] ^ (encoding[j][1] & sBlockMasks[mMtx[i][j]]);
			}
		}
	}


	void KProbeMatrix::load(std::istream & in)
	{
		u64 n, h;

		in.read((char*)&n, sizeof(u64));
		in.read((char*)&h, sizeof(u64));

#ifdef ENCODABLE_KPROBE 
		mEncodingSize = h + n;
#else 
		mEncodingSize = h;
#endif

		mMtx.clear();
		mMtx.resize(n); 

		for (u64 i = 0; i < n; ++i)
		{
			mMtx[i].resize(h); 

			in.read((char*)mMtx[i].data(), mMtx[i].sizeBytes());
		}
	}

	void KProbeMatrix::save(std::ostream & out) const
	{

		u64 n = mMtx.size();

#ifdef ENCODABLE_KPROBE
		u64 h = mEncodingSize - n; 
#else
		u64 h = mEncodingSize; 
#endif

		
		out.write((char*)& n, sizeof(u64));
		out.write((char*)& h, sizeof(u64));

		for (auto& row : mMtx)
		{
			out.write((char*)row.data(), row.sizeBytes());
		}
	}

}
