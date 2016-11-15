#pragma once
#include "VerilogTokenizer.h"
#include "Circuit/SequentialCircuit.h"
#include <unordered_map>

namespace osuCrypto
{


	class VerilogParser
	{
	public:
		VerilogParser();
		~VerilogParser();

		void init(std::istream& in);

		void parse(SequentialCircuit& out);






	private:
		void sort();



		std::string printError(std::string msg, std::string loc);

		void startModule();
		void startModuleNode();
		void startWire();
		void startInput();
		void startOutput();
		void startAssign();

		//void parseGateVars(std::string& para1, std::string& para2, std::string& out);
		void parseVarName(std::string& name, bool useNext = true);
		void parseVarName(std::vector<std::string>& names);
		void parseGateParams(const std::vector<std::string>& labels, std::vector<std::string>& param);


		void addOutputWire(const std::string& name);
		void addInputWire(const std::string& name);
		void addInvert(const std::string& in, const std::string& out);
		void addGate(const std::string& in1, const std::string& in2, const std::string& out, GateType type);
		void addAlias(const std::string& alias, const std::string& src);

		//struct Name { Name(u64 idx) :mIdx(idx), mAssigned(false) {} u64 mIdx; bool mAssigned; };
		std::unordered_map < std::string, u64> mNameMap;
		
		//std::vector<std::string> mInputNames;
		std::vector< u64> mReadyNodes, mOutputs;

		u64 mOutputIdx;

		struct Node
		{
			Node()
			{
				mType = GateType::Zero;
				mInput0 = mInput1 = mWireIdx = 0;
				mInCount = 2;
				mOutIdx = 0;
			}
			GateType mType;
			
			u64 mInput0, mInput1, mWireIdx, mInCount, mOutIdx;
			std::vector<u64> mChildren;

			std::string mName;
		};
		std::vector<Node> mNodes;

		SequentialCircuit* mCir;
		VerilogTokenizer mTkz;
	};

}
