#pragma once
#include "BetaCircuit.h"
#include "Common/Defines.h"
#include <unordered_map>

namespace osuCrypto
{
    class CircuitLibrary
    {
    public:
        CircuitLibrary();
        ~CircuitLibrary();


        std::unordered_map<std::string, BetaCircuit*> mCirMap;

        BetaCircuit* int_int_add(u64 aSize, u64 bSize, u64 cSize);
        BetaCircuit* int_int_mult(u64 aSize, u64 bSize, u64 cSize);


        void int_int_add_built(
            BetaCircuit& cd,
            BetaBundle & a1,
            BetaBundle & a2,
            BetaBundle & sum,
            BetaBundle & temps);

        void int_int_mult_build(
            BetaCircuit& cd,
            BetaBundle & a1,
            BetaBundle & a2,
            BetaBundle & prod);

        //u64 aSize, u64 bSize, u64 cSize);

    };

}