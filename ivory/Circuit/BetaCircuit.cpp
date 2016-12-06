#include "BetaCircuit.h"
#include <vector>
#include <unordered_map>
#include "Common/BitVector.h"
namespace osuCrypto
{

    BetaCircuit::BetaCircuit()
        :mNonXorGateCount(0),
        mWireCount(0)
    {
    }



    BetaCircuit::~BetaCircuit()
    {
    }

    void BetaCircuit::addTempWireBundle(BetaBundle & in)
    {
        for (u64 i = 0; i < in.mWires.size(); ++i)
        {
            in.mWires[i] = mWireCount++;
        }
    }

    void BetaCircuit::addInputBundle(BetaBundle & in)
    {
        for (u64 i = 0; i < in.mWires.size(); ++i)
        {
            in.mWires[i] = mWireCount++;
        }

        mInputs.push_back(in);
    }
    

    void BetaCircuit::addOutputBundle(BetaBundle & out)
    {
        for (u64 i = 0; i < out.mWires.size(); ++i)
        {
            out.mWires[i] = mWireCount++;
        }

        mOutputs.push_back(out);
    }


    void BetaCircuit::addGate(
        BetaWire in0, 
        BetaWire in1, 
        GateType gt, 
        BetaWire out)
    {
        if (gt == GateType::a ||
            gt == GateType::b ||
            gt == GateType::na ||
            gt == GateType::nb ||
            gt == GateType::One ||
            gt == GateType::Zero)
            throw std::runtime_error("");

        if (gt != GateType::Xor && gt != GateType::Nxor) ++mNonXorGateCount;
        mGates.emplace_back(in0, in1, gt, out);

    }

    void BetaCircuit::addPrint(BetaBundle in)
    {
        for (auto& i : in.mWires)
        {
            addPrint(i);
        }
    }

    void osuCrypto::BetaCircuit::addPrint(BetaWire wire)
    {
        mPrints.emplace_back(mGates.size(), wire, "");
    }

    void osuCrypto::BetaCircuit::addPrint(std::string str)
    {
        mPrints.emplace_back(mGates.size(), -1,str);
    }
    void BetaCircuit::evaluate(ArrayView<BitVector> input, ArrayView<BitVector> output, bool print)
    {
        BitVector mem(mWireCount);

        if (input.size() != mInputs.size())
        {
            throw std::runtime_error(LOCATION);
        }

        for (u64 i = 0; i < input.size(); ++i)
        {
            if (input[i].size() != mInputs[i].mWires.size())
                throw std::runtime_error(LOCATION);

            for (u64 j = 0; j < input[i].size(); ++j)
            {
                mem[mInputs[i].mWires[j]] = input[i][j];
            }
        }
        auto iter = mPrints.begin();

        for (u64 i = 0; i < mGates.size(); ++i)
        {
            while (print && iter != mPrints.end() && std::get<0>(*iter) == i)
            {
                auto wireIdx = std::get<1>(*iter);
                auto str = std::get<2>(*iter);

                if(wireIdx != -1)
                    std::cout << (u64)mem[wireIdx];
                if(str.size())
                    std::cout << str;

                ++iter;
            }

            u64 idx0 = mGates[i].mInput[0];
            u64 idx1 = mGates[i].mInput[1];
            u64 idx2 = mGates[i].mOutput;

            u8 a = mem[idx0];
            u8 b = mem[idx1];

            mem[idx2] = GateEval(mGates[i].mType, (bool)a, (bool)b);

        }
        while (print && iter != mPrints.end())
        {
            auto wireIdx = std::get<1>(*iter);
            auto str = std::get<2>(*iter);

            if (wireIdx != -1)
                std::cout << (u64)mem[wireIdx];
            if (str.size())
                std::cout << str;

            ++iter;
        }


        if (output.size() != mOutputs.size())
        {
            throw std::runtime_error(LOCATION);
        }

        for (u64 i = 0; i < output.size(); ++i)
        {
            if (output[i].size() != mOutputs[i].mWires.size())
                throw std::runtime_error(LOCATION);

            for (u64 j = 0; j < output[i].size(); ++j)
            {
                output[i][j] = mem[mOutputs[i].mWires[j]];
            }
        }
    }

    void BetaCircuit::levelize()
    {
        mLevelGates.clear();
        mLevelGates.emplace_back();


        std::unordered_map<u64, u64> levelMap;


        for (u64 i = 0; i < mGates.size(); ++i)
        {
            u64 level = 0;


            static_assert(sizeof(BetaWire) == sizeof(u32), "");

            auto idx = mGates[i].mInput[0];
            auto iter = levelMap.find(idx);

            if (iter != levelMap.end())
            {
                level = iter->second + 1;
            }

            idx = mGates[i].mInput[1];
            iter = levelMap.find(idx);

            if (iter != levelMap.end())
            {
                level = std::max(iter->second + 1, level);
            }

            idx = mGates[i].mOutput;
            levelMap[idx] = level;


            if (level == mLevelGates.size())
                mLevelGates.emplace_back();

            if (mGates[i].mType == GateType::Xor || mGates[i].mType == GateType::Nxor)
            {
                mLevelGates[level].mXorGates.push_back(mGates[i]);
            }
            else
            {
                mLevelGates[level].mAndGates.push_back(mGates[i]);
            }
        }
    }
}