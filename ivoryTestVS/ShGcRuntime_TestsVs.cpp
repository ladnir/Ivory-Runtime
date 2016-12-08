#include "stdafx.h"
#ifdef  _MSC_VER
#include "CppUnitTest.h"

#include "Common.h"
#include "ShGcRuntime_tests.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(LocalChannel_Tests)
{
public:

	TEST_METHOD(ShGcRuntime_BasicArithetic)
	{
        InitDebugPrinting();
		ShGcRuntime_basicArith_Test();
	}


    TEST_METHOD(ShGcRuntime_SequentialOp)
    {
        InitDebugPrinting();
        ShGcRuntime_SequentialOp_Test();
    }


};
#endif
