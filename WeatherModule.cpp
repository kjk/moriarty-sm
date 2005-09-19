#include "WeatherModule.h"
#include "InfoManPreferences.h"
#include "HyperlinkHandler.h"
#include "LookupManager.h"

#include <UTF8_Processor.hpp>
#include <UniversalDataFormat.hpp>
#include <TextElement.hpp>
#include <LineBreakElement.hpp>
#include <Text.hpp>

#ifdef _WIN32
#include "WeatherMainDialog.h"
#endif

MODULE_STARTER_DEFINE(Weather)

WeatherPrefs::WeatherPrefs():
    location(NULL),
    locationToServer(NULL),
    celsiusMode(false),
    udf(NULL) 
{
}

WeatherPrefs::~WeatherPrefs()
{
    free(location);
    free(locationToServer);  
    delete udf; 
}   

void WeatherPrefs::serialize(Serializer& ser)
{
    ser.text(location);
    ser.text(locationToServer);
    ser(celsiusMode);  
}


#define prefixSky               _T("Sky: ")
#define prefixTemperatureDay    _T("Day: ")
#define betweenDayAndNight      _T("     ")
#define prefixTemperatureNight  _T("Night: ")
#define prefixPrecip            _T("Precipitation: ")

#define prefixFTemperature  _T("Now: ")
#define prefixFSky          _T("Sky: ")
#define prefixFFeelsLike    _T("Feels like: ")
#define prefixFUVIndex      _T("UV index: ")
#define prefixFDewPoint     _T("Dew point: ")    //ignored
#define prefixFHumidity     _T("Humidity: ")
#define prefixFVisibility   _T("Visibility: ")
#define prefixFPressure     _T("Pressure: ")
#define prefixFWind         _T("Wind: ")

static const char_t degSymbol = _T('\xB0');

#define TXT(txt) if (errNone != model->appendText(txt)) goto Error
#define LBR() if (errNone != model->append(new_nt LineBreakElement())) goto Error

DefinitionModel* WeatherExtractFromUDF(const UniversalDataFormat& udf, ulong_t i)
{
    const WeatherPrefs& prefs = GetPreferences()->weatherPrefs;
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;
        
    char_t buffer[32];
    char_t* text = NULL; 
    
    ++i;
    if (NULL == (text = StrAppend(text, -1, prefixSky, -1))) goto Error;
    if (NULL == (text = StrAppend(text, -1, udf.getItemText(i, dailySkyInUDF), -1))) goto Error;
    TXT(text);
    free(text); text = NULL;  
    LBR(); 

    if (!StrStartsWith(udf.getItemData(i, dailyDayInUDF), "Tonight"))
    {
        if (NULL == (text = StrAppend(text, -1, prefixTemperatureDay, -1))) goto Error;
        int d = udf.getItemNumericValue(i, dailyTemperatureDayInUDF);
        const char_t* ft = _T("%d\260F");
        if (prefs.celsiusMode)
        {
            d = WeatherFahrenheitToCelsius(d);
            ft = _T("%d\260C");
        }
        tprintf(buffer, ft, d);
        if (NULL == (text = StrAppend(text, -1, buffer, -1))) goto Error;
        if (NULL == (text = StrAppend(text, -1, betweenDayAndNight, -1))) goto Error;
        TXT(text);
        free(text); text = NULL;
    }  

    if (NULL == (text = StrAppend(text, -1, prefixTemperatureNight, -1))) goto Error;
    int d = udf.getItemNumericValue(i, dailyTemperatureNightInUDF);
    const char_t* ft = _T("%d\260F");
    if (prefs.celsiusMode)
    {
        d = WeatherFahrenheitToCelsius(d);
        ft = _T("%d\260C");
    }
    tprintf(buffer, ft, d);
    if (NULL == (text = StrAppend(text, -1, buffer, -1))) goto Error;
    TXT(text);
    free(text); text = NULL;
    
    LBR(); 
    if (NULL == (text = StrAppend(text, -1, prefixPrecip, -1))) goto Error;
    tprintf(buffer, _T("%ld%%"), udf.getItemNumericValue(i, dailyPrecipInUDF));
    if (NULL == (text = StrAppend(text, -1, buffer, -1))) goto Error;
    
    TXT(text);
    free(text); text = NULL;   
 
    LBR();
    
    if (1 != i)
        return model;
         
    if (NULL == (text = StrAppend(text, -1, prefixFSky, -1))) goto Error;
    if (NULL == (text = StrAppend(text, -1, udf.getItemText(0, detailedSkyInUDF), -1))) goto Error;
    TXT(text);
    free(text); text = NULL;
    LBR();
    
    if (NULL == (text = StrAppend(text, -1, prefixFTemperature, -1))) goto Error;
    d = udf.getItemNumericValue(0, detailedTemperatureInUDF);
    tprintf(buffer, ft, d);
    if (NULL == (text = StrAppend(text, -1, buffer, -1))) goto Error;
    TXT(text);
    free(text); text = NULL;
    LBR();
    
    if (NULL == (text = StrAppend(text, -1, prefixFFeelsLike, -1))) goto Error;
    d = udf.getItemNumericValue(0, detailedFeelsLikeInUDF);
    tprintf(buffer, ft, d);
    if (NULL == (text = StrAppend(text, -1, buffer, -1))) goto Error;
    TXT(text);
    free(text); text = NULL;
    LBR();

    if (NULL == (text = StrAppend(text, -1, prefixFHumidity, -1))) goto Error;
    tprintf(buffer, _T("%ld%%"), udf.getItemNumericValue(0, detailedHumidityInUDF));
    if (NULL == (text = StrAppend(text, -1, buffer, -1))) goto Error;
    TXT(text);
    free(text); text = NULL;
    LBR();
   
    if (NULL == (text = StrAppend(text, -1, prefixFUVIndex, -1))) goto Error;
    if (NULL == (text = StrAppend(text, -1, udf.getItemText(0, detailedUVIndexInUDF), -1))) goto Error;
    TXT(text);
    free(text); text = NULL;
    LBR();     
    
    // TODO: finish weather data transformations      
/*
       // visibility
        toDisplay.assign(prefixFVisibility).append(weatherData_->getItemText(0,detailedVisibilityInUDF));
        localizeNumber(toDisplay);
        elems.push_back(text=new TextElement(toDisplay));
        elems.push_back(new LineBreakElement());
        // pressure
        toDisplay.assign(prefixFPressure).append(weatherData_->getItemText(0,detailedPressureInUDF));
        localizeNumber(toDisplay);
        elems.push_back(text=new TextElement(toDisplay));
        elems.push_back(new LineBreakElement());
        // wind
        toDisplay.assign(prefixFWind).append(weatherData_->getItemText(0,detailedWindInUDF));
        elems.push_back(text=new TextElement(toDisplay));
        elems.push_back(new LineBreakElement());

    }    
 */  
        
    return model;
Error:
    free(text);
    delete model;
    return NULL;   
}

status_t WeatherFetchData()
{
    char* url = StringCopy(urlSchemaWeather urlSeparatorSchemaStr);
    if (NULL == url)
        return memErrNotEnoughSpace;
    
    const WeatherPrefs& prefs = GetPreferences()->weatherPrefs;  
    char* loc = UTF8_FromNative(prefs.location);
    if (NULL == loc)
    {
        free(url);
        return memErrNotEnoughSpace;
    }
       
    url = StrAppend(url, -1, loc, -1);
    free(loc);
    if (NULL == url)
        return memErrNotEnoughSpace;
        
    LookupManager* lm = GetLookupManager();
    status_t err = lm->fetchUrl(url);
    free(url);
    return err;  
}