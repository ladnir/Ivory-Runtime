#include "sInt.h"

namespace osuCrypto
{



    sInt::sInt(Runtime& runtime, u64 bitCount)
        : mRuntime(runtime),
        mLabels(bitCount)
    {


    }

    sInt::sInt(const sInt & v)
        : mRuntime(v.mRuntime)
        , mLabels(v.mLabels)
    {
    }

    sInt::sInt(sInt &&v)
        : mRuntime(v.mRuntime)
        , mLabels(std::move(v.mLabels))
    {
    }


    sInt::~sInt()
    {
    }


    sInt& sInt::operator=(const sInt & copy)
    {
        mLabels = copy.mLabels;
        return *this;
    }

    sInt sInt::operator+(const sInt& in2)
    {

        std::vector<block>& h2 = *(std::vector<block>*)&in2.mLabels;
        sInt ret(mRuntime, std::max(mLabels.size(), h2.size()));

        std::array<ArrayView<block>, 3> io{mLabels, h2, ret.mLabels};

        mRuntime.scheduleOp(Op::Add, io);

        //mRuntime.scheduleCrt(mAddBetaCir, mLabels, h2, ret.mLabels);

        return ret;
    }


    void sInt::operator+=(const sInt& in2)
    {
        std::vector<block>& h2 = *(std::vector<block>*)&in2.mLabels;


        std::array<ArrayView<block>, 3> io{ mLabels, h2, mLabels };

        mRuntime.scheduleOp(Op::Add, io);

        //mRuntime.scheduleCrt(mAddBetaCir, mLabels, h2, mLabels);

    }

    sInt sInt::operator*(const sInt& in2)
    {

        std::vector<block>& h2 = *(std::vector<block>*)&in2.mLabels;
        sInt ret(mRuntime, std::max(mLabels.size(), h2.size()));
        //sInt ret(mRuntime);

        std::array<ArrayView<block>, 3> io{ mLabels, h2, ret.mLabels };

        mRuntime.scheduleOp(Op::Multiply, io);

        //mRuntime.scheduleCrt(mMultBetaCir, mLabels, h2, ret.mLabels);

        return ret;
    }

    BitVector sInt::valueToBV(const ValueType & val)
    {
        return BitVector((u8*)&val, sizeof(ValueType) * 8);
    }

    sInt::ValueType sInt::valueFromBV(const BitVector & val)
    {
        if (val.size() != mLabels.size())throw std::runtime_error("");

        ValueType ret = 0;
        memcpy(&ret, val.data(), val.sizeBytes());

        return ret;
    }


    //const u32 sInt32::N(32);

    //sInt32::sInt32(Runtime & runtime)
    //    : sInt(runtime, N)
    //{
    //}
    //sInt32::sInt32(const sInt &)

    //{
    //}
}

//
//    void sInt<N>::staticInit()
//    {
//        staticInitAddBetaCir();
//        staticInitMultBetaCir();
//    }
//
//    BetaCircuit sInt<N>::mAddBetaCir =  BetaCircuit();
//    BetaCircuit sInt<N>::mMultBetaCir = BetaCircuit();
//
//    void sInt<N>::staticInitAddBetaCir()
//    {
//
//        auto& cd = mAddBetaCir;
//
//        BetaBundle a1(N);
//        BetaBundle a2(N);
//        BetaBundle sum(N);
//        BetaBundle temps(3);
//
//        cd.addInputBundle(a1);
//        cd.addInputBundle(a2);
//        cd.addOutputBundle(sum);
//        cd.addTempWireBundle(temps);
//
//        staticBuildAddBetaCir(cd, a1, a2, sum, temps);
//
//        cd.levelize();
//    }
//
//
//    void sInt<N>::staticBuildAddBetaCir(
//        BetaCircuit& cd,
//        BetaBundle & a1,
//        BetaBundle & a2,
//        BetaBundle & sum,
//        BetaBundle & temps)
//    {
//
//        if (a1.mWires.size() != a2.mWires.size() ||
//            a1.mWires.size() != sum.mWires.size() ||
//            temps.mWires.size() < 3)
//            throw std::runtime_error("");
//
//        BetaWire& carry = temps.mWires[0];
//        BetaWire& aXorC = temps.mWires[1];
//        BetaWire& temp = temps.mWires[2];
//
//        // half adder
//        cd.addGate(a1.mWires[0], a2.mWires[0], GateType::Xor, sum.mWires[0]);
//
//        if (a1.mWires[0] == sum.mWires[0] ||
//            a2.mWires[0] == sum.mWires[0])
//            throw std::runtime_error("");
//
//
//        if (a1.mWires.size() > 1)
//        {
//            if (a1.mWires[1] == sum.mWires[1] ||
//                a2.mWires[1] == sum.mWires[1])
//                throw std::runtime_error("");
//
//            // compute the carry from the 0 bits
//            cd.addGate(a1.mWires[0], a2.mWires[0], GateType::And, carry);
//
//            cd.addGate(a1.mWires[1], carry, GateType::Xor, aXorC);
//            cd.addGate(a2.mWires[1], aXorC, GateType::Xor, sum.mWires[1]);
//
//            for (u64 i = 2; i < a1.mWires.size(); ++i)
//            {
//                if (a1.mWires[i] == sum.mWires[i] ||
//                    a2.mWires[i] == sum.mWires[i])
//                    throw std::runtime_error("");
//
//                // compute the previous carry
//                cd.addGate(a2.mWires[i - 1], carry, GateType::Xor, temp);
//                cd.addGate(temp, aXorC, GateType::And, temp);
//                cd.addGate(temp, carry, GateType::Xor, carry);
//
//                cd.addGate(a1.mWires[i], carry, GateType::Xor, aXorC);
//                cd.addGate(a2.mWires[i], aXorC, GateType::Xor, sum.mWires[i]);
//            }
//        }
//    }
//
//
//    void sInt<N>::staticInitMultBetaCir()
//    {
//
//        BetaBundle multiplicand(N);
//        BetaBundle multiplier(N);
//        BetaBundle product(N);
//        //BetaBundle temp(N);
//
//        auto& cd = mMultBetaCir;
//
//
//        cd.addInputBundle(multiplicand);
//        cd.addInputBundle(multiplier);
//        cd.addInputBundle(product);
//        //cd.addWireBundle(temp);
//
//        std::vector<BetaBundle> terms;
//        terms.reserve(N);
//
//        for (u64 i = 0; i < N; ++i)
//        {
//            const BetaWire& multBit = multiplier.mWires[i];
//
//
//            terms.emplace_back(N - i);
//            cd.addTempWireBundle(terms.back());
//
//            if (i == 0)
//            {
//                terms[0].mWires[0] = product.mWires[0];
//            }
//
//            for (u64 j = 0; j + i< N; ++j)
//            {
//                cd.addGate(
//                    multBit,
//                    multiplicand.mWires[j],
//                    GateType::And,
//                    terms.back().mWires[j]);
//            }
//        }
//#ifdef SERIAL
//        BetaBundle temp(3), temp2(N - 1);
//        cd.addTempWireBundle(temp);
//        cd.addTempWireBundle(temp2);
//
//        std::array<BetaBundle, 2> temps{ temp2, terms[0] };
//
//        for (u64 i = 1; i < N; i++)
//        {
//            auto& t0 = temps[(i & 1)];
//            auto& t1 = temps[(i & 1) ^ 1];
//
//            t0.mWires.erase(t0.mWires.begin());
//            t1.mWires.resize(t0.mWires.size());
//
//            t1.mWires[0] = product.mWires[i];
//
//            staticBuildAddCir(cd, t0, terms[i], t1, temp);
//        }
//#else
//
//        u64 k = 1, p = 1;
//        while (terms.size() > 1)
//        {
//            std::vector<BetaBundle> newTerms;
//
//
//            for (u64 i = 0; i < terms.size(); i += 2)
//            {
//                BetaBundle temp(3);
//                cd.addTempWireBundle(temp);
//
//                newTerms.emplace_back(terms[i + 1].mWires.size());
//                auto& prod = newTerms.back();
//                cd.addTempWireBundle(prod);
//
//                if (i == 0)
//                {
//                    for (u64 j = 0; j < k; ++j)
//                    {
//                        prod.mWires[j] = product.mWires[p++];
//                    }
//
//                    k *= 2;
//                }
//
//                auto sizeDiff = terms[i].mWires.size() - terms[i + 1].mWires.size();
//
//                std::vector<BetaWire> bottomBits(
//                    terms[i].mWires.begin(),
//                    terms[i].mWires.begin() + sizeDiff);
//
//                terms[i].mWires.erase(
//                    terms[i].mWires.begin(),
//                    terms[i].mWires.begin() + sizeDiff);
//
//                staticBuildAddBetaCir(cd, terms[i], terms[i + 1], prod, temp);
//
//                prod.mWires.insert(prod.mWires.begin(), bottomBits.begin(), bottomBits.end());
//            }
//
//            terms = std::move(newTerms);
//        }
//
//#endif
//        cd.levelize();
//    }
//
//}
