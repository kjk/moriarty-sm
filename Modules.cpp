#include "Modules.h"
#include "InfoMan.h"

#include "WeatherModule.h"
#include "RecipesModule.h"
#include "HoroscopesModule.h"
#include "BoxOfficeModule.h"
#include "DreamsModule.h"
#include "StocksModule.h"

#ifdef _WIN32
#include "ModuleDialog.h"
#endif

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
#define MOD(id, name, dispName, palmIcon, palmLargeIcon, winIcon, mainFormId, free, tracksUpdate) \
{id, name, dispName, palmIcon, palmLargeIcon, mainFormId, free, false, false, false, tracksUpdate, Module::neverUpdated}
#endif

// TODO: use commented-out version when data readers are ported to WinCE
#ifdef _WIN32

#define MOD_ICON_LARGE(name) IDB_ ## name ## _LARGE
#define MOD_ICON_SMALL(name) IDB_ ## name ## _SMALL

#define  MOD(id, name, dispName, palmIcon, palmLargeIcon, winIcon, mainFormId, starter, free, tracksUpdate) \
{id, name, dispName, MOD_ICON_SMALL(winIcon), MOD_ICON_LARGE(winIcon), starter, free, false, false, false, tracksUpdate, Module::neverUpdated}
// {id, name, dispName, winIcon, starter, dataReader, free, false, false, false, tracksUpdate, neverUpdated}
#endif

static Module modules[] = {
    MOD(moduleIdWeather, weatherModuleName, _T("Weather"), weatherSmallBitmap, frmInvalidObjectId, WEATHER, weatherMainForm, WeatherStart, true, true),
    MOD(moduleId411, m411ModuleName, _T("Phone book"), m411SmallBitmap, frmInvalidObjectId, M411, m411MainForm, NULL, false, false),
    MOD(moduleIdMovies, moviesModuleName, _T("Movie times"), moviesSmallBitmap, frmInvalidObjectId, MOVIES, moviesMainForm, NULL, true, false),
    MOD(moduleIdAmazon, amazonModuleName, _T("Amazon"), amazonSmallBitmap, frmInvalidObjectId, AMAZON, amazonMainForm, NULL, false, false),
    MOD(moduleIdBoxOffice, boxOfficeModuleName, _T("Box office"), boxofficeSmallBitmap, frmInvalidObjectId, BOXOFFICE, boxOfficeMainForm, BoxOfficeStart, true, true),
    MOD(moduleIdCurrency, currencyModuleName, _T("Currency"), currencySmallBitmap, frmInvalidObjectId, CURRENCY, currencyMainForm, NULL, true, false),
    MOD(moduleIdStocks, stocksModuleName, _T("Stocks"), stocksSmallBitmap, frmInvalidObjectId, STOCKS, stocksMainForm, StocksStart, true, false),
    MOD(moduleIdJokes, jokesModuleName, _T("Jokes"), jokesSmallBitmap, frmInvalidObjectId, JOKES, jokesMainForm, NULL, false, true),
    MOD(moduleIdGasPrices, gasPricesModuleName, _T("Gas prices"), gasPricesSmallBitmap, frmInvalidObjectId, GAS, gasPricesMainForm, NULL, true, false),
    MOD(moduleIdHoroscopes, horoscopeModuleName, _T("Horoscopes"), horoscopesSmallBitmap, frmInvalidObjectId, HOROSCOPES, horoscopesMainForm, HoroscopesStart, true, true) ,
    MOD(moduleIdDreams, dreamsModuleName, _T("Dreams"), dreamsSmallBitmap, frmInvalidObjectId, DREAMS, dreamsMainForm, DreamsStart, false, true),
    MOD(moduleIdAbout, "about", _T("About"), aboutSmallBitmap, frmInvalidObjectId, ABOUT, frmInvalidObjectId, NULL, false, false),
#ifndef SHIPPING
    // this one's special: it never ships
    MOD(moduleIdTestWiki, "test", _T("Test parser"), aboutSmallBitmap, frmInvalidObjectId, NONE, testWikiMainForm, NULL, true, false),
#endif
#ifndef SHIPPING

    // WARNING!!!
    // When changing module to appear in "shipping" version remember to change this in MoriartyPreferences.cpp as well
    // (Preferences::serialize) and MoriartyApplication::createForm() in this file
    
    MOD(moduleIdNetflix, netflixModuleName, _T("Netflix"), netflixSmallBitmap, frmInvalidObjectId, NETFLIX, netflixMainForm, NULL, false, false),
    MOD(moduleIdPedia, pediaModuleName, _T("Encyclopedia"), encyclopediaSmallBitmap, frmInvalidObjectId, ENCYCLOPEDIA, pediaMainForm, NULL, true, false),
    MOD(moduleIdDict, dictModuleName, _T("Dictionary"), dictionarySmallBitmap, frmInvalidObjectId, DICTIONARY, dictMainForm, NULL, true, false),
    MOD(moduleIdLyrics, lyricsModuleName, _T("Lyrics"), lyricsSmallBitmap, frmInvalidObjectId, LYRICS, lyrics2MainForm, NULL, false, false),
    MOD(moduleIdRecipes, recipesModuleName, _T("Recipes"), epicuriousSmallBitmap, frmInvalidObjectId, RECIPES, epicuriousMainForm, RecipesStart, false, false),
    MOD(moduleIdListsOfBests, listsOfBestsModuleName, _T("Lists of bests"), listofbestsSmallBitmap, frmInvalidObjectId, LISTOFBESTS, listsOfBestsMainForm, NULL, false, false),
    // MOD(moduleIdTvListings, _T("TV Listings"), tvListingsSmallBitmap, frmInvalidObjectId, NONE, tvListingsMainForm, NULL, true, false),
    MOD(moduleIdQuotes, quotesModuleName, _T("Quotes"), quotationsSmallBitmap, frmInvalidObjectId, QUOTES, quotesMainForm, NULL, true, true),
    MOD(moduleIdEBooks, ebookModuleName, _T("eBooks"),  ebooksSmallBitmap, frmInvalidObjectId, EBOOKS, ebookMainForm, NULL, false, false),
    MOD(moduleIdFlights, flightsModuleName, _T("Flights"),  flightsSmallBitmap, frmInvalidObjectId, FLIGHTS, flightsMainForm, NULL, true, false),
    MOD(moduleIdEBay, eBayModuleName, _T("eBay"), eBaySmallBitmap, frmInvalidObjectId, EBAY, eBayMainForm, NULL, false, false),
    MOD(moduleIdFlickr, flickrModuleName, _T("Flickr"), flickrSmallBitmap, frmInvalidObjectId, FLICKR, flickrMainForm, NULL, false, true),
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

static InfoManModule* runningModule = NULL;

Module* ModuleGetRunning()
{
    return runningModule;
}

ModuleID ModuleGetRunningId()
{
    if (NULL == runningModule)
        return moduleIdNone;
    
    return runningModule->id;  
}

status_t ModuleRun(ModuleID id)
{
    status_t err = errNone; 
    InfoManModule* mod = NULL;
    
    if (moduleIdNone != id)
    {  
        mod = ModuleGetById(id);
        if (NULL == mod || !mod->active())
            return sysErrParamErr;
    };
        
#ifdef _WIN32
    ModuleDialogDestroyCurrent();
    if (moduleIdNone != id)
    {
        if (NULL == mod->starter)
            return sysErrParamErr;
             
        ModuleDialog* d = mod->starter();
        if (NULL != d)
        {
            runningModule = mod;
            ModuleDialogSetCurrent(d); 
            d->show();
        }
        else
        {
            DWORD err = GetLastError();
            assert(false);
            return sysErrParamErr;
            // TODO: show some alert?!
        }
        
    }   
    else
        runningModule = mod; 
#endif
    
#ifdef _PALM_OS
    if (moduleIdNone != id)
        if (frmInvalidObjectId != mod->mainFormId)
            FrmGotoForm(mod->mainFormId);
        else
        {
            assert(moduleIdAbout == id);
            // TODO: show about form            
        }
    }
    else
        FrmGotoForm(mainForm);

    runningModule = mod;
#endif

    return errNone; 
}

void ModuleTouchRunning()
{
    if (NULL == runningModule)
        return;

    if (!runningModule->tracksUpdateTime)
        return;

#ifdef _PALM_OS
    runningModule->lastUpdateTime = ticks();  
#endif

#ifdef _WIN32
    _time64(&runningModule->lastUpdateTime);
#endif 
}
