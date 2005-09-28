#ifndef JOKES_MAIN_DIALOG_H__
#define JOKES_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

class JokesMainDialog: public ModuleDialog {


    JokesMainDialog();
    ~JokesMainDialog();

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 

public:
    
    MODULE_DIALOG_CREATE_DECLARE(JokesMainDialog);

};

#endif // JOKES_MAIN_DIALOG_H__