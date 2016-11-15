#include "VerilogParser.h"
#include "Common/Log.h"
namespace osuCrypto
{


	VerilogParser::VerilogParser()
	{
	}


	VerilogParser::~VerilogParser()
	{
	}
	void VerilogParser::init(std::istream & in)
	{
		mOutputIdx = 0;
		mTkz.init(in);
	}
	void VerilogParser::parse(SequentialCircuit & out)
	{
		mCir = &out;

		startModule();



		sort();
	}

	void VerilogParser::sort()
	{

		Log::out << "sorted... " << Log::endl;
		/*
				std::vector<u64> sortedNodes;
				sortedNodes.reserve(mNodes.size());*/
		std::vector<u64> freeIdxs;// , sortedReadyNodes;
		u64 nextIdx = 0;

		//sortedReadyNodes.reserve(mReadyNodes.size());

		//for (u64 i = 0; i < mReadyNodes.size(); ++i)
		//{
		//	auto idx = mReadyNodes[i];

		//	if (idx != -1)
		//	{
		//		auto& node = mNodes[idx];


		//		auto var = node.mChildren[0];



		//	}

		//}

		//mCir->setInputSize(mReadyNodes.size());

		while (mReadyNodes.size())
		{
			auto nodeIdx = mReadyNodes.back();
			mReadyNodes.pop_back();




			if (mNodes[nodeIdx].mType != GateType::Zero)
			{



				auto& p0 = mNodes[mNodes[nodeIdx].mInput0];

				// if this parent is an input node, see if we still need to schedule it.
				bool scheduleP0 = (p0.mType == GateType::Zero) && (p0.mInCount-- == 0);
				if (scheduleP0)
				{
					if (freeIdxs.size() == 0)
						freeIdxs.push_back(nextIdx++);

					auto newIdx = freeIdxs.back();
					freeIdxs.pop_back();

					//Log::out << "scheduling p0 " << p0.mWireIdx << " @ " << newIdx << Log::endl;


					mCir->addInputWire(p0.mWireIdx, newIdx);
					p0.mWireIdx = newIdx;
				}

				// ok, lets see if we can free the parents of this node. 
				// note: that its ok to store the child in the parent if this is the last child of it.
				auto pos = std::find(p0.mChildren.begin(), p0.mChildren.end(), mNodes[nodeIdx].mWireIdx);
				if (pos == p0.mChildren.end()) throw std::runtime_error("");
				p0.mChildren.erase(pos);



				u64 input0 = p0.mWireIdx;
				u64 input1;


				if (mNodes[nodeIdx].mType != GateType::na)
				{
					auto& p1 = mNodes[mNodes[nodeIdx].mInput1];

					// if this parent is an input node, see if we still need to schedule it.
					bool scheduleP1 = (p1.mType == GateType::Zero) && (p1.mInCount-- == 0);
					if (scheduleP1)
					{
						if (freeIdxs.size() == 0)
							freeIdxs.push_back(nextIdx++);


						auto newIdx = freeIdxs.back();
						freeIdxs.pop_back();


						//Log::out << "scheduling p1 " << p1.mWireIdx << " @ " << newIdx << Log::endl;

						mCir->addInputWire(p1.mWireIdx, newIdx);
						p1.mWireIdx = newIdx;
					}

					// OK, see if we can free the parents of this node
					pos = std::find(p1.mChildren.begin(), p1.mChildren.end(), mNodes[nodeIdx].mWireIdx);
					if (pos == p1.mChildren.end()) throw std::runtime_error("");
					p1.mChildren.erase(pos);

					if (p1.mChildren.size() == 0)
						freeIdxs.push_back(p1.mWireIdx);

					input1 = p1.mWireIdx;
				}
				else
				{
					input1 = -1;
				}



				if (p0.mChildren.size() == 0)
					freeIdxs.push_back(p0.mWireIdx);



				if (freeIdxs.size() == 0)
					freeIdxs.push_back(nextIdx++);

				// reindex it
				mNodes[nodeIdx].mWireIdx = freeIdxs.back();
				freeIdxs.pop_back();
				mCir->addGate(input0, input1, mNodes[nodeIdx].mWireIdx, mNodes[nodeIdx].mType);


				if (mNodes[nodeIdx].mOutIdx)
				{
					mCir->addOutputWire(mNodes[nodeIdx].mOutIdx - 1, mNodes[nodeIdx].mWireIdx);
				}
			}





			// mark the child as beinging ready
			for (auto childIdx : mNodes[nodeIdx].mChildren)
			{
				auto& child = mNodes[childIdx];


				if (--child.mInCount == 0)
				{
					// enque this child to be scheduled
					mReadyNodes.push_back(childIdx);
				}
			}
		}

		for (u64 i = 0; i < mNodes.size(); ++i)
		{
			if (mNodes[i].mChildren.size())
			{
				Log::out << "cycle found " << i << Log::endl;
				throw std::runtime_error("cycle found");
			}
		}
		mCir->mWireCount = nextIdx;

		Log::out << "max working space = " << nextIdx << Log::endl;
	}

	std::string VerilogParser::printError(std::string msg, std::string loc)
	{
		std::stringstream ss;
		auto charNum = mTkz.curTokenCharIdx();
		auto lineIdx = mTkz.curTokenLineIdx();
		auto line = mTkz.curTokenLine();

		Log::out
			<< "While reading Line# " << lineIdx << "  char# " << charNum << " the Parser encountered an error." << Log::endl
			<< Log::endl
			<< "Line: " << line << Log::endl
			<< std::string(charNum + 6, ' ') << Log::Color::Red << "^" << Log::ColorDefault << Log::endl
			<< Log::endl
			<< "Message: " << msg << Log::endl
			<< "Thrown from: " << loc << Log::endl;

		return  "while reading Line " + std::to_string(lineIdx) + " the Parser encountered an error.";
	}

	void VerilogParser::startModule()
	{

		if (mTkz.curToken() != VlogToken::Module)
			throw std::runtime_error(printError("Expecting module token", LOCATION));


		if (mTkz.nextToken() != VlogToken::Word)
			throw std::runtime_error(printError("Expecting module name token", LOCATION));

		if (mTkz.nextToken() != VlogToken::Paren)
			throw std::runtime_error(printError("Expecting module paren open", LOCATION));



		if (mTkz.nextToken() != VlogToken::Word || mTkz.curWord() != "clk")
			throw std::runtime_error(printError("Expecting module clk variable", LOCATION));

		if (mTkz.nextToken() != VlogToken::Comma)
			throw std::runtime_error(printError("Expecting module param list comma", LOCATION));



		if (mTkz.nextToken() != VlogToken::Word || mTkz.curWord() != "rst")
			throw std::runtime_error(printError("Expecting module rst variable", LOCATION));

		if (mTkz.nextToken() != VlogToken::Comma)
			throw std::runtime_error(printError("Expecting module param list comma", LOCATION));



		if (mTkz.nextToken() != VlogToken::Word || (mTkz.curWord() != "g_input" && mTkz.curWord() != "g_init"))
			throw std::runtime_error(printError("Expecting module g_input variable", LOCATION));

		if (mTkz.nextToken() != VlogToken::Comma)
			throw std::runtime_error(printError("Expecting module param list comma", LOCATION));



		if (mTkz.nextToken() != VlogToken::Word || (mTkz.curWord() != "e_input" && mTkz.curWord() != "e_init"))
			throw std::runtime_error(printError("Expecting module e_input variable", LOCATION));

		if (mTkz.nextToken() != VlogToken::Comma)
			throw std::runtime_error(printError("Expecting module param list comma", LOCATION));




		if (mTkz.nextToken() != VlogToken::Word || mTkz.curWord() != "o")
			throw std::runtime_error(printError("Expecting module o variable", LOCATION));

		if (mTkz.nextToken() != VlogToken::EndParen)
			throw std::runtime_error(printError("Expecting module param list end paren", LOCATION));



		if (mTkz.nextToken() != VlogToken::Semicolon)
			throw std::runtime_error(printError("Expecting module param list end semicolon", LOCATION));



		while (mTkz.nextToken() != VlogToken::EndModule)
		{
			if (mTkz.curToken() == VlogToken::eof)
				throw std::runtime_error(printError("Expecting endmodule token before eof (end of file) token", LOCATION));

			startModuleNode();
		}



		if (mTkz.nextToken() != VlogToken::eof)
			throw std::runtime_error(printError("Expecting eof (end of file) token", LOCATION));

	}
	std::vector<std::string> params(6);

	void VerilogParser::startModuleNode()
	{
		switch (mTkz.curToken())
		{
		case VlogToken::Wire:
			startWire();
			break;
		case VlogToken::Input:
			startInput();
			break;
		case VlogToken::Output:
			startOutput();
			break;
		case VlogToken::Assign:
			startAssign();
			break;
		case VlogToken::Word:
		{

			if (mTkz.curWord() == "OR" || mTkz.curWord() == "Or")
			{
				parseGateParams({"A","B","Z"}, params);
				addGate(params[0], params[1], params[2], GateType::Or);
				break;
			}
			else if (mTkz.curWord() == "AND" || mTkz.curWord() == "And")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::And);
				break;
			}
			else if (mTkz.curWord() == "XNOR" || mTkz.curWord() == "Nxor")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::Nxor);
				break;
			}
			else if (mTkz.curWord() == "XOR" || mTkz.curWord() == "Xor")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::Xor);
				break;
			}
			else if (mTkz.curWord() == "NOR" || mTkz.curWord() == "Nor")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::Nor);
				break;
			}
			else if (mTkz.curWord() == "NAND" || mTkz.curWord() == "Nand")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::Nand);
				break;
			}
			else if (mTkz.curWord() == "ORN" || mTkz.curWord() == "na_Or")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::na_Or);
				break;
			}
			else if (mTkz.curWord() == "NORN" || mTkz.curWord() == "nb_And")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::nb_And);
				break;
			}
			else if (mTkz.curWord() == "NANDN" || mTkz.curWord() == "nb_Or")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::nb_Or);
				break;
			}
			else if (mTkz.curWord() == "ANDN" || mTkz.curWord() == "na_And")
			{
				parseGateParams({ "A","B","Z" }, params);
				addGate(params[0], params[1], params[2], GateType::na_And);
				break;
			}
			else if (mTkz.curWord() == "IV")
			{
				parseGateParams({ "A","Z" }, params);
				addInvert(params[0], params[1]);
				break;
			}
			else if (mTkz.curWord() == "DFF")
			{
				parseGateParams({ "D","CLK","RST", "I", "Q" }, params);
				Log::out << "implement DFF" << Log::endl;
				break;
			}
			else if(mTkz.curWord() == "MUX")
			{
				parseGateParams({ "IN0", "IN1","SEL","F" }, params);
				Log::out << "implement MUX" << Log::endl;
				break;
			}
		}
		default:
			throw std::runtime_error(printError("Invalid module node token. EXpecting {wire, input, output, OR, AND, ...}", LOCATION));
			break;
		}

	}

	void VerilogParser::startWire()
	{

		if (mTkz.nextToken() == VlogToken::SBracket)
		{
			if (mTkz.nextToken() != VlogToken::Number)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting a number.", LOCATION));

			auto upperVal = mTkz.curNumber();


			if (mTkz.nextToken() != VlogToken::Colon)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting a colon.", LOCATION));

			if (mTkz.nextToken() != VlogToken::Number)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting a number.", LOCATION));

			auto lowerVal = mTkz.curNumber();


			if (mTkz.nextToken() != VlogToken::EndSBracket)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting close SBracker.", LOCATION));


			if (mTkz.nextToken() != VlogToken::Word)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting var name.", LOCATION));

			auto name = mTkz.curWord();


			//for(u64 i = lowerVal; i < upperVal; ++i)
			//	addAlias(name + "[" + std::to_string(i) + "]")


			if (mTkz.nextToken() != VlogToken::Semicolon)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting semicolon.", LOCATION));

		}
		else
		{
			if (mTkz.curToken() != VlogToken::Word)
				throw std::runtime_error(printError("Invalid wire node. Expecting var name.", LOCATION));

			auto name = mTkz.curWord();

			while (mTkz.nextToken() == VlogToken::Comma)
			{
				if (mTkz.nextToken() != VlogToken::Word)
					throw std::runtime_error(printError("Invalid input wire node. Expecting var name.", LOCATION));


				const auto& name = mTkz.curWord();

				//if (name != "clk" && name != "rst")
				//	addInputWire(name);
			}

			if (mTkz.curToken() != VlogToken::Semicolon)
				throw std::runtime_error(printError("Invalid wire node. Expecting semicolon.", LOCATION));

		}
	}

	void VerilogParser::startInput()
	{

		if (mTkz.nextToken() == VlogToken::SBracket)
		{
			if (mTkz.nextToken() != VlogToken::Number)
				throw std::runtime_error(printError("Invalid input wire node. Expecting a number.", LOCATION));

			auto upperVal = mTkz.curNumber();


			if (mTkz.nextToken() != VlogToken::Colon)
				throw std::runtime_error(printError("Invalid input wire node. Expecting a colon.", LOCATION));

			if (mTkz.nextToken() != VlogToken::Number)
				throw std::runtime_error(printError("Invalid input wire node. Expecting a number.", LOCATION));

			auto lowerVal = mTkz.curNumber();

			if (mTkz.nextToken() != VlogToken::EndSBracket)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting close SBracker.", LOCATION));


			if (mTkz.nextToken() != VlogToken::Word)
				throw std::runtime_error(printError("Invalid input wire node. Expecting var name.", LOCATION));



			const auto& name = mTkz.curWord();

			for (u64 i = lowerVal; i <= upperVal; ++i)
				addInputWire(name + "[" + std::to_string(i) + "]");


			if (mTkz.nextToken() != VlogToken::Semicolon)
				throw std::runtime_error(printError("Invalid input wire node. Expecting semicolon.", LOCATION));

		}
		else
		{
			if (mTkz.curToken() != VlogToken::Word)
				throw std::runtime_error(printError("Invalid input wire node. Expecting var name.", LOCATION));

			const auto& name = mTkz.curWord();

			if (name != "clk" && name != "rst")
				addInputWire(name);

			while (mTkz.nextToken() == VlogToken::Comma)
			{
				if (mTkz.nextToken() != VlogToken::Word)
					throw std::runtime_error(printError("Invalid input wire node. Expecting var name.", LOCATION));


				const auto& name = mTkz.curWord();

				if (name != "clk" && name != "rst")
					addInputWire(name);
			}

			if (mTkz.curToken() != VlogToken::Semicolon)
				throw std::runtime_error(printError("Invalid input wire node. Expecting semicolon.", LOCATION));

		}

	}

	void VerilogParser::startOutput()
	{
		if (mTkz.nextToken() == VlogToken::SBracket)
		{
			if (mTkz.nextToken() != VlogToken::Number)
				throw std::runtime_error(printError("Invalid output wire node. Expecting a number.", LOCATION));

			auto upperVal = mTkz.curNumber();


			if (mTkz.nextToken() != VlogToken::Colon)
				throw std::runtime_error(printError("Invalid output wire node. Expecting a colon.", LOCATION));

			if (mTkz.nextToken() != VlogToken::Number)
				throw std::runtime_error(printError("Invalid output wire node. Expecting a number.", LOCATION));

			auto lowerVal = mTkz.curNumber();



			if (mTkz.nextToken() != VlogToken::EndSBracket)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting close SBracker.", LOCATION));


			if (mTkz.nextToken() != VlogToken::Word)
				throw std::runtime_error(printError("Invalid output wire node. Expecting var name.", LOCATION));

			const auto& name = mTkz.curWord();

			for (u64 i = lowerVal; i <= upperVal; ++i)
				addOutputWire(name + "[" + std::to_string(i) + "]");


			if (mTkz.nextToken() != VlogToken::Semicolon)
				throw std::runtime_error(printError("Invalid output wire node. Expecting semicolon.", LOCATION));

		}
		else
		{
			if (mTkz.nextToken() != VlogToken::Word)
				throw std::runtime_error(printError("Invalid output wire node. Expecting var name.", LOCATION));

			const auto& name = mTkz.curWord();
			addOutputWire(name);

			if (mTkz.nextToken() != VlogToken::Semicolon)
				throw std::runtime_error(printError("Invalid output wire node. Expecting semicolon.", LOCATION));

		}


	}


	void VerilogParser::startAssign()
	{
		std::vector<std::string> dest;
		parseVarName(dest);

		if (mTkz.curToken() != VlogToken::Eq)
			throw std::runtime_error(printError("Invalid assign node. Expecting Eq.", LOCATION));

		std::vector<std::string> src;
		parseVarName(src);

		if (dest.size() == src.size())
		{
			for (u64 i = 0; i < dest.size(); ++i)
			{
				addAlias(dest[0], src[0]);
			}
		}
		else
		{
			if (dest.size() != 1)
				throw std::runtime_error(printError("Invalid assign node. Destination must have the same dimension as src.", LOCATION));

			for (u64 i = 0; i < src.size(); ++i)
			{
				addAlias(dest[0] + "[" + std::to_string(i) + "]", src[i]);
			}

		}

		if (mTkz.curToken() != VlogToken::Semicolon)
			throw std::runtime_error(printError("Invalid assign node. Expecting Semicolon.", LOCATION));

	}


	void VerilogParser::parseVarName(std::string &para1, bool useNext)
	{
		auto tk = useNext ? mTkz.nextToken() : mTkz.curToken();

		if (tk != VlogToken::Word)
			throw std::runtime_error(printError("Invalid gate node. Expecting gate param .A  var name.", LOCATION));

		para1 = mTkz.curWord();

		if (mTkz.nextToken() == VlogToken::SBracket)
		{
			if (mTkz.nextToken() != VlogToken::Number)
				throw std::runtime_error(printError("Invalid gate node. Expecting input wire index.", LOCATION));

			para1 += "[" + std::to_string(mTkz.curNumber()) + "]";


			if (mTkz.nextToken() != VlogToken::EndSBracket)
				throw std::runtime_error(printError("Invalid gate node. Expecting gate param index end bracket.", LOCATION));


			mTkz.nextToken();
		}
	}

	void VerilogParser::parseVarName(std::vector<std::string>& names)
	{

		if (mTkz.nextToken() != VlogToken::Word)
			throw std::runtime_error(printError("Invalid multi wire node. Expecting var name.", LOCATION));

		auto name = mTkz.curWord();


		if (mTkz.nextToken() == VlogToken::SBracket)
		{
			if (mTkz.nextToken() != VlogToken::Number)
				throw std::runtime_error(printError("Invalid multi wire node. Expecting a number.", LOCATION));

			auto upperVal = mTkz.curNumber();


			if (mTkz.nextToken() == VlogToken::Colon)
			{
				if (mTkz.nextToken() != VlogToken::Number)
					throw std::runtime_error(printError("Invalid multi wire node. Expecting a number.", LOCATION));

				auto lowerVal = mTkz.curNumber();

				for (u64 i = lowerVal; i <= upperVal; ++i)
				{
					names.push_back(name + "[" + std::to_string(i) + "]");
				}


				if (mTkz.nextToken() != VlogToken::EndSBracket)
					throw std::runtime_error(printError("Invalid multi wire node. Expecting close SBracker.", LOCATION));

			}
			else
			{
				names.push_back(name + "[" + std::to_string(upperVal) + "]");


				if (mTkz.curToken() != VlogToken::EndSBracket)
					throw std::runtime_error(printError("Invalid multi wire node. Expecting close SBracker.", LOCATION));

			}


			mTkz.nextToken();

		}
		else
		{
			names.push_back(name);
		}

	}

	void VerilogParser::parseGateParams(
		const std::vector<std::string>& labels, 
		std::vector<std::string>& params)
	{
		std::string temp;
		parseVarName(temp);

		if (mTkz.curToken() != VlogToken::Paren)
			throw std::runtime_error(printError("Invalid gate node. Expecting open paren.", LOCATION));

		//  B param
		for (u64 i = 0; i < labels.size(); ++i)
		{

			if(i && mTkz.nextToken() != VlogToken::Comma)
				throw std::runtime_error(printError("Invalid gate node. Expecting comma before gate param label.", LOCATION));


			if (mTkz.nextToken() != VlogToken::Dot)
				throw std::runtime_error(printError("Invalid gate node. Expecting dot before gate param label.", LOCATION));


			if (mTkz.nextToken() != VlogToken::Word)
				throw std::runtime_error(printError("Invalid gate node. Expecting gate param label.", LOCATION));

			const auto& label = mTkz.curWord();

			auto iter = std::find(labels.begin(), labels.end(), label);
			if(iter == labels.end())
				throw std::runtime_error(printError("Invalid gate node. Unknown label " + label + ".", LOCATION));

			auto idx = iter - labels.begin();

			if (mTkz.nextToken() != VlogToken::Paren)
				throw std::runtime_error(printError("Invalid gate node. Expecting open paren.", LOCATION));

			if (mTkz.nextToken() == VlogToken::Number)
			{

				if (mTkz.curNumber() != 1)
					throw std::runtime_error(printError("Exepecting 1'b0 or 1'b1", LOCATION));

				if (mTkz.nextToken() != VlogToken::SingleQuote)
					throw std::runtime_error(printError("Exepecting 1'b0 or 1'b1", LOCATION));

				if (mTkz.nextToken() != VlogToken::Word)
					throw std::runtime_error(printError("Exepecting 1'b0 or 1'b1", LOCATION));

				if (mTkz.curWord() != "b0")
				{
					params[idx] = "1'b0";
				}
				else if (mTkz.curWord() != "b1")
				{
					params[idx] = "1'b1";
				}
				else
				{
					throw std::runtime_error(printError("Exepecting 1'b0 or 1'b1", LOCATION));
				}

				mTkz.nextToken();
			}
			else
			{
				parseVarName(params[idx], false);
			}

			if (mTkz.curToken() != VlogToken::EndParen)
				throw std::runtime_error(printError("Invalid gate node. Expecting close paren.", LOCATION));
		}


		if (mTkz.nextToken() != VlogToken::EndParen)
			throw std::runtime_error(printError("Invalid gate node. Expecting close paren.", LOCATION));


		if (mTkz.nextToken() != VlogToken::Semicolon)
			throw std::runtime_error(printError("Invalid gate node. Expecting Semicolon.", LOCATION));

	}

	void VerilogParser::addOutputWire(const std::string & name)
	{
		//Log::out << "input: " << name << Log::endl;


		u64 in1Idx;

		auto inIter = mNameMap.find(name);
		if (inIter == mNameMap.end())
		{
			in1Idx = mNameMap.size();
			mNameMap.emplace(name, in1Idx);

			mNodes.emplace_back();
			mNodes[in1Idx].mWireIdx = in1Idx;
			mNodes[in1Idx].mName = name;
		}
		else
		{
			in1Idx = inIter->second;
		}

		mNodes[in1Idx].mOutIdx = ++mOutputIdx;
		//mNodes[in1Idx].mInCount = 0;

		mOutputs.push_back(in1Idx);
	}

	void VerilogParser::addInputWire(const std::string & in1)
	{
		//Log::out << "input: " << in1 << Log::endl;


		u64 in1Idx;

		auto inIter = mNameMap.find(in1);
		if (inIter == mNameMap.end())
		{
			in1Idx = mNameMap.size();
			mNameMap.emplace(in1, in1Idx);

			mNodes.emplace_back();
			mNodes[in1Idx].mWireIdx = in1Idx;
			mNodes[in1Idx].mName = in1;
		}
		else
		{
			in1Idx = inIter->second;
		}

		mNodes[in1Idx].mType = GateType::Zero;
		mNodes[in1Idx].mInCount = 0;

		mReadyNodes.push_back(in1Idx);
	}

	void VerilogParser::addInvert(const std::string & in1, const std::string & out)
	{


		u64 in1Idx, in2Idx, outIdx;


		// find or make input 1 
		auto inIter = mNameMap.find(in1);
		if (inIter == mNameMap.end())
		{
			in1Idx = mNameMap.size();
			mNameMap.emplace(in1, in1Idx);

			mNodes.emplace_back();
			mNodes[in1Idx].mWireIdx = in1Idx;
			mNodes[in1Idx].mName = in1;
		}
		else
		{
			in1Idx = inIter->second;
		}



		// find or make output
		inIter = mNameMap.find(out);
		if (inIter == mNameMap.end())
		{
			outIdx = mNameMap.size();
			mNameMap.emplace(out, outIdx);


			mNodes.emplace_back();
			mNodes[outIdx].mWireIdx = outIdx;
			mNodes[outIdx].mName = out;

		}
		else
		{
			outIdx = inIter->second;

			if ((mNodes[outIdx].mInput0 | mNodes[outIdx].mInput1) != 0)
				throw std::runtime_error(printError("output variable " + out + "already used", LOCATION));
		}




		mNodes[outIdx].mType = GateType::na;
		mNodes[outIdx].mInput0 = in1Idx;
		mNodes[outIdx].mInput1 = -1;
		mNodes[outIdx].mInCount = 1;

		mNodes[in1Idx].mChildren.push_back(outIdx);

	}

	void VerilogParser::addGate(
		const std::string & in1,
		const std::string & in2,
		const std::string & out,
		GateType type)
	{
		//Log::out << "add gate: " << in1 << "  " << in2 << " -> " << gateToString(type) << "  " << out << Log::endl;

		if(in1 == "1'b0" || in1 == "1b'1")
			throw std::runtime_error(printError("input variable 0 constant.", LOCATION));
		if (in2 == "1'b0" || in2 == "1b'1")
			throw std::runtime_error(printError("input variable 1 constant.", LOCATION));

		u64 in1Idx, in2Idx, outIdx;


		// find or make input 1 
		auto inIter = mNameMap.find(in1);
		if (inIter == mNameMap.end())
		{
			in1Idx = mNameMap.size();
			mNameMap.emplace(in1, in1Idx);

			mNodes.emplace_back();
			mNodes[in1Idx].mWireIdx = in1Idx;
			mNodes[in1Idx].mName = in1;
		}
		else
		{
			in1Idx = inIter->second;
		}


		// find or make input 2
		inIter = mNameMap.find(in2);
		if (inIter == mNameMap.end())
		{
			in2Idx = mNameMap.size();
			mNameMap.emplace(in2, in2Idx);


			mNodes.emplace_back();
			mNodes[in2Idx].mWireIdx = in2Idx;
			mNodes[in2Idx].mName = in2;
		}
		else
		{
			in2Idx = inIter->second;
		}



		// find or make output
		inIter = mNameMap.find(out);
		if (inIter == mNameMap.end())
		{
			outIdx = mNameMap.size();
			mNameMap.emplace(out, outIdx);


			mNodes.emplace_back();
			mNodes[outIdx].mWireIdx = outIdx;
			mNodes[outIdx].mName = out;

		}
		else
		{
			outIdx = inIter->second;

			if ((mNodes[outIdx].mInput0 | mNodes[outIdx].mInput1) != 0)
				throw std::runtime_error(printError("output variable " + out + "already used", LOCATION));
		}



		//Log::out << "          " << in1Idx << "  " << in2Idx << " -> " << gateToString(type) << "  " << outIdx << Log::endl;

		mNodes[outIdx].mType = type;
		mNodes[outIdx].mInput0 = in1Idx;
		mNodes[outIdx].mInput1 = in2Idx;

		mNodes[in1Idx].mChildren.push_back(outIdx);
		mNodes[in2Idx].mChildren.push_back(outIdx);

	}

	void VerilogParser::addAlias(const std::string & alias, const std::string & src)
	{
		//Log::out << alias << " = " << src << Log::endl;


		// find or make output
		auto aIter = mNameMap.find(alias);
		auto sIter = mNameMap.find(src);

		if (sIter == mNameMap.end())
		{
			auto sIdx = mNameMap.size();
			mNameMap.emplace(src, sIdx);
			sIter = mNameMap.find(src);


			mNodes.emplace_back();
			mNodes[sIdx].mWireIdx = sIdx;
			mNodes[sIdx].mName = src;
		}



		if (aIter == mNameMap.end())
		{
			// we've never seen this alias, just map it to the src idx
			mNameMap.emplace(alias, sIter->second);
		}
		else
		{
			auto oldIdx = aIter->second;
			auto newIdx = sIter->second;

			if ((mNodes[oldIdx].mInput0 | mNodes[oldIdx].mInput1) != 0)
				throw std::runtime_error(printError("output variable " + alias + "already used", LOCATION));

			mNodes[oldIdx].mType == GateType::a;



			//throw std::runtime_error(printError("todo, implement renaming on alias", LOCATION));


			for (u64 i = oldIdx; i < mNodes.size(); ++i)
			{
				mNodes[i].mInput0 = (mNodes[i].mInput0 == oldIdx) ? newIdx : mNodes[i].mInput0;
				mNodes[i].mInput1 = (mNodes[i].mInput1 == oldIdx) ? newIdx : mNodes[i].mInput1;
			}


		}

	}

}
