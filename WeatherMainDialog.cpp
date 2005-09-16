#include "WeatherMainDialog.h"
#include "InfoMan.h"
#include "HyperlinkHandler.h"
#include "Modules.h"
#include "LookupManager.h"
#include "WeatherModule.h"
#include "InfoManPreferences.h"

#include <SysUtils.hpp>
#include <UniversalDataFormat.hpp>
#include <Text.hpp>

using namespace DRA;

WeatherMainDialog::WeatherMainDialog():
    ModuleDialog(IDR_WEATHER_MENU),
    displayMode_(showSummary)
{
}

WeatherMainDialog::~WeatherMainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(WeatherMainDialog, IDD_WEATHER_MAIN)

bool WeatherMainDialog::handleInitDialog(HWND wnd, long lp)
{
	Rect r;
	innerBounds(r);
	
	list_.create(WS_TABSTOP | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL, r, handle());
	list_.setStyleEx(LVS_EX_GRADIENT | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);
	
	combo_.attachControl(handle(), IDC_WEATHER_DAY);
	
	renderer_.create(WS_VISIBLE | WS_TABSTOP, 0, 0, r.width(), r.height() - combo_.height() - SCALEY(5) * 2, handle());
	renderer_.definition.setHyperlinkHandler(GetHyperlinkHandler());
	renderer_.definition.setInteractionBehavior(0);
	renderer_.definition.setNavOrderOptions(Definition::navOrderFirst);
	
    ModuleDialog::handleInitDialog(wnd, lp);
    WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    if (NULL == prefs.location || 0 == Len(prefs.location))
    {
        // TODO: show "enter location dialog"
 
    }   
    else if (NULL == prefs.udf)
    {
        // TODO: fetch actual weather data 
    }   
    else 
    { 
    	createComboItems();
        createListItems();
    }

    setDisplayMode(displayMode_);
    resyncTempMenu(); 
   
    return false; 
}

bool WeatherMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    
   return ModuleDialog::handleLookupFinished(event, data); 
}

long WeatherMainDialog::handleCommand(ushort notify_code, ushort id, HWND sender)
{
	return ModuleDialog::handleCommand(notify_code, id, sender);
}

long WeatherMainDialog::handleResize(UINT sizeType, ushort width, ushort height)
{
    combo_.setBounds(SCALEX(5), SCALEY(5), width - 2 * SCALEX(5), combo_.height(), repaintWidget);
    long hh = 2 * SCALEY(5) + combo_.height();
    renderer_.anchor(anchorRight, 0, anchorBottom, hh, repaintWidget);
    list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);  
	return messageHandled;
}

void WeatherMainDialog::resyncTempMenu()
{
    HMENU menu = menuBar().subMenu(IDM_VIEW);
    WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    UINT item = (prefs.celsiusMode ? ID_TEMPERATURE_CELSIUS : ID_TEMPERATURE_FAHRENHEIT);
    CheckMenuRadioItem(menu, ID_TEMPERATURE_FAHRENHEIT, ID_TEMPERATURE_CELSIUS, item, MF_BYCOMMAND);
}

void WeatherMainDialog::setDisplayMode(DisplayMode dm)
{
    HMENU menu = menuBar().subMenu(IDM_VIEW);
    WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    UINT state = (NULL != prefs.udf ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND;
    EnableMenuItem(menu, ID_VIEW_DETAILED, state);
    EnableMenuItem(menu, ID_VIEW_SUMMARY, state);
    EnableMenuItem(menu, ID_TEMPERATURE_CELSIUS, state);
    EnableMenuItem(menu, ID_TEMPERATURE_FAHRENHEIT, state);   

    if (displayMode_ == dm)
        return;
         
    switch (displayMode_ = dm)
    {
        case showSummary:
            renderer_.hide();
            combo_.hide();
            list_.show();
            CheckMenuRadioItem(menu, ID_VIEW_DETAILED, ID_VIEW_SUMMARY, ID_VIEW_SUMMARY, MF_BYCOMMAND);
            list_.focus();
            break;
        case showDetails:
            list_.hide();
            combo_.show();
            renderer_.show();
            renderer_.focus();
            CheckMenuRadioItem(menu, ID_VIEW_DETAILED, ID_VIEW_SUMMARY, ID_VIEW_DETAILED, MF_BYCOMMAND);
            break;
        default:
            assert(false);
    }  
}


void WeatherMainDialog::createListItems()
{
    SYSTEMTIME st; 
    GetLocalTime(&st);
    char_t buffer[64];
    
    list_.clear(); 
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    for (int i = 0; i < 7; ++i)
    {
         
    }  
          
}

void WeatherMainDialog::createComboItems()
{
    SYSTEMTIME st;
    GetLocalTime(&st); 
    char_t buffer[64];
    
    combo_.clear();  
    for (int i = 0; i < 7; ++i)
    {
        if (0 == i)
        {
            char_t* s = LoadString(IDS_TODAY);
            if (NULL == s)
                goto Error;
            combo_.addString(s);
            free(s); 
        } 
        else
        {
            if (0 == GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, buffer, 64))
                goto Error;
            combo_.addString(buffer);
        }
        TimeRollDays(st, 1);
    }
    combo_.setSelection(0); 
    return; 
Error:
    DumpErrorMessage(GetLastError());
    return;      
}