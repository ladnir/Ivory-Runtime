#pragma once
#include "Common/Defines.h"
#include "Common/ArrayView.h"

namespace osuCrypto
{

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

        virtual void scheduleInput(ArrayView<block> enc, u64 pIdx, BitVector& value) = 0;
        virtual void scheduleInput(ArrayView<block> enc, u64 pIdx) = 0;


        virtual void scheduleOp(Op op, ArrayView<ArrayView<block>> io) = 0;


        virtual void scheduleOutput(ArrayView<block> labels, u64 partyIdx) = 0;
        virtual void scheduleOutput(ArrayView<block> labels, BitVector& value) = 0;


        //virtual LocalParty getLocalParty() = 0;
        //virtual RemoteParty getRemoteParty(u64 pIdx = 0) = 0;
    };

}
