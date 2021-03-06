#include "FieldMetaData.h"
#include "InfoManConnection.h"
#include "Modules.h"
#include <algorithm>
#include <Text.hpp>

/* 
static ResponseFieldDescriptor CreateDescriptor(const char* name, ResponseFieldType fieldType, ResponseFieldValueHandler valueHandler, ResponsePayloadCompletionHandler payloadHandler, LookupResult result, const char* dataSinkName, bool sinkIsHistoryCache)
{
    ResponseFieldDescriptor desc;
    desc.name = name;
    desc.lookupResult = result;
    desc.type = fieldType;
    desc.valueHandler = valueHandler;
    desc.payloadCompletionHandler = payloadHandler;
    desc.dataSinkName = dataSinkName;
    desc.dataSinkIsHistoryCache = sinkIsHistoryCache;
    return desc;        
}
 */

#define FIELD_NAME(name) field##name
#define FIELD_HANDLER(name) &InfoManConnection:: FIELD_HANDLER_NAME(name)

#define FIELD_VALUE_RESULT(name, handler, result) \
    {(name), fieldTypeValue, (handler), NULL, (result), NULL, false}
#define FIELD_VALUE(name, handler) FIELD_VALUE_RESULT(name, handler, lookupResultNone)

//#define FRES(name, result) \
//    FIELD_VALUE_RESULT(FIELD_NAME(name), NULL, (result))

#define FIELD_BCF_RESULT(name, result, dataSink, sinkIsHistoryCache) \
    {(name), fieldTypePayload, FIELD_HANDLER(DefinitionModel), &InfoManConnection::completeDefinitionModelField, (result), (const char*)(dataSink), (sinkIsHistoryCache)}

#define FBRS(name, result, dataSink, sinkIsHistoryCache) \
    FIELD_BCF_RESULT(FIELD_NAME(name), result, dataSink, sinkIsHistoryCache)
  
#define FVAL(name) \
    FIELD_VALUE(FIELD_NAME(name), FIELD_HANDLER(name)) 
#define FVRS(name, res) \
    FIELD_VALUE_RESULT(FIELD_NAME(name), FIELD_HANDLER(name), (res))
#define FRES(name, res) \
    FIELD_VALUE_RESULT(FIELD_NAME(name), NULL, (res))
   
#define FIELD_UDF_RESULT(name, result, dataSink, sinkIsHistoryCache) \
    {(name), fieldTypePayload, FIELD_HANDLER(Udf), &InfoManConnection::completeUdfField, (result), (const char*)(dataSink), (sinkIsHistoryCache)}

#define FURS(name, result, dataSink, sinkIsHistoryCache) \
    FIELD_UDF_RESULT(FIELD_NAME(name), result, dataSink, sinkIsHistoryCache) 
    
#define FIELD_STRING_LIST_RESULT(name, result) \
    {(name), fieldTypePayload, FIELD_HANDLER(StringList), &InfoManConnection::completeStringListField, (result), NULL, false}

#define FSLR(name, res) \
    FIELD_STRING_LIST_RESULT(FIELD_NAME(name), (res))

static const ResponseFieldDescriptor descriptors[] = {
    FVAL(Cookie),
    FURS(Currency, lookupResultCurrency, currencyDataStream, false),
    FURS(CurrentBoxOffice, lookupResultBoxOfficeData, boxOfficeDataStream, false), 
    FURS(Dream, lookupResultDreamData, dreamsDataStream, false), 
    FVRS(Error, lookupResultServerError), 
    FBRS(GetUrlEBookBrowse, lookupResultEBookBrowse, ebookHistoryCacheName, true), 
    FURS(Horoscope, lookupResultHoroscope, horoscopeDataStream, false), 
    FURS(Joke, lookupResultJoke, jokesJokeStream, false),
    FURS(JokesList, lookupResultJokesList, jokesJokesListStream, false),
    FVAL(LatestClientVersion),
    FSLR(LocationAmbiguous, lookupResultLocationAmbiguous),
    FRES(LocationUnknown, lookupResultLocationUnknown),
    FURS(MoviesData, lookupResultMoviesData, moviesDataStream, false),
    FRES(NoResults, lookupResultNoResults),
    FURS(Recipe, lookupResultRecipe, recipesItemStream, false), 
    FURS(RecipesList, lookupResultRecipesList, recipesListStream, false), 
    FURS(Stock, lookupResultStock, NULL, false), 
    FURS(StocksList, lookupResultStocksList, NULL, false), 
    FURS(StocksListByName, lookupResultStocksListByName, NULL, false), 
    FVAL(TransactionId),
    FURS(Weather, lookupResultWeatherData, weatherDataStream, false),
    FURS(WeatherMultiselect, lookupResultWeatherMultiselect, NULL, false),
    FVAL(EBookVersion),
};


bool ResponseFieldDescriptor::operator <(const ResponseFieldDescriptor& other) const
{
    using namespace std;
    return strcmp(name, other.name) < 0;
}

const ResponseFieldDescriptor* ResponseFieldFind(const char* name)
{
    ResponseFieldDescriptor desc = {name};
    const ResponseFieldDescriptor* end = descriptors + ARRAY_SIZE(descriptors);
    const ResponseFieldDescriptor* res = std::lower_bound(descriptors, end, desc);
    if (res == end)
        return NULL;

    if (!StrEquals(name, res->name))
        return NULL;

    return res;
}

#ifndef NDEBUG
void test_ResponseFieldsSorted()
{
    for (ulong_t i = 1; i < ARRAY_SIZE(descriptors); ++i)
    {
        const ResponseFieldDescriptor& prev = descriptors[i - 1];
        const ResponseFieldDescriptor& curr = descriptors[i];
        assert(prev < curr);
    }  
}
#endif
