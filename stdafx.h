// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <WindowsCE/Config.hpp>

#define WM_HANDLER(name) LRESULT name##(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
#define WM_HANDLE(name) name##(wnd, message, wParam, lParam)
#define WM_HANDLE_DEF() DefWindowProc(wnd, message, wParam, lParam)

