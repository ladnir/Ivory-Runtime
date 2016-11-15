#pragma once
#include "Circuit/Gate.h"
#include "Circuit/Circuit.h"
#include "Common/Defines.h"


namespace osuCrypto {


	class CircuitNode
	{
	public:
		virtual u64 wireIdx() = 0; 
		virtual GateType& type() = 0;
		virtual bool isInvert() = 0;
		virtual void invertOutput() = 0;
		virtual void invertInput(CircuitNode* input) = 0;

		std::vector<CircuitNode*> mChildren, mParents;

	};

	class InputNode : public CircuitNode
	{
	public:
		u64 mWireIdx;

		void invertInput(CircuitNode* input) override { throw std::runtime_error(""); }
		void invertOutput() override { throw std::runtime_error(""); }
		u64 wireIdx() override { return mWireIdx; }
		GateType& type() override { throw std::runtime_error(""); }
		bool isInvert() override { return false; }
	};

	class OutputNode //: public CircuitNode
	{
	public: 

		CircuitNode * mParent;
	};

	class GateNode :public CircuitNode
	{
	public: 
		GateType mType;
		u64 mWireIdx;

		//bool isReady();

		//void add(Circuit& cir);
		void reduceInvert(u64& numGates);

		void invertInput(CircuitNode* input) override;
		void invertOutput() override;
		bool isInvert() override { return mType == GateType::na && mParents.size() == 1; }
		u64 wireIdx() override { return mWireIdx; }
		GateType& type() override { return mType; }
	};


	class DagCircuit
	{
	public:
		DagCircuit();
		~DagCircuit();

		void readBris(std::istream& in);

		void writeBris(std::ostream& out);


		void removeInvertGates();

		void toCircuit(Circuit& cir);

		u64 mWireCount, mNonInvertGateCount;
		u64 mInputCounts[2];
		std::vector<InputNode> mInputs;
		std::vector<OutputNode> mOutputs;
		std::vector<GateNode> mGates;

		std::vector<CircuitNode*> mNodes;

		u64 InputWireCount() { return mInputCounts[0] + mInputCounts[1]; }

	};

}