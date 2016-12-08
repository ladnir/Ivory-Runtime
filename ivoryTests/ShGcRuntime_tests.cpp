#include "ShGcRuntime_tests.h"
#include "Runtime/Party.h"
#include "Runtime/sInt.h"
#include <functional>
#include "Network/BtIOService.h"
#include "Network/BtEndpoint.h"
#include  "Common/Log.h"

#include "Common.h"

using namespace osuCrypto;

void runProgram(std::function<void(Runtime&)>  program)
{
    PRNG prng(OneBlock);

    BtIOService ios(0);


    std::thread thrd([&]() {
        setThreadName("party1");

        BtEndpoint ep1(ios, "127.0.0.1:1212", false, "n");
        Channel& chl1 = ep1.addChannel("n");
        PRNG prng(ZeroBlock);

        ShGcRuntime rt1;
        rt1.init(chl1, prng.get<block>(), ShGcRuntime::Evaluator, 1);


        program(rt1);


        chl1.close();
        ep1.stop();

    });

    setThreadName("party0");
    BtEndpoint ep0(ios, "127.0.0.1:1212", true, "n");
    Channel& chl0 = ep0.addChannel("n");
    ShGcRuntime rt0;
    rt0.init(chl0, prng.get<block>(), ShGcRuntime::Garbler, 0);

    program(rt0);


    thrd.join();
    chl0.close();
    ep0.stop();
    ios.stop();
}


void ShGcRuntime_basicArith_Test()
{
    i32 addResult = 0;
    i32 subResult = 0;
    i32 mulResult = 0;
    i32 divResult = 0;
    i32 gteResult = 0;
    i32 lstResult = 0;
    i32 maxresult = 0;

    i64 inputVal0 = 254324;
    i64 inputVal1 = -5323;

    auto program = [&](Runtime& rt)
    {


        std::array<Party, 2> parties{
            Party(rt, 0),
            Party(rt, 1)
        };

        // choose how large the arithmetic should be.
        u64 bitCount0 = 32;
        u64 bitCount1 = 32;

        // get the two input variables. If this party is
        // the local party, then lets use our input value.
        // Otherwise the remote party will provide the value.
        auto input0 = parties[0].isLocalParty() ?
            parties[0].input<sInt>(inputVal0, bitCount0) :
            parties[0].input<sInt>(bitCount0);

        auto input1 = parties[1].isLocalParty() ?
            parties[1].input<sInt>(inputVal1, bitCount1) :
            parties[1].input<sInt>(bitCount1);


        // perform some computation
        auto add = input1 + input0;
        auto sub = input1 - input0;
        auto mul = input1 * input0;
        auto div = input1 / input0;


        auto gteq = input1 >= input0;
        auto lt = input1 < input0;


        auto max = gteq.ifelse(input1, input0);


        // reveal this output to party 0 and then party 1.
        parties[0].reveal(add);
        parties[0].reveal(sub);
        parties[0].reveal(mul);
        parties[0].reveal(div);
        parties[1].reveal(gteq);
        parties[1].reveal(lt);
        parties[1].reveal(max);


        if (parties[0].isLocalParty())
        {
            addResult = add.getValue();
            subResult = sub.getValue();
            mulResult = mul.getValue();
            divResult = div.getValue();
        }else{
            gteResult = gteq.getValue();
            lstResult = lt.getValue();
            maxresult =  max.getValue() ;
        }

        parties[0].getRuntime().processesQueue();
    };
    runProgram(program);


    if (addResult != inputVal1 + inputVal0) throw UnitTestFail();
    if (subResult != inputVal1 - inputVal0) throw UnitTestFail();
    if (mulResult != inputVal1 * inputVal0) throw UnitTestFail();
    if (divResult != inputVal1 / inputVal0) throw UnitTestFail();
    if (gteResult != inputVal1 >=inputVal0) throw UnitTestFail();
    if (lstResult != inputVal1 < inputVal0) throw UnitTestFail();
    if (maxresult != std::max(inputVal0, inputVal1)) throw UnitTestFail();

}

void ShGcRuntime_SequentialOp_Test()
{
    i32 addResult = 0;
    i32 subResult = 0;
    i32 mulResult = 0;
    i32 divResult = 0;
    i32 gteResult = 0;
    i32 lstResult = 0;
    i32 maxresult = 0;

    i64 inputVal0 = 254324;
    i64 inputVal1 = -5323;

    auto program = [&](Runtime& rt)
    {


        std::array<Party, 2> parties{
            Party(rt, 0),
            Party(rt, 1)
        };

        // choose how large the arithmetic should be.
        u64 bitCount0 = 32;
        u64 bitCount1 = 32;

        // get the two input variables. If this party is
        // the local party, then lets use our input value.
        // Otherwise the remote party will provide the value.
        auto input0 = parties[0].isLocalParty() ?
            parties[0].input<sInt>(inputVal0, bitCount0) :
            parties[0].input<sInt>(bitCount0);

        auto input1 = parties[1].isLocalParty() ?
            parties[1].input<sInt>(inputVal1, bitCount1) :
            parties[1].input<sInt>(bitCount1);


        auto add = ~input1 + input0;

        // reveal this output to party 0 and then party 1.
        parties[0].reveal(add);


        if (parties[0].isLocalParty())
        {
            addResult = add.getValue();
        }

        parties[0].getRuntime().processesQueue();
    };
    runProgram(program);


    if (addResult != ~inputVal1 + inputVal0)
    {
        std::cout << "act " << addResult << std::endl;
        std::cout << "exp " << (~inputVal1 + inputVal0) << std::endl;
        std::cout << "oth " << (inputVal1 + inputVal0) << std::endl;
        throw UnitTestFail();
    }
    //if (maxresult != std::max(inputVal0, inputVal1)) throw UnitTestFail();


}
