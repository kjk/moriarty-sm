#include "stdafx.h"



#include "InfoMan.h"
#include "InfoManPreferences.h"
#include "MainWindow.h"
#include "Tests.h"
#include "LookupManager.h"

#include <Text.hpp>
#include <SysUtils.hpp>
#include <DataStore.hpp>
#include <ExtendedEvent.hpp>
#include <DefinitionStyle.hpp>

#include <windows.h>
#include <commctrl.h>

#define MAX_LOADSTRING 100

HINSTANCE g_hInst = NULL;			// current instance

HINSTANCE GetInstance() {return g_hInst;}

static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

void CleanUp()
{
	DestroyLookupManager();
	StyleDisposeStaticStyles();
	PrefsDispose();
	DataStore::dispose();
	DeinitLogging();
}

status_t DataStoreInit()
{
	char_t* path = GetAppDataPath();
	if (NULL == path)
		return memErrNotEnoughSpace;
		
	path = StrAppend(path, -1, TEXT("\\InfoMan"), -1);
	if (NULL == path)
		return memErrNotEnoughSpace;
	
	BOOL res = CreateDirectory(path, NULL);
	if (!res && ERROR_ALREADY_EXISTS != GetLastError())
	{
		free(path);
		return GetLastError();
	}
	
	path = StrAppend(path, -1, TEXT("\\AppData.dat"), -1);
	if (NULL == path)
		return memErrNotEnoughSpace;
	
	status_t err = DataStore::initialize(path);
	free(path);
	return err;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int nCmdShow)
{
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
		return FALSE;
	
#ifndef NDEBUG
	test_ExtEventSend();
#endif	

#ifndef WIN32_PLATFORM_WFSP
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INFOMAN));
#endif // !WIN32_PLATFORM_WFSP

	// Main message loop:
	LookupManager* lm = GetLookupManager();
	if (NULL == lm)
	{
		Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
		CleanUp();
		return memErrNotEnoughSpace;
	}
	MSG msg;
	status_t err;
	while (true) 
	{
		err = lm->connectionManager().waitForMessage(msg, 15000);
		if (errNone != err)
			break;
			
		if (WM_QUIT == msg.message)
		{
			err = msg.wParam;
			break;
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
			lm->handleLookupEvent(msg.lParam);
			ExtEventFree(msg.lParam);
		}
	}
	PrefsSave();
	CleanUp();
	return err;
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
   
	LogAddDebuggerLog(eLogAlways);

	status_t err = DataStoreInit();
	if (errNone != err)
	{
		if (memErrNotEnoughSpace == err)
			Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
		CleanUp();
		return FALSE;
	}
   
	if (NULL == GetPreferences())
	{
		Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
		CleanUp();
		return FALSE;
	}
	
	StylePrepareStaticStyles();
	MainWindow* w = MainWindow::create(szTitle, szWindowClass);
    if (NULL == w)
	{
		// TODO: show some alert
		CleanUp();
        return FALSE;
    }

	w->show(nCmdShow);
	w->update();

	RunTests(w->handle());

    return TRUE;
}

