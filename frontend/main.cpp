#include "Network/BtIOService.h"
#include "Network/BtEndpoint.h"



#include "Circuit/Parser/VerilogTokenizer.h"
#include "Circuit/Parser/VerilogParser.h"
#include <fstream>
#include <iostream>
#include "Common/Log.h"
#include "Common/Timer.h"
#include "Runtime/CrtRuntime.h"
#include "Runtime/CrtInt.h"
#include "Runtime/CrtInput.h"

#include <string>
#include "Crypto/PRNG.h"

using namespace osuCrypto;


u32 program(CrtRemoteParty& them, CrtLocalParty& me)
{
    u64 repeatCount = 100;

    auto input0 = me.getIdx() == 0 ? me.input<CrtInt32>(500) : them.input<CrtInt32>();
    auto input1 = me.getIdx() == 1 ? me.input<CrtInt32>(16) : them.input<CrtInt32>();


    //for (u64 i = 0; i < repeatCount; ++i)
    //    input1 = input0 + input1;

    input1 = input1 *  input0;


    u32 ret = 0;
    if (me.getIdx() == 0)
    {
        ret = me.reveal(input1);
        them.reveal(input1);
    }
    else
    {
        them.reveal(input1);
        ret = me.reveal(input1);
    }

    return ret;
}

int main(char* argv, int argc)
{
    PRNG prng(OneBlock);

    CrtInt32::staticInit();

    BtIOService ios(0);


    std::thread thrd([&]() {


        Log::setThreadName("party1");

        CrtRuntime rt1;
        BtEndpoint ep1(ios, "127.0.0.1:1212", false, "n");
        Channel& chl1 = ep1.addChannel("n");

        PRNG prng(ZeroBlock);

        rt1.init(chl1, prng.get<block>(), CrtRuntime::Evaluator, 1);

        CrtRemoteParty them(rt1, 0);
        CrtLocalParty me(rt1, 1);

        program(them, me);

        chl1.close();
        ep1.stop();

    });

    Log::setThreadName("party0");

    CrtRuntime rt0;
    BtEndpoint ep0(ios, "127.0.0.1:1212", true, "n");
    Channel& chl0 = ep0.addChannel("n");


    rt0.init(chl0, prng.get<block>(), CrtRuntime::Garbler, 0);

    CrtLocalParty me(rt0, 0);
    CrtRemoteParty them(rt0, 1);

    auto in0 = program(them, me);

    Log::out << Log::lock <<"in0 "<< in0 << Log::endl << Log::unlock;


    thrd.join();
    chl0.close();
    ep0.stop();
    ios.stop();
    return 0;
}

