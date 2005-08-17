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

	explicit ModuleDialog(AutoDeleteOption ad = autoDelete, bool inputDialog = false, DWORD initDialogFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN /* |SHIDIF_EMPTYMENU */ );
	
	~ModuleDialog();
    
    bool create(UINT resourceId); 

};

void ModuleDialogSetCurrent(ModuleDialog* dialog);
void ModuleDialogDestroyCurrent();
ModuleDialog* ModuleDialogGetCurrent();

#define MODULE_DIALOG_CREATE_DECLARE(Class) static Class* create()

#define MODULE_DIALOG_CREATE_IMPLEMENT(Class, resourceId) \
Class* Class::create() \
{ \
    Class* dlg = new_nt Class(); \
    if (NULL == dlg) \
        return NULL; \
    if (!dlg->ModuleDialog::create(resourceId)) \
    { \
        delete dlg; \
        return NULL; \
    } \
    return dlg; \
}

#endif