#pragma once

#include "Circuit/Gate.h"
//#include "Circuit/Gate.h"
#include <istream>
#include <string>

namespace osuCrypto
{

	enum class VlogToken
	{
		Wire,
		Input,
		Output,
		Always,
		If,
		Else,
		Poseedge,
		Module,
		EndModule,
		Semicolon,
		Colon,
		BackSlash,
		Eq,
		Comma,
		Dot,
		At,
		Paren,
		EndParen,
		SBracket,
		EndSBracket,
		SingleQuote,
		Assign,
		And,
		Or,
		Number,
		Word,
		eof
	};

	std::string tokenToStr(VlogToken t);


	//class VlogKeyword : public VlogToken
	//{
	//	enum Type
	//	{
	//		Wire,
	//		Input,
	//		Output,
	//		Always,
	//		If,
	//		Else,
	//		Poseedge,
	//		Module,
	//		EndModule,
	//		Semicolon,
	//		Comma,
	//		Dot,
	//		At,
	//		Paren,
	//		EndParen,
	//		SBracket,
	//		EndSBracket,
	//		Assign,
	//		And,
	//		Or
	//	};


	//	Type mType;

	//};

	//class VlogGate : public VlogToken
	//{
	//	Gate mType;
	//};

	//class  vLogVariable : public VlogToken
	//{
	//	std::string mVal;
	//};
	class VerilogParser;
	class VerilogTokenizer
	{

		friend class VerilogParser;

	public:
		VerilogTokenizer();
		~VerilogTokenizer();

		std::istream* mIn;

		void init(std::istream& in);


		const std::string& curWord();
		VlogToken curToken();
		VlogToken nextToken();
		i64 curNumber();


		std::string& curTokenLine();
		u64 curTokenCharIdx();
		u64 curTokenLineIdx();

	private:

		std::string printError(char* location);

		VlogToken mCurToken;
		std::array<std::string,2> mLines;
		u8 mCharLineBit, mTokenLineBit;

		std::string::iterator mCurChar;
		char  mEOF;
		std::string mCurWord;
		u64 mLineNumber, mCurTokenCharIdx, mCurTokenLineIdx;


		i64 mCurNumber;

		bool isEOF() { return mEOF; }
		bool isSpecialChar() { return !isEOF() && !isAlphaNumeric(); }
		bool isNumeric() { return !isEOF() && (curChar() >= 48 && curChar() < 58); }
		bool isAlpha() { return !isEOF() && ((curChar() >= 65 && curChar() < 91) || (curChar() >= 97 && curChar() < 123) || curChar() == '_'); }
		bool isAlphaNumeric() { return isNumeric() || isAlpha(); }



		std::string& curLine();

		char curChar() {
			return *mCurChar;
		}
		char nextChar();
		void nextLine();

		VlogToken _nextToken();


		VlogToken  startForwardSlash();
		VlogToken  startParen();
		VlogToken  startWord();
		VlogToken  startNumeric();
		VlogToken startEscapeWord();
	};

}
