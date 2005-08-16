#ifndef INFOMAN_MODULE_DIALOG_H__
#define INFOMAN_MODULE_DIALOG_H__

#include <WindowsCE/Dialog.hpp>
#include <ExtendedEvent.hpp>

struct LookupFinishedEventData;

class ModuleDialog: public Dialog {
    ExtEventHelper extEventHelper_;

protected:
    
    LRESULT callback(UINT msg, WPARAM wParam, LPARAM lParam); 

    virtual bool handleLookupFinished(Event& event, const LookupFinishedEventData* data);

    bool handleInitDialog(HWND focus_widget_handle, long init_param);

public:

	explicit ModuleDialog(AutoDeleteOption ad = autoDeleteNot, bool inputDialog = false, DWORD initDialogFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN);
    
    bool create(UINT resourceId); 

}; 

#endif