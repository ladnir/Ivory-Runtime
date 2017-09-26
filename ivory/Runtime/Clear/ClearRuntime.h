#pragma once
#include "ivory/Runtime/Runtime.h"
#include "cryptoTools/Network/Channel.h"
#include "cryptoTools/Common/BitVector.h"

namespace osuCrypto
{
    struct ClearIntRuntimeData //: public RuntimeData
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

        void scheduleInput(ClearIntRuntimeData* data, const BitVector& value)override;
        void scheduleInput(ClearIntRuntimeData* data, u64 pIdx)override;


        void scheduleOp(Op op, ArrayView<RuntimeData*> io)override;


        void scheduleOutput(RuntimeData* labels, u64 partyIdx)override;
        void scheduleOutput(RuntimeData* labels, std::future<BitVector>& future)override;
        
        void processesQueue() override {}

        u64 getPartyIdx() override { return mPartyIdx; }

        Channel* mChannel;
        u64 mPartyIdx;


    };

}
