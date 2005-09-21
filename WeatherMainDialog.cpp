#include "WeatherMainDialog.h"
#include "InfoMan.h"
#include "HyperlinkHandler.h"
#include "Modules.h"
#include "LookupManager.h"
#include "WeatherModule.h"
#include "InfoManPreferences.h"
#include "ChangeLocationDialog.h"
#include "StringListDialog.h"

#include <SysUtils.hpp>
#include <UniversalDataFormat.hpp>
#include <Text.hpp>
#include <UniversalDataHandler.hpp>

class WeatherLocationListModel: public StringListDialogModel {
    const UniversalDataFormat& udf_;
public:
    explicit WeatherLocationListModel(const UniversalDataFormat& udf):
        udf_(udf)
    {
    }
    
    ~WeatherLocationListModel()
    {
    }
    
    ulong_t size() const {
        return udf_.getItemsCount();
    }
    
    const char_t* operator[](ulong_t i) const {
        return udf_.getItemText(i, 0);
    }
    
};

using namespace DRA;

static const UINT weatherBitmaps[] = {
    IDB_WEATHER_SUNNY,
    IDB_WEATHER_CLOUDS,
    IDB_WEATHER_RAIN,
    IDB_WEATHER_SNOW,
    IDB_WEATHER_STORM,
    IDB_WEATHER_SUNNY_CLOUDS,
    IDB_WEATHER_SUNNY_RAIN,
    IDB_WEATHER_SUNNY_SNOW,
    IDB_WEATHER_SUNNY_STORM
};

static const uint_t weatherBitmapSize = 80;

class WeatherBitmap: public Widget {

    WeatherCategory category_;
    HBITMAP bitmap_;
    
    static ATOM registerClass()
    {
        static ATOM res = Widget::registerClass(0, GetInstance(), NULL, NULL, HBRUSH(COLOR_WINDOW + 1), _T("WeatherBitmap"));
        return res;
    }

public:

    WeatherBitmap():
        Widget(autoDelete),
        category_(weatherUnknown),
        bitmap_(NULL)
    {
    }

    bool create(const RECT& r, HWND parent)
    {
        return Widget::create(registerClass(), NULL, WS_CHILD, r, parent, NULL, NULL);
    }
    
    void setCategory(WeatherCategory cat)
    {
        if (cat == category_)
            return;
            
        category_ = cat;
        if (NULL != bitmap_)
        {
            DeleteObject(bitmap_);
            bitmap_ = NULL;
        }
        
        if (weatherUnknown != category_)
        {
            UINT bmpId = weatherBitmaps[category_];
            bitmap_ = LoadBitmap(GetInstance(), MAKEINTRESOURCE(bmpId));
        }
        
        invalidate(erase);
    }

protected:

    long handlePaint(HDC dc, PAINTSTRUCT* ps)
    {
        if (NULL != bitmap_)
        {
            BITMAP bmp;
            GetObject(bitmap_, sizeof(bmp), &bmp);
            assert(weatherBitmapSize == bmp.bmWidth);
            assert(weatherBitmapSize == bmp.bmHeight);
            BOOL res = TransparentImage(dc, 0, 0, bmp.bmWidth, bmp.bmHeight, bitmap_, 0, 0, bmp.bmWidth, bmp.bmHeight, RGB(255, 0, 255));
        }
        return messageHandled;
    }
};

WeatherMainDialog::WeatherMainDialog():
    ModuleDialog(IDR_WEATHER_MENU),
    displayMode_(showSummary),
    bitmap_(NULL)
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

    // list_.create(WS_TABSTOP | LVS_REPORT | /* LVS_NOCOLUMNHEADER | */ LVS_SINGLESEL, r, handle());
    list_.attachControl(handle(), IDC_WEATHER_LIST); 
    list_.setStyleEx(LVS_EX_GRADIENT | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);

    combo_.attachControl(handle(), IDC_WEATHER_DAY);

    renderer_.create(WS_VISIBLE | WS_TABSTOP, SCALEX(5), combo_.height() + 2 * SCALEX(5), r.width() - 2 * SCALEX(5), r.height() - combo_.height() - SCALEY(5) * 2, handle());
    renderer_.definition.setHyperlinkHandler(GetHyperlinkHandler());
    renderer_.definition.setInteractionBehavior(0);
    renderer_.definition.setNavOrderOptions(Definition::navOrderFirst);
    
    WeatherBitmap* bmp = new_nt WeatherBitmap();
    if (NULL != bmp)
    {
        renderer_.bounds(r);
        r.set(r.x() + r.width() - weatherBitmapSize, r.y() + r.height() - weatherBitmapSize, weatherBitmapSize, weatherBitmapSize);
        if (bmp->create(r, handle()))
        {
            bitmap_ = bmp;
            SetWindowPos(bmp->handle(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
    }

    ModuleDialog::handleInitDialog(wnd, lp);

    createListColumns(); 
    setDisplayMode(displayMode_);
    resyncTempMenu(); 
   
    WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    if (NULL == prefs.location || 0 == Len(prefs.location))
    {
        char_t* loc = NULL; 
        if (IDOK == ChangeLocationDialog::showModal(loc, handle()))
        {
            free(prefs.location);
            prefs.location = loc;
        }
        else
        {
            ModuleRunMain();
            return false;
        }
    }   
    
Again:    
    if (NULL == prefs.udf)
    {
        const InfoManModule* m = ModuleGetById(moduleIdWeather);
        if (m->neverUpdated != m->lastUpdateTime)
        {
            Module::Time_t t = _time64(NULL);
            t -= m->lastUpdateTime;
            if (t > 24 * 60 * 60)
                goto Fetch;
            prefs.udf = UDF_ReadFromStream(weatherDataStream);
            if (NULL != prefs.udf)
                goto Again;
        }
Fetch:        
        status_t err = WeatherFetchData();
        if (errNone != err)
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
    }
    else 
    { 
        createComboItems();
        createListItems();
        prepareWeather(0);
        list_.focusItem(0);
    }
    return false; 
}

struct WeatherColumnDesc {
    UINT textId;
    uint_t width;
};

static const WeatherColumnDesc weatherColumns[] = {
    {IDS_WEATHER_DAY, 60},
    {IDS_WEATHER_TEMP, 40}
};  

void WeatherMainDialog::createListColumns()
{
    LVCOLUMN col;
    ZeroMemory(&col, sizeof(col));
    col.mask = LVCF_ORDER | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    ulong_t w = list_.width();
    for (ulong_t i = 0; i < ARRAY_SIZE(weatherColumns); ++i)
    {
        char_t* text = LoadString(weatherColumns[i].textId);
        if (NULL == text)
            goto Error; 
        col.iSubItem = i;
        col.iOrder = i;
        col.pszText = text;
        if (0 != i)
        {
            col.mask |= LVCF_FMT;
            col.fmt = LVCFMT_JUSTIFYMASK | LVCFMT_RIGHT;
        }
        col.cx = (weatherColumns[i].width * w) / 100;
        int res = ListView_InsertColumn(list_.handle(), i, &col);
        free(text);
        if (-1 == res)
            goto Error;
    }  
    return; 
Error: 
    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY); 
}

bool WeatherMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    switch (data->result)
    {
        case lookupResultWeatherData:
            PassOwnership(lm->udf, prefs.udf);
            assert(NULL != prefs.udf);
            prepareWeather(0);
            createComboItems();
            createListItems();
            setDisplayMode(showSummary);
            combo_.setSelection(0);
            list_.focusItem(0);
            ModuleTouchRunning();
            return true;
            
        case lookupResultLocationUnknown:
            Alert(IDS_ALERT_LOCATION_UNKNOWN);
            if (NULL == prefs.udf)
            {
                char_t* loc = StringCopy(prefs.location); 
                if (IDOK != ChangeLocationDialog::showModal(loc, handle()))
                    return true;

                free(prefs.location);
                prefs.location = loc;
                status_t err = WeatherFetchData();
                if (errNone != err)
                    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);

            }
            return true;

        case lookupResultWeatherMultiselect:
        {
            UniversalDataFormat* udf = NULL;
            PassOwnership(lm->udf, udf);
            handleMultiselect(udf);
            return true;
        }
        
    }
    return ModuleDialog::handleLookupFinished(event, data); 
}

bool WeatherMainDialog::handleMultiselect(UniversalDataFormat* udf)
{
    WeatherLocationListModel model(*udf);
    long sel = StringListDialog::showModal(IDS_CHOOSE_LOCATION, handle(), &model, StringListDialog::ownModelNot);
    if (-1 == sel)
    {
        delete udf;
        return false;
    }
    WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    char_t* loc = StringCopy(udf->getItemText(sel, 1));
    if (NULL == loc)
    {
        delete udf;
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return false;
    }
    free(prefs.location);
    prefs.location = loc;
    delete udf;
    status_t err = WeatherFetchData();
    if (errNone != err)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return false;
    }
    return true;
}

long WeatherMainDialog::handleCommand(ushort notify_code, ushort id, HWND sender)
{
    switch (id) 
    {
        case IDC_WEATHER_DAY:
            if (CBN_SELCHANGE == notify_code)
            {
                long item = combo_.selection();
                list_.focusItem(item);
                prepareWeather(item);
                return messageHandled;  
            }
            break;
        case ID_VIEW_DETAILED:
            setDisplayMode(showDetails);
            return messageHandled;
        case ID_VIEW_SUMMARY:
            setDisplayMode(showSummary);
            return messageHandled;
        case ID_VIEW_UPDATE:
        {
            WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
            if (NULL == prefs.location || 0 == Len(prefs.location))
            {
                changeLocation();
                return messageHandled;
            }
            status_t err = WeatherFetchData();
            if (errNone != err)
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return messageHandled; 
        }

        case ID_TEMPERATURE_CELSIUS:
        case ID_TEMPERATURE_FAHRENHEIT:
            changeTempScale(ID_TEMPERATURE_CELSIUS == id);
            return messageHandled;
            
        case ID_VIEW_CHANGE_LOCATION:
            changeLocation();
            return messageHandled;
    }
    return ModuleDialog::handleCommand(notify_code, id, sender);
}

void WeatherMainDialog::changeLocation()
{
    WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    char_t* loc = StringCopy(prefs.location);
    if (NULL == loc)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    if (IDOK == ChangeLocationDialog::showModal(loc, handle()))
    {
        free(prefs.location);
        prefs.location = loc;
        status_t err = WeatherFetchData();
        if (errNone != err)
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
    }
}

void WeatherMainDialog::changeTempScale(bool celsius)
{
    WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    if (prefs.celsiusMode == celsius)
        return;

    prefs.celsiusMode = celsius;
    resyncTempMenu();
    createListItems();
    prepareWeather(combo_.selection());
}

long WeatherMainDialog::handleResize(UINT sizeType, ushort width, ushort height)
{
    combo_.setBounds(SCALEX(5), SCALEY(5), width - 2 * SCALEX(5), combo_.height(), repaintWidget);
    long hh = 2 * SCALEY(5) + combo_.height();
    renderer_.anchor(anchorRight, 2 * SCALEX(5), anchorBottom, hh, repaintWidget);
    list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    if (NULL != bitmap_)
        bitmap_->setBounds(width - SCALEX(5) - weatherBitmapSize, height - SCALEY(5) - weatherBitmapSize, weatherBitmapSize, weatherBitmapSize, repaintWidget);

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

    switch (displayMode_ = dm)
    {
        case showSummary:
            if (NULL != bitmap_) 
                bitmap_->hide();
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
            if (NULL != bitmap_)
                bitmap_->show();
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
    item.mask = LVIF_PARAM | LVIF_TEXT;
    for (int i = 0; i < 7; ++i)
    {
        if (0 == i)
        {
            char_t* s = LoadString(IDS_TODAY);
            if (NULL == s)
            {
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
                return; 
            }
            tprintf(buffer, _T("%s"), s);
            free(s);
        }
        else
            GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, buffer, 64);

        item.iItem = item.lParam = i;
        item.pszText = buffer;
        item.iSubItem = 0;
        list_.insertItem(item);
        // TODO: add additional columns   
                
        
        TimeRollDays(st, 1);
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
            if (0 == GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, buffer, 64))
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

void WeatherMainDialog::prepareWeather(ulong_t index)
{
    const WeatherPrefs& prefs = GetPreferences()->weatherPrefs; 
    assert(NULL != prefs.udf); 
    DefinitionModel* model = WeatherExtractFromUDF(*prefs.udf, index);
    if (NULL == model)
    { 
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    renderer_.setModel(model, Definition::ownModel);

    if (NULL != bitmap_)
    {
        const char* sky = prefs.udf->getItemData(index + 1, dailySkyInUDF);
        WeatherCategory cat = WeatherGetCategory(sky);
        ((WeatherBitmap*)bitmap_)->setCategory(cat);
    }
}

long WeatherMainDialog::handleListItemActivate(int controlId, const NMLISTVIEW& header)
{
    long item = header.iItem;
    combo_.setSelection(item);
   
    prepareWeather(item);
    setDisplayMode(showDetails);
    return messageHandled;   
}


