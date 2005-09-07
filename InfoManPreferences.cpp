#include "InfoManPreferences.h"
#include "InfoManGlobals.h"
#include <Serializer.hpp>
#include <Text.hpp>
#include <Logging.hpp>
#include <DataStore.hpp>

static Preferences* prefs = NULL;

void PrefsLoad()
{
	assert(NULL != prefs);
	
	DataStore* ds = DataStore::instance();
	if (NULL == ds)
	{
		Log(eLogError, _T("PrefsLoad(): DataStore::instance() is NULL"), true);
		return;
	}
	
    DataStoreReader reader(*ds);
    status_t err = reader.open(globalPrefsStream);
	if (errNone != err)
	{
		LogStrUlong(eLogError, _T("PrefsLoad(): DataStoreReader::open() returned error: "), err);
		return;
	}
	
    Serializer serializer(reader);
    err = prefs->serialize(serializer);
    if (errNone != err)
    {
        LogStrUlong(eLogError, _T("PrefsLoad(): Preferences::serialize() returned error: "), err);
        return;
    }
}

void PrefsSave()
{
	assert(NULL != prefs);
	
	DataStore* ds = DataStore::instance();
	if (NULL == ds)
	{
		Log(eLogError, _T("PrefsSave(): DataStore::instance() is NULL"), true);
		return;
	}
	
    DataStoreWriter writer(*ds);
    status_t err = writer.open(globalPrefsStream);
    if (errNone != err)
	{
		LogStrUlong(eLogError, _T("PrefsSave(): DataStoreWriter::open() returned error: "), err);
		return;
	}
	
    Serializer serializer(writer);
    err = prefs->serialize(serializer);
    if (errNone != err)
        LogStrUlong(eLogError, _T("PrefsSave: Preferences::serialize() returned error: "), err);
}

static void PrefsInitialize()
{
	assert(NULL == prefs);
	prefs = new_nt Preferences();
	if (NULL == prefs)
		return;

	PrefsLoad();
}

Preferences* GetPreferences() 
{
	if (NULL == prefs)
		PrefsInitialize();

	return prefs;
}

void PrefsDispose()
{
	delete prefs;
	prefs = NULL;
}

Preferences::Preferences():
	serverAddress(StringCopy(SERVER_ADDRESS)),
	cookie(StringCopy("")),
	regCode(StringCopy("")),
	lastClientVersion(NULL)
{}

Preferences::~Preferences()
{
	free(serverAddress);
	free(cookie);
	free(regCode);
	free(lastClientVersion);
}


// NOTE: don't add in the middle of this enum or re-arrange the order.
// We need ids to be stable across different versions of the program
// (i.e. when user upgrades the program we want the new version to be
// able to use preferences from previous version)
enum {
    serialIdServerAddress,
    serialIdCookie,
    serialIdRegCode,
    serialIdKeybardLaunchEnabled,
    serialIdRenderingPreferences,
    
    // This id is declared "frozen" and can't be used. 
    // It was serialized out in shipping version of InfoMan in which this module wasn't available.
    // To prevent serialization errors I assign to such modules fresh id and disable their
    // serialization until they become available.
    _serialIdEpicuriousPreferencesFrozen, 
    
    serialIdMoviesLocation,
    serialIdWeatherPreferences,
    serialIdJokesPreferences,
    serialIdStocksPreferences,
    serialIdCurrencyPreferences,
    _serialIdAmazonPreferencesFrozen,
    _serialIdNetflixPreferencesFrozen,
    serialIdGasPricesPreferences,
    _serialIdListsOfBestsPreferencesFrozen,
    serialIdHoroscopesPreferences,
    serialIdModulesDisabledStatus, 
    // Added alias for previous id to accomodate change in object's semantics
    serialIdModulesState = serialIdModulesDisabledStatus,
    serialIdBoxOfficeCompactView,
    _serialIdLyricsPreferencesFrozen,
    serialIdLatestClientVersion,
    serialIdTvListingsPreferences,
    serialIdPediaPreferences,
    _serialIdEpicuriousPreferences, // renamed to serialIdRecipesPreferences as it's more general
    serialIdRecipesPreferences =  _serialIdEpicuriousPreferences,
    serialIdAmazonPreferences,
    serialIdNetflixPreferences,
    serialIdListsOfBestsPreferences,
    serialIdLyricsPreferences,
    serialIdFlightsPreferences,
    serialIdEBookPreferences,
    serialIdEBayPreferences,
    serialIdFlickrPreferences,
    serialIdMainFromView,
    serialIdDictionaryPreferences,
    
    serialIdDreamsPreferences,  
};

	
status_t Preferences::serialize(Serializer& serialize)
{
	ErrTry {
		serialize.narrow(cookie, NULL, serialIdCookie);
		serialize.narrow(regCode, NULL, serialIdRegCode);
		serialize(recipesPrefs, serialIdRecipesPreferences);
		serialize(horoscopesPrefs, serialIdHoroscopesPreferences);
		serialize(dreamsPrefs, serialIdDreamsPreferences);
		serialize(stocksPrefs, serialIdStocksPreferences);
	}
	ErrCatch (ex) {
		return ex;
	} ErrEndCatch;
	return errNone;
}
