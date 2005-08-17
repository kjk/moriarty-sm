#ifndef INFOMAN_WEATHER_MAIN_DIALOG_H__
#define INFOMAN_WEATHER_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/WinTextRenderer.hpp>
#include <WindowsCE/CommandBar.hpp>

class WeatherMainDialog: public ModuleDialog {
    
    TextRenderer renderer_; 
   
#ifdef SHELL_MENUBAR
    CommandBar menuBar_;
#endif      

    WeatherMainDialog();

protected:
    
    bool handleInitDialog(HWND focus_widget_handle, long init_param);

    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data);
   
    long handleCommand(ushort notify_code, ushort id, HWND sender);
    
public:
    
   MODULE_DIALOG_CREATE_DECLARE(WeatherMainDialog);
    
};

#endif