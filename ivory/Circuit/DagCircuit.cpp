#include "DagCircuit.h"
#include "Common/Log.h"
#include <stack>

namespace osuCrypto {



	DagCircuit::DagCircuit()
	{
	}


	DagCircuit::~DagCircuit()
	{
	}



	GateType invert(u64 i, const GateType& src)
	{
		if (i == 0)
		{
			// swap bit 0/1 and 2/3
			u8 s = (u8)src;

			return (GateType)(
				((s & 1) << 1) | // bit 0 -> bit 1
				((s & 2) >> 1) | // bit 1 -> bit 0
				((s & 4) << 1) | // bit 3 -> bit 4
				((s & 8) >> 1)); // bit 4 -> bit 3
		}
		else if (i == 1)
		{
			// swap bit (0,1)/(2,3)
			u8 s = (u8)src;

			return (GateType)(
				((s & 3) << 2) |  // bit (0,1) -> bit (2,3)
				((s & 12) >> 2)); // bit (2,3) -> bit (0,1)
		}
		else
			throw std::runtime_error("");
	}



	//bool GateNode::isReady()
	//{
	//	// we haven't added this gate yet, see if its ready for adding
	//	bool ready = true;
	//	for (auto input : mParents)
	//		ready &= input->mAdded;
	//	return ready;
	//}

	//void GateNode::add(Circuit& cir)
	//{
	//	if (mAdded == false && isReady())
	//	{
	//		u64 input0 = mParents[0]->mOutWireIdx;
	//		u64 input1 = mParents.size() == 2 ? mParents[1]->mOutWireIdx : 0;

	//		mOutWireIdx = cir.AddGate(input0, input1, mType);
	//		mAdded = true;

	//		// now recursively add any gates that newly became ready
	//		for (auto outGate : mChildren)
	//			outGate->add(cir);
	//	}
	//}


	void GateNode::invertInput(CircuitNode* input)
	{
		bool set = false;
		for (u64 i = 0; i < mParents.size(); ++i)
		{
			if (mParents[i] == input)
			{
				mType = invert(i, mType);
				set = true;
			}
		}
		if (set == false)
			throw std::runtime_error("");
	}

	void GateNode::invertOutput()
	{
		// this funtion invets the output of this gate and then inverts the inputs
		// of the downstream gates, effectively not changing the circuit. We do this 
		// to get of invert gates on output wires...

		mType = (GateType)((~(u8)mType) & 15);

		for (auto child : mChildren)
		{
			child->invertInput(this);
		}
	}

	void GateNode::reduceInvert(u64& numGates)
	{

		if (isInvert())
		{
			--numGates;

			// lets remove this node from the list of children
			*std::find(mParents[0]->mChildren.begin(), mParents[0]->mChildren.end(), this) = mParents[0]->mChildren.back();
			mParents[0]->mChildren.pop_back();
		}
		else
		{
			// lets remove any inverts on this gate's inputs by absorbing them into the truth table
			for (u64 i = 0; i < mParents.size(); ++i)
			{
				if (mParents[i]->isInvert())
				{
					auto newParent = mParents[i]->mParents[0];
					mType = invert(i, mType);

					//Log::out << "reduceInvert " << mWireIdx << "  on " << i << " par " << mParents[i]->wireIdx() << " new " << newParent->wireIdx() << Log::endl;


					if (newParent->isInvert())
						throw std::runtime_error("");

					newParent->mChildren.push_back(this);
					mParents[i] = newParent;
				}
			}
		}
	}


	void DagCircuit::readBris(std::istream& in)
	{
		if (in.eof())
			throw std::runtime_error("DagCircuit::readBris input istream is emprty");

		mNonInvertGateCount = 0;
		u64 numGates, outputCount, fanIn, fanOut, inputs[2], output, outputStartIdx;
		std::string gateType;

		in >> numGates >> mWireCount
			>> mInputCounts[0] >> mInputCounts[1]
			>> outputCount;

		//std::vector<GateNode*> invGates;
		outputStartIdx = mWireCount - outputCount;

		mOutputs.resize(outputCount);
		mNodes.resize(mWireCount);
		mGates.resize(numGates);
		mInputs.resize(InputWireCount());

		for (u64 i = 0; i < InputWireCount(); ++i)
		{
			mInputs[i].mWireIdx = i;// .emplace_back(i);
			mNodes[i] = &mInputs[i];
		}
		for (u64 i = InputWireCount(), j = 0; i < mNodes.size(); ++i, ++j)
			mNodes[i] = &mGates[j];

		// parse in the gates and construct a Dag with them
		for (u64 i = 0; i < numGates; ++i)
		{
			in >> fanIn >> fanOut;
			for (u64 j = 0; j < fanIn; ++j)
				in >> inputs[j];
			in >> output >> gateType;

			if (fanIn > 2) throw std::runtime_error("");
			if (fanOut != 1) throw std::runtime_error("");

			auto& gate = mGates[output - InputWireCount()];
			gate.mWireIdx = output;

			if (output >= outputStartIdx)
			{
				mOutputs[output - outputStartIdx].mParent = &gate;
			}

			gate.mParents.resize(fanIn);
			// add this gate to the doubly linked dag
			for (u64 j = 0; j < fanIn; ++j)
			{
				gate.mParents[j] = mNodes[inputs[j]];
				gate.mParents[j]->mChildren.push_back(&gate);
			}

			if (gateType == "XOR")
			{
				++mNonInvertGateCount;
				gate.mType = GateType::Xor;
			}
			else if (gateType == "AND")
			{
				++mNonInvertGateCount;
				gate.mType = GateType::And;
			}
			else if (gateType == "INV")
			{
				gate.mType = GateType::na;
			}
			else
				throw std::runtime_error("");
		}

	}

	void DagCircuit::writeBris(std::ostream& out)
	{
		out << mGates.size() << " " << mWireCount << std::endl;
		out << mInputCounts[0] << " " << mInputCounts[1] << " " << mOutputs.size() << std::endl;
		out << std::endl;

		for (auto& gate : mGates)
		{
			out << gate.mParents.size() << " " << 1 << " ";

			for (auto parent : gate.mParents)
				out << parent->wireIdx() << " ";

			out << gate.mWireIdx << " ";

			if (gate.type() == GateType::Xor)
				out << "XOR" << std::endl;
			else if (gate.type() == GateType::And)
				out << "AND" << std::endl;
			else if (gate.isInvert())
				out << "INV" << std::endl;
			else
				throw std::runtime_error("");
		}
	}

	void DagCircuit::removeInvertGates()
	{

		u64 numGates = mGates.size();

		for (auto& output : mOutputs)
		{
			if (output.mParent->isInvert())
			{
				auto newParent = output.mParent->mParents[0];
				// invert the trueth table of the gate before this invert gate. 
				// which allows us to remove this invert gate...
				newParent->invertOutput();

				// make sure that the output was inverted from na to a so that we can logically remove it
				if (output.mParent->type() != GateType::a)
					throw std::runtime_error("");

				// remove this node from the new parent
				*std::find(newParent->mChildren.begin(), newParent->mChildren.end(), output.mParent) = newParent->mChildren.back();
				newParent->mChildren.pop_back();

				// take the output of the previous gate 
				output.mParent = output.mParent->mParents[0];
			}
		}

		// remove all invert gates by absorbing them into the downstream logic tables
		for (auto& gate : mGates)
		{
			gate.reduceInvert(numGates);
		}

	}




	void depthFirstAdd(
		std::vector<u64>& newWireIdx,
		std::vector<bool>& added,
		std::stack<CircuitNode*>& addStack,
		Circuit& cir,
		bool explore)
	{
		CircuitNode* node = addStack.top();
		addStack.pop();

		if (added[node->wireIdx()] == false)
		{

			added[node->wireIdx()] = true;

			for (auto input : node->mParents)
			{
				if (added[input->wireIdx()] == false)
				{
					//Log::out << "-";
					addStack.push(input);
					depthFirstAdd(newWireIdx, added, addStack, cir, false);
				}
			}


			//Log::out << "#";
			if (node->mParents.size() == 1)
				newWireIdx[node->wireIdx()] = cir.AddGate(
					newWireIdx[node->mParents[0]->wireIdx()],
					0, node->type());
			else
				newWireIdx[node->wireIdx()] = cir.AddGate(
					newWireIdx[node->mParents[0]->wireIdx()],
					newWireIdx[node->mParents[1]->wireIdx()],
					node->type());

		}

		for (auto child : node->mChildren)
		{
			if (added[child->wireIdx()] == false)
			{
				addStack.push(child);
				if (explore)
				{
					depthFirstAdd(newWireIdx, added, addStack, cir, true);
				}
			}
		}
	}

	void DagCircuit::toCircuit(Circuit& cir)
	{

		std::vector<u64> newWireIdx(mWireCount, (u64)-1);
		std::vector<bool> added(mWireCount);
		//std::vector<bool> explored(mWireCount);

		std::stack<CircuitNode*> addStack;

		for (u64 i = 0; i < InputWireCount(); ++i)
		{
			added[i] = true;
			newWireIdx[i] = i;

			addStack.push(&mInputs[i]);
		}

		cir.mInputs[0] = mInputCounts[0];
		cir.mInputs[1] = mInputCounts[1];
		cir.mWireCount = InputWireCount();

		while (addStack.size())
			depthFirstAdd(newWireIdx, added, addStack, cir, true);



		//for (auto& input : mInputs)
		//{
		//	for (auto gate : input.mChildren)
		//	{
		//		if(added[gate->wireIdx()] == false)
		//	}
		//}

		for (u64 i = 0; i < mOutputs.size(); ++i)
		{
			u64 idx = mOutputs[i].mParent->wireIdx();
			cir.AddOutputWire(newWireIdx[idx]);
		}
	}
}
