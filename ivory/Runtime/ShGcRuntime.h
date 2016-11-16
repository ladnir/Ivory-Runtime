#pragma once
#include "Runtime/Runtime.h"
#include "Circuit/Circuit.h"
#include "Circuit/BetaCircuit.h"
#include "Common/ArrayView.h"
//#include "boost/lockfree/spsc_queue.hpp"
#include <queue>
#include "OT/TwoChooseOne/IknpOtExtSender.h"
#include "Common/ByteStream.h"
#include "OT/TwoChooseOne/IknpOtExtReceiver.h"
#include "Circuit/CircuitLibrary.h"


namespace osuCrypto
{


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

        void scheduleInput(ArrayView<block> enc, u64 pIdx, BitVector& value)override;
        void scheduleInput(ArrayView<block> enc, u64 pIdx)override;
       
       
        void scheduleOp(Op op, ArrayView<ArrayView<block>> io)override;
       
       
        void scheduleOutput(ArrayView<block> labels, u64 partyIdx)override;
        void scheduleOutput(ArrayView<block> labels, BitVector& value)override;
       
       
        CircuitLibrary mLibrary;
        //LocalParty getLocalParty()override;
        //RemoteParty getRemoteParty(u64 pIdx = 0)override;


        //void scheduleCrt(
        //    BetaCircuit & cir,
        //    ArrayView<block> in1,
        //    ArrayView<block> in2,
        //    ArrayView<block> out);




        //void scheduleInput(
        //    ArrayView<block> labels,
        //    BitVector& value, 
        //    u64 partyIdx);

        //void scheduleInput(
        //    ArrayView<block> labels,
        //    u64 partyIdx);


        //void scheduleOutput(
        //    ArrayView<block> labels,
        //    BitVector& future);

        //void scheduleOutput(
        //    ArrayView<block> labels,
        //    u64 partyIdx);







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
        std::array<block,2>mTweaks;

        struct CircuitItem
        {
            BetaCircuit* mCircuit;
            std::vector<ArrayView<block>> mLabels;
            //BitVector mInputVal;
            u64 mInputBundleCount;
        };

        struct InputItem
        {
            BitVector mInputVal;
            ArrayView<block> mLabels;
        };

        struct OutputItem
        {
            BitVector* mOutputVal;
            //std::promise<BitVector>* mProm;
            ArrayView<block> mLabels;
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