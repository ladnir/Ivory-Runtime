#include "stdafx.h"
#include "CppUnitTest.h"
#include "Common.h"

#include "Circuit_Tests.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace libBDXTests
{
	TEST_CLASS(Circuit_Tests)
	{
	public:

		TEST_METHOD(Circuit_Adder)
		{
            InitDebugPrinting();
            Circuit_Adder_Test();
		}

        TEST_METHOD(Circuit_Subtractor)
        {
            InitDebugPrinting();
            Circuit_Subtractor_Test();
        }

        TEST_METHOD(Circuit_Multiply)
        {
            InitDebugPrinting();
            Circuit_Multiply_Test();
        }
	};
}