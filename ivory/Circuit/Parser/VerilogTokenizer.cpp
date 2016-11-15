#include "VerilogTokenizer.h"
#include "Common/Log.h"

namespace osuCrypto
{

	VerilogTokenizer::VerilogTokenizer()
	{
	}


	VerilogTokenizer::~VerilogTokenizer()
	{
	}

	void VerilogTokenizer::init(std::istream & in)
	{
		mIn = &in;
		mLineNumber = 0;
		mCharLineBit = 0;
		mTokenLineBit = 0;

		curLine() = " ";
		mCurChar = curLine().end() - 1;

		nextChar();
		nextToken();
	}

	const std::string& VerilogTokenizer::curWord()
	{
		return mCurWord;
	}

	VlogToken VerilogTokenizer::curToken()
	{
		return mCurToken;
	}

	VlogToken VerilogTokenizer::nextToken()
	{
		mCurToken = _nextToken();

		//mCurTokenLineIdx = mLineNumber;

		return mCurToken;
	}

	i64 VerilogTokenizer::curNumber()
	{
		return mCurNumber;
	}

	u64 VerilogTokenizer::curTokenCharIdx()
	{
		return mCurTokenCharIdx;
	}

	u64 VerilogTokenizer::curTokenLineIdx()
	{
		return mCurTokenLineIdx;
	}

	std::string VerilogTokenizer::printError(char * location)
	{
		std::stringstream ss;

		ss << "while reading Line# " << mLineNumber << "  char# " << (mCurChar - curLine().begin()) << " the tokenizer encountered an error."
			<< std::endl << location << std::endl << std::endl;

		ss << curLine() << std::endl;

		auto iter = curLine().begin();
		while (iter != mCurChar)
		{
			++iter;
			ss << ' ';
		}

		Log::out << ss.str() << Log::Color::Red << "^" << Log::ColorDefault << Log::endl;



		return  "while reading Line " + std::to_string(mLineNumber) + " the tokenizer encountered an error.";
	}

	std::string & VerilogTokenizer::curLine()
	{
		return mLines[mCharLineBit];
	}

	std::string & VerilogTokenizer::curTokenLine()
	{
		return mLines[mTokenLineBit];
	}

	char VerilogTokenizer::nextChar()
	{
		//
		if (mCurChar == curLine().end() - 1)
		{
			nextLine();
		}
		else
		{
			++mCurChar;
		}

		if (mEOF)
			return 0;

		return *mCurChar;
	}

	void VerilogTokenizer::nextLine()
	{
		//if (mCharLineBit == mTokenLineBit)
		mCharLineBit = mTokenLineBit ^ 1;

		if (mIn->eof())
		{
			mCurChar = curLine().end();
			mEOF = 1;
		}
		else
		{
			++mLineNumber;

			std::getline(*mIn, curLine());

			mCurChar = curLine().begin();


			if (curLine().size() == 0)
				nextLine();
		}

	}

	VlogToken  VerilogTokenizer::_nextToken()
	{
		mCurTokenCharIdx = mCurChar - curLine().begin();
		mTokenLineBit = mCharLineBit;
		mCurTokenLineIdx = mLineNumber;

		if (isSpecialChar())
		{
			switch (curChar())
			{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				nextChar();
				return _nextToken();
			case '/':
				return startForwardSlash();
			case '(':
				return startParen();
			case ')':
				nextChar();
				return VlogToken::EndParen;
			case '[':
				nextChar();
				return VlogToken::SBracket;
			case ']':
				nextChar();
				return VlogToken::EndSBracket;
			case ',':
				nextChar();
				return VlogToken::Comma;
			case '.':
				nextChar();
				return VlogToken::Dot;
			case ';':
				nextChar();
				return VlogToken::Semicolon;
			case ':':
				nextChar();
				return VlogToken::Colon;
			case '\\':
				return startEscapeWord();
			case '=':
				nextChar();
				return VlogToken::Eq;
			case '\'':
				nextChar();
				return VlogToken::SingleQuote;
			default:



				throw std::runtime_error(printError(LOCATION));
				break;
			}
		}
		else if (isAlpha())
		{
			return startWord();
		}
		else if (isNumeric())
		{
			return startNumeric();
		}
		else if (isEOF())
		{
			return VlogToken::eof;
		}
		else
		{
			throw std::runtime_error(printError(LOCATION));
		}
	}

	VlogToken  VerilogTokenizer::startForwardSlash()
	{
		nextChar();

		if (curChar() == '*')
		{
			// comment
			while (curChar() != '/')
			{
				nextChar();
				while (curChar() != '*')
				{
					nextChar();
				}
			}

			return _nextToken();
		}
		else
		{
			throw std::runtime_error("tokenizer error " LOCATION);
		}
	}
	VlogToken  VerilogTokenizer::startParen()
	{
		nextChar();

		if (curChar() == '*')
		{
			// comment
			while (curChar() != ')')
			{
				nextChar();
				while (curChar() != '*')
				{
					nextChar();
				}
			}

			return _nextToken();
		}
		else
		{
			return VlogToken::Paren;
		}
	}


	VlogToken  VerilogTokenizer::startWord()
	{

		mCurWord.clear();

		mCurWord.push_back(curChar());
		nextChar();

		while (isAlphaNumeric())
		{
			mCurWord.push_back(curChar());

			nextChar();
		}


		if (mCurWord == "module")
		{
			return VlogToken::Module;
		}
		else if (mCurWord == "endmodule")
		{
			return VlogToken::EndModule;
		}
		else if (mCurWord == "wire")
		{
			return VlogToken::Wire;
		}
		else if (mCurWord == "input")
		{
			return VlogToken::Input;
		}
		else if (mCurWord == "output")
		{
			return VlogToken::Output;
		}
		else if (mCurWord == "assign")
		{
			return VlogToken::Assign;
		}

		return VlogToken::Word;
	}

	VlogToken VerilogTokenizer::startNumeric()
	{
		mCurWord.clear();

		mCurWord.push_back(curChar());
		nextChar();

		while (isNumeric())
		{
			mCurWord.push_back(curChar());

			nextChar();
		}

		mCurNumber = std::stoll(mCurWord);


		return VlogToken::Number;
	}

	VlogToken VerilogTokenizer::startEscapeWord()
	{
		mCurWord.clear();

		mCurWord.push_back(curChar());
		nextChar();

		while (curChar() != ' ')
		{
			mCurWord.push_back(curChar());

			nextChar();
		}

		return VlogToken::Word;
	}

	std::string tokenToStr(VlogToken t)
	{


		if (VlogToken::Wire == t) return 	"Wire";
		if (VlogToken::Input == t) return 	"Input";
		if (VlogToken::Output == t) return 	"Output";
		if (VlogToken::Always == t) return 	"Always	";
		if (VlogToken::If == t) return 	"If";
		if (VlogToken::Else == t) return 	"Else";
		if (VlogToken::Poseedge == t) return 	"Poseedge";
		if (VlogToken::Module == t) return 	"Module";
		if (VlogToken::EndModule == t) return 	"EndModule";
		if (VlogToken::Semicolon == t) return 	"Semicolon";
		if (VlogToken::Colon == t) return 	"Colon";
		if (VlogToken::BackSlash == t) return 	"BackSlash";
		if (VlogToken::Eq == t) return 	"Eq";
		if (VlogToken::Comma == t) return 	"Comma";
		if (VlogToken::Dot == t) return 	"Dot";
		if (VlogToken::At == t) return 	"At";
		if (VlogToken::Paren == t) return 	"Paren";
		if (VlogToken::EndParen == t) return 	"EndParen";
		if (VlogToken::SBracket == t) return 	"SBracket";
		if (VlogToken::SingleQuote == t) return "SingleQuote";
		if (VlogToken::EndSBracket == t) return 	"EndSBracket";
		if (VlogToken::Assign == t) return 	"Assign";
		if (VlogToken::And == t) return 	"And";
		if (VlogToken::Or == t) return 	"Or";
		if (VlogToken::Number == t) return 	"Number";
		if (VlogToken::Word == t) return 	"word";
		if (VlogToken::eof == t) return 	"eof";

		return "";
	}
}
