#include "ModuleDialog.h"
#include "LookupManager.h"
#include <SysUtils.hpp>

ModuleDialog::ModuleDialog(AutoDeleteOption ad, bool inputDialog, DWORD initDialogFlags):
    Dialog(ad, inputDialog, initDialogFlags)
{}

bool ModuleDialog::handleInitDialog(HWND focus_widget_handle, long init_param)
{
    extEventHelper_.start(handle());
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
