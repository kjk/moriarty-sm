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

static void test_PropertyPages(HWND wnd);


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
                //SHINITDLGINFO shidi;
                //shidi.dwMask = SHIDIM_FLAGS;
                //shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                //shidi.hDlg = hDlg;
                //SHInitDialog(&shidi);
            }
#endif // SHELL_AYGSHELL
			{
				// TextRenderer* r = new_nt TextRenderer(Widget::autoDelete);
				// r->create(WS_VISIBLE|WS_TABSTOP, 10, 40, 220, 160, hDlg, GetInstance());
			}
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                // EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;

        case WM_CLOSE:
            // EndDialog(hDlg, message);
            return (INT_PTR)TRUE;

#ifdef _DEVICE_RESOLUTION_AWARE
        case WM_SIZE:
            {
		//DRA::RelayoutDialog(
		//	GetInstance(), 
		//	hDlg, 
		//	DRA::GetDisplayMode() != DRA::Portrait ? MAKEINTRESOURCE(IDD_ABOUTBOX_WIDE) : MAKEINTRESOURCE(IDD_ABOUTBOX));
            }
            break;
#endif
    }
    return (INT_PTR)FALSE;
}
#endif // !WIN32_PLATFORM_WFSP
 
MainWindow::MainWindow():
	Window(autoDelete),
	lastItemIndex_(0)
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
	if (!menuBar_.create(handle(), 0, IDR_MAIN_MENU))
	    return createFailed;
#endif

	Rect r;
	bounds(r);
	//if (!renderer_.create(WS_TABSTOP, SCALEX(1), SCALEY(1), r.width() - SCALEX(2), r.height() - SCALEY(2), handle(), cs.hInstance))
	//	return createFailed;

    if (!listView_.create(WS_VISIBLE | WS_TABSTOP | LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_ICON, SCALEX(1), SCALEY(1), r.width() - SCALEX(2), r.height() - SCALEY(2), handle(), cs.hInstance)) //, LVS_EX_DOUBLEBUFFER
        return createFailed;
        
#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0
#endif

#ifndef LVS_EX_GRADIENT
#define LVS_EX_GRADIENT 0
#endif

    listView_.setStyleEx(/* LVS_EX_DOUBLEBUFFER | */ LVS_EX_GRADIENT | LVS_EX_ONECLICKACTIVATE | LVS_EX_NOHSCROLL);
    listView_.setTextBkColor(CLR_NONE);

    HMENU menu = menuBar_.subMenu(IDM_VIEW);
    CheckMenuItem(menu, ID_VIEW_LIST, MF_UNCHECKED);
    CheckMenuItem(menu, ID_VIEW_ICONS, MF_CHECKED);
    

    if (!createModuleItems())
        return createFailed; 

    updateListViewFocus();
    
	return Window::handleCreate(cs);
}

long MainWindow::handleDestroy()
{
	PostQuitMessage(0);
	return Window::handleDestroy();
}

long MainWindow::handleCommand(ushort notify_code, ushort id, HWND sender)
{
    HMENU menu;
    switch (id)
    {

        case ID_VIEW_ICONS:
            listView_.modifyStyle(LVS_ICON, LVS_LIST);
            menu = menuBar_.subMenu(IDM_VIEW);
            CheckMenuItem(menu, ID_VIEW_LIST, MF_UNCHECKED);
            CheckMenuItem(menu, ID_VIEW_ICONS, MF_CHECKED);
            return messageHandled;
        
        case ID_VIEW_LIST:
            listView_.modifyStyle(LVS_LIST, LVS_ICON);
            menu = menuBar_.subMenu(IDM_VIEW);
            CheckMenuItem(menu, ID_VIEW_ICONS, MF_UNCHECKED);
            CheckMenuItem(menu, ID_VIEW_LIST, MF_CHECKED);
            return messageHandled;

        case IDCLOSE:
#ifdef WIN32_PLATFORM_WFSP
            destroy();
#endif // WIN32_PLATFORM_WFSP
#ifndef WIN32_PLATFORM_WFSP
            close();
#endif // !WIN32_PLATFORM_WFSP
            return messageHandled;
            
	}
    return Window::handleCommand(notify_code, id, sender);
}


long MainWindow::handleResize(UINT sizeType, ushort width, ushort height)
{
	//renderer_.anchor(anchorRight, SCALEX(2), anchorBottom, SCALEY(2), repaintWidget);
	listView_.anchor(anchorRight, SCALEX(2), anchorBottom, SCALEY(2), repaintWidget);
    uint_t x = GetSystemMetrics(SM_CXVSCROLL);
    long iconWidth = (width - x - SCALEX(2)) / ((width - x - SCALEX(2)) / SCALEX(70));
    long iconHeight = height / (height / SCALEY(54));
    ListView_SetIconSpacing(listView_.handle(), iconWidth, iconHeight);
    listView_.invalidate(erase);
    ListView_RedrawItems(listView_.handle(), 0, listView_.itemCount() - 1);
	
	return Window::handleResize(sizeType, width, height);
}

LRESULT MainWindow::callback(UINT msg, WPARAM wParam, LPARAM lParam)
{
    //if (extEvent == msg && extEventLookupFinished == ExtEventGetID(lParam))
    //{
    //    LookupManager* lm = GetLookupManager();
    //    if (lm->handleLookupFinishedInForm(lParam))
    //        return messageHandled;
    //        
    //    const LookupFinishedEventData* data = LookupFinishedData(lParam);
    //    if (lookupResultEBookBrowse == data->result)
    //        renderer_.setModel(lm->releaseDefinitionModel(), Definition::ownModel);

    //    return messageHandled;
    //} 
    return Window::callback(msg, wParam, lParam); 
}

/*
static int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam) {

    if (uMsg == PSCB_INITIALIZED) 
    {
        // Get tab control
        HWND hwndTabs = GetDlgItem (hwndDlg, 0x3020);

        DWORD dwStyle = GetWindowLong (hwndTabs, GWL_STYLE);
        SetWindowLong (hwndTabs, GWL_STYLE, dwStyle | TCS_BOTTOM);
    } 
    else if (uMsg ==  PSCB_GETVERSION)
        return COMCTL32_VERSION;
    return 0;
}


void test_PropertyPages(HWND parent)
{
    PROPSHEETPAGE page;
    ZeroMemory(&page, sizeof(page));
    page.dwSize = sizeof(page);
    page.dwFlags = PSP_PREMATURE;
    page.hInstance = GetInstance();
    page.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG1);
    page.hIcon = NULL;
    page.pszTitle = NULL;
    page.pfnDlgProc = About;
    page.lParam = 0;
    page.pfnCallback = NULL;
    page.pcRefParent = NULL;
    HPROPSHEETPAGE p1 = CreatePropertySheetPage(&page);
    if (NULL == p1)
    {
        DWORD err = GetLastError();
        assert(false); 
        return;
    }           

    page.pszTemplate = MAKEINTRESOURCE(IDD_WEATHER_MAIN);  
    HPROPSHEETPAGE p2 = CreatePropertySheetPage(&page);
    if (NULL == p2)
    {
        DWORD err = GetLastError();
        assert(false); 
        return;
    }           

    HPROPSHEETPAGE pp[] = {p1, p2};
    
    PROPSHEETHEADER header;
    ZeroMemory(&header, sizeof(header));
    header.dwSize = sizeof(header);
    header.dwFlags = PSH_MAXIMIZE | PSH_NOAPPLYNOW | PSH_USECALLBACK;
    header.hwndParent = parent;
    header.hInstance = GetInstance();
    header.hIcon = NULL;
    header.pszCaption = MAKEINTRESOURCE(IDS_APP_TITLE);
    header.nPages = 2;
    header.nStartPage = 0;
    header.phpage = pp;
    header.pfnCallback = PropSheetProc;
    
    HWND wnd = (HWND)PropertySheet(&header);
    //if (NULL == wnd)
    //{
    //    DWORD err = GetLastError();
    //    assert(false); 
    //    return;
    //}           

    //ShowWindow(wnd, SW_SHOW);
    //UpdateWindow(wnd);      
}
 */

static void BitmapSize(HBITMAP bmp, LONG& w, LONG& h)
{
    BITMAP b;
    int res = GetObject(bmp, sizeof(b), &b);
    assert(0 != res);
    w = b.bmWidth;
    h = b.bmHeight;
}

bool MainWindow::createModuleItems()
{
    bool scaleIcons = false;
    if (SCALEX(50) >= 100)
        scaleIcons = true;   
    
    ulong_t actCount = ModuleActiveCount();
    ulong_t count = ModuleCount();
     
    HINSTANCE inst = GetInstance();
    HIMAGELIST smallIcons = NULL;
    HIMAGELIST largeIcons = NULL;
    HBITMAP bmp = NULL;
    
    LONG sw, sh, lw, lh;
    ulong_t index = 0; 
    for (ulong_t i = 0; i < count; ++i) 
    {
        const Module* module = ModuleGet(i);
        if (!module->active())
            continue;
        
        bmp = LoadBitmap(inst, MAKEINTRESOURCE(module->smallIconId));
        if (NULL == bmp)
            goto Error;
        if (NULL == smallIcons)
        {
            BitmapSize(bmp, sw, sh);

#ifndef ILC_COLOR16
#define ILC_COLOR16 ILC_COLOR
#endif

        if (scaleIcons)
        {
            sw = SCALEX(sw); 
            sh = SCALEY(sh);
        }
            
            smallIcons = ImageList_Create(sw, sh, ILC_COLOR16|ILC_MASK, actCount, 1);
            if (NULL == smallIcons)
                goto Error;
        }
#if _MSC_VER > 1200
        if (scaleIcons)
        {
            HBITMAP b;
            if (!ImageList_StretchBitmap(bmp, &b, sw, sh, 1, 1))
                goto Error;
            DeleteObject(bmp);
            bmp = b;
        } 
#endif

        if (-1 == ImageList_AddMasked(smallIcons, bmp, RGB(255, 0, 255)))
            goto Error;
        
        DeleteObject(bmp);
        
        bmp = LoadBitmap(inst, MAKEINTRESOURCE(module->largeIconId));
        if (NULL == bmp)
            goto Error;
        if (NULL == largeIcons)
        {
            BitmapSize(bmp, lw, lh);
            if (scaleIcons) 
            {
                lw = SCALEX(lw); 
                lh = SCALEY(lh);
            } 
            largeIcons = ImageList_Create(lw, lh, ILC_COLOR16|ILC_MASK, actCount, 1);
            if (NULL == smallIcons)
                goto Error;
        }
#if _MSC_VER > 1200
        if (scaleIcons)
        {
            HBITMAP b;
            if (!ImageList_StretchBitmap(bmp, &b, lw, lh, 1, 1))
                goto Error;
            DeleteObject(bmp);
            bmp = b;
        } 
#endif

        if (-1 == ImageList_AddMasked(largeIcons, bmp, RGB(255, 0, 255)))
            goto Error;
        
        DeleteObject(bmp);
        bmp = NULL;
        ++index; 
    }
    
    smallIcons = listView_.setImageList(smallIcons, LVSIL_SMALL);
    if (NULL != smallIcons)
        ImageList_Destroy(smallIcons);

    largeIcons = listView_.setImageList(largeIcons, LVSIL_NORMAL);
    if (NULL != largeIcons)
        ImageList_Destroy(largeIcons); 

    listView_.clear();
    index = 0; 
    for (ulong_t i = 0; i < count; ++i)
    {
        const Module* module = ModuleGet(i);
        if (!module->active())
            continue;
        
        const char_t* name = module->displayName;
        LVITEM item;
        ZeroMemory(&item, sizeof(item));

        item.mask = LVIF_TEXT | LVIF_IMAGE;
        item.iItem = index++;
        item.iSubItem = 0;
        item.pszText = const_cast<char_t*>(name);
        item.iImage = item.iItem;
        
        if (-1 == listView_.insertItem(item))
            goto Error;
    }   
    return true;
Error:
    DWORD res = GetLastError();
    if (NULL != bmp)
        DeleteObject(bmp); 
    if (NULL != smallIcons)
        ImageList_Destroy(smallIcons);
    if (NULL != largeIcons)
        ImageList_Destroy(largeIcons);
    return false;  
} 

long MainWindow::handleNotify(int controlId, const NMHDR& header)
{
    if  (LVN_ITEMACTIVATE == header.code)
    {
        const NMLISTVIEW& h = (const NMLISTVIEW&)header;
        if (-1 == h.iItem)
            goto Default;
        
        lastItemIndex_ = h.iItem;
        const Module* module = ModuleGetActive(lastItemIndex_);
        assert(NULL != module);
        status_t err = ModuleRun(module->id);
        if (memErrNotEnoughSpace == err)
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        else if (errNone != err)
            Alert(IDS_ALERT_NOT_IMPLEMENTED);
        return messageHandled;
    }
Default:
    return Window::handleNotify(controlId, header); 
}

long MainWindow::handleActivate(ushort action, bool minimized, HWND prev)
{
    //if (WA_ACTIVE == action || WA_CLICKACTIVE == action)
    //    updateListViewFocus();
                 
    return Window::handleActivate(action, minimized, prev); 
}

void MainWindow::updateListViewFocus()
{
    listView_.focus();
    long l = ListView_GetSelectionMark(listView_.handle());
    if (-1 == l)
        l = lastItemIndex_;
        
    ListView_SetItemState(listView_.handle(), l, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}
