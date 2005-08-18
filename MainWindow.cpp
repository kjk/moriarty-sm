#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>

#include "InfoMan.h"
#include "InfoManGlobals.h"
#include "MainWindow.h"
#include <SysUtils.hpp>

#include "LookupManager.h"
#include "Modules.h"

using namespace DRA;

/*
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
			{
				TextRenderer* r = new_nt TextRenderer(Widget::autoDelete);
				r->create(WS_VISIBLE|WS_TABSTOP, 10, 40, 220, 160, hDlg, GetInstance());
			}
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
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
 */
 
MainWindow::MainWindow():
	Window(autoDelete)
{
    setOverrideNavBarText(true);
}

MainWindow* MainWindow::create(const char_t* title, const char_t* windowClass)
{
	MainWindow* w = new_nt MainWindow();
	if (NULL == w)
		return NULL;

	HINSTANCE instance = GetInstance();
	static ATOM wc = registerClass(
		CS_HREDRAW | CS_VREDRAW, 
		instance, 
		LoadIcon(instance, MAKEINTRESOURCE(IDI_INFOMAN)), 
		NULL,
		(HBRUSH) GetStockObject(WHITE_BRUSH),
		windowClass);
	
	if (NULL == wc)
		goto Error;

	if (!w->Window::create(wc, title, WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance))
		goto Error;

#ifdef WIN32_PLATFORM_PSPC
	assert(w->menuBar_.valid());
	w->menuBar_.adjustParentSize();
#endif
	
	return w;
Error:
	delete w;
	return NULL;
}

long MainWindow::handleCreate(const CREATESTRUCT& cs)
{
    extEventHelper_.start(handle());
    
#ifdef SHELL_MENUBAR
	if (!menuBar_.create(handle(), 0, IDR_MENU))
	    return createFailed;
#endif

	Rect r;
	bounds(r);
	if (!renderer_.create(WS_VISIBLE|WS_TABSTOP, SCALEX(1), SCALEY(1), r.width() - SCALEX(2), r.height() - SCALEY(2), handle(), cs.hInstance))
		return createFailed;

	return Window::handleCreate(cs);
}

long MainWindow::handleDestroy()
{
	PostQuitMessage(0);
	return Window::handleDestroy();
}

long MainWindow::handleCommand(ushort notify_code, ushort id, HWND sender)
{
    switch (id)
    {

//#ifndef WIN32_PLATFORM_WFSP
        case IDM_HELP_ABOUT: 
        {
            ModuleRun(moduleIdWeather);
            return 0;
        }
//#endif // !WIN32_PLATFORM_WFSP

#ifdef WIN32_PLATFORM_WFSP
        case IDCLOSE:
            destroy();
            return 0;
#endif // WIN32_PLATFORM_WFSP

#ifndef WIN32_PLATFORM_WFSP
        case IDCLOSE:
            close();
            return 0;
#endif // !WIN32_PLATFORM_WFSP

	}
    return Window::handleCommand(notify_code, id, sender);
}


long MainWindow::handleResize(UINT sizeType, ushort width, ushort height)
{
	renderer_.anchor(anchorRight, SCALEX(2), anchorBottom, SCALEY(2), repaintWidget);
	return Window::handleResize(sizeType, width, height);
}

LRESULT MainWindow::callback(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (extEvent == msg && extEventLookupFinished == ExtEventGetID(lParam))
    {
        LookupManager* lm = GetLookupManager();
        if (lm->handleLookupFinishedInForm(lParam))
            return messageHandled;
            
        const LookupFinishedEventData* data = LookupFinishedData(lParam);
        if (lookupResultEBookBrowse == data->result)
        {
            renderer_.setModel(lm->definitionModel, Definition::ownModel);
            lm->definitionModel = NULL;
        }
        return messageHandled;
    } 
    return Window::callback(msg, wParam, lParam); 
}
