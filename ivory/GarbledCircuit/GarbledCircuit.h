#pragma once
#include "cryptoTools/Common/Defines.h"
#include "Circuit/Circuit.h"
#include <istream>
#include <ostream>
#include <functional>

namespace osuCrypto{
   class GarbledCircuit
   {
   public:

      virtual void Garble(const Circuit& cd, const block& seed) = 0;
      virtual bool Validate(const Circuit& cd, const block& seed) = 0;
      virtual void evaluate(const Circuit& cd, std::vector<block>& labels) = 0;
      virtual void translate(const Circuit& cd, std::vector<block>& labels, BitVector& output) = 0;
   };

}