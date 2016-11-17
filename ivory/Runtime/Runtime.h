#pragma once
#include "Common/Defines.h"
#include "Common/ArrayView.h"
#include <future>
#include <memory>

namespace osuCrypto
{

    struct RuntimeData
    {

    };

    enum class Op
    {
        Add,
        Subtract,
        Multiply,
        Divide,
        Mod,
        And,
        Or,
        Not,
        BitWiseAnd,
        BitWiseOr
    };

    class Runtime
    {
    public:
        Runtime();
        ~Runtime();

        virtual void initVar(std::unique_ptr<RuntimeData>& data, u64 bitCount) = 0;
        virtual void copyVar(std::unique_ptr<RuntimeData>& data, RuntimeData* copy) = 0;

        virtual void scheduleInput(RuntimeData* data, u64 pIdx, const BitVector& value) = 0;
        virtual void scheduleInput(RuntimeData* data, u64 pIdx) = 0;


        virtual void scheduleOp(Op op, ArrayView<RuntimeData*> io) = 0;


        virtual void scheduleOutput(RuntimeData* data, u64 partyIdx) = 0;
        virtual void scheduleOutput(RuntimeData* data, std::future<BitVector>& future) = 0;


        //virtual LocalParty getLocalParty() = 0;
        //virtual RemoteParty getRemoteParty(u64 pIdx = 0) = 0;
    };

}
