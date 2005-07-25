#include "stdafx.h"

#include <Debug.hpp>
#include <ExtendedEvent.hpp>

#include <windows.h>

#include "InfoMan.h"
#include "MainWindow.h"
#include <DefinitionStyle.hpp>
#include "Tests.h"

#include <commctrl.h>

#define MAX_LOADSTRING 100

HINSTANCE g_hInst = NULL;			// current instance

HINSTANCE GetInstance() {return g_hInst;}

static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	
#ifndef NDEBUG
	test_ExtEventSend();
#endif	

#ifndef WIN32_PLATFORM_WFSP
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INFOMAN));
#endif // !WIN32_PLATFORM_WFSP

	// Main message loop:
	BOOL res;
	while (0 != (res = GetMessage(&msg, NULL, 0, 0))) 
	{
		if (-1 == res)
		{
			StyleDisposeStaticStyles();
			return GetLastError();
		}
			
#ifndef WIN32_PLATFORM_WFSP
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
#endif // !WIN32_PLATFORM_WFSP
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (extEvent == msg.message)
		{
#ifndef NDEBUG
			test_ExtEventReceive(msg.lParam);
#endif	
			ExtEventFree(msg.lParam);
		}
	}
	StyleDisposeStaticStyles();
	return (int) msg.wParam;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance; // Store instance handle in our global variable

    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name


#ifdef WIN32_PLATFORM_PSPC
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the Pocket PC special controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();
#endif // WIN32_PLATFORM_PSPC

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_INFOMAN, szWindowClass, MAX_LOADSTRING);

    //If it is already running, then focus on the window, and exit
    HWND hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 

#if 1

	StylePrepareStaticStyles();
	MainWindow* w = MainWindow::create(szTitle, szWindowClass);
    if (NULL == w)
	{
		StyleDisposeStaticStyles(); 
        return FALSE;
    }

	w->show(nCmdShow);
	w->update();

	RunTests(w->handle());

#else
 
	HWND wnd = MainWindow_Create(szTitle, szWindowClass);
	ShowWindow(wnd, nCmdShow);
	UpdateWindow(wnd);
	 	
	RunTests(wnd);

#endif
	
    return TRUE;
}

