#include "ModuleDialog.h"
#include "LookupManager.h"
#include "Modules.h"

#include <SysUtils.hpp>
#include <WindowsCE/Controls.hpp>
#include <Text.hpp>

MenuDialog::MenuDialog(AdvancedOption, bool inputDialog, DWORD initDialogFlags):
    Dialog(autoDelete, inputDialog, initDialogFlags),
    menuBarId_(menuBarNone),
    menuBarFlags_(0) 
{
    setOverrideNavBarText(true);
}

MenuDialog::MenuDialog(UINT menuBarId, bool inputDialog):
    Dialog(autoDelete, inputDialog, SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN),
    menuBarId_(menuBarId),
    menuBarFlags_(0) 
{
    setOverrideNavBarText(true);
}

MenuDialog::~MenuDialog()
{
}

bool MenuDialog::createSipPrefControl()
{
#ifdef WIN32_PLATFORM_PSPC
    return NULL != CreateWindow(TEXT("SIPPREF"), NULL, 0, -10, -10, 6, 6, handle(), NULL, GetInstance(), NULL);
#else
    return true;
#endif    
} 

bool MenuDialog::handleInitDialog(HWND focus_widget_handle, long init_param)
{
#ifdef SHELL_MENUBAR

    UINT mb = menuBarId_;
    DWORD flags = menuBarFlags_; 
    if (menuBarNone == mb)
#ifdef WIN32_PLATFORM_WFSP
        mb = IDR_DONE;
#else
    {
        mb = 0;
        flags |= SHCMBF_EMPTYBAR;
    }
#endif 
        
    if (!menuBar_.create(handle(), flags, mb))
    {  
        DWORD err = GetLastError();
        assert(false);
    }  
#endif // SHELL_MENUBAR
    
    return Dialog::handleInitDialog(focus_widget_handle, init_param); 
}

#ifndef VK_TBACK
#define VK_TBACK VK_ESCAPE
#endif

LRESULT MenuDialog::callback(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_MEASUREITEM:
            return handleMeasureItem(wParam, *(MEASUREITEMSTRUCT*)lParam);

        case WM_HOTKEY:
            if (HIWORD(lParam) == VK_TBACK && handleBackKey(uMsg, wParam, lParam))
                return messageHandled;
    }
    return Dialog::callback(uMsg, wParam, lParam);   
}

bool MenuDialog::handleBackKey(UINT msg, WPARAM wParam, LPARAM lParam)
{
    bool keyUp = (0 != (UINT(LOWORD(lParam)) & MOD_KEYUP));
    HWND wnd = GetFocus();
    if (NULL == wnd)
    { 
        if (keyUp) 
        {
            PostMessage(handle(), WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
            return true;
        }
        else
            return true;
    }
        
    char_t name[8];
    int res = GetClassName(wnd, name, 5);
    if (0 == res || !(equalsIgnoreCase(name, WINDOW_CLASS_EDITBOX) || equalsIgnoreCase(name, _T("CAPEDIT"))))
    { 
        if (keyUp)
        { 
            //SHNavigateBack(); 
            PostMessage(handle(), WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
            return true;
        }
        else
            return true;
    }

#ifdef SHELL_TPCSHELL
    SHSendBackToFocusWindow(msg, wParam, lParam);
    return true;
#else
    return false;
#endif 
}

void MenuDialog::overrideBackKey()
{
	LPARAM lparam = MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY, SHMBOF_NODEFAULT | SHMBOF_NOTIFY);
	menuBar_.sendMessage(SHCMBM_OVERRIDEKEY, VK_TBACK, lparam);
}

long MenuDialog::handleNotify(int controlId, const NMHDR& header)
{
    switch (header.code)
    {
        case NM_CUSTOMDRAW:
        {
            const NMLVCUSTOMDRAW& h = (const NMLVCUSTOMDRAW&)header;
            if (CDDS_PREPAINT == h.nmcd.dwDrawStage)
                return CDRF_NOTIFYITEMDRAW;
            else if (CDDS_ITEMPREPAINT == h.nmcd.dwDrawStage)
            {
                if (drawListViewItem(h))
                    return CDRF_SKIPDEFAULT;
            }
            return CDRF_DODEFAULT;
        }
    }  
    return Dialog::handleNotify(controlId, header);
}

bool MenuDialog::drawListViewItem(const NMLVCUSTOMDRAW& data)
{
    return false;
}

bool MenuDialog::handleMeasureItem(UINT controlId, MEASUREITEMSTRUCT& data)
{
    return false;
}


ModuleDialog::ModuleDialog(AdvancedOption adv, bool inputDialog, DWORD initDialogFlags):
    MenuDialog(adv, inputDialog, initDialogFlags)
{
}

ModuleDialog::ModuleDialog(UINT menuBarId, bool inputDialog):
    MenuDialog(menuBarId, inputDialog)
{
}   

ModuleDialog::~ModuleDialog()
{
}

bool ModuleDialog::handleInitDialog(HWND focus_widget_handle, long init_param)
{
    extEventHelper_.start(handle());
    return MenuDialog::handleInitDialog(focus_widget_handle, init_param); 
}

bool ModuleDialog::create(UINT resourceId)
{
    return MenuDialog::create(GetInstance(), resourceId, ExtEventGetWindow());
}

long ModuleDialog::showModal(UINT resourceId)
{
    return MenuDialog::showModal(GetInstance(), resourceId, ExtEventGetWindow());
}
    
LRESULT ModuleDialog::callback(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (extEvent == msg && extEventLookupFinished == ExtEventGetID(lParam) && handleLookupFinished(lParam, LookupFinishedData(lParam)))
        return messageHandled;
    return MenuDialog::callback(msg, wParam, lParam); 
}

bool ModuleDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    return lm->handleLookupFinishedInForm(event);
}

void ModuleDialog::endModal(int code)
{
    extEventHelper_.stop();
    MenuDialog::endModal(code);
}

long ModuleDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id) 
    {
        case IDOK:
        case IDCANCEL:
            ModuleRunMain();
            return messageHandled;
    }
    return MenuDialog::handleCommand(nc, id, sender);   
}







static ModuleDialog* currentModuleDialog = NULL;

void ModuleDialogSetCurrent(ModuleDialog* dialog)
{
    ModuleDialogDestroyCurrent();
    currentModuleDialog = dialog; 
}

void ModuleDialogDestroyCurrent()
{
    if (NULL == currentModuleDialog)
        return;

    currentModuleDialog->destroy();
    currentModuleDialog = NULL;  
}

ModuleDialog* ModuleDialogGetCurrent()
{
    return currentModuleDialog;
}
