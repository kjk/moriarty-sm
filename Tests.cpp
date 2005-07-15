#include "Tests.h"

#include <UTF8_Processor.hpp>
#include <Text.hpp>

void RunTests()
{

#ifndef NDEBUG
	test_TextUnitTestAll();
	test_UTF8_ToNative();
//	test_UTF8_FromNative();

#endif
}

