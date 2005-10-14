#include "AmazonMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"

AmazonMainDialog::AmazonMainDialog()
//:
//ModuleDialog(IDR_AMAZON_MENU)
{
}

AmazonMainDialog::~AmazonMainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(AmazonMainDialog, IDD_EMPTY)
//MODULE_DIALOG_CREATE_IMPLEMENT(AmazonMainDialog, IDD_AMAZON_MAIN)

bool AmazonMainDialog::handleInitDialog(HWND fw, long ip)
{
    return ModuleDialog::handleInitDialog(fw, ip);
}

long AmazonMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    return ModuleDialog::handleResize(st, w, h);
}

long AmazonMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool AmazonMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    //switch (data->result)
    //{
    //}
    return ModuleDialog::handleLookupFinished(event, data);
}
