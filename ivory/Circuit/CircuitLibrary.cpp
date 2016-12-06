#include "CircuitLibrary.h"


namespace osuCrypto
{
    CircuitLibrary::CircuitLibrary()
    {
    }


    CircuitLibrary::~CircuitLibrary()
    {
    }

    BetaCircuit * osuCrypto::CircuitLibrary::int_int_add(u64 aSize, u64 bSize, u64 cSize)
    {
        auto key = "add" + ToString(aSize) + "x" + ToString(bSize) + "x" + ToString(cSize);

        auto iter = mCirMap.find(key);

        if (iter == mCirMap.end())
        {
            auto* cd = new BetaCircuit;

            BetaBundle a(aSize);
            BetaBundle b(bSize);
            BetaBundle c(cSize);
            BetaBundle t(3);

            cd->addInputBundle(a);
            cd->addInputBundle(b);

            cd->addOutputBundle(c);

            cd->addTempWireBundle(t);

            int_int_add_built(*cd, a, b, c, t);

            iter = mCirMap.insert(std::make_pair(key, cd)).first;
        }

        return iter->second;
    }

    BetaCircuit * CircuitLibrary::int_int_subtract(u64 aSize, u64 bSize, u64 cSize)
    {
        auto key = "subtract" + ToString(aSize) + "x" + ToString(bSize) + "x" + ToString(cSize);

        auto iter = mCirMap.find(key);

        if (iter == mCirMap.end())
        {
            auto* cd = new BetaCircuit;

            BetaBundle a(aSize);
            BetaBundle b(bSize);
            BetaBundle c(cSize);
            BetaBundle t(3);

            cd->addInputBundle(a);
            cd->addInputBundle(b);

            cd->addOutputBundle(c);

            cd->addTempWireBundle(t);

            int_int_subtract_built(*cd, a, b, c, t);

            iter = mCirMap.insert(std::make_pair(key, cd)).first;
        }

        return iter->second;
    }

    BetaCircuit * CircuitLibrary::int_int_mult(u64 aSize, u64 bSize, u64 cSize)
    {
        auto key = "mult" + ToString(aSize) + "x" + ToString(bSize) + "x" + ToString(cSize);

        auto iter = mCirMap.find(key);

        if (iter == mCirMap.end())
        {
            auto* cd = new BetaCircuit;

            BetaBundle a(aSize);
            BetaBundle b(bSize);
            BetaBundle c(cSize);

            cd->addInputBundle(a);
            cd->addInputBundle(b);

            cd->addOutputBundle(c);

            int_int_mult_build(*cd, a, b, c);

            iter = mCirMap.insert(std::make_pair(key, cd)).first;
        }

        return iter->second;

    }

    BetaCircuit * CircuitLibrary::int_int_bitwiseAnd(u64 aSize, u64 bSize, u64 cSize)
    {
        auto key = "bitwiseAnd" + ToString(aSize) + "x" + ToString(bSize) + "x" + ToString(cSize);

        auto iter = mCirMap.find(key);

        if (iter == mCirMap.end())
        {
            auto* cd = new BetaCircuit;

            BetaBundle a(aSize);
            BetaBundle b(bSize);
            BetaBundle c(cSize);

            cd->addInputBundle(a);
            cd->addInputBundle(b);

            cd->addOutputBundle(c);

            int_int_bitwiseAnd_build(*cd, a, b, c);

            iter = mCirMap.insert(std::make_pair(key, cd)).first;
        }

        return iter->second;
    }


    void CircuitLibrary::int_int_add_built(
        BetaCircuit& cd,
        BetaBundle & a1,
        BetaBundle & a2,
        BetaBundle & sum,
        BetaBundle & temps)
    {

        if (temps.mWires.size() < 3)
            throw std::runtime_error(LOCATION);

        if (sum.mWires.size() > std::max<u64>(a1.mWires.size(), a2.mWires.size()))
            throw std::runtime_error(LOCATION);

        BetaWire& carry = temps.mWires[0];
        BetaWire& aXorC = temps.mWires[1];
        BetaWire& temp = temps.mWires[2];

        if (a1.mWires[0] == sum.mWires[0] ||
            a2.mWires[0] == sum.mWires[0])
            throw std::runtime_error("");

        u64 a1Size = a1.mWires.size();
        u64 a2Size = a2.mWires.size();
        u64 minSize = std::min<u64>(sum.mWires.size(), std::max<u64>(a1.mWires.size(), a2.mWires.size()));

        // sum is computed as a1[i] ^ a2[i] ^ carry[i-1]
        // carry[i] is computed as
        //
        //  carry[i-1] -*--*--------------------*
        //              |  |                    |
        //              |  >= xor ---*          >= xor --- carry[i]
        //              |  |         |          |
        //  a2[i] ------|--*          >= and ---*
        //              |            |
        //              >==== xor ---*
        //              |            
        //  a1[i] ------*


        // half adder
        cd.addGate(a1.mWires[0], a2.mWires[0], GateType::Xor, sum.mWires[0]);


        // now do the full adder while we have inputs from both a1,a2
        u64 i = 1;
        if (minSize > 1)
        { 

            // compute the carry from the 0 bits (special case)
            cd.addGate(a1.mWires[i - 1], a2.mWires[i - 1], GateType::And, carry);

            u64 a1Idx = std::min<u64>(i, a1Size - 1);
            u64 a2Idx = std::min<u64>(i, a2Size - 1);

            // compute the sum
            cd.addGate(a1.mWires[a1Idx], carry, GateType::Xor, aXorC);
            cd.addGate(a2.mWires[a2Idx], aXorC, GateType::Xor, sum.mWires[i]);

            // general case.
            for (i = 2; i < minSize; ++i)
            {
                // compute the previous carry
                cd.addGate(a2.mWires[a2Idx], carry, GateType::Xor, temp);
                cd.addGate(temp, aXorC, GateType::And, temp);
                cd.addGate(temp, carry, GateType::Xor, carry);

                a1Idx = std::min<u64>(i, a1Size - 1);
                a2Idx = std::min<u64>(i, a2Size - 1);

                cd.addGate(a1.mWires[a1Idx], carry, GateType::Xor, aXorC);
                cd.addGate(a2.mWires[a2Idx], aXorC, GateType::Xor, sum.mWires[i]);
            }
        }


        // now special case the situation that a1 and a2 are different sizes
        //auto& aa = a1.mWires.size() > a2.mWires.size() ? a1 : a2;
        //minSize = std::min<u64>(aa.mWires.size(), sum.mWires.size());

        //if (i < minSize)
        //{
        //    // compute the previous carry (special case)
        //    if (i == 1)
        //    {
        //        cd.addGate(a1.mWires[i - 1], a2.mWires[i - 1], GateType::And, carry);
        //    }
        //    else
        //    {
        //        cd.addGate(a2.mWires[i - 1], carry, GateType::Xor, temp);
        //        cd.addGate(temp, aXorC, GateType::And, temp);
        //        cd.addGate(temp, carry, GateType::Xor, carry);
        //    }


        //    cd.addGate(carry, aa.mWires[i], GateType::Xor, sum.mWires[i]);
        //    ++i;


        //    // compute the general case
        //    for (; i < minSize; ++i)
        //    {
        //        // compute the previous carry
        //        cd.addGate(carry, aa.mWires[i - 1], GateType::And, carry);

        //        cd.addGate(carry, aa.mWires[i], GateType::Xor, sum.mWires[i]);
        //    }
        //}
    }

    void CircuitLibrary::int_int_subtract_built(
        BetaCircuit & cd,
        BetaBundle & a1,
        BetaBundle & a2,
        BetaBundle & diff,
        BetaBundle & temps)
    {
        if (diff.mWires.size() > std::max<u64>(a1.mWires.size(), a2.mWires.size()))
            throw std::runtime_error(LOCATION);


        u64 a1Size = a1.mWires.size();
        u64 a2Size = a2.mWires.size();
        u64 minSize = std::min(std::max(a1.mWires.size(), a2.mWires.size()), diff.mWires.size());

        BetaWire borrow = temps.mWires[0];
        BetaWire aXorBorrow = temps.mWires[1];
        BetaWire temp = temps.mWires[2];
        std::vector<BetaWire>& d = diff.mWires;

        // we are computing a1 - a2 = diff
        // diff is computed as a1[i] ^ a2[i] ^ borrow[i-1]
        // borrow[i] is computed as
        //
        //  a1[i] ------*--*-------------------*
        //              |  |                   |
        //              |  >= xor ---*         >= xor --- borrow[i]
        //              |  |         |         |
        //  a2[i] ------|--*          >= or ---*
        //              |            |
        //              >==== xor ---*
        //              |  
        // borrow[i-1] -*

        u64 i = 0;
        if (minSize)
        {
            cd.addGate(a1.mWires[0], a2.mWires[0], GateType::Xor, diff.mWires[0]);
            ++i;

            if (minSize > 1)
            {
                cd.addGate(a1.mWires[0], a2.mWires[0], GateType::na_And, borrow);

                u64 a1Idx = std::min<u64>(1, a1Size - 1);
                u64 a2Idx = std::min<u64>(1, a2Size - 1);

                // second bit is the xor of borrow and input;
                cd.addGate(borrow, a1.mWires[a1Idx], GateType::Xor, aXorBorrow);
                cd.addGate(aXorBorrow, a2.mWires[a2Idx], GateType::Xor, d[1]);
                ++i;

                for (; i < minSize; ++i)
                {                             
                    // compute the borrow of the previous bit which itself has a borrow in.
                    cd.addGate(a1.mWires[a1Idx], a2.mWires[a2Idx], GateType::Xor, temp);
                    cd.addGate(aXorBorrow, temp, GateType::Or, temp);
                    cd.addGate(temp, a1.mWires[a1Idx], GateType::Xor, borrow);

                    a1Idx = std::min<u64>(i, a1Size - 1);
                    a2Idx = std::min<u64>(i, a2Size - 1);

                    // compute the difference as the xor of the input and prev borrow.
                    cd.addGate(borrow, a1.mWires[a1Idx], GateType::Xor, aXorBorrow);
                    cd.addGate(aXorBorrow, a2.mWires[a2Idx], GateType::Xor, d[i]);
                }
            }
        }

        // special case for when one of the inputs is longer than the other
        //u64 minSizeA1 = std::min(a1.mWires.size(), diff.mWires.size());
        //u64 minSizeA2 = std::min(a2.mWires.size(), diff.mWires.size());
        //if (minSizeA1 > i || minSizeA2 > i)
        //{

        //    // compute the borrow from the last normal subtraction bit index
        //    if (i == 1)
        //    {
        //        cd.addGate(a1.mWires[0], a2.mWires[0], GateType::na_And, borrow);
        //    }
        //    else
        //    {
        //        // compute the borrow of the previous bit which itself has a borrow in.
        //        cd.addGate(a1.mWires[i - 1], a2.mWires[i - 1], GateType::Xor, temp);
        //        cd.addGate(aXorBorrow, temp, GateType::Or, temp);
        //        cd.addGate(temp, a1.mWires[i - 1], GateType::Xor, borrow);
        //    }

        //    if (minSizeA1 > i)
        //    {

        //        cd.addGate(a1.mWires[i], borrow, GateType::Xor, d[i]);
        //        ++i;
        //    }

        //    //if (minSizeA1 > i)
        //    //{
        //    //    // a1 is longer than 

        //    //    cd.addGate(a1.mWires[i], borrow, GateType::Xor, d[i]);
        //    //    ++i;

        //    //    for (; i < minSizeA1; ++i)
        //    //    {
        //    //        cd.addGate(a1.mWires[i - 1], borrow, GateType::na_And, borrow);
        //    //        cd.addGate(a1.mWires[i], borrow, GateType::Xor, d[i]);
        //    //    }
        //    //}
        //    //else
        //    //{
        //    //    cd.addGate(a2.mWires[i], borrow, GateType::Xor, d[i]);
        //    //    ++i;

        //    //    for (; i < minSizeA2; ++i)
        //    //    {
        //    //        cd.addGate(a2.mWires[i - 1], borrow, GateType::Or, borrow);
        //    //        cd.addGate(a2.mWires[i], borrow, GateType::Xor, d[i]);
        //    //    }
        //    //}
        //}

    }

    void CircuitLibrary::int_int_mult_build(
        BetaCircuit & cd,
        BetaBundle & a,
        BetaBundle & b,
        BetaBundle & c)
    {

        u64 N = c.mWires.size();

        std::vector<BetaBundle> terms;
        terms.reserve(N);


        // first, we compute the AND between the two inputs.
        for (u64 i = 0; i < b.mWires.size(); ++i)
        {
            const BetaWire& multBit = b.mWires[i];

            // this will hold the b[i] * a, a vector of N-i bits
            terms.emplace_back(N - i);

            // initialize some unused wires, these will
            // hold intermediate sums.
            cd.addTempWireBundle(terms.back());

            if (i == 0)
            {
                // later, we will sum together all the 
                // terms, and this term at idx 0 will be 
                // the running total, so we want it to be 
                // the wires that represent the product c.
                terms[0].mWires[0] = c.mWires[0];
            }

            // compute the AND between b[i] * a[j].
            for (u64 j = 0; j + i < N; ++j)
            {
                cd.addGate(
                    multBit,
                    a.mWires[j],
                    GateType::And,
                    terms[i].mWires[j]);
            }
        }

#define SERIAL
#ifdef SERIAL

        BetaBundle temp(3), temp2(N - 1);
        cd.addTempWireBundle(temp);
        cd.addTempWireBundle(temp2);

        std::array<BetaBundle, 2> temps{ temp2, terms[0] };

        for (u64 i = 1; i < N; i++)
        {
            auto& t0 = temps[(i & 1)];
            auto& t1 = temps[(i & 1) ^ 1];

            t0.mWires.erase(t0.mWires.begin());
            t1.mWires.resize(t0.mWires.size());

            t1.mWires[0] = c.mWires[i];

            int_int_add_built(cd, t0, terms[i], t1, temp);
        }
#else

        // while the serial code above should work, it is more sequential. 
        // as such, then using the 'leveled' presentation, fewer operations
        // can be pipelined. 

        u64 k = 1, p = 1;
        while (terms.size() > 1)
        {
            std::vector<BetaBundle> newTerms;


            for (u64 i = 0; i < terms.size(); i += 2)
            {
                BetaBundle temp(3);
                cd.addTempWireBundle(temp);

                newTerms.emplace_back(terms[i + 1].mWires.size());
                auto& prod = newTerms.back();
                cd.addTempWireBundle(prod);

                if (i == 0)
                {
                    for (u64 j = 0; j < k; ++j)
                    {
                        prod.mWires[j] = c.mWires[p++];
                    }

                    k *= 2;
                }

                auto sizeDiff = terms[i].mWires.size() - terms[i + 1].mWires.size();

                std::vector<BetaWire> bottomBits(
                    terms[i].mWires.begin(),
                    terms[i].mWires.begin() + sizeDiff);

                terms[i].mWires.erase(
                    terms[i].mWires.begin(),
                    terms[i].mWires.begin() + sizeDiff);

                int_int_add_built(cd, terms[i], terms[i + 1], prod, temp);

                prod.mWires.insert(prod.mWires.begin(), bottomBits.begin(), bottomBits.end());
        }

            terms = std::move(newTerms);
    }

#endif
        cd.levelize();
}
    void CircuitLibrary::int_int_bitwiseAnd_build(BetaCircuit & cd, BetaBundle & a1, BetaBundle & a2, BetaBundle & out)
    {
        for (u64 j = 0; j < out.mWires.size(); ++j)
        {
            cd.addGate(
                a1.mWires[j],
                a2.mWires[j],
                GateType::And,
                out.mWires[j]);
        }

    }
}