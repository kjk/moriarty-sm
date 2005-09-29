#ifndef JOKES_MAIN_DIALOG_H__
#define JOKES_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>
#include <WindowsCE/WinTextRenderer.hpp>

class JokesMainDialog: public ModuleDialog {
    
    TextRenderer renderer_;
    ListView list_;

    JokesMainDialog();
    ~JokesMainDialog();
    
    enum DisplayMode {
        showJoke,
        showList
    } displayMode_;
    
    void setDisplayMode(DisplayMode dm);
    
    void createListColumns();
    void createListItems();
    void resyncViewMenu();

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 

public:
    
    MODULE_DIALOG_CREATE_DECLARE(JokesMainDialog);

};

#endif // JOKES_MAIN_DIALOG_H__