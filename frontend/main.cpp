#include "Network/BtIOService.h"
#include "Network/BtEndpoint.h"

#include <fstream>
#include <iostream>
#include "Common/Log.h"
#include "Common/Timer.h"
#include "Runtime/ShGcRuntime.h"
#include "Runtime/ClearRuntime.h"
#include "Runtime/sInt.h"
#include "Runtime/Party.h"

#include <string>
#include "Crypto/PRNG.h"

using namespace osuCrypto;


u32 program(RemoteParty& them, LocalParty& me, u64 val)
{
    u64 bitCount = 32;

    auto input0 = me.getIdx() == 0 ? me.input<sInt>(val, bitCount) : them.input<sInt>(bitCount);
    auto input1 = me.getIdx() == 1 ? me.input<sInt>(val, bitCount) : them.input<sInt>(bitCount);


    auto out = input1 * input0;


    if (me.getIdx() == 0)
    {
        me.reveal(out);
        them.reveal(out);
    }
    else
    {
        them.reveal(out);
        me.reveal(out);
    }

    return out.getValue();
}

int main(char* argv, int argc)
{
    PRNG prng(OneBlock);

    //sInt<32>::staticInit();

    BtIOService ios(0);


    std::thread thrd([&]() {


        Log::setThreadName("party1");



        BtEndpoint ep1(ios, "127.0.0.1:1212", false, "n");
        Channel& chl1 = ep1.addChannel("n");

        PRNG prng(ZeroBlock);

        ShGcRuntime rt1;
        rt1.init(chl1, prng.get<block>(), ShGcRuntime::Evaluator, 1);

        //ClearRuntime rt1;
        //rt1.init(chl1, prng.get<block>(), 1);



        RemoteParty them(rt1, 0);
        LocalParty me(rt1, 1);

        program(them, me, 44);

        chl1.close();
        ep1.stop();

    });

    Log::setThreadName("party0");


    BtEndpoint ep0(ios, "127.0.0.1:1212", true, "n");
    Channel& chl0 = ep0.addChannel("n");


    ShGcRuntime rt0;
    rt0.init(chl0, prng.get<block>(), ShGcRuntime::Garbler, 0);

    //ClearRuntime rt0;
    //rt0.init(chl0, prng.get<block>(), 0);


    LocalParty me(rt0, 0);
    RemoteParty them(rt0, 1);

    auto in0 = program(them, me, 23);

    Log::out << Log::lock <<"in0 "<< in0 << Log::endl << Log::unlock;


    thrd.join();
    chl0.close();
    ep0.stop();
    ios.stop();
    return 0;
}

