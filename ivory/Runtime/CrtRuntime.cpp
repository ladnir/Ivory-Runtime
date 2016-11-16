#include "CrtRuntime.h"
#include "Common/ByteStream.h"
#include "OT/Base/naor-pinkas.h"
#include "Common/Log.h"
namespace osuCrypto
{


    CrtRuntime::CrtRuntime()
    {
        mBytesSent = 0;
    }


    CrtRuntime::~CrtRuntime()
    {
    }

    void CrtRuntime::init(Channel & chl, block seed, Role role, u64 partyIdx)
    {
        mPrng.SetSeed(seed);
        mAes.setKey(mPrng.get<block>());
        mChannel = &chl;
        mGlobalOffset = mPrng.get<block>();
        mZeroAndGlobalOffset[1] = mGlobalOffset;
        mRole = role;
        mPartyIdx = partyIdx;


        if (role == Garbler)
        {
            NaorPinkas base;
            BitVector choices(128);
            std::vector<block> msg(128);
            base.receive(choices, msg, mPrng, chl, 4);
            mOtExtSender.setBaseOts(msg, choices);
        }
        else
        {
            NaorPinkas base;
            std::vector<std::array<block, 2>> msg(128);
            base.send(msg, mPrng, chl, 4);
            mOtExtRecver.setBaseOts(msg);
        }

    }

    void CrtRuntime::scheduleCrt(
        BetaCircuit & cir,
        ArrayView<block> in1,
        ArrayView<block> in2,
        ArrayView<block> out)
    {
        //sharedMem.resize(cir.mWireCount);
        //sharedGates.resize(cir.mNonXorGateCount);
        //std::array<block, 2> tweak;
        //garble(cir, sharedMem, tweak, sharedGates.data());
        mCrtQueue.emplace();
        CircuitItem& item = mCrtQueue.back();

        item.mCircuit = &cir;
        item.mLabels.resize(3);// = ArrayView<ArrayView<block>>(3);
        item.mLabels[0] = in1;
        item.mLabels[1] = in2;
        item.mLabels[2] = out;
        item.mInputBundleCount = 2;

        process();

    }

    void CrtRuntime::scheduleInput(
        ArrayView<block> input,
        BitVector& value,
        u64 partyIdx)
    {
        mInputQueue.emplace();

        /*mOtChoices*/

        if (mRole == Evaluator)
        {
            mOtChoices.append(value);
            mOtCount += value.size();
        }

        auto& item = mInputQueue.back();

        item.mInputVal = value;
        item.mLabels = input;

        process();

    }

    void CrtRuntime::scheduleInput(ArrayView<block> input, u64 partyIdx)
    {
        if (mRole == Garbler)
        {
            mOtCount += input.size();

        }


        mInputQueue.emplace();

        auto& item = mInputQueue.back();

        item.mLabels = input;

        process();

    }

    void CrtRuntime::scheduleOutput(ArrayView<block> labels, u64 partyIdx)
    {
        mOutputQueue.emplace();

        auto& item = mOutputQueue.back();

        item.mLabels = labels;
        item.mOutputVal = nullptr;

        process();

    }

    void CrtRuntime::scheduleOutput(ArrayView<block> labels,
        BitVector& future)
    {

        mOutputQueue.emplace();

        auto& item = mOutputQueue.back();

        item.mLabels = labels;
        item.mOutputVal = &future;
        item.mOutputVal->resize(item.mLabels.size());

        process();
    }

    void CrtRuntime::process()
    {

        if (true)
        {
            if (mRole == Garbler)
            {
                garblerInput();
                garblerCircuit();
                garblerOutput();
            }
            else
            {
                evaluatorInput();
                evaluatorCircuit();
                evaluatorOutput();
            }
        }
    }


    void CrtRuntime::garblerInput()
    {
        std::vector<std::array<block, 2>> messages(mOtCount);
        if (mOtCount)
        {

            mOtCount = 0;

            mOtExtSender.send(messages, mPrng, *mChannel);


        }

        auto iter = messages.begin();

        while (mInputQueue.size())
        {


            auto& item = mInputQueue.front();

            if (item.mInputVal.size())
            {
                mAes.ecbEncCounterMode(mInputIdx, item.mLabels.size(), item.mLabels.data());
                mInputIdx += item.mLabels.size();

                std::unique_ptr<ByteStream> buff(new ByteStream(item.mLabels.size() * sizeof(block)));
                auto view = buff->getArrayView<block>();

                for (u64 i = 0; i < item.mLabels.size(); ++i)
                {
                    view[i] = item.mLabels[i] ^ mZeroAndGlobalOffset[item.mInputVal[i]];
                }
                mChannel->asyncSend(std::move(buff));
            }
            else
            {
                std::unique_ptr<ByteStream> buff(new ByteStream(item.mLabels.size() * sizeof(block)));
                auto view = buff->getArrayView<block>();
                for (u64 i = 0; i < item.mLabels.size(); ++i, ++iter)
                {
                    item.mLabels[i] = (*iter)[0];
                    view[i] = (*iter)[1] ^ (*iter)[0] ^ mGlobalOffset;
                }
                mChannel->asyncSend(std::move(buff));
            }

            mInputQueue.pop();
        }
    }

    void CrtRuntime::evaluatorInput()
    {
        static const std::array<block, 2> zeroAndAllOnesBlk{ ZeroBlock, AllOneBlock };

        if (mOtChoices.size())
        {

            if (sharedMem.size() < mOtCount)
                sharedMem.resize(mOtCount);

            //sharedMem.resize(mOtCount);
            ArrayView<block> view(sharedMem.begin(), sharedMem.begin() + mOtCount);

            mOtExtRecver.receive(mOtChoices, sharedMem, mPrng, *mChannel);

            mOtChoices.resize(0);
            mOtCount = 0;

        }

        auto iter = sharedMem.begin();

        while (mInputQueue.size())
        {

            auto& item = mInputQueue.front();

            if (item.mInputVal.size())
            {
                mChannel->recv(sharedBuff);
                auto view = sharedBuff.getArrayView<block>();

                for (u64 i = 0; i < item.mLabels.size(); ++i)
                {
                    item.mLabels[i] = *iter++ ^ (zeroAndAllOnesBlk[item.mInputVal[i]] & view[i]);
                }
            }
            else
            {
                mChannel->recv(item.mLabels.data(), item.mLabels.size() * sizeof(block));
            }
            mInputQueue.pop();
        }
    }

    void CrtRuntime::garblerCircuit()
    {

        while (mCrtQueue.size())
        {
            auto& item = mCrtQueue.front();


            if (sharedMem.size() < item.mCircuit->mWireCount)
            {
                sharedMem.resize(item.mCircuit->mWireCount);
            }

            auto iter = sharedMem.begin();


            for (u64 i = 0; i < item.mInputBundleCount; ++i)
            {
                std::copy(item.mLabels[i].begin(), item.mLabels[i].end(), iter);
                iter += item.mLabels[i].size();
            }

            std::unique_ptr<ByteStream> sendBuff(new ByteStream(item.mCircuit->mNonXorGateCount * sizeof(GarbledGate<2>)));

            auto gates = sendBuff->getArrayView<GarbledGate<2>>();

            garble(*item.mCircuit, sharedMem, mTweaks, gates);

            mChannel->asyncSend(std::move(sendBuff));

            for (u64 i = item.mInputBundleCount; i < item.mLabels.size(); ++i)
            {
                std::copy(iter, iter + item.mLabels[i].size(), item.mLabels[i].begin());
                iter += item.mLabels[i].size();
            }

            mCrtQueue.pop();
        }


    }


    void CrtRuntime::evaluatorCircuit()
    {
        while (mCrtQueue.size())
        {
            auto& item = mCrtQueue.front();


            if (sharedMem.size() < item.mCircuit->mWireCount)
            {
                sharedMem.resize(item.mCircuit->mWireCount);
            }

            auto iter = sharedMem.begin();

            for (u64 i = 0; i < item.mInputBundleCount; ++i)
            {
                std::copy(item.mLabels[i].begin(), item.mLabels[i].end(), iter);
                iter += item.mLabels[i].size();
            }


            sharedGates.resize(item.mCircuit->mNonXorGateCount);

            mChannel->recv(sharedGates.data(), sharedGates.size() * sizeof(GarbledGate<2>));


            evaluate(*item.mCircuit, sharedMem, mTweaks, sharedGates);


            for (u64 i = item.mInputBundleCount; i < item.mLabels.size(); ++i)
            {
                std::copy(iter, iter + item.mLabels[i].size(), item.mLabels[i].begin());
                iter += item.mLabels[i].size();
            }


            mCrtQueue.pop();
        }

    }


    void CrtRuntime::garblerOutput()
    {

        while (mOutputQueue.size())
        {
            auto& item = mOutputQueue.front();


            if (item.mOutputVal)
            {
                if (sharedMem.size() < item.mLabels.size())
                {
                    sharedMem.resize(item.mLabels.size());
                }

                mChannel->recv(sharedMem.data(), item.mLabels.size() * sizeof(block));


                for (u64 i = 0; i < item.mLabels.size(); ++i)
                {
                    if (neq(sharedMem[i], item.mLabels[i]) && neq(sharedMem[i], item.mLabels[i] ^ mGlobalOffset))
                    {
                        Log::out << "output reveal error at " << i << ":\n   " << sharedMem[i] << "  != " << item.mLabels[i]
                            << " (0) AND \n   " << sharedMem[i] << "  != " << (item.mLabels[i] ^ mGlobalOffset) << Log::endl;

                        throw std::runtime_error(LOCATION);
                    }

                    (*item.mOutputVal)[i] = PermuteBit(sharedMem[i] ^ item.mLabels[i]);
                }
            }
            else
            {
                std::unique_ptr<BitVector> sendBuff(new BitVector(item.mLabels.size()));

                for (u64 i = 0; i < item.mLabels.size(); ++i)
                {
                    (*sendBuff)[i] = PermuteBit(item.mLabels[i]);
                }

                mChannel->asyncSend(std::move(sendBuff));
            }

            mOutputQueue.pop();
        }
    }

    void CrtRuntime::evaluatorOutput()
    {
        while (mOutputQueue.size())
        {
            auto& item = mOutputQueue.front();

            if (item.mOutputVal)
            {
                mChannel->recv((*item.mOutputVal));

                for (u64 i = 0; i < item.mLabels.size(); ++i)
                {
                    (*item.mOutputVal)[i] = (*item.mOutputVal)[i] ^ PermuteBit(item.mLabels[i]);
                }
            }
            else
            {
                mChannel->asyncSendCopy(item.mLabels.data(), item.mLabels.size() * sizeof(block));
            }

            mOutputQueue.pop();
        }
    }












    void CrtRuntime::evaluate(
        BetaCircuit & cir,
        ArrayView<block> wires,
        std::array<block, 2>& tweaks,
        ArrayView<GarbledGate<2>> garbledGates)
    {
        auto garbledGateIter = garbledGates.begin();


        if (cir.mLevelGates.size())
        {
            block hashs[2], temp[2],
                zeroAndGarbledTable[2][2]
            { { ZeroBlock,ZeroBlock },{ ZeroBlock,ZeroBlock } };
        
            for (const auto& level : cir.mLevelGates)
            {
        
                for (const auto& gate : level.mXorGates)
                {
                    auto& aIdx1 = gate.mInput[0];
                    auto& bIdx1 = gate.mInput[1];
                    auto& cIdx1 = gate.mOutput;
        
                    auto& a = wires[aIdx1];
                    auto& b = wires[bIdx1];
                    auto& c = wires[cIdx1];
                    auto& gt = gate.mType;
        
                    c = a ^ b;
                }
                for (const auto& gate : level.mAndGates)
                {
                    auto& a = wires[gate.mInput[0]];
                    auto& b = wires[gate.mInput[1]];
                    auto& c = wires[gate.mOutput];
                    auto& gt = gate.mType;
        
                    // compute the hashs
                    hashs[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
                    hashs[1] = _mm_slli_epi64(b, 1) ^ tweaks[1];
                    mAesFixedKey.ecbEncTwoBlocks(hashs, temp);
                    hashs[0] = temp[0] ^ hashs[0]; // a
                    hashs[1] = temp[1] ^ hashs[1]; // b
        
                                                   // increment the tweaks
                    tweaks[0] = tweaks[0] + OneBlock;
                    tweaks[1] = tweaks[1] + OneBlock;
        
        
                    auto& garbledTable = garbledGateIter++->mGarbledTable;
        
                    zeroAndGarbledTable[1][0] = garbledTable[0];
                    zeroAndGarbledTable[1][1] = garbledTable[1] ^ a;
        
                    // compute the output wire label
                    c = hashs[0] ^
                        hashs[1] ^
                        zeroAndGarbledTable[PermuteBit(a)][0] ^
                        zeroAndGarbledTable[PermuteBit(b)][1];
                }
            }
        }
        else
        {
            block hashs[2], temp[2],
                zeroAndGarbledTable[2][2]
            { { ZeroBlock,ZeroBlock },{ ZeroBlock,ZeroBlock } };

            for (const auto& gate : cir.mGates)
            {
                auto& a = wires[gate.mInput[0]];
                auto& b = wires[gate.mInput[1]];
                auto& c = wires[gate.mOutput];
                auto& gt = gate.mType;

                if (gt == GateType::Xor || gt == GateType::Nxor)
                {
                    c = a ^ b;
                }
                else
                {
                    // compute the hashs
                    hashs[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
                    hashs[1] = _mm_slli_epi64(b, 1) ^ tweaks[1];
                    mAesFixedKey.ecbEncTwoBlocks(hashs, temp);
                    hashs[0] = temp[0] ^ hashs[0]; // a
                    hashs[1] = temp[1] ^ hashs[1]; // b

                                                   // increment the tweaks
                    tweaks[0] = tweaks[0] + OneBlock;
                    tweaks[1] = tweaks[1] + OneBlock;



                    auto& garbledTable = garbledGateIter++->mGarbledTable;

                    zeroAndGarbledTable[1][0] = garbledTable[0];
                    zeroAndGarbledTable[1][1] = garbledTable[1] ^ a;



                    // compute the output wire label
                    c = hashs[0] ^
                        hashs[1] ^
                        zeroAndGarbledTable[PermuteBit(a)][0] ^
                        zeroAndGarbledTable[PermuteBit(b)][1];

                }
                    
            }
        }


    }

    void CrtRuntime::garble(
        BetaCircuit& cir,
        ArrayView<block> wires,
        std::array<block, 2>& tweaks,
        ArrayView<GarbledGate<2>> gates)
    {
        auto gateIter = gates.data();

        if (cir.mLevelGates.size())
        {
            u8 aPermuteBit, bPermuteBit, bAlphaBPermute, cPermuteBit;
            block hash[16], temp[16];
        
            for (const auto& level : cir.mLevelGates)
            {
        
                for (const auto& gate : level.mXorGates)
                {
        
                    auto& a = wires[gate.mInput[0]];
                    auto& b = wires[gate.mInput[1]];
                    auto& c = wires[gate.mOutput];
                    auto& gt = gate.mType;
        
                    c = a ^ b ^ mZeroAndGlobalOffset[(u8)gt & 1];
                }
        
        
                for (u64 i = 0; i < level.mAndGates.size();)
                {
                    switch (level.mAndGates.size() - i)
                    {
                    case 1:
                    {
        
                        auto& gate = level.mAndGates[i];
                        i += 1;
        
                        auto& a = wires[gate.mInput[0]];
                        auto& b = wires[gate.mInput[1]];
                        auto& c = wires[gate.mOutput];
                        auto& gt = gate.mType;
        
                        // compute the gate modifier variables
                        auto& aAlpha = gate.mAAlpha;
                        auto& bAlpha = gate.mBAlpha;
                        auto& cAlpha = gate.mCAlpha;
        
                        //signal bits of wire 0 of input0 and wire 0 of input1
                        aPermuteBit = PermuteBit(a);
                        bPermuteBit = PermuteBit(b);
                        bAlphaBPermute = bAlpha ^ bPermuteBit;
                        cPermuteBit = ((aPermuteBit ^ aAlpha) && (bAlphaBPermute)) ^ cAlpha;
        
                        // compute the hashs of the wires as H(x) = AES_f( x * 2 ^ tweak) ^ (x * 2 ^ tweak)    
                        hash[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
                        hash[1] = _mm_slli_epi64((a ^ mGlobalOffset), 1) ^ tweaks[0];
                        hash[2] = _mm_slli_epi64(b, 1) ^ tweaks[1];
                        hash[3] = _mm_slli_epi64((b ^ mGlobalOffset), 1) ^ tweaks[1];
                        mAesFixedKey.ecbEncFourBlocks(hash, temp);
                        hash[0] = hash[0] ^ temp[0]; // H( a0 )
                        hash[1] = hash[1] ^ temp[1]; // H( a1 )
                        hash[2] = hash[2] ^ temp[2]; // H( b0 )
                        hash[3] = hash[3] ^ temp[3]; // H( b1 )
        
                                                     // increment the tweaks
                        tweaks[0] = tweaks[0] + OneBlock;
                        tweaks[1] = tweaks[1] + OneBlock;
        
                        // generate the garbled table
                        auto& garbledTable = gateIter++->mGarbledTable;
        
                        // compute the table entries
                        garbledTable[0] = hash[0] ^ hash[1] ^ mZeroAndGlobalOffset[bAlphaBPermute];
                        garbledTable[1] = hash[2] ^ hash[3] ^ a ^ mZeroAndGlobalOffset[aAlpha];
        
                        // compute the out wire
                        c = hash[aPermuteBit] ^
                            hash[2 ^ bPermuteBit] ^
                            mZeroAndGlobalOffset[cPermuteBit];
                        break;
                    }
                    //case 2:
                    //case 3:
                    default:
                    {
        
                        auto& gate0 = level.mAndGates[i];
                        auto& gate1 = level.mAndGates[i + 1];
                        i += 2;
        
        
                        auto& a0 = wires[gate0.mInput[0]];
                        auto& b0 = wires[gate0.mInput[1]];
                        auto& c0 = wires[gate0.mOutput];
                        auto& gt0 = gate0.mType;
        
                        auto&  a1 = wires[gate1.mInput[0]];
                        auto&  b1 = wires[gate1.mInput[1]];
                        auto&  c1 = wires[gate1.mOutput];
                        auto& gt1 = gate1.mType;
        
                        // compute the gate modifier variables
                        auto& aAlpha0 = gate0.mAAlpha;
                        auto& bAlpha0 = gate0.mBAlpha;
                        auto& cAlpha0 = gate0.mCAlpha;
        

                        // compute the gate modifier variables
                        auto& aAlpha1 = gate1.mAAlpha;
                        auto& bAlpha1 = gate1.mBAlpha;
                        auto& cAlpha1 = gate1.mCAlpha;

                        //signal bits of wire 0 of input0 and wire 0 of input1
                        auto aPermuteBit0 = PermuteBit(a0);
                        auto bPermuteBit0 = PermuteBit(b0);
                        auto aPermuteBit1 = PermuteBit(a1);
                        auto bPermuteBit1 = PermuteBit(b1);
                        auto bAlphaBPermute0 = bAlpha0 ^ bPermuteBit0;
                        auto bAlphaBPermute1 = bAlpha1 ^ bPermuteBit1;
                        auto cPermuteBit0 = ((aPermuteBit0 ^ aAlpha0) && (bAlphaBPermute0)) ^ cAlpha0;
                        auto cPermuteBit1 = ((aPermuteBit1 ^ aAlpha1) && (bAlphaBPermute1)) ^ cAlpha1;

                        // increment the tweaks
                        auto t2 = tweaks[0] + OneBlock;
                        auto t3 = tweaks[1] + OneBlock;

                        // compute the hashs of the wires as H(x) = AES_f( x * 2 ^ tweak) ^ (x * 2 ^ tweak)    
                        hash[0] = _mm_slli_epi64(a0, 1) ^ tweaks[0];
                        hash[1] = _mm_slli_epi64((a0 ^ mGlobalOffset), 1) ^ tweaks[0];
                        hash[2] = _mm_slli_epi64(b0, 1) ^ tweaks[1];
                        hash[3] = _mm_slli_epi64((b0 ^ mGlobalOffset), 1) ^ tweaks[1];
                        hash[4] = _mm_slli_epi64(a1, 1) ^ t2;
                        hash[5] = _mm_slli_epi64((a1 ^ mGlobalOffset), 1) ^ t2;
                        hash[6] = _mm_slli_epi64(b1, 1) ^ t3;
                        hash[7] = _mm_slli_epi64((b1 ^ mGlobalOffset), 1) ^ t3;
                        mAesFixedKey.ecbEncBlocks(hash, 8, temp);
                        hash[0] = hash[0] ^ temp[0]; // H( a0 )
                        hash[1] = hash[1] ^ temp[1]; // H( a1 )
                        hash[2] = hash[2] ^ temp[2]; // H( b0 )
                        hash[3] = hash[3] ^ temp[3]; // H( b1 )						
                        hash[4] = hash[4] ^ temp[4]; // H( a0 )
                        hash[5] = hash[5] ^ temp[5]; // H( a1 )
                        hash[6] = hash[6] ^ temp[6]; // H( b0 )
                        hash[7] = hash[7] ^ temp[7]; // H( b1 )

                        // increment the tweaks
                        tweaks[0] = t2 + OneBlock;
                        tweaks[1] = t3 + OneBlock;

                        // generate the garbled table
                        auto& garbledTable0 = gateIter++->mGarbledTable;
                        auto& garbledTable1 = gateIter++->mGarbledTable;

                        // compute the table entries
                        garbledTable0[0] = hash[0] ^ hash[1] ^ mZeroAndGlobalOffset[bAlphaBPermute0];
                        garbledTable0[1] = hash[2] ^ hash[3] ^ a0 ^ mZeroAndGlobalOffset[aAlpha0];

                        garbledTable1[0] = hash[4] ^ hash[5] ^ mZeroAndGlobalOffset[bAlphaBPermute1];
                        garbledTable1[1] = hash[6] ^ hash[7] ^ a1 ^ mZeroAndGlobalOffset[aAlpha1];

                        // compute the out wire
                        c0 = hash[aPermuteBit0] ^
                            hash[2 ^ bPermuteBit0] ^
                            mZeroAndGlobalOffset[cPermuteBit0];

                        c1 = hash[4 | aPermuteBit1] ^
                            hash[6 ^ bPermuteBit1] ^
                            mZeroAndGlobalOffset[cPermuteBit1];

                        break;
                    }
                    }
                }
            }
        }
        else
        {
            u8 aPermuteBit, bPermuteBit, bAlphaBPermute, cPermuteBit;
            block hash[4], temp[4];

            for (const auto& gate : cir.mGates)
            {

                auto& a = wires[gate.mInput[0]];
                auto& b = wires[gate.mInput[1]];
                auto& c = wires[gate.mOutput];
                auto& gt = gate.mType;


                if (gt == GateType::Xor || gt == GateType::Nxor)
                {
                    c = a ^ b ^ mZeroAndGlobalOffset[(u8)gt & 1];
                }
                else
                {
                    // compute the gate modifier variables
                    auto& aAlpha = gate.mAAlpha;
                    auto& bAlpha = gate.mBAlpha;
                    auto& cAlpha = gate.mCAlpha;

                    //signal bits of wire 0 of input0 and wire 0 of input1
                    aPermuteBit = PermuteBit(a);
                    bPermuteBit = PermuteBit(b);
                    bAlphaBPermute = bAlpha ^ bPermuteBit;
                    cPermuteBit = ((aPermuteBit ^ aAlpha) && (bAlphaBPermute)) ^ cAlpha;

                    // compute the hashs of the wires as H(x) = AES_f( x * 2 ^ tweak) ^ (x * 2 ^ tweak)    
                    hash[0] = _mm_slli_epi64(a, 1) ^ tweaks[0];
                    hash[1] = _mm_slli_epi64((a ^ mGlobalOffset), 1) ^ tweaks[0];
                    hash[2] = _mm_slli_epi64(b, 1) ^ tweaks[1];
                    hash[3] = _mm_slli_epi64((b ^ mGlobalOffset), 1) ^ tweaks[1];
                    mAesFixedKey.ecbEncFourBlocks(hash, temp);
                    hash[0] = hash[0] ^ temp[0]; // H( a0 )
                    hash[1] = hash[1] ^ temp[1]; // H( a1 )
                    hash[2] = hash[2] ^ temp[2]; // H( b0 )
                    hash[3] = hash[3] ^ temp[3]; // H( b1 )

                                                 // increment the tweaks
                    tweaks[0] = tweaks[0] + OneBlock;
                    tweaks[1] = tweaks[1] + OneBlock;

                    // generate the garbled table
                    auto& garbledTable = gateIter++->mGarbledTable;

                    // compute the table entries
                    garbledTable[0] = hash[0] ^ hash[1] ^ mZeroAndGlobalOffset[bAlphaBPermute];
                    garbledTable[1] = hash[2] ^ hash[3] ^ a ^ mZeroAndGlobalOffset[aAlpha];


                    // compute the out wire
                    c = hash[aPermuteBit] ^
                        hash[2 ^ bPermuteBit] ^
                        mZeroAndGlobalOffset[cPermuteBit];
                }

            }
        }
        //Log::out << Log::unlock;

    }


}