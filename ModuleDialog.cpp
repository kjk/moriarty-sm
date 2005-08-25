#include "ModuleDialog.h"
#include "LookupManager.h"
#include <SysUtils.hpp>

ModuleDialog::ModuleDialog(AdvancedOption, bool inputDialog, DWORD initDialogFlags):
    Dialog(autoDelete, inputDialog, initDialogFlags),
    menuBarId_(menuBarNone) 
{
    setOverrideNavBarText(true);
}

ModuleDialog::ModuleDialog(UINT menuBarId, bool inputDialog):
#ifdef WIN32_PLATFORM_WFSP 
    Dialog(autoDelete, inputDialog, SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN),
#else
    Dialog(autoDelete, inputDialog, SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | (menuBarNone == menuBarId ? SHIDIF_EMPTYMENU : 0 )),
#endif
    menuBarId_(menuBarId)
{
#ifdef WIN32_PLATFORM_WFSP
    if (menuBarNone == menuBarId_)
        menuBarId_ = IDR_DONE;
#endif     

    setOverrideNavBarText(true);
}  

ModuleDialog::~ModuleDialog()
{
}

bool ModuleDialog::handleInitDialog(HWND focus_widget_handle, long init_param)
{
    extEventHelper_.start(handle());
    
#ifdef SHELL_MENUBAR
    if (menuBarNone != menuBarId_ && !menuBar_.create(handle(), 0, menuBarId_))
    {  
        DWORD err = GetLastError();
        assert(false);
    }  
#endif

    return Dialog::handleInitDialog(focus_widget_handle, init_param); 
}

bool ModuleDialog::create(UINT resourceId)
{
    return Dialog::create(GetInstance(), resourceId, ExtEventGetWindow());
}

    
LRESULT ModuleDialog::callback(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (extEvent == msg && extEventLookupFinished == ExtEventGetID(lParam) && handleLookupFinished(lParam, LookupFinishedData(lParam)))
        return messageHandled;
    return Dialog::callback(msg, wParam, lParam); 
}

bool ModuleDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    return lm->handleLookupFinishedInForm(event);
}

void ModuleDialog::endModal(int code)
{
    extEventHelper_.stop();
    Dialog::endModal(code);
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
