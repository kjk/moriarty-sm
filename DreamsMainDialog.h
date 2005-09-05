#ifndef DREAMS_MAIN_DIALOG_H__
#define DREAMS_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>
#include <WindowsCE/WinTextRenderer.hpp>

class DreamsMainDialog: public ModuleDialog {

    Widget info_;
    EditBox term_; 
    TextRenderer renderer_; 
    
    DreamsMainDialog();
    ~DreamsMainDialog();
   
    char_t* title_; 
   
    void search(const char_t* term, long len); 

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 

public:
    
    MODULE_DIALOG_CREATE_DECLARE(DreamsMainDialog);

};

#endif // DREAMS_MAIN_DIALOG_H__