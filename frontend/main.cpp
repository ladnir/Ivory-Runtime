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
    u64 bitCount = 32;

    auto cir = lib.int_int_mult(bitCount, bitCount, bitCount);

    for (i32 a = 0; a < 100; ++a)
    {
        for (i32 b = 0; b < 100; ++b)
        {
            std::vector<BitVector> input(2);
            std::vector<BitVector> output(1);

            input[0].append((u8*)&a, bitCount);
            input[1].append((u8*)&b, bitCount);

            output[0] = BitVector(bitCount);

            cir->evaluate(input, output);

            output[0].reserve(32);
            i32 c = *(u32*)output[0].data();

            if (c != ((a * b) & (1 << bitCount)))
            {
                std::cout  << "bad " << c << "  " << (a*b) << "  " << a << " " << b << std::endl;
                return;
            }
        }
    }
}


i32 program(std::array<Party, 2> parties, i64 myInput)
{
    // choose how large the arithmetic should be.
    u64 bitCount0 = 16;
    u64 bitCount1 = 32;

    // get the two input variables. If this party is
    // the local party, then lets use our input value.
    // Otherwise the remote party will provide the value.
    auto input0 = parties[0].isLocalParty() ?
        parties[0].input<sInt>(myInput, bitCount0) :
        parties[0].input<sInt>(bitCount0);

    auto input1 = parties[1].isLocalParty() ?
        parties[1].input<sInt>(myInput, bitCount1) :
        parties[1].input<sInt>(bitCount1);


    // perform some computation
    auto out = input1 + input0;


    // reveal this output to party 0 and then party 1.
    parties[0].reveal(out);
    parties[1].reveal(out);



    i32 result = out.getValue();

    std::cout  << result << std::endl;


    // Get the value what was just revealed to us.
    return result;
}

int main(int argc, char**argv)
{
    //multTest();
    //return 0;

    PRNG prng(OneBlock);



    BtIOService ios(0);


    std::thread thrd([&]() {


        setThreadName("party1");



        BtEndpoint ep1(ios, "127.0.0.1:1212", false, "n");
        Channel& chl1 = ep1.addChannel("n");

        PRNG prng(ZeroBlock);

        ShGcRuntime rt1;
        rt1.init(chl1, prng.get<block>(), ShGcRuntime::Evaluator, 1);


        std::array<Party, 2> parties {
            Party(rt1, 0),
            Party(rt1, 1)
        };

        program(parties, 44);

        chl1.close();
        ep1.stop();

    });

    setThreadName("party0");



    BtEndpoint ep0(ios, "127.0.0.1:1212", true, "n");
    Channel& chl0 = ep0.addChannel("n");


    ShGcRuntime rt0;
    rt0.init(chl0, prng.get<block>(), ShGcRuntime::Garbler, 0);


    std::array<Party, 2> parties{
        Party(rt0, 0),
        Party(rt0, 1)
    };

    program(parties, 23);



    thrd.join();
    chl0.close();
    ep0.stop();
    ios.stop();
    return 0;
}

