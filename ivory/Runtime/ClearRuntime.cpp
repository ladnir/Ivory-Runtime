#include "ClearRuntime.h"

namespace osuCrypto
{
    ClearRuntime::ClearRuntime()
    {
    }


    ClearRuntime::~ClearRuntime()
    {
    }

    void ClearRuntime::initVar(std::unique_ptr<RuntimeData>& data, u64 bitCount)
    {
        if (data) throw std::runtime_error(LOCATION);

        data.reset(new ClearIntRuntimeData(bitCount));
    }

    void ClearRuntime::copyVar(std::unique_ptr<RuntimeData>& data, RuntimeData * copy)
    {
        auto& src = *static_cast<ClearIntRuntimeData*>(copy);
        data.reset(new ClearIntRuntimeData(src.mBitCount));

        auto& dest = *static_cast<ClearIntRuntimeData*>(data.get());

        dest.mVal = src.mVal;
    }

    void ClearRuntime::init(Channel & chl, block seed, u64 partyIdx)
    {

        mChannel = &chl;
        mPartyIdx = partyIdx;
    }

    void ClearRuntime::scheduleInput(RuntimeData* data, const BitVector & value)
    {
        auto& input = *static_cast<ClearIntRuntimeData*>(data);

        if (mPartyIdx)
        {
            mChannel->send(value);
        }
        else
        {
            if (value.size() != input.mBitCount) throw std::runtime_error(LOCATION);

            memcpy(&input.mVal, value.data(), value.sizeBytes());
        }
    }

    void ClearRuntime::scheduleInput(RuntimeData* data, u64 pIdx)
    {

        auto& input = *static_cast<ClearIntRuntimeData*>(data);

        if (mPartyIdx == 0)
        {

            BitVector value(input.mBitCount);

            mChannel->recv(value);

            memcpy(&input.mVal, value.data(), value.sizeBytes());
        }
    }


    void ClearRuntime::scheduleOp(Op op, ArrayView<RuntimeData*> io)
    {


        switch (op)
        {
        case osuCrypto::Op::Add:
        {

            if (io.size() != 3) throw std::runtime_error(LOCATION);

            auto& a = *static_cast<ClearIntRuntimeData*>(io[0]);
            auto& b = *static_cast<ClearIntRuntimeData*>(io[1]);
            auto& c = *static_cast<ClearIntRuntimeData*>(io[2]);

            c.mVal = a.mVal + b.mVal;

            // clear top bits in case of carry.
            c.mVal &= ((u64(1) << c.mBitCount) - 1);

            break;
        }
        case osuCrypto::Op::Subtract:
            throw std::runtime_error(LOCATION);
            break;
        case osuCrypto::Op::Multiply:
        {
            if (io.size() != 3) throw std::runtime_error(LOCATION);

            auto& a = *static_cast<ClearIntRuntimeData*>(io[0]);
            auto& b = *static_cast<ClearIntRuntimeData*>(io[1]);
            auto& c = *static_cast<ClearIntRuntimeData*>(io[2]);

            c.mVal = a.mVal * b.mVal;

            // clear top bits in case of carry.
            c.mVal &= ((u64(1) << c.mBitCount) - 1);

            break;
        }
        case osuCrypto::Op::Divide:
            throw std::runtime_error(LOCATION);
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
        case osuCrypto::Op::BitWiseAnd:
            throw std::runtime_error(LOCATION);
            break;
        case osuCrypto::Op::BitWiseOr:
            throw std::runtime_error(LOCATION);
            break;
        default:
            throw std::runtime_error(LOCATION);
            break;
        }

    }


    void ClearRuntime::scheduleOutput(RuntimeData* data, u64 partyIdx)
    {
        auto& out = *static_cast<ClearIntRuntimeData*>(data);

        if (partyIdx && mPartyIdx == 0)
        {
            mChannel->asyncSendCopy(&out.mVal, sizeof(u64));
        }

    }

    void ClearRuntime::scheduleOutput(RuntimeData* data, std::future<BitVector> & value)
    {

        auto& out = *static_cast<ClearIntRuntimeData*>(data);

        if (mPartyIdx)
        {
            mChannel->recv(&out.mVal, sizeof(u64));
        }

        std::promise<BitVector> prom;

        BitVector val;

        val.append((u8*)&out.mVal, out.mBitCount);

        value = prom.get_future();
        prom.set_value(val);

    }
}
