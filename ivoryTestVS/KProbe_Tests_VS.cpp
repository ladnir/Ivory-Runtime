#include "stdafx.h"
#include "CppUnitTest.h"
#include "KProbe_Tests.h"
#include "Common.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace libBDXTests
{


	TEST_CLASS(KProbee_Tests)
	{
	public:

		TEST_METHOD(KProbe_Build_Test)
		{
			KProbe_Build_Test_Impl();
		}

		TEST_METHOD(KProbe_XORTransitive_Test)
		{
			InitDebugPrinting("../test.out");
			KProbe_XORTransitive_Test_Impl();
		}
#ifdef ENCODABLE_KPROBE
		TEST_METHOD(KProbe_BitVector_Test)
		{
			KProbe_BitVector_Test_Impl();
		}

		TEST_METHOD(KProbe_ZeroLabels_Test)
		{
			KProbe_ZeroLabels_Test_Impl();
		}

		TEST_METHOD(KProbe_Labels_Test)
		{
			KProbe_Labels_Test_Impl();
		}
#endif
		TEST_METHOD(KProbe_SaveLoad_Test)
		{
			KProbe_SaveLoad_Test_Impl();
		}

		TEST_METHOD(KProbe_BlockBitVector_Test)
		{
			KProbe_BlockBitVector_Test_Impl();
		}

	};
}