#ifndef INFOMAN_WEATHER_MODULE_H__
#define INFOMAN_WEATHER_MODULE_H__

#include "Modules.h"
#include <Serializer.hpp>

MODULE_STARTER_DECLARE(Weather);

struct UniversalDataFormat;
class DefinitionModel;
DefinitionModel* WeatherExtractFromUDF(const UniversalDataFormat& udf, ulong_t index);

struct WeatherPrefs: public Serializable {
    char_t* location;
    char_t* locationToServer;
    bool celsiusMode;
   
    WeatherPrefs();
    ~WeatherPrefs();
   
   UniversalDataFormat* udf; 
    
    void serialize(Serializer& ser);
};    

enum WeatherDetailsUDFIndexes {
    detailedTemperatureInUDF,
    detailedSkyInUDF,
    detailedFeelsLikeInUDF,
    detailedUVIndexInUDF,
    detailedDevPointInUDF,
    detailedHumidityInUDF,
    detailedVisibilityInUDF,
    detailedPressureInUDF,
    detailedWindInUDF,
};

enum WeatherDailyUDFIndexes {
    dailyDayInUDF,
    dailyDateInUDF,
    dailySkyInUDF,
    dailyTemperatureDayInUDF,
    dailyTemperatureNightInUDF,
    dailyPrecipInUDF
};

inline int WeatherFahrenheitToCelsius(int f)
{
    f -= 32;
    if (f >= 0)
        return (10 * f + 5) / 18;
    else
        return (10 * f - 5) / 18;
} 

status_t WeatherFetchData();

#endif // INFOMAN_WEATHER_MODULE_H__