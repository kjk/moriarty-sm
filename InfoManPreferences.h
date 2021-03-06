#ifndef INFOMAN_PREFERENCES_H__
#define INFOMAN_PREFERENCES_H__

#include "RecipesModule.h"
#include "HoroscopesModule.h"
#include "DreamsModule.h"
#include "StocksModule.h"
#include "WeatherModule.h"
#include "CurrencyModule.h"
#include "JokesModule.h"

#include <Utility.hpp>

class Serializer;

class Preferences: private NonCopyable {
public:

    RecipesPrefs recipesPrefs;
    HoroscopesPrefs horoscopesPrefs; 
    DreamsPrefs dreamsPrefs; 
    StocksPrefs stocksPrefs; 
    WeatherPrefs weatherPrefs; 
    CurrencyPrefs currencyPrefs;
    JokesPrefs jokesPrefs;

    Preferences();

    ~Preferences();

    char* serverAddress;
    enum {cookieLength=32};
    enum {regCodeLength=32};
    char* cookie;
    char* regCode;
    char* lastClientVersion;
    char_t* moviesLocation;

    status_t serialize(Serializer& ser);

};

void PrefsDispose();
void PrefsLoad();
void PrefsSave();

#endif