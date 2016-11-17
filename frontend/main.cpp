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


void multTest()
{
    CircuitLibrary lib;


    auto cir = lib.int_int_mult(32, 32, 32);

    for (i32 a = 0; a < 100; ++a)
    {
        for (i32 b = 0; b < 100; ++b)
        {
            std::vector<BitVector> input(2);
            std::vector<BitVector> output(1);

            input[0].append((u8*)&a, 32);
            input[1].append((u8*)&b, 32);

            output[0] = BitVector(32);

            cir->evaluate(input, output);

            i32 c = *(u32*)output[0].data();

            if (c != a * b)
            {
                Log::out << "bad " << c << "  " << (a*b) << "  " << a << " " << b << Log::endl;
                return;
            }
        }
    }
}


i32 program(RemoteParty& them, LocalParty& me, i64 val)
{
    Log::out << "start " << me.getIdx() << Log::endl;
    u64 bitCount = 32;

    auto input0 = me.getIdx() == 0 ? me.input<sInt>(val, bitCount) : them.input<sInt>(bitCount);
    auto input1 = me.getIdx() == 1 ? me.input<sInt>(val, bitCount) : them.input<sInt>(bitCount);

    Log::out << "input " << me.getIdx() << Log::endl;

    //auto test =  


    auto out = input1 + input0;

    Log::out << "comp " << me.getIdx() << Log::endl;

    if (me.getIdx() == 0)
    {
        me.reveal(input0);
        me.reveal(input1);

        Log::out << "input0 " << input0.getValue() << Log::endl;
        Log::out << "input1 " << input1.getValue() << Log::endl;

        me.reveal(out);
        them.reveal(out);
    }
    else
    {
        them.reveal(input0);
        them.reveal(input1);

        them.reveal(out);
        me.reveal(out);
    }


    Log::out << "reveal " << me.getIdx() << Log::endl;

    return (i32) out.getValue();
}

int main(int argc, char**argv)
{

    //multTest();
    //return 0;
    PRNG prng(OneBlock);



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

    program(them, me, 23);



    thrd.join();
    chl0.close();
    ep0.stop();
    ios.stop();
    return 0;
}

