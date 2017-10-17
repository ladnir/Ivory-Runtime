#include "sIntClear.h"
#include <ivory/Runtime/Clear/ClearRuntime.h>
namespace osuCrypto
{

    i64 clear::signExtend(i64 v, u64 bitIdx)
    {
        u8 bit = (v >> bitIdx) & 1;
        u64 topMask = bit * (u64(-1) << (64 - bitIdx));
        u64 botMask = ~topMask;

        return (v & botMask) | topMask;
    }



    ClearInt::ClearInt(ClearRuntime & rt, u64 bc)
        : mRt(rt)
        , mBitCount(bc)
    { }


    void ClearInt::copy(sIntBasePtr & c)
    {
        auto& cc = static_cast<ClearInt&>(*c.get());
        mValue =  clear::signExtend(cc.mValue, mBitCount);
    }
    u64 ClearInt::bitCount()
    {
        return mBitCount;
    }
    Runtime & ClearInt::getRuntime()
    {
        return mRt;
    }
    sIntBasePtr ClearInt::add(sIntBasePtr & b)
    {
        auto ret = new ClearInt()
    }
}

