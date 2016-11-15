#pragma once
#include "Common/Defines.h"
#include "Common/ArrayView.h"
#include "Circuit/Gate.h"
namespace osuCrypto
{

	class CircuitStream
	{
	public:
		CircuitStream();
		~CircuitStream();

		virtual bool hasMoreGates() = 0;
		virtual ArrayView<Gate> getMoreGates() = 0;
		virtual ArrayView<u64> getOutputIndices() = 0;
		virtual ArrayView<u64> getInputIndices() = 0;

		virtual u64 getInternalWireBuffSize() const = 0;
		virtual u64 getInputWireBuffSize() const = 0;
		virtual u64 getNonXorGateCount() const = 0;
	};

}
