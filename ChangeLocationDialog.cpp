#include "ChangeLocationDialog.h"
#include "InfoMan.h"

#include <Text.hpp>
#include <SysUtils.hpp>

ChangeLocationDialog::ChangeLocationDialog(char_t *location):
    MenuDialog(IDR_DONE_CANCEL_MENU, true),
    location_(location)
{
    setAutoDelete(autoDeleteNot);
}

ChangeLocationDialog::~ChangeLocationDialog()
{
    free(location_); 
}

bool ChangeLocationDialog::handleInitDialog(HWND fw, long ip)
{
    edit_.attachControl(handle(), IDC_ZIP_CODE);
    
    MenuDialog::handleInitDialog(fw, ip);

    if (NULL != location_)
    {
        edit_.setCaption(location_);
        edit_.focus();
        edit_.setSelection();
    }
    else
        edit_.focus();
    return false;  
}

long ChangeLocationDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id) {
        case IDOK:
        {
            char_t* t = edit_.text();
            char_t* tt = StrStripCopy(t);
            free(t);
            if (NULL == tt)
            {
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
                return messageHandled; 
            }
            if (0 == Len(tt))
            {
                edit_.focus();
                edit_.setSelection();
                free(tt);
                return messageHandled;   
            }
            free(location_);
            location_ = tt;
        }
        // Intentional fall-through
        case IDCANCEL:
            endModal(id);
            return messageHandled;
    }
    return MenuDialog::handleCommand(nc, id, sender); 
}

long ChangeLocationDialog::showModal(char_t *&location, HWND parent)
{
    ChangeLocationDialog dlg(location);
    long res = dlg.MenuDialog::showModal(NULL, IDD_CHANGE_LOCATION, parent);
    if (IDOK == res)
    {
        free(location);
        location = dlg.location_;
        dlg.location_ = NULL; 
    } 
    SHSipPreference(parent, SIP_FORCEDOWN);
    return res; 
}