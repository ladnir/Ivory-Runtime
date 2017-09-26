#pragma once
#include <memory>
#include <vector>
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/Matrix.h"
#include "ivory/Circuit/AlphaCircuit.h"

namespace osuCrypto
{
	namespace Thunder
	{
		struct TMemory
		{
			TMemory() = default;
			TMemory(const TMemory&) = default;
			TMemory(u32 c) :mBitCount(c) {}
			//_Memory(u64 size)
			//	: mData(new u64[size])
			//{ }
			//gsl::multi_span<u64, 2,gsl::dynamic_range> mData;
			std::array<u64, 2> mData;
			//bool nIsPublic;
			u32 mBitCount;
		};
		using Memory = std::shared_ptr<TMemory>;

		struct WorkItem
		{
			WorkItem() = default;
			WorkItem(WorkItem&&) = default;

			AlphaCircuit* mCircuit;
			std::vector<Memory> mMemory;
			u16 mInputBundleCount, mParentCounts;
			std::vector<WorkItem*> mChildren;
		};

		struct InputItem
		{
			Memory mMem;
		};
	}

}