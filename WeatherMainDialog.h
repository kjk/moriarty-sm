#ifndef INFOMAN_WEATHER_MAIN_DIALOG_H__
#define INFOMAN_WEATHER_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/WinTextRenderer.hpp>
#include <WindowsCE/Controls.hpp>

struct UniversalDataFormat;

class WeatherMainDialog: public ModuleDialog {

    TextRenderer renderer_;
    ListView list_;
    ComboBox combo_;
    Widget* bitmap_;

    WeatherMainDialog();

    void createListItems(); 
    void createComboItems();
    void createListColumns(); 

    enum DisplayMode {
        showDetails,
        showSummary
    } displayMode_;  

    void setDisplayMode(DisplayMode dm); 
    void resyncTempMenu(); 

    ~WeatherMainDialog(); 

    void prepareWeather(ulong_t index); 
    void changeTempScale(bool celsius);
    void changeLocation();
    bool handleMultiselect(UniversalDataFormat* udf);

protected:
    
    bool handleInitDialog(HWND focus_widget_handle, long init_param);

    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data);
   
    long handleCommand(ushort notify_code, ushort id, HWND sender);
   
    long handleResize(UINT sizeType, ushort width, ushort height);
   
    long handleListItemActivate(int controlId, const NMLISTVIEW& header); 
    
public:
    
   MODULE_DIALOG_CREATE_DECLARE(WeatherMainDialog);
    
};

#endif