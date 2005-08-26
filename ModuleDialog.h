#ifndef INFOMAN_MODULE_DIALOG_H__
#define INFOMAN_MODULE_DIALOG_H__

#include <WindowsCE/Dialog.hpp>
#include <WindowsCE/CommandBar.hpp>
#include <ExtendedEvent.hpp>

struct LookupFinishedEventData;

class ModuleDialog: public Dialog {
    ExtEventHelper extEventHelper_;

    UINT menuBarId_;
#ifdef SHELL_MENUBAR
    CommandBar menuBar_;
#endif      

protected:
    
    LRESULT callback(UINT msg, WPARAM wParam, LPARAM lParam); 

    virtual bool handleLookupFinished(Event& event, const LookupFinishedEventData* data);

    bool handleInitDialog(HWND focus_widget_handle, long init_param);
   
#ifdef SHELL_MENUBAR   
    CommandBar& menuBar() {return menuBar_;}
    const CommandBar& menuBar() const  {return menuBar_;}
#endif    

public:
    
    enum {menuBarNone = UINT(-1)}; 
    enum AdvancedOption {advanced}; 

	explicit ModuleDialog(AdvancedOption, bool inputDialog = false, DWORD initDialogFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN);
	
	explicit ModuleDialog(UINT menuBarId = menuBarNone, bool inputDialog = false);
	
	~ModuleDialog();
    
    bool create(UINT resourceId); 
   
    void endModal(int code); 

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