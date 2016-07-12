#pragma once
#include "GarbledCircuit.h"
#include "Circuit/Wire.h"
#include "Circuit/Gate.h"
#include <unordered_map>
#include "Common/Defines.h"
#include "Crypto/AES.h"

namespace osuCrypto{
   class ClassicGarbledCircuit : GarbledCircuit
   {
   public:
      ClassicGarbledCircuit(const  Circuit& cd);
      ~ClassicGarbledCircuit();
 
      std::vector<block> mWires;
      std::vector<GarbledGate<4>> mGates;
	  BitVector mTranslationTable;

      PRNG mSeedKey;
      u64 mNextRandIdx;
      block mGlobalOffset;

      void Garble(const Circuit& cir, const block& seed) override;
      void evaluate(const Circuit& cir, std::vector<block>& labels) override;
      bool Validate(const Circuit& cir, const block&) override;
      void translate(const Circuit& cir, std::vector<block>&  labels, BitVector& output);
   };

}