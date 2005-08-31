#ifndef INFOMAN_MODULE_DIALOG_H__
#define INFOMAN_MODULE_DIALOG_H__

#include <WindowsCE/Dialog.hpp>
#include <WindowsCE/CommandBar.hpp>
#include <ExtendedEvent.hpp>

struct LookupFinishedEventData;

class MenuDialog: public Dialog {
    DWORD menuBarFlags_;
    UINT menuBarId_;
#ifdef SHELL_MENUBAR
    CommandBar menuBar_;
#endif  

    virtual bool handleBackKey(UINT msg, WPARAM wParam, LPARAM lParam);
    
protected:

#ifdef SHELL_MENUBAR   
    CommandBar& menuBar() {return menuBar_;}
    const CommandBar& menuBar() const  {return menuBar_;}
#endif

    void setMenuBarId(UINT menuBarId) {menuBarId_ = menuBarId;}
    void setMenuBarFlags(DWORD flags) {menuBarFlags_ = flags;}
    void overrideBackKey(); 

    bool handleInitDialog(HWND focus_widget_handle, long init_param);
   
    LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam); 

public:

    
    enum {menuBarNone = UINT(-1)}; 
    enum AdvancedOption {advanced}; 

	explicit MenuDialog(AdvancedOption, bool inputDialog = false, DWORD initDialogFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN);
	
	explicit MenuDialog(UINT menuBarId = menuBarNone, bool inputDialog = false);
	
	~MenuDialog();


};

class ModuleDialog: public MenuDialog {
    ExtEventHelper extEventHelper_;

protected:
    
    LRESULT callback(UINT msg, WPARAM wParam, LPARAM lParam); 

    virtual bool handleLookupFinished(Event& event, const LookupFinishedEventData* data);

    bool handleInitDialog(HWND focus_widget_handle, long init_param);
   
public:
    
	explicit ModuleDialog(AdvancedOption, bool inputDialog = false, DWORD initDialogFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN);
	
	explicit ModuleDialog(UINT menuBarId = menuBarNone, bool inputDialog = false);
	
	~ModuleDialog();
    
    bool create(UINT resourceId); 
   
    long showModal(UINT resourceId); 
   
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

#define MODULE_DIALOG_SHOW_MODAL_DECLARE(Class) static long showModal()

#define MODULE_DIALOG_SHOW_MODAL_IMPLEMENT(Class, resourceId) \
long Class::showModal() \
{ \
    Class* dlg = new_nt Class(); \
    if (NULL == dlg) \
    { \
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY); \
        return 0; \
    } \
    long res = dlg->ModuleDialog::showModal(resourceId); \
    delete dlg; \
    return res; \
}

#endif