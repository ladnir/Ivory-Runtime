#include "ClassicGarbledCircuit.h"
#include "Crypto/AES.h"

namespace osuCrypto{
   ClassicGarbledCircuit::ClassicGarbledCircuit(const Circuit& cd) 
   {
   }


   ClassicGarbledCircuit::~ClassicGarbledCircuit()
   {
   }



#define Classic_GC_PRINT
#define Classic_GC_EVAL_INLINE

   void ClassicGarbledCircuit::Garble(const Circuit& cir, const block& seed)
   {
      mWires.clear(); mGates.clear();
      mWires.reserve(cir.WireCount());
      mGates.reserve(cir.Gates().size());

	  mSeedKey.SetSeed(seed);
      mNextRandIdx = 0;

      mGlobalOffset = mSeedKey.get_block();
      (u8*)&(mGlobalOffset)[0] |= 1; // make sure the bottom bit is a 1 for point-n-permutue


	  mWires.resize(cir.WireCount());
	  mSeedKey.get_u8s((u8*)mWires.data(), mWires.size() * sizeof(block));
      for (u64 i = 0; i < mWires.size(); ++i)
            mWires.emplace_back(mSeedKey.get_block());

	  std::array<AES, 2> aKeys, bKeys;


	  auto cIter = mWires.begin() + cir.InputWireCount();
      for (const auto& gate : cir.Gates())
      {
         auto& c = *cIter++;
         auto& a = mWires[gate.mInput[0]];
         auto& b = mWires[gate.mInput[1]];

         std::vector<u8> permute{ 0, 1, 2, 3 };
         if (PermuteBit(a)){
            std::swap(permute[0], permute[1]);
            std::swap(permute[2], permute[3]);
         }
         if (PermuteBit(b)){
            std::swap(permute[0], permute[2]);
            std::swap(permute[1], permute[3]);
         }

         AES128::EncKeyGen(a, aKeys[0]);
         AES128::EncKeyGen(a^(mGlobalOffset), aKeys[1]);
         AES128::EncKeyGen(b, bKeys[0]);
         AES128::EncKeyGen(b^(mGlobalOffset), bKeys[1]);

         mGates.emplace_back();
         for (u64 i = 0; i < 4; ++i)
         {
            const block& wireValue = ((u8)gate.Type() & (1<<i)) ? c^(mGlobalOffset) : c;
            block& garbledRow = mGates.back().mGarbledTable[permute[i]];

            AES128::EcbEncBlock(aKeys[i % 2], wireValue, garbledRow);
            AES128::EcbEncBlock(bKeys[i / 2], garbledRow, garbledRow);

         }
      }


	  mTranslationTable.reset(cir.Outputs().size());
	  for (u64 i = 0; i < cir.Outputs().size(); i++)
	  {
		  auto& wireIdx = cir.Outputs()[i];
		  mTranslationTable[i] = (PermuteBit(mWires[wireIdx]));
	  }
   }

   void ClassicGarbledCircuit::evaluate(const Circuit& cir, std::vector<block>& labels)
   {
      auto garbledGateIter = mGates.begin();
	  if (labels.size() != cir.InputWireCount())
		  throw std::runtime_error("");

	  if (mWires.size())
	  {
		  for (u64 i = 0; i < cir.InputWireCount(); ++i)
		  {
			  if (notEqual(labels[i], mWires[i]) && notEqual(labels[i], mWires[i]^(mGlobalOffset)))
				  throw std::runtime_error("");
		  }
	  }

      for (const auto& gate : cir.Gates())
      {
         labels.emplace_back();
         auto& newOutLabel = labels.back();
         const auto& garbledTable = garbledGateIter++->mGarbledTable;
         const block& a = labels[gate.mInput[0]];
         const block& b = labels[gate.mInput[1]];
         const u64 tableIdx = (PermuteBit(a) ? 1 : 0) + (PermuteBit(b) ? 2 : 0);

         AES128::Key aKey, bKey;
         AES128::DecKeyGen(a, aKey);
         AES128::DecKeyGen(b, bKey);

         AES128::EcbDecBlock(bKey, garbledTable[tableIdx], newOutLabel);
         AES128::EcbDecBlock(aKey, newOutLabel, newOutLabel);
      }
   }

   bool ClassicGarbledCircuit::Validate(const Circuit& cir, const block&)
   {
      throw new not_implemented();

      //for (auto& wire : cd.mWires)
      //{
      //   labels.push_back(wire->labels[1]);
      //}

      //for (auto& gate : cd.mGates)
      //{

      //   Gate& gate = *dynamic_cast<Gate*>(wire.get());

      //   u64 aIdx = gate.mInputs[0]->mIdx;
      //   u64 bIdx = gate.mInputs[1]->mIdx;


      //   //Lg::out << "a    " << (i32)labels[aIdx][0] << Lg::endl;
      //   //Lg::out << "b    " << (i32)labels[bIdx][0] << Lg::endl;

      //   u64 tableIdx{};
      //   if (labels[aIdx][0] & 1) tableIdx += 1;
      //   if (labels[bIdx][0] & 1) tableIdx += 2;

      //   CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption deca(labels[aIdx], labels[aIdx].size());
      //   CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption decb(labels[bIdx], labels[bIdx].size());

      //   labels.emplace_back(16);

      //   //Lg::out << "tableIdx = " << tableIdx << Lg::endl;

      //   decb.ProcessData(labels.back(), gate.mTable[tableIdx], gate.mTable[tableIdx].size());
      //   deca.ProcessData(labels.back(), labels.back(), gate.mTable[tableIdx].size());
      //   //Lg::out << "c    " << (i32)labels.back()[0] << Lg::endl;
      //}
   }

   void ClassicGarbledCircuit::translate(const Circuit& cir, std::vector<block>&  labels, BitVector& output)
   {
      output.reset(mTranslationTable.size());

	  for (u64 i = 0; i < cir.Outputs().size(); ++i)
	  {
         output[i] = (mTranslationTable[i] ^ PermuteBit(labels[cir.Outputs()[i]]));

		 if (mWires.size())
		 {
			 if (output[i] == 0)
			 {
				 if (notEqual(labels[cir.Outputs()[i]], mWires[cir.Outputs()[i]]))
					 throw std::runtime_error("");
			 }
			 else
			 {
				 if (notEqual(labels[cir.Outputs()[i]], mWires[cir.Outputs()[i]]^(mGlobalOffset)))
					 throw std::runtime_error("");
			 }
		 }
	  }
   }
 

   //#ifdef Classic_GC_PRINT
   //      Lg::out << "gate " << Lg::endl;
   //      Lg::out << "c1 " << c^(mGlobalOffset) << "  c0 " << c.mZeroLabel << Lg::endl;
   //      Lg::out << "Permute[0] " <<(int) permute[0] << Lg::endl;
   //      Lg::out << "Permute[1] " <<(int) permute[1] << Lg::endl;
   //      Lg::out << "Permute[2] " <<(int) permute[2] << Lg::endl;
   //      Lg::out << "Permute[3] " <<(int) permute[3] << Lg::endl;
   //#endif
   //#ifdef Classic_GC_PRINT 
   //         block aLabel = (aIdx) ? a^(mGlobalOffset) : a.mZeroLabel;
   //         block bLabel = (bIdx) ? b^(mGlobalOffset) : b.mZeroLabel;
   //
   //         AES128::Key aKey2;
   //         AES128::EncKeyGen(aLabel, aKey2);
   //
   //         Lg::out << "garbledTable[" << permutedIdx << "] = Enc(Enc(" << wireValue << "))       " << aLabel << Lg::endl;
   //         Lg::out << "garbledTable[" << permutedIdx << "] = Enc(    " << temp << ")        " << bLabel << Lg::endl;
   //         Lg::out << "garbledTable[" << permutedIdx << "] =         " << garbledRow << Lg::endl << Lg::endl;
   //
   //         block temp2, temp3;
   //         aGoldKeys[aIdx].ProcessData((u8*)&(temp2), (u8*)&(wireValue), 16);
   //         bGoldKeys[bIdx].ProcessData((u8*)&(temp3), (u8*)&(temp2), 16);
   //
   //         assert(temp3 == garbledRow);
   //
   //         AES128::Key aDecKey, bDecKey;
   //         AES128::DecKeyGen(aLabel, aDecKey);
   //         AES128::DecKeyGen(bLabel, bDecKey);
   //
   //
   //         AES128::EcbDecBlock(bDecKey, garbledRow, temp);
   //
   //         assert(temp == temp2);
   //
   //         AES128::EcbDecBlock(aDecKey, temp, temp);
   //
   //         assert(temp == wireValue);
   //#endif
   //#ifdef Classic_GC_PRINT
   //      Lg::out << "a    " << a << Lg::endl;
   //      Lg::out << "b    " << b << Lg::endl;
   //      Lg::out << "tableIdx = " << tableIdx << Lg::endl;
   //      Lg::out << "c''  " << gate.mGarbledTable[tableIdx] << Lg::endl;
   //      Lg::out << "c'   " << temp << Lg::endl;
   //      Lg::out << "c    " << labels.back() << Lg::endl << Lg::endl;
   //#endif

}
