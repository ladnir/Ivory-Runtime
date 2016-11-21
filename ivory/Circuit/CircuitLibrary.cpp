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


        if (a1.mWires.size() != a2.mWires.size() ||
            a1.mWires.size() != sum.mWires.size() ||
            temps.mWires.size() < 3)
            throw std::runtime_error("");

        BetaWire& carry = temps.mWires[0];
        BetaWire& aXorC = temps.mWires[1];
        BetaWire& temp = temps.mWires[2];

        // half adder
        cd.addGate(a1.mWires[0], a2.mWires[0], GateType::Xor, sum.mWires[0]);

        if (a1.mWires[0] == sum.mWires[0] ||
            a2.mWires[0] == sum.mWires[0])
            throw std::runtime_error("");


        if (a1.mWires.size() > 1)
        {
            if (a1.mWires[1] == sum.mWires[1] ||
                a2.mWires[1] == sum.mWires[1])
                throw std::runtime_error("");

            // compute the carry from the 0 bits
            cd.addGate(a1.mWires[0], a2.mWires[0], GateType::And, carry);

            cd.addGate(a1.mWires[1], carry, GateType::Xor, aXorC);
            cd.addGate(a2.mWires[1], aXorC, GateType::Xor, sum.mWires[1]);

            for (u64 i = 2; i < a1.mWires.size(); ++i)
            {
                if (a1.mWires[i] == sum.mWires[i] ||
                    a2.mWires[i] == sum.mWires[i])
                    throw std::runtime_error("");

                // compute the previous carry
                cd.addGate(a2.mWires[i - 1], carry, GateType::Xor, temp);
                cd.addGate(temp, aXorC, GateType::And, temp);
                cd.addGate(temp, carry, GateType::Xor, carry);

                cd.addGate(a1.mWires[i], carry, GateType::Xor, aXorC);
                cd.addGate(a2.mWires[i], aXorC, GateType::Xor, sum.mWires[i]);
            }
        }
    }

    void CircuitLibrary::int_int_mult_build(
        BetaCircuit & cd,
        BetaBundle & a,
        BetaBundle & b,
        BetaBundle & c)
    {
        //cd.addInputBundle(a);
        //cd.addInputBundle(b);
        //cd.addOutputBundle(c);

        u64 N = c.mWires.size();

        std::vector<BetaBundle> terms;
        terms.reserve(N);


        // first, we compute the AND between the two inputs.
        for (u64 i = 0; i < b.mWires.size(); ++i)
        {
            const BetaWire& multBit = b.mWires[i];

            // this will hold the b[i] * a, a vector of N-i bits
            terms.emplace_back(N - i);


            if (i == 0)
            {
                // later, we will sum together all the 
                // terms, and this term at idx 0 will be 
                // the running total, so we want it to be 
                // the wires that represent the product c.
                terms[0].mWires[0] = c.mWires[0];
            }
            else
            {
                // initialize some unused wires, these will
                // hold intermediate sums.
                cd.addTempWireBundle(terms.back());
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

//#define SERIAL
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
        for (u64 j = 0; j  < out.mWires.size(); ++j)
        {
            cd.addGate(
                a1.mWires[j],
                a2.mWires[j],
                GateType::And,
                out.mWires[j]);
        }

    }
}