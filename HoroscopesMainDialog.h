#ifndef HOROSCOPES_MAIN_DIALOG_H__
#define HOROSCOPES_MAIN_DIALOG_H__

#include "ModuleDialog.h"

#include <WindowsCE/Controls.hpp>
#include <WindowsCE/WinTextRenderer.hpp>

struct UniversalDataFormat;

class HoroscopesMainDialog: public ModuleDialog {

    HoroscopesMainDialog();
    ~HoroscopesMainDialog();
   
    UniversalDataFormat* udf_;
    ListView listView_;
    TextRenderer renderer_;        
   
    enum DisplayMode {
        showSigns,
        showHoroscope
    } displayMode_;
   
    void setDisplayMode(DisplayMode dm); 
    void prepareSigns(); 
   
protected:

    bool handleInitDialog(HWND fw, long ip);    
   
    long handleResize(UINT sizeType, ushort width, ushort height); 
   
    long handleCommand(ushort nc, ushort id, HWND sender);
   
    long handleNotify(int controlId, const NMHDR& header);  
   
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 

public:

    MODULE_DIALOG_CREATE_DECLARE(HoroscopesMainDialog);
};

#endif // HOROSCOPES_MAIN_DIALOG_H__