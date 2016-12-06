#include "Circuit_Tests.h"

#include "Circuit/CircuitLibrary.h"

#include <fstream>
#include "Common.h"
#include "Common/Log.h"
#include "Common/BitVector.h"
#include "Crypto/AES.h"
#include "Crypto/PRNG.h"
#include "DebugCircuits.h"

using namespace osuCrypto;


#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
i64 signExtend(i64 v, u64 b)
{
    bool sign = v & (1 << (b - 1));

    i64 mask = sign ? i64(-1) << b : 0;

    return v | mask;

}

void Circuit_Adder_Test()
{
    setThreadName("CP_Test_Thread");


    CircuitLibrary lib;


    PRNG prng(ZeroBlock);
    u64 tries = 1000;


    for (u64 i = 0; i < tries; ++i)
    {

        u64 aSize = prng.get<u32>() % 64 + 1,
            bSize = prng.get<u32>() % 64 + 1,
            cSize = std::min<u64>(prng.get<u32>() % 64 + 1, std::max(aSize, bSize));

        auto* cir = lib.int_int_add(aSize, bSize, cSize);
        i64 aMask = (i64(1) << aSize) - 1;
        i64 bMask = (i64(1) << bSize) - 1;
        i64 cMask = (i64(1) << cSize) - 1;


        i64 a = prng.get<i64>() & aMask;
        i64 b = prng.get<i64>() & bMask;
        i64 c = signExtend((a + b) & cMask, cSize);


        std::vector<BitVector> inputs(2), output(1);
        inputs[0].append((u8*)&a, aSize);
        inputs[1].append((u8*)&b, bSize);
        output[0].resize(cSize);

        cir->evaluate(inputs, output);

        i64 cc = 0;
        memcpy(&cc, output[0].data(), output[0].sizeBytes());

        cc =  signExtend(cc, cSize);

        if (cc != c)
        {
            BitVector cExp;
            cExp.append((u8*)&c, cSize);
            std::cout << "a  : " << inputs[0] << std::endl;
            std::cout << "b  : " << inputs[1] << std::endl;
            std::cout << "exp: " << cExp << std::endl;
            std::cout << "act: " << output[0] << std::endl;

            throw UnitTestFail();
        }

    }
}



void Circuit_Subtractor_Test()
{
    setThreadName("CP_Test_Thread");


    CircuitLibrary lib;


    PRNG prng(ZeroBlock);
    u64 tries = 1000;


    for (u64 i = 0; i < tries; ++i)
    {

        u64 aSize = prng.get<u32>() % 64 + 1,
            bSize = prng.get<u32>() % 64 + 1,
            cSize = std::min<u64>(prng.get<u32>() % 64 + 1, std::max(aSize, bSize));

        auto* cir = lib.int_int_subtract(aSize, bSize, cSize);
        i64 aMask = (i64(1) << aSize) - 1;
        i64 bMask = (i64(1) << bSize) - 1;
        i64 cMask = (i64(1) << cSize) - 1;


        i64 a = prng.get<i64>() & aMask;
        i64 b = prng.get<i64>() & bMask;
        i64 c = signExtend((a - b) & cMask, cSize);


        std::vector<BitVector> inputs(2), output(1);
        inputs[0].append((u8*)&a, aSize);
        inputs[1].append((u8*)&b, bSize);
        output[0].resize(cSize);

        cir->evaluate(inputs, output);

        i64 cc = 0;
        memcpy(&cc, output[0].data(), output[0].sizeBytes());
        cc = signExtend(cc, cSize);

        if (cc != c)
        {
            BitVector cExp;
            cExp.append((u8*)&c, cSize);
            std::cout << "a  : " << inputs[0] << std::endl;
            std::cout << "b  : " << inputs[1] << std::endl;
            std::cout << "exp: " << cExp << std::endl;
            std::cout << "act: " << output[0] << std::endl;

            throw UnitTestFail();
        }

    }
}

