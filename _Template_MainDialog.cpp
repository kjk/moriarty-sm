#include "$(ModuleName)MainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"

$(ModuleName)MainDialog::$(ModuleName)MainDialog():
    ModuleDialog(IDR_$(MODULE)_MENU)
{
}

$(ModuleName)MainDialog::~$(ModuleName)MainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT($(ModuleName)MainDialog, IDD_$(MODULE)_MAIN)

bool $(ModuleName)MainDialog::handleInitDialog(HWND fw, long ip)
{
    return ModuleDialog::handleInitDialog(fw, ip);
}

long $(ModuleName)MainDialog::handleResize(UINT st, ushort w, ushort h)
{
    return ModuleDialog::handleResize(st, w, h);
}

long $(ModuleName)MainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool $(ModuleName)MainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    switch (data->result)
    {
    }
    return ModuleDialog::handleLookupFinished(event, data);
}
