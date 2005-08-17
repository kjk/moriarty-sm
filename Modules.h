#ifndef INFOMAN_MODULES_H__
#define INFOMAN_MODULES_H__

#include <Debug.hpp>

enum ModuleID {
    moduleIdAbout,
    moduleId411,
    moduleIdAmazon,
    moduleIdBoxOffice,
    moduleIdCurrency,
    moduleIdDreams,
    moduleIdGasPrices,
    moduleIdJokes,
    moduleIdListsOfBests,
    moduleIdMovies,
    moduleIdNetflix,
    moduleIdRecipes,
    moduleIdStocks,
    moduleIdWeather,
    moduleIdHoroscopes,
    moduleIdLyrics,
    moduleIdTestWiki,
    moduleIdTvListings,
    moduleIdPedia,
    moduleIdDict,
    moduleIdQuotes,
    moduleIdEBooks,
    moduleIdFlights,
    moduleIdEBay,
    moduleIdFlickr,
   
    // Add other ids before this
    moduleIdNone = uint_t(-1) 
};

#ifdef _WIN32
class ModuleDialog;
typedef ModuleDialog* (*ModuleStarter)();
#endif

typedef status_t (*ModuleDataReader)();

struct Module {
    ModuleID id;
    const char* name;
    char_t* displayName;    
    uint_t iconId;
#ifdef _PALM_OS
    uint_t largeIconId;
    uint_t mainFormId; 
#endif
#ifdef _WIN32
    ModuleStarter starter;
#endif 
    ModuleDataReader dataReader; 
    bool free;
    bool dataReady;
    bool disabledRemotely;
    bool disabledByUser;
    bool tracksUpdateTime;
    tick_t lastUpdateTime;
    
    bool active() const {return !(disabledByUser || disabledRemotely);}    
   
    enum {neverUpdated = tick_t(-1)};     
};

ulong_t ModuleCount();
ulong_t ModuleActiveCount();
Module* ModuleGet(ulong_t index);
Module* ModuleGetByName(const char* name);
Module* ModuleGetById(ModuleID id);
Module* ModuleGetActive(ulong_t index);

Module* ModuleGetRunning();
ModuleID ModuleGetRunningId();
status_t ModuleRun(ModuleID id);
inline void ModuleRunMain() {ModuleRun(moduleIdNone);}
void ModuleTouchRunning();

#define MODULE_DATA_STREAM(name) name "-" dataStreamPostfix
#define MODULE_PREFS_STREAM(name) name "-" prefsStreamPostfix
#define MODULE_HISTORY_CACHE(name) _T(name) _T("-HistoryCache")

#define moviesModuleName       "movies"
#define moviesDataStream  MODULE_DATA_STREAM(moviesModuleName)
// #define moviesPrefsStream MODULE_PREFS_STREAM(moviesModuleName)

#define weatherModuleName      "weather"
#define weatherDataStream  MODULE_DATA_STREAM(weatherModuleName)
 //#define weatherPrefsStream MODULE_PREFS_STREAM(weatherModuleName)

#define m411ModuleName         "411"
#define m411DataStream  MODULE_DATA_STREAM(m411ModuleName)
// #define m411PrefsStream MODULE_PREFS_STREAM(m411ModuleName)
#define m411PersonDataStream   MODULE_DATA_STREAM(m411ModuleName "-person")
#define m411BusinessDataStream MODULE_DATA_STREAM(m411ModuleName "-business")

#define recipesModuleName                             "recipes"
#define recipesListStream epicuriousModuleName "-recipesList"
#define recipesItemStream epicuriousModuleName "-recipe"
// #define epicuriousPrefsStream MODULE_PREFS_STREAM(epicuriousModuleName)

#define boxOfficeModuleName                  "boxOffice"
#define boxOfficeDataStream MODULE_DATA_STREAM(boxOfficeModuleName)

#define currencyModuleName                   "currency"
#define currencyDataStream MODULE_DATA_STREAM(currencyModuleName)

#define dreamsModuleName                     "dreams"
#define dreamsDataStream MODULE_DATA_STREAM(dreamsModuleName)

#define horoscopeModuleName                  "horoscope"
#define horoscopeDataStream MODULE_DATA_STREAM(horoscopeModuleName)

#define jokesModuleName                      "jokes"
#define jokesJokesListStream jokesModuleName "-jokesList"
#define jokesJokeStream      jokesModuleName "-joke"

#define stocksModuleName                        "stocks"
#define stocksStocksListStream stocksModuleName "-stocksList"

#define gasPricesModuleName                 "gasPrices"
#define gasPricesStream MODULE_DATA_STREAM(gasPricesModuleName)

#define amazonModuleName                        "amazon"
#define amazonHistoryCacheName MODULE_HISTORY_CACHE(amazonModuleName)

#define netflixModuleName                         "netflix"
#define netflixHistoryCacheName MODULE_HISTORY_CACHE(netflixModuleName)
// #define netflixDataStream netflixModulePrefix dataStreamPostfix

#define listsOfBestsModuleName                              "listsOfBests"
#define listsOfBestsHistoryCacheName MODULE_HISTORY_CACHE(listsOfBestsModuleName)

#define lyricsModuleName                          "lyrics"
#define lyricsHistoryCacheName MODULE_HISTORY_CACHE(lyricsModuleName)

#define dictModuleName                         "dict"
#define dictDefStream          dictModuleName  "-def"
#define dictHistoryCacheName   MODULE_HISTORY_CACHE(dictModuleName)

#define pediaModuleName                        "pedia"
#define pediaHistoryCacheName MODULE_HISTORY_CACHE(pediaModuleName)

#define quotesModuleName                         "quotes"
#define quotesDataStream MODULE_DATA_STREAM(quotesModuleName)

#define ebookModuleName                           "ebooks"
#define ebookHistoryCacheName               MODULE_HISTORY_CACHE(ebookModuleName)
#define ebookWelcomeCacheName            _T(ebookModuleName) _T("-WelcomeCache")

#define flightsModuleName                         "flights"
#define flightsDataStream MODULE_DATA_STREAM(flightsModuleName)

#define eBayModuleName                         "eBay"
#define eBayHistoryCacheName MODULE_HISTORY_CACHE(eBayModuleName)

#define flickrModuleName "flickr"

#endif