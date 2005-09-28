#include "JokesMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"

JokesMainDialog::JokesMainDialog()
//:
//    ModuleDialog(IDR_JOKES_MENU)
{
}

JokesMainDialog::~JokesMainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(JokesMainDialog, IDD_EMPTY) //IDD_JOKES_MAIN

bool JokesMainDialog::handleInitDialog(HWND fw, long ip)
{
    return ModuleDialog::handleInitDialog(fw, ip);
}

long JokesMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    return ModuleDialog::handleResize(st, w, h);
}

long JokesMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool JokesMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    //switch (data->result)
    //{
    //}
    return ModuleDialog::handleLookupFinished(event, data);
}
