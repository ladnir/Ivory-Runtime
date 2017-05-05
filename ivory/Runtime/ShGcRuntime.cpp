#include "ShGcRuntime.h"
#include "cryptoTools/Common/ByteStream.h"
#include "libOTe/Base/naor-pinkas.h"
#include "cryptoTools/Common/Log.h"
namespace osuCrypto
{


    ShGcRuntime::ShGcRuntime()
    {
        mTweaks[0] = ZeroBlock;
        mTweaks[1] = OneBlock;

        mOtCount = 0;
        mBytesSent = 0;
    }


    ShGcRuntime::~ShGcRuntime()
    {
    }

    void ShGcRuntime::initVar(std::unique_ptr<RuntimeData>& data, u64 bitCount)
    {
        if (data) throw std::runtime_error(LOCATION);

        data.reset(new ShGcRuntimeData(bitCount));
    }

    void ShGcRuntime::copyVar(std::unique_ptr<RuntimeData>& data, RuntimeData * copy)
    {
        if (data == nullptr)
        {
            data.reset(new ShGcRuntimeData(static_cast<ShGcRuntimeData*>(copy)->mLabels->size()));
        }
        else if(static_cast<ShGcRuntimeData*>(data.get())->mLabels->size() != static_cast<ShGcRuntimeData*>(copy)->mLabels->size())
        {
            throw std::runtime_error("unsupported uperation of assignment with different bit lengths." LOCATION);
        }

        static_cast<ShGcRuntimeData*>(data.get())->mLabels = static_cast<ShGcRuntimeData*>(copy)->mLabels;
    }

    void ShGcRuntime::init(Channel & chl, block seed, Role role, u64 partyIdx)
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

    void ShGcRuntime::scheduleOp(
        Op op,
        ArrayView<RuntimeData*> io)
    {
        mCrtQueue.emplace();
        CircuitItem& item = mCrtQueue.back();

        item.mLabels.resize(io.size());
        std::vector<u64> sizes(io.size());

        for (u64 i = 0; i < io.size(); ++i)
        {
            item.mLabels[i] = static_cast<ShGcRuntimeData*>(io[i])->mLabels;
            sizes[i] = item.mLabels[i]->size();
        }


        switch (op)
        {
        case osuCrypto::Op::Add:

            item.mCircuit = mLibrary.int_int_add(sizes[0], sizes[1], sizes[2]);
            item.mInputBundleCount = 2;
            break;
        case osuCrypto::Op::Subtract:
            item.mCircuit = mLibrary.int_int_subtract(sizes[0], sizes[1], sizes[2]);
            item.mInputBundleCount = 2;
            break;
        case osuCrypto::Op::Multiply:
            item.mCircuit = mLibrary.int_int_mult(sizes[0], sizes[1], sizes[2]);
            item.mInputBundleCount = 2;
            break;
        case osuCrypto::Op::Divide:
            item.mCircuit = mLibrary.int_int_div(sizes[0], sizes[1], sizes[2]);
            item.mInputBundleCount = 2;
            break;
        case osuCrypto::Op::LT:
            item.mCircuit = mLibrary.int_int_lt(sizes[0], sizes[1]);
            item.mInputBundleCount = 2;
            break;
        case osuCrypto::Op::GTEq:
            item.mCircuit = mLibrary.int_int_gteq(sizes[0], sizes[1]);
            item.mInputBundleCount = 2;
            break;
        case osuCrypto::Op::Mod:
            throw std::runtime_error(LOCATION);
            break;
        case osuCrypto::Op::And:
            throw std::runtime_error(LOCATION);
            break;
        case osuCrypto::Op::Or:
            throw std::runtime_error(LOCATION);
            break;
        case osuCrypto::Op::Not:
            throw std::runtime_error(LOCATION);
            break;
        case osuCrypto::Op::BitwiseAnd:
            item.mCircuit = mLibrary.int_int_bitwiseAnd(sizes[0], sizes[1], sizes[2]);
            item.mInputBundleCount = 2;
            break;
        case osuCrypto::Op::BitWiseOr:
            throw std::runtime_error(LOCATION);
            break;
        case osuCrypto::Op::BitwiseNot:
            item.mCircuit = mLibrary.int_bitInvert(sizes[0]);
            item.mInputBundleCount = 1;
            break;
        case osuCrypto::Op::IfElse:
            if(sizes[0] != sizes[1] || sizes[0] != sizes[3])
                throw std::runtime_error("IfElse must be performed with variables of the same bit length. " LOCATION);
            if (sizes[2] != 1)
                throw std::runtime_error(LOCATION);

            item.mCircuit = mLibrary.int_int_multiplex(sizes[0]);
            item.mInputBundleCount = 3;
            break;
        default:
            throw std::runtime_error(LOCATION);
            break;
        }


        //process();

    }

    void ShGcRuntime::scheduleInput(
        RuntimeData* data, const BitVector& value)
    {
        mInputQueue.emplace();

        auto& enc = static_cast<ShGcRuntimeData*>(data)->mLabels;

        if (mRole == Evaluator)
        {
            mOtChoices.append(value);
            mOtCount += value.size();
        }

        auto& item = mInputQueue.back();

        item.mInputVal = value;
        item.mLabels = enc;

        //process();

    }

    void ShGcRuntime::scheduleInput(RuntimeData* data, u64 partyIdx)
    {
        auto& input = *static_cast<ShGcRuntimeData*>(data);

        if (mRole == Garbler)
        {
            mOtCount += input.mLabels->size();
        }


        mInputQueue.emplace();

        auto& item = mInputQueue.back();

        item.mLabels = input.mLabels;

        //process();

    }

    void ShGcRuntime::scheduleOutput(RuntimeData* data, u64 partyIdx)
    {
        auto& input = *static_cast<ShGcRuntimeData*>(data);

        mOutputQueue.emplace();

        auto& item = mOutputQueue.back();

        item.mLabels = input.mLabels;
        item.mOutputVal = nullptr;

        //process();

    }

    void ShGcRuntime::scheduleOutput(RuntimeData* data,
        std::future<BitVector>& future)
    {

        auto& input = *static_cast<ShGcRuntimeData*>(data);
        mOutputQueue.emplace();

        auto& item = mOutputQueue.back();

        item.mLabels = input.mLabels;
        item.mOutputVal = new std::promise<BitVector>();

        future = item.mOutputVal->get_future();

        //process();
    }

    void ShGcRuntime::processesQueue() 
    {
    
        // TODO: add logic to decide when to process the queue 
        // and when to simply queue things up. For now,
        // always keep the queue at size 1 or less.
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


    void ShGcRuntime::garblerInput()
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
                mAes.ecbEncCounterMode(mInputIdx, item.mLabels->size(), item.mLabels->data());
                mInputIdx += item.mLabels->size();

                std::unique_ptr<ByteStream> buff(new ByteStream(item.mLabels->size() * sizeof(block)));
                auto view = buff->getArrayView<block>();

                for (u64 i = 0; i < item.mLabels->size(); ++i)
                {
                    view[i] = (*item.mLabels)[i] ^ mZeroAndGlobalOffset[item.mInputVal[i]];
                }
                mChannel->asyncSend(std::move(buff));
            }
            else
            {
                std::unique_ptr<ByteStream> buff(new ByteStream(item.mLabels->size() * sizeof(block)));
                auto view = buff->getArrayView<block>();
                for (u64 i = 0; i < item.mLabels->size(); ++i, ++iter)
                {
                    (*item.mLabels)[i] = (*iter)[0];
                    view[i] = (*iter)[1] ^ (*iter)[0] ^ mGlobalOffset;
                }
                mChannel->asyncSend(std::move(buff));
            }

            mInputQueue.pop();
        }
    }

    void ShGcRuntime::evaluatorInput()
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

                for (u64 i = 0; i < item.mLabels->size(); ++i)
                {
                    (*item.mLabels)[i] = *iter++ ^ (zeroAndAllOnesBlk[item.mInputVal[i]] & view[i]);
                }
            }
            else
            {
                mChannel->recv(item.mLabels->data(), item.mLabels->size() * sizeof(block));
            }
            mInputQueue.pop();
        }
    }

    void ShGcRuntime::garblerCircuit()
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

                std::copy(item.mLabels[i]->begin(), item.mLabels[i]->end(), iter);
                iter += item.mLabels[i]->size();
            }


            std::unique_ptr<ByteStream> sendBuff(new ByteStream(item.mCircuit->mNonXorGateCount * sizeof(GarbledGate<2>)));

            auto gates = sendBuff->getArrayView<GarbledGate<2>>();


            garble(*item.mCircuit, sharedMem, mTweaks, gates);


            if(item.mCircuit->mNonXorGateCount)
                mChannel->asyncSend(std::move(sendBuff));


            for (u64 i = item.mInputBundleCount; i < item.mLabels.size(); ++i)
            {

                std::copy(iter, iter + item.mLabels[i]->size(), item.mLabels[i]->begin());
                iter += item.mLabels[i]->size();
            }

            mCrtQueue.pop();
        }


    }


    void ShGcRuntime::evaluatorCircuit()
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
                std::copy(item.mLabels[i]->begin(), item.mLabels[i]->end(), iter);
                iter += item.mLabels[i]->size();
            }


            sharedGates.resize(item.mCircuit->mNonXorGateCount);


            if(item.mCircuit->mNonXorGateCount)
                mChannel->recv(sharedGates.data(), sharedGates.size() * sizeof(GarbledGate<2>));


            evaluate(*item.mCircuit, sharedMem, mTweaks, sharedGates);


            for (u64 i = item.mInputBundleCount; i < item.mLabels.size(); ++i)
            {
                std::copy(iter, iter + item.mLabels[i]->size(), item.mLabels[i]->begin());
                iter += item.mLabels[i]->size();
            }


            //std::cout  << IoStream::lock;
            //for (auto ii = 0; ii < item.mLabels[2]->size(); ++ii)
            //{
            //    std::cout  << "e out[" << ii << "] " << (*item.mLabels[2])[ii] << std::endl;
            //}
            //std::cout  << IoStream::unlock;

            mCrtQueue.pop();
        }

    }


    void ShGcRuntime::garblerOutput()
    {

        while (mOutputQueue.size())
        {
            auto& item = mOutputQueue.front();


            if (item.mOutputVal)
            {
                if (sharedMem.size() < item.mLabels->size())
                {
                    sharedMem.resize(item.mLabels->size());
                }

                mChannel->recv(sharedMem.data(), item.mLabels->size() * sizeof(block));


                BitVector val(item.mLabels->size());


                for (u64 i = 0; i < item.mLabels->size(); ++i)
                {
                    if (neq(sharedMem[i], (*item.mLabels)[i]) && neq(sharedMem[i], (*item.mLabels)[i] ^ mGlobalOffset))
                    {
                        std::cout  << IoStream::lock << "output reveal error at " << i << ":\n   " << sharedMem[i] << "  != " << (*item.mLabels)[i]
                            << " (0) AND \n   " << sharedMem[i] << "  != " << ((*item.mLabels)[i] ^ mGlobalOffset) << std::endl << IoStream::unlock;

                        throw std::runtime_error(LOCATION);
                    }

                    //if (i == 1)
                    //{
                    //    std::cout  << IoStream::lock << "output reveal at " << i << ":\n   " << sharedMem[i] << "  ?= " << (*item.mLabels)[i]
                    //        << " (0) AND \n   " << sharedMem[i] << "  ?= " << ((*item.mLabels)[i] ^ mGlobalOffset) << std::endl << IoStream::unlock;

                    //}

                    val[i] = PermuteBit(sharedMem[i] ^ (*item.mLabels)[i]);
                }

                item.mOutputVal->set_value(val);
            }
            else
            {
                std::unique_ptr<BitVector> sendBuff(new BitVector(item.mLabels->size()));

                for (u64 i = 0; i < item.mLabels->size(); ++i)
                {
                    (*sendBuff)[i] = PermuteBit((*item.mLabels)[i]);
                }

                mChannel->asyncSend(std::move(sendBuff));
            }

            mOutputQueue.pop();
        }
    }

    void ShGcRuntime::evaluatorOutput()
    {
        while (mOutputQueue.size())
        {
            auto& item = mOutputQueue.front();

            if (item.mOutputVal)
            {
                BitVector val(item.mLabels->size());

                mChannel->recv(val);

                for (u64 i = 0; i < item.mLabels->size(); ++i)
                {
                    val[i] = val[i] ^ PermuteBit((*item.mLabels)[i]);
                }

                item.mOutputVal->set_value(val);
            }
            else
            {
                mChannel->asyncSendCopy(item.mLabels->data(), item.mLabels->size() * sizeof(block));
            }

            mOutputQueue.pop();
        }
    }




















    void ShGcRuntime::evaluate(
        BetaCircuit & cir,
        ArrayView<block> wires,
        std::array<block, 2>& tweaks,
        ArrayView<GarbledGate<2>> garbledGates)
    {
        auto garbledGateIter = garbledGates.begin();
        //std::cout  << IoStream::lock;

        //u64 i = 0;

        if (cir.mLevelGates.size() && 0)
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

                    c = a ^ b;
                }
                for (const auto& gate : level.mAndGates)
                {
                    auto& a = wires[gate.mInput[0]];
                    auto& b = wires[gate.mInput[1]];
                    auto& c = wires[gate.mOutput];

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

                if (gt == GateType::a)
                {
                    u64 src = gate.mInput[0];
                    u64 len = gate.mInput[1];
                    u64 dest = gate.mOutput;

                    memcpy(&*(wires.begin() + dest), &*(wires.begin() + src),i32( len * sizeof(block)));

                }
                else if (gt == GateType::Xor || gt == GateType::Nxor)
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


                    //std::cout  << "e "<<i<<" " << garbledTable[0] << " " << garbledTable[1] << std::endl;

                    zeroAndGarbledTable[1][0] = garbledTable[0];
                    zeroAndGarbledTable[1][1] = garbledTable[1] ^ a;



                    // compute the output wire label
                    c = hashs[0] ^
                        hashs[1] ^
                        zeroAndGarbledTable[PermuteBit(a)][0] ^
                        zeroAndGarbledTable[PermuteBit(b)][1];

                    //std::cout  << "e " << i++ << gateToString(gate.mType) << std::endl <<
                    //    " gt  " << garbledTable[0] << "  " << garbledTable[1] << std::endl <<
                    //    " t   " << tweaks[0] << "  " << tweaks[1] << std::endl <<
                    //    " a   " << a << std::endl <<
                    //    " b   " << b << std::endl <<
                    //    " c   " << c << std::endl;

                }

            }
        }

        //std::cout  << IoStream::unlock;

    }

    void ShGcRuntime::garble(
        BetaCircuit& cir,
        ArrayView<block> wires,
        std::array<block, 2>& tweaks,
        ArrayView<GarbledGate<2>> gates)
    {
        auto gateIter = gates.begin();
        //std::cout  << IoStream::lock;
        //u64 i = 0;

        if (cir.mLevelGates.size() & 0)
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

                        auto&  a1 = wires[gate1.mInput[0]];
                        auto&  b1 = wires[gate1.mInput[1]];
                        auto&  c1 = wires[gate1.mOutput];

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

                if (gt == GateType::a)
                {
                    u64 src =  gate.mInput[0];
                    u64 len =  gate.mInput[1];
                    u64 dest = gate.mOutput;

                    memcpy(&*(wires.begin() + dest), &*(wires.begin() + src), u32(len * sizeof(block)));

                }
                else if (gt == GateType::Xor || gt == GateType::Nxor)
                {
                    c = a ^ b ^ mZeroAndGlobalOffset[(u8)gt & 1];
                }
                else
                {
#ifndef  NDEBUG
                    if (gt == GateType::a ||
                        gt == GateType::b ||
                        gt == GateType::na ||
                        gt == GateType::nb ||
                        gt == GateType::One ||
                        gt == GateType::Zero)
                        throw std::runtime_error(LOCATION);
#endif // ! NDEBUG

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

                    //std::cout  << "g "<<i<<" " << garbledTable[0] << " " << garbledTable[1] << std::endl;


                    // compute the out wire
                    c = hash[aPermuteBit] ^
                        hash[2 ^ bPermuteBit] ^
                        mZeroAndGlobalOffset[cPermuteBit];

                    //std::cout  << "g " << i++  << gateToString(gate.mType) << std::endl <<
                    //    " gt  " << garbledTable[0] << "  " << garbledTable[1] << std::endl <<
                    //    " t   " << tweaks[0] << "  " << tweaks[1] << std::endl <<
                    //    " a   " << a << "  " << (a ^ mGlobalOffset) << std::endl <<
                    //    " b   " << b << "  " << (b ^ mGlobalOffset) << std::endl <<
                    //    " c   " << c << "  " << (c ^ mGlobalOffset) << std::endl;

                }

            }
        }

        for (u64 i = 0; i < cir.mOutputs.size(); ++i)
        {
            auto& out = cir.mOutputs[i].mWires;

            for (u64 j = 0; j < out.size(); ++j)
            {
                if (cir.mWireFlags[out[j]] == BetaWireFlag::InvWire)
                {
                    wires[out[j]] = wires[out[j]] ^ mGlobalOffset;
                }
            }
        }
        //std::cout  << IoStream::unlock;

    }


}