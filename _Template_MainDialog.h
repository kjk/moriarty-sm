#ifndef $(MODULE)_MAIN_DIALOG_H__
#define $(MODULE)_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

class $(ModuleName)MainDialog: public ModuleDialog {


    $(ModuleName)MainDialog();
    ~$(ModuleName)MainDialog();

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 

public:
    
    MODULE_DIALOG_CREATE_DECLARE($(ModuleName)MainDialog);

};

#endif // $(MODULE)_MAIN_DIALOG_H__