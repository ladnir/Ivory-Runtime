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

    auto input0 = me.getIdx() == 0 ? me.input<CrtInt32>(534) : them.input<CrtInt32>();
    auto input1 = me.getIdx() == 1 ? me.input<CrtInt32>(534) : them.input<CrtInt32>();


    for (u64 i = 0; i < repeatCount; ++i)
        input1 += input0;


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


    //auto input0 = me.input<CrtInt32>(1245);
    //auto input1 = them.input<CrtInt32>();

    Timer t;
    t.setTimePoint("start");
    auto in0 = program(them, me);

    //for (u64 i = 0; i < repeatCount; ++i)
    //    input1 += input0;
    t.setTimePoint("end");


    Log::out << t << Log::endl;


    //auto in0 = me.reveal(input0);
    //auto in1 = me.reveal(input1);
    //auto val = me.reveal(output);

    Log::out << rt0.mBytesSent << Log::endl;
    Log::out << Log::lock <<"in0 "<< in0 << Log::endl << Log::unlock;
    //Log::out << Log::lock <<"in1 "<< in1 << Log::endl << Log::unlock;
    //Log::out << Log::lock <<"val "<< val << Log::endl << Log::unlock;



    thrd.join();
    chl0.close();
    ep0.stop();
    ios.stop();
    return 0;
}


//std::ifstream in;
//in.open("C:\\Users\\peter\\source\\repo\\TinyGarble\\scd\\netlists\\aes_11cc.v", in.in);
//in.open("C:\\Users\\peter\\source\\repo\\TinyGarble\\scd\\netlists\\mult_8bit_8cc.v", in.in);
//in.open("C:\\Users\\peter\\Documents\\rindalp\\hamming_netlist.v", in.in);

//if (in.good() == false || in.is_open() == false)
//{
//	std::cout << "failed to open input file" << std::endl;
//	return 1;
//}

//VerilogParser scp;

//scp.init(in);

//SequentialCircuit cir;

//scp.parse(cir);
//BitVector input(cir.inputCount()), output;
//
//Log::out << "input count " << cir.inputCount() << Log::endl;
//
//
//input[0] = 1;
//input[1] = 1;
//input[8] = 1;
//cir.evaluate(input, output);
//
//
