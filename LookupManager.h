#ifndef INFOMAN_LOOKUP_MANAGER_H__
#define INFOMAN_LOOKUP_MANAGER_H__

#include <LookupManagerBase.hpp>
#include "InfoManGlobals.h"

enum LookupResult {
    lookupResultNone,
    
    lookupResultMoviesData,
    lookupResultLocationUnknown,
    lookupResultLocationAmbiguous,
    
    lookupResultWeatherData,
    lookupResultWeatherMultiselect,
    
    lookupResultRecipe,
    lookupResultRecipesList,

    lookupResult411NoCity, 
    lookupResult411TooManyResults,
    lookupResult411ReverseZip,
    lookupResult411ReverseArea,    
    lookupResult411AreaByCity,
    lookupResult411AreaByCityMultiselect,
    lookupResult411ZipByCity,
    lookupResult411ZipByCityMultiselect,
    lookupResult411PersonSearch,
    lookupResult411PersonSearchCityMultiselect, 
    lookupResult411ReversePhone,
    lookupResult411InternationalCode,
    lookupResult411BusinessSearch,
    lookupResult411BusinessSearchMultiselect,
    
    lookupResultBoxOfficeData,
    
    lookupResultDreamData,

    lookupResultJoke,
    lookupResultJokesList,

    lookupResultStock,
    lookupResultStocksList,
    lookupResultStocksListByName,

    lookupResultAmazon,

    lookupResultNetflix,
    lookupResultNetflixRequestPassword,
    lookupResultNetflixLoginUnknown,
    lookupResultNetflixLoginOk,
    lookupResultNetflixQueue,
    lookupResultNetflixAddOk,
    lookupResultNetflixAddFailed,
    lookupResultNetflixAddAlreadyInQueue,

    lookupResultListsOfBests,

    lookupResultQuotes,

    lookupResultLyrics,

    lookupResultCurrency,

    lookupResultGasPrices,

    lookupResultFlights,

    lookupResultEBay,
    lookupResultEBayNoCache,
    lookupResultEBayRequestPassword,
    lookupResultEBayLoginUnknown,
    lookupResultEBayLoginOk,

    lookupResultHoroscope,
    
    lookupResultConnectionCancelledByUser,

    lookupResultNoResults,  //to handle all no results from many modules

    lookupResultServerError,
    lookupResultError,

    lookupResultRegCodeValid,
    lookupResultRegCodeInvalid,
    
    lookupResultTvProviders,
    lookupResultTvListingsPartial,
    lookupResultTvListingsFull,

    lookupResultDictDef,
    
    lookupResultPediaArticle,
    lookupResultPediaSearch,
    lookupResultPediaLanguages,
    lookupResultPediaStats,
    
    lookupResultEBookSearchResults,
    lookupResultEBookDownload,
    lookupResultEBookBrowse,
    lookupResultEBookHome,
};

#define DEF_SERVER_ERR(error, code, alert, stringId) \
    error = code,

enum ServerError {
#include "ServerErrors.hpp"
};

struct LookupFinishedEventData: public ExtEventObject {
	LookupResult result;
	status_t error;
	ServerError serverError;	
};

const LookupFinishedEventData* LookupFinishedData(Event& event);

class DefinitionModel;
struct UniversalDataFormat;
class InfoManConnection;

class LookupManager: public LookupManagerBase {
public:
	
	LookupManager();

	~LookupManager();
	
	bool crossModuleLookup;
	const char_t* historyCacheName;
	const char_t* moduleName;
	
	
	DefinitionModel* definitionModel;
	UniversalDataFormat* udf;
	char_t** strings;
	ulong_t stringsCount;

	void setDefinitionModel(DefinitionModel* model);
	void setUDF(UniversalDataFormat* udf);
	void setStrings(char_t** strings, ulong_t stringsCount);
	
    DefinitionModel* releaseDefinitionModel();
    UniversalDataFormat* releaseUDF();
    void releaseStrings(char_t**& strings, ulong_t& stringsCount);  
	
	ulong_t regCodeDaysToExpire;
	enum {regCodeDaysNotSet = ulong_t(-1)};
	bool clientVersionChecked;
	
	long eBookVersion;
	
	
	status_t fetchUrl(const char* url);
	
	bool handleLookupFinishedInForm(Event& event);
	
	void acquire() {connectionManager().acquire();}
	void release() {connectionManager().release();}
	
	typedef Guard<LookupManager> Guard;
	

private:

    void handleConnectionError(status_t error);

    void handleServerError(ServerError serverError);

    InfoManConnection* createConnection();

    status_t enqueueConnection(InfoManConnection* conn);

};

void LookupManagerDispose();

#endif