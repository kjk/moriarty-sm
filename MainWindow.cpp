#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>

#include "InfoMan.h"
#include "MainWindow.h"
#include "Utility.h"

struct MainWindowData {

#ifdef SHELL_SAI
	SHACTIVATEINFO sai;
#endif

	HWND menuBar;

	MainWindowData(): menuBar(NULL) {
#ifdef SHELL_SAI
		Zero(sai);
		sai.cbSize = sizeof(sai);
#endif
	}

};

static LRESULT CALLBACK MainWindow_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static ATOM MainWindow_RegisterClass(HINSTANCE hInstance, const TCHAR* szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWindow_WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(LONG_PTR);
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_INFOMAN));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

HWND MainWindow_Create(const TCHAR* title, const TCHAR* windowClass)
{
	HINSTANCE inst = GetInstance();

	if (!MainWindow_RegisterClass(inst, windowClass))
    	return NULL;
	
    HWND hWnd = CreateWindow(windowClass, title, WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, inst, NULL);
    if (NULL == hWnd)
        return NULL;

#ifdef WIN32_PLATFORM_PSPC
    // When the main window is created using CW_USEDEFAULT the height of the menubar (if one
    // is created is not taken into account). So we resize the window after creating it
    // if a menubar is present

	MainWindowData* data = GetWindowData<MainWindowData>(hWnd);
	if (NULL != data->menuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(data->menuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }
#endif // WIN32_PLATFORM_PSPC

	return hWnd;
}

#ifndef WIN32_PLATFORM_WFSP
// Message handler for about box.
static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
#ifdef SHELL_AYGSHELL
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
#endif // SHELL_AYGSHELL

            return (INT_PTR)TRUE;

        case WM_COMMAND:
#ifdef SHELL_AYGSHELL
            if (LOWORD(wParam) == IDOK)
#endif
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return (INT_PTR)TRUE;

#ifdef _DEVICE_RESOLUTION_AWARE
        case WM_SIZE:
            {
		DRA::RelayoutDialog(
			GetInstance(), 
			hDlg, 
			DRA::GetDisplayMode() != DRA::Portrait ? MAKEINTRESOURCE(IDD_ABOUTBOX_WIDE) : MAKEINTRESOURCE(IDD_ABOUTBOX));
            }
            break;
#endif
    }
    return (INT_PTR)FALSE;
}
#endif // !WIN32_PLATFORM_WFSP

static WM_HANDLER(MainWindow_OnCommand);
static WM_HANDLER(MainWindow_OnCreate);
static WM_HANDLER(MainWindow_OnDestroy);
static WM_HANDLER(MainWindow_OnPaint);

LRESULT CALLBACK MainWindow_WndProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
		case WM_COMMAND:
			return WM_HANDLE(MainWindow_OnCommand);

        case WM_CREATE:
			return WM_HANDLE(MainWindow_OnCreate);

		case WM_PAINT:
			return WM_HANDLE(MainWindow_OnPaint);

        case WM_DESTROY:
			return WM_HANDLE(MainWindow_OnDestroy);

#ifdef SHELL_SAI

        case WM_ACTIVATE:
            // Notify shell of our activate message
            SHHandleWMActivate(wnd, wParam, lParam, &GetWindowData<MainWindowData>(wnd)->sai, FALSE);
            break;

        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(wnd, wParam, lParam, &GetWindowData<MainWindowData>(wnd)->sai);
            break;

#endif // SHELL_SAI

        default:
            return DefWindowProc(wnd, message, wParam, lParam);
    }
    return 0;
}

WM_HANDLER(MainWindow_OnCommand)
{
	int wmId    = LOWORD(wParam); 
    int wmEvent = HIWORD(wParam); 

    // Parse the menu selections:
    switch (wmId)
    {

#ifndef WIN32_PLATFORM_WFSP
        case IDM_HELP_ABOUT:
            DialogBox(GetInstance(), (LPCTSTR)IDD_ABOUTBOX, wnd, About);
            break;
#endif // !WIN32_PLATFORM_WFSP

#ifdef WIN32_PLATFORM_WFSP
        case IDM_OK:
            DestroyWindow(wnd);
            break;
#endif // WIN32_PLATFORM_WFSP

#ifndef WIN32_PLATFORM_WFSP
        case IDM_OK:
            SendMessage(wnd, WM_CLOSE, 0, 0);				
            break;
#endif // !WIN32_PLATFORM_WFSP

        default:
            return WM_HANDLE_DEF();
    }
	return 0;
}

WM_HANDLER(MainWindow_OnCreate)
{
	MainWindowData* data = new MainWindowData();
	if (NULL == data)
		return -1;

	SetWindowLong(wnd, GWL_USERDATA, PtrToLong(data));

#ifdef SHELL_AYGSHELL
    SHMENUBARINFO mbi;
    Zero(mbi);
    mbi.cbSize     = sizeof(SHMENUBARINFO);
    mbi.hwndParent = wnd;
    mbi.nToolBarId = IDR_MENU;
    mbi.hInstRes   = GetInstance();

    if (SHCreateMenuBar(&mbi)) 
		data->menuBar = mbi.hwndMB;
#endif // SHELL_AYGSHELL
	
	return WM_HANDLE_DEF();
}

WM_HANDLER(MainWindow_OnDestroy)
{
	MainWindowData* data = GetWindowData<MainWindowData>(wnd);
#ifdef SHELL_AYGSHELL
	CommandBar_Destroy(data->menuBar);
	data->menuBar = NULL;
#endif // SHELL_AYGSHELL
	
	delete data;
	SetWindowLong(wnd, GWL_USERDATA, 0);
    PostQuitMessage(0);
	return WM_HANDLE_DEF();
}

WM_HANDLER(MainWindow_OnPaint)
{
	PAINTSTRUCT ps;
    HDC hdc = BeginPaint(wnd, &ps);
    
    // TODO: Add any drawing code here...
    
    EndPaint(wnd, &ps);
	return WM_HANDLE_DEF();
}