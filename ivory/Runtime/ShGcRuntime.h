#pragma once
#include "Runtime/Runtime.h"
#include "Circuit/Circuit.h"
#include "Circuit/BetaCircuit.h"
#include "Common/ArrayView.h"
//#include "boost/lockfree/spsc_queue.hpp"
#include <queue>
#include "TwoChooseOne/IknpOtExtSender.h"
#include "Common/ByteStream.h"
#include "TwoChooseOne/IknpOtExtReceiver.h"
#include "Circuit/CircuitLibrary.h"


namespace osuCrypto
{
    typedef std::vector<block> ShGcLabelVec;

    struct ShGcRuntimeData :public RuntimeData
    {
        ShGcRuntimeData(u64 bitCount)
            : mLabels(std::make_shared<ShGcLabelVec>(bitCount))
        { }

        std::shared_ptr<ShGcLabelVec> mLabels;
    };

    class ShGcRuntime : public Runtime
    {
    public:
        enum Role
        {
            Garbler,
            Evaluator
        };

        ShGcRuntime();
        ~ShGcRuntime();

        void init(Channel& chl, block seed, Role role, u64 partyIdx);

        void initVar(std::unique_ptr<RuntimeData>& data, u64 bitCount) override;
        void copyVar(std::unique_ptr<RuntimeData>& data, RuntimeData* copy) override;


        void scheduleInput(RuntimeData* enc, const BitVector& value)override;
        void scheduleInput(RuntimeData* enc, u64 pIdx)override;


        void scheduleOp(Op op, ArrayView<RuntimeData*> io)override;


        void scheduleOutput(RuntimeData* labels, u64 partyIdx)override;
        void scheduleOutput(RuntimeData* labels, std::future<BitVector>& value)override;

        void processesQueue() override {}

        u64 getPartyIdx() override { return mPartyIdx; }

        CircuitLibrary mLibrary;



        void evaluate(
            BetaCircuit& cir,
            ArrayView<block> memory,
            std::array<block, 2>& tweaks,
            ArrayView<GarbledGate<2>> garbledGates);


        void garble(
            BetaCircuit& cir,
            ArrayView<block> memory,
            std::array<block, 2>& tweaks,
            ArrayView<GarbledGate<2>>  garbledGateIter
        );

        Role mRole;
        u64 mPartyIdx;

        u64 mBytesSent;
        block mZeroAndGlobalOffset[2];
        block mGlobalOffset;
        AES mAes;
        PRNG mPrng;
        u64 mInputIdx;
        Channel* mChannel;

        IknpOtExtReceiver mOtExtRecver;
        IknpOtExtSender mOtExtSender;

        std::vector<block> sharedMem;
        std::vector<GarbledGate<2>> sharedGates;
        ByteStream sharedBuff;
        std::array<block, 2>mTweaks;

        struct CircuitItem
        {
            BetaCircuit* mCircuit;
            std::vector<std::shared_ptr<ShGcLabelVec>> mLabels;
            u64 mInputBundleCount;
        };

        struct InputItem
        {
            BitVector mInputVal;
            std::shared_ptr<ShGcLabelVec> mLabels;
        };

        struct OutputItem
        {
            std::promise<BitVector>* mOutputVal;
            std::shared_ptr<ShGcLabelVec> mLabels;
        };

        void process();

        void garblerOutput();
        void garblerCircuit();
        void garblerInput();

        void evaluatorInput();
        void evaluatorCircuit();
        void evaluatorOutput();

        BitVector mOtChoices;
        u64 mMaxQueueSize;
        u64 mOtCount;
        std::queue<CircuitItem> mCrtQueue;
        std::queue<InputItem> mInputQueue;
        std::queue<OutputItem> mOutputQueue;

        //std::queue<CircuitItem> mWorkQueue;

        //boost::lockfree::spsc_queue<CircuitItem*> mWorkQueue;
    };


}