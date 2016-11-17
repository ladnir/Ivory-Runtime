#pragma once
#include "Runtime/Runtime.h"
#include "Network/Channel.h"
#include "Common/BitVector.h"

namespace osuCrypto
{
    struct ClearIntRuntimeData : public RuntimeData
    {

        ClearIntRuntimeData(u64 bitCount)
            :mVal(0),
            mBitCount(bitCount)
        {}

        i64 mVal;
        u64 mBitCount;
    };

    class ClearRuntime : public Runtime
    {
    public:
        ClearRuntime();
        ~ClearRuntime();

        void initVar(std::unique_ptr<RuntimeData>& data, u64 bitCount) override;
        void copyVar(std::unique_ptr<RuntimeData>& data, RuntimeData* copy) override;


        void init(Channel& chl, block seed, u64 partyIdx);

        void scheduleInput(RuntimeData* data, u64 pIdx, BitVector& value)override;
        void scheduleInput(RuntimeData* data, u64 pIdx)override;


        void scheduleOp(Op op, ArrayView<RuntimeData*> io)override;


        void scheduleOutput(RuntimeData* labels, u64 partyIdx)override;
        void scheduleOutput(RuntimeData* labels, std::future<BitVector>& future)override;


        Channel* mChannel;
        u64 mPartyIdx;


    };

}
