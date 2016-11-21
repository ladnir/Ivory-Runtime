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
        BitwiseAnd,
        BitWiseOr
    };

    class Runtime
    {
    public:
        Runtime();
        ~Runtime();


        // initVar(...) should be used to initialize a new RuntimeData object
        //    which has a symatic bitCount number of bits. 
        // Assumption: data = nullptr
        // Result: data = derived type of RuntimeData for this runtime. This call may be 
        //    asynchronous but will be performed before the next call to processesQueue() returns.
        virtual void initVar(std::unique_ptr<RuntimeData>& data, u64 bitCount) = 0;

        // copyVar(...) should be used to initialize a new RuntimeData object
        //    which is a copy of another RuntimeData. E.g. for GC based runtimes,  
        //    copy the wire labels.
        // Assumption: copy != nullptr
        // Result: data = derived type of RuntimeData for this runtime and has 
        //    the same value as copy. This call may be asynchronous but will be performed
        //    before the next call to processesQueue() returns.
        virtual void copyVar(std::unique_ptr<RuntimeData>& data, RuntimeData* copy) = 0;

        // This scheduleInput(...) should be used to assign a variable a value. 
        //     This will be called in the case that the variable is known to the local party
        // Assumption: copy != nullptr, value.size() == data.size()
        // Result: value will be internally record and eventually the ecrypted version
        //     of value will be placed inside the data variable. This encryption operation 
        //     may be asynchronous but it will be performed before the next call to processesQueue() returns.
        virtual void scheduleInput(RuntimeData* data, const BitVector& value) = 0;

        // This scheduleInput(...) should be used to assign a variable  a value
        //    known to party[pIdx].  The encrypted version/meta data of the value will be 
        //    stored in data.
        // Assumptions: data != nullptr, pIdx = remote party index.
        // result: At some point, data will hold the encrypted version/meta data of this 
        //    varaible. This operation may be asychronously, but will be completed before the next 
        //    call to processesQueue() returns.
        virtual void scheduleInput(RuntimeData* data, u64 pIdx) = 0;

        // scheduleOp(...) should be called to schedule a new operation that should be performed on the 
        // data provided by the io parameter. e.g. c = a + b  where op = add, io = {a,b,c}.
        // Assumptions: io contains the correct number of variables to perform the operation.
        //    Each value in io has been innitialized.
        // Result: The output variable(s) of io will be assigned the [encrypted] value of the operation.
        //    The input variables of io should remain unchanged. This operation may be performed 
        //    asynchronously in that it may be performed at some later time but before the next 
        //    call to processesQueue() returns.
        virtual void scheduleOp(Op op, ArrayView<RuntimeData*> io) = 0;

        // This scheduleOutput(...) marks the current state of data as being ready to 
        //    be revealed to party[pIdx]. 
        // Assumptions: data != nullptr, pIdx = remote party index
        // Result: The current state of data will be revealed to pIdx. This operation may be
        //     asynchronous but will be completed before the next call to processesQueue() returns.
        virtual void scheduleOutput(RuntimeData* data, u64 pIdx) = 0;

        // This scheduleOutput(...) marks the current state of data as being ready to 
        //    be revealed to the local party.
        // Assumptions: data != nullptr
        // Result: The current state of data will be revealed to pIdx. This operation may be
        //     asynchronous but will be completed before the next call to processesQueue() returns.
        virtual void scheduleOutput(RuntimeData* data, std::future<BitVector>& future) = 0;

        // processesQueue() will ensure that all scheduled operations have been completed 
        //    before returning. This should be called before any the future from 
        //    scheduleOutput(...) is received.
        // Assumptions: None
        // Result: Upon return, all operations are either in process or completed.
        virtual void processesQueue() = 0;


        // getPartyIdx() returns the index of the local party.
        virtual u64 getPartyIdx() = 0;
    };

}
