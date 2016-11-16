#include "CrtInt.h"

namespace osuCrypto
{

    u64 CrtInt32::sBitCount = 32;

    CrtInt32::CrtInt32(CrtRuntime& runtime)
        : mRuntime(runtime),
        mLabels(sBitCount)
    {


    }

    CrtInt32::CrtInt32(const CrtInt32 & v)
        : mRuntime(v.mRuntime)
        , mLabels(v.mLabels)
    {
    }

    CrtInt32::CrtInt32(CrtInt32 &&v)
        : mRuntime(v.mRuntime)
        , mLabels(std::move(v.mLabels))
    {
    }


    CrtInt32::~CrtInt32()
    {
    }


    CrtInt32& CrtInt32::operator=(const CrtInt32 & copy)
    {
        mLabels = copy.mLabels;
        return *this;
    }

    CrtInt32 CrtInt32::operator+(const CrtInt32& in2)
    {

        std::vector<block>& h2 = *(std::vector<block>*)&in2.mLabels;
        CrtInt32 ret(mRuntime);


        mRuntime.scheduleCrt(mAddBetaCir, mLabels, h2, ret.mLabels);

        return ret;
    }


    void CrtInt32::operator+=(const CrtInt32& in2)
    {
        std::vector<block>& h2 = *(std::vector<block>*)&in2.mLabels;

        mRuntime.scheduleCrt(mAddBetaCir, mLabels, h2, mLabels);

    }

    CrtInt32 CrtInt32::operator*(const CrtInt32& in2)
    {

        std::vector<block>& h2 = *(std::vector<block>*)&in2.mLabels;
        CrtInt32 ret(mRuntime);


        mRuntime.scheduleCrt(mMultBetaCir, mLabels, h2, ret.mLabels);

        return ret;
    }

    BitVector CrtInt32::valueToBV(const ValueType & val)
    {
        return BitVector((u8*)&val, sizeof(ValueType) * 8);
    }

    CrtInt32::ValueType CrtInt32::valueFromBV(const BitVector & val)
    {
        if (val.size() != sizeof(ValueType) * 8)throw std::runtime_error("");
        return *(ValueType*)val.data();
    }


    void CrtInt32::staticInit()
    {
        staticInitAddBetaCir();
        staticInitMultBetaCir();
    }

    BetaCircuit CrtInt32::mAddBetaCir =  BetaCircuit();
    BetaCircuit CrtInt32::mMultBetaCir = BetaCircuit();

    void CrtInt32::staticInitAddBetaCir()
    {

        auto& cd = mAddBetaCir;

        BetaBundle a1(sBitCount);
        BetaBundle a2(sBitCount);
        BetaBundle sum(sBitCount);
        BetaBundle temps(3);

        cd.addInputBundle(a1);
        cd.addInputBundle(a2);
        cd.addOutputBundle(sum);
        cd.addTempWireBundle(temps);

        staticBuildAddBetaCir(cd, a1, a2, sum, temps);

        cd.levelize();
    }


    void CrtInt32::staticBuildAddBetaCir(
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


    void CrtInt32::staticInitMultBetaCir()
    {

        BetaBundle multiplicand(sBitCount);
        BetaBundle multiplier(sBitCount);
        BetaBundle product(sBitCount);
        //BetaBundle temp(sBitCount);

        auto& cd = mMultBetaCir;


        cd.addInputBundle(multiplicand);
        cd.addInputBundle(multiplier);
        cd.addInputBundle(product);
        //cd.addWireBundle(temp);

        std::vector<BetaBundle> terms;
        terms.reserve(sBitCount);

        for (u64 i = 0; i < sBitCount; ++i)
        {
            const BetaWire& multBit = multiplier.mWires[i];


            terms.emplace_back(sBitCount - i);
            cd.addTempWireBundle(terms.back());

            if (i == 0)
            {
                terms[0].mWires[0] = product.mWires[0];
            }

            for (u64 j = 0; j + i< sBitCount; ++j)
            {
                cd.addGate(
                    multBit,
                    multiplicand.mWires[j],
                    GateType::And,
                    terms.back().mWires[j]);
            }
        }
#ifdef SERIAL
        BetaBundle temp(3), temp2(sBitCount - 1);
        cd.addTempWireBundle(temp);
        cd.addTempWireBundle(temp2);

        std::array<BetaBundle, 2> temps{ temp2, terms[0] };

        for (u64 i = 1; i < sBitCount; i++)
        {
            auto& t0 = temps[(i & 1)];
            auto& t1 = temps[(i & 1) ^ 1];

            t0.mWires.erase(t0.mWires.begin());
            t1.mWires.resize(t0.mWires.size());

            t1.mWires[0] = product.mWires[i];

            staticBuildAddCir(cd, t0, terms[i], t1, temp);
        }
#else

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
                        prod.mWires[j] = product.mWires[p++];
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

                staticBuildAddBetaCir(cd, terms[i], terms[i + 1], prod, temp);

                prod.mWires.insert(prod.mWires.begin(), bottomBits.begin(), bottomBits.end());
            }

            terms = std::move(newTerms);
        }

#endif
        cd.levelize();
    }

}
