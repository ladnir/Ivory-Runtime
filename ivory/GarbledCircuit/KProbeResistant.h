#pragma once
#include "Common/Defines.h"
#include "Common/BitVector.h"
#include "Crypto/PRNG.h"
#include <vector>

#define ENCODABLE_KPROBE

namespace osuCrypto
{
	class KProbeMatrix
	{
	public:

		KProbeMatrix():mEncodingSize(0) {}

		/// <summary>Constructs a new K-Probe resistant matrix</summary>
		KProbeMatrix(u64 inputSize, u64 secParam, PRNG& prng);

		/// <summary>Constructs a existing K-Probe resistant matrix from a file</summary>
		/// <param name='filename'> the path to the file that contains the k-Probe matrix specification</param>
		KProbeMatrix(std::string filename);

		~KProbeMatrix();


#ifdef ENCODABLE_KPROBE
		/// <summary> Encodes BitVector x as x' such that M x'=x. This function is typically used by the evaluator to compute its encoded input.</summary>
		/// <param name='input'>x = the evaluator's plaintext input value</param>
		/// <param name='encoding'> x' = the evaluator's encoded input value that will be used with the k-probe matrix</param>
		/// <param name='prng'>random number generator used to generate a random encoding</param>
		void encode(const BitVector& input, BitVector& encoding, PRNG& prng) const;

		///<summary>Allows the garbler to transforms the evaluator's unencoded garbled input wire to garbled input wires that are encoded under this k-probe resistant matrix. i.e. if we have M x'=x, then we are making labels that encode x'</summary>
		///<param name='originalLabels'>all of the input zero wire label keys.</param>
		///<param name='encodedLabels'>the encoded labels that the evaluator will passed through the k-probe matrix.</param>
		///<param name='prng'>random number generator used to generate a random encoding</param>
		void encode(
			const block* originalLabels,
			std::vector<block>& encodedLabels,
			PRNG& prng) const;
#endif
		/// <summary>Given M and the bit vector x', this function computes the bitVector x s.t. M x'=x</summary>
		/// <param name='encoding'>the input value x' encoded under this k-probe matrix.</param>
		/// <param name='input'>returns this, the decoded value x.</param>
		void decode(const BitVector& encoding, BitVector& input) const;

		/// <summary>Given M and the label vector x', this function computes the labels x s.t. M x'=x</summary>
		/// <param name='encoding'>the input value x' encoded under this k-probe matrix.</param>
		/// <param name='input'>returns this, the decoded value x.</param>
		void decode(const std::vector<block>& encoding, std::vector<block>& input) const;

		void decode(const std::vector<block>& encoding, std::vector<block>::iterator input) const;


		void decode(const std::vector<std::array<block, 2>>& encoding,
			std::vector<std::array<block, 2>>& input) const;


		void loadOrBuild(u64 inputSize, u64 secParam, PRNG& prng);

		void load(std::istream& in);
		void save(std::ostream& out) const;


		/// <summary>Builds the k probe resistant matrix using the prng as a source of randomness</summary>
		/// <param name='inputSize'></param>
		/// <param name='secParam'></param>
		/// <param name='prng'></param>
		void build(
			u64 inputSize, 
			u64 secParam,
			PRNG& prng);

		u64 encodingSize() const { return mEncodingSize; }

		u64 mEncodingSize;
		std::vector<BitVector> mMtx;

		//std::vector<std::vector<u8>> mMtx;
		//std::vector<std::vector<block>> mBlockMaskMtx;

		//const u64& signature();
		u64 mSignature;
	};

}