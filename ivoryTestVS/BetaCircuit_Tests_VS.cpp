#include "stdafx.h"
#include "CppUnitTest.h"
#include "Common.h"

#include "BetaCircuit_Tests.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace libBDXTests
{
	TEST_CLASS(BetaCircuit_Tests)
	{
	public:

        TEST_METHOD(BetaCircuit_SequentialOp)
        {
            InitDebugPrinting();
            BetaCircuit_SequentialOp_Test();
        }


        TEST_METHOD(BetaCircuit_int_Adder)
        {
            InitDebugPrinting();
            BetaCircuit_int_Adder_Test();
        }

        TEST_METHOD(BetaCircuit_uint_Adder)
        {
            InitDebugPrinting();
            BetaCircuit_uint_Adder_Test();
        }

        TEST_METHOD(BetaCircuit_int_Adder_const)
        {
            InitDebugPrinting();
            BetaCircuit_int_Adder_const_Test();
        }

        TEST_METHOD(BetaCircuit_int_Subtractor)
        {
            InitDebugPrinting();
            BetaCircuit_int_Subtractor_Test();
        }

        TEST_METHOD(BetaCircuit_int_Subtractor_const)
        {
            InitDebugPrinting();
            BetaCircuit_int_Subtractor_const_Test();
        }

        TEST_METHOD(BetaCircuit_uint_Subtractor)
        {
            InitDebugPrinting();
            BetaCircuit_uint_Subtractor_Test();
        }

        TEST_METHOD(BetaCircuit_int_Multiply)
        {
            InitDebugPrinting();
            BetaCircuit_int_Multiply_Test();
        }

        TEST_METHOD(BetaCircuit_int_Divide)
        {
            InitDebugPrinting();
            BetaCircuit_int_Divide_Test();
        }

        TEST_METHOD(BetaCircuit_int_LessThan)
        {
            InitDebugPrinting();
            BetaCircuit_int_LessThan_Test();
        }

        TEST_METHOD(BetaCircuit_int_GreaterThanEq)
        {
            InitDebugPrinting();
            BetaCircuit_int_GreaterThanEq_Test();
        }

        TEST_METHOD(BetaCircuit_uint_LessThan)
        {
            InitDebugPrinting();
            BetaCircuit_uint_LessThan_Test();
        }


        TEST_METHOD(BetaCircuit_uint_GreaterThanEq)
        {
            InitDebugPrinting();
            BetaCircuit_uint_GreaterThanEq_Test();
        }


        TEST_METHOD(BetaCircuit_multiplex)
        {
            InitDebugPrinting();
            BetaCircuit_multiplex_Test();
        }


        TEST_METHOD(BetaCircuit_bitInvert)
        {
            InitDebugPrinting();
            BetaCircuit_bitInvert_Test();
        }

        TEST_METHOD(BetaCircuit_int_negate)
        {
            InitDebugPrinting();
            BetaCircuit_negate_Test();
        }

        TEST_METHOD(BetaCircuit_int_removeSign)
        {
            InitDebugPrinting();
            BetaCircuit_removeSign_Test();
        }

        TEST_METHOD(BetaCircuit_int_addSign)
        {
            InitDebugPrinting();
            BetaCircuit_addSign_Test();
        }
	};
}