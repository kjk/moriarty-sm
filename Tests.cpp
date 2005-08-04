#include "Tests.h"

#include <UTF8_Processor.hpp>
#include <Text.hpp>
#include <HistoryCache.hpp>
#include <PopupMenu.hpp>
#include <DeviceInfo.hpp>

void RunTests(HWND wnd)
{

#ifndef NDEBUG
	test_TextUnitTestAll();
	test_UTF8_ToNative();
	test_HistoryCache();
//	test_UTF8_FromNative();
	test_DeviceInfoToken();
	test_PopupMenu(wnd);
#endif
}

