#include "Modules.h"
#include <Text.hpp>

/*
struct Module {
    ModuleID id;
    const char* name;
    char_t* displayName    
    uint_t iconId;
#ifdef _PALM_OS
    uint_t largeIconId;
#endif
    ModuleStarter starter;
    ModuleDataReader dataReader; 
    bool free;
    bool dataReady;
    bool disabledRemotely;
    bool disabledByUser;
    bool tracksUpdateTime;
    tick_t lastUpdateTime;
    
    bool active() const {return !(disabledByUser || disabledRemotely);}    
   
    enum {neverUpdated == ulong_t(-1)};     
};
*/


#ifdef _PALM_OS
#define MOD(id, name, dispName, palmIcon, palmLargeIcon, winIcon, mainFormId, starter, dataReader, free, tracksUpdate) \
{id, name, dispName, palmIcon, palmLargeIcon, mainFormId, dataReader, free, false, false, false, tracksUpdate, Module::neverUpdated}
#endif

// TODO: use commented-out version when data readers are ported to WinCE
#ifdef _WIN32
#define  MOD(id, name, dispName, palmIcon, palmLargeIcon, winIcon, mainFormId, starter, dataReader, free, tracksUpdate) \
{id, name, dispName, winIcon, starter, NULL, free, false, false, false, tracksUpdate, Module::neverUpdated}
// {id, name, dispName, winIcon, starter, dataReader, free, false, false, false, tracksUpdate, neverUpdated}
#endif

static Module modules[] = {
    MOD(moduleIdWeather, weatherModuleName, _T("Weather"), weatherSmallBitmap, frmInvalidObjectId, uint_t(-1), weatherMainForm, NULL, weatherDataRead, true, false),
    MOD(moduleId411, m411ModuleName, _T("Phone book"), m411SmallBitmap, frmInvalidObjectId, uint_t(-1), m411MainForm, NULL, m411DataRead, false, false),
    MOD(moduleIdMovies, moviesModuleName, _T("Movie times"), moviesSmallBitmap, frmInvalidObjectId, uint_t(-1), moviesMainForm, NULL, moviesDataRead, true, false),
    MOD(moduleIdAmazon, amazonModuleName, _T("Amazon"), amazonSmallBitmap, frmInvalidObjectId, uint_t(-1), amazonMainForm, NULL, NULL, false, false),
    MOD(moduleIdBoxOffice, boxOfficeModuleName, _T("Box office"), boxofficeSmallBitmap, frmInvalidObjectId, uint_t(-1), boxOfficeMainForm, NULL, boxOfficeDataRead, true, true),
    MOD(moduleIdCurrency, currencyModuleName, _T("Currency"), currencySmallBitmap, frmInvalidObjectId, uint_t(-1), currencyMainForm, NULL, currencyDataRead, true, false),
    MOD(moduleIdStocks, stocksModuleName, _T("Stocks"), stocksSmallBitmap, frmInvalidObjectId, uint_t(-1), stocksMainForm, NULL, stocksDataRead, true, false),
    MOD(moduleIdJokes, jokesModuleName, _T("Jokes"), jokesSmallBitmap, frmInvalidObjectId, uint_t(-1), jokesMainForm, NULL, jokesDataRead, false, true),
    MOD(moduleIdGasPrices, gasPricesModuleName, _T("Gas prices"), gasPricesSmallBitmap, frmInvalidObjectId, uint_t(-1), gasPricesMainForm, NULL, gasPricesDataRead, true, false),
    MOD(moduleIdHoroscopes, horoscopeModuleName, _T("Horoscopes"), horoscopesSmallBitmap, frmInvalidObjectId, uint_t(-1), horoscopesMainForm, NULL, horoscopeDataRead, true, true) ,
    MOD(moduleIdDreams, dreamsModuleName, _T("Dreams"), dreamsSmallBitmap, frmInvalidObjectId, uint_t(-1), dreamsMainForm, NULL, dreamsDataRead, false, true),
    MOD(moduleIdAbout, "about", _T("About"), aboutSmallBitmap, frmInvalidObjectId, uint_t(-1), frmInvalidObjectId, NULL, NULL, false, false),
#ifndef SHIPPING
    // this one's special: it never ships
    MOD(moduleIdTestWiki, "test", _T("Test parser"), aboutSmallBitmap, frmInvalidObjectId, uint_t(-1), testWikiMainForm, NULL, NULL, true, false),
#endif
#ifndef SHIPPING

    // WARNING!!!
    // When changing module to appear in "shipping" version remember to change this in MoriartyPreferences.cpp as well
    // (Preferences::serialize) and MoriartyApplication::createForm() in this file
    
    MOD(moduleIdNetflix, netflixModuleName, _T("Netflix"), netflixSmallBitmap, frmInvalidObjectId, uint_t(-1), netflixMainForm, NULL, netflixDataRead, false, false),
    MOD(moduleIdPedia, pediaModuleName, _T("Encyclopedia"), encyclopediaSmallBitmap, frmInvalidObjectId, uint_t(-1), pediaMainForm, NULL, NULL, true, false),
    MOD(moduleIdDict, dictModuleName, _T("Dictionary"), dictionarySmallBitmap, frmInvalidObjectId, uint_t(-1), dictMainForm, NULL, NULL, true, false),
    MOD(moduleIdLyrics, lyricsModuleName, _T("Lyrics"), lyricsSmallBitmap, frmInvalidObjectId, uint_t(-1), lyrics2MainForm, NULL, NULL, false, false),
    MOD(moduleIdRecipes, recipesModuleName, _T("Recipes"), epicuriousSmallBitmap, frmInvalidObjectId, uint_t(-1), epicuriousMainForm, NULL, epicuriousDataRead, false, false),
    MOD(moduleIdListsOfBests, listsOfBestsModuleName, _T("Lists of bests"), listofbestsSmallBitmap, frmInvalidObjectId, uint_t(-1), listsOfBestsMainForm, NULL, NULL, false, false),
    // MOD(moduleIdTvListings, _T("TV Listings"), tvListingsSmallBitmap, frmInvalidObjectId, uint_t(-1), tvListingsMainForm, NULL, NULL, true, false),
    MOD(moduleIdQuotes, quotesModuleName, _T("Quotes"), quotationsSmallBitmap, frmInvalidObjectId, uint_t(-1), quotesMainForm, NULL, quotesDataRead, true, true),
    MOD(moduleIdEBooks, ebookModuleName, _T("eBooks"),  ebooksSmallBitmap, frmInvalidObjectId, uint_t(-1), ebookMainForm, NULL, NULL, false, false),
    MOD(moduleIdFlights, flightsModuleName, _T("Flights"),  flightsSmallBitmap, frmInvalidObjectId, uint_t(-1), flightsMainForm, NULL, flightsDataRead, true, false),
    MOD(moduleIdEBay, eBayModuleName, _T("eBay"), eBaySmallBitmap, frmInvalidObjectId, uint_t(-1), eBayMainForm, NULL, NULL, false, false),
    MOD(moduleIdFlickr, flickrModuleName, _T("Flickr"), flickrSmallBitmap, frmInvalidObjectId, uint_t(-1), flickrMainForm, NULL, NULL, false, true),
#endif
};

ulong_t ModuleCount() 
{
    return ARRAY_SIZE(modules);
};

ulong_t ModuleActiveCount()
{
    ulong_t res = 0;
    for (ulong_t i = 0; i < ARRAY_SIZE(modules); ++i)
        if (modules[i].active())
            ++res;
    return res;  
}

Module* ModuleGet(ulong_t index)
{
    assert(index < ARRAY_SIZE(modules));
    return modules + index; 
}

Module* ModuleGetByName(const char* name)
{
    for (ulong_t i = 0; i < ARRAY_SIZE(modules); ++i)
        if (StrEquals(name, modules[i].name)) 
            return modules + i;
    assert(false);
    return NULL;
}

Module* ModuleGetById(ModuleID id)
{
    for (ulong_t i = 0; i < ARRAY_SIZE(modules); ++i)
        if (id == modules[i].id)
            return modules + i;
    assert(false);
    return NULL; 
}

Module* ModuleGetActive(ulong_t index)
{
    ulong_t count = 0;
    for (ulong_t i = 0; i < ARRAY_SIZE(modules); ++i)
        if (modules[i].active())
        {
            if (count == index)
                return modules + i;
            else
                ++count;
        }
    assert(false);
    return NULL; 
}
