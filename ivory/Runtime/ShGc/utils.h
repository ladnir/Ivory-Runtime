#pragma once
#include <ivory/Circuit/BetaCircuit.h>
#include <cryptoTools/Common/BitVector.h>

#include <future>
#include <memory>
#include <vector>

namespace osuCrypto
{
    namespace ShGc
    {
        typedef std::shared_ptr<std::vector<block>> GarbledMem;

        struct CircuitItem
        {
            CircuitItem() :mCircuit(nullptr), mDebugFlag(false) {}
            CircuitItem(CircuitItem&&) = default;

            BetaCircuit* mCircuit;
            std::vector<GarbledMem> mLabels;

            bool mDebugFlag;
            u64 mInputBundleCount;
            u64 mCopyBegin = 0;
            u64 mCopyEnd = -1;
            i64 mLeftShift = 0;
        };

        struct InputItem
        {
            InputItem() = default;
            InputItem(InputItem&&) = default;

            BitVector mInputVal;
            GarbledMem mLabels;
        };

        struct OutputItem
        {
            OutputItem() = default;
            OutputItem(OutputItem&&) = default;

            GarbledMem mLabels;
            std::vector<u64> mOutPartyIdxs;
            std::unique_ptr<std::promise<BitVector>> mOutputProm;
        };


    }
}