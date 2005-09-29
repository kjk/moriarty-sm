#ifndef INFOMAN_FIELD_METADATA_H__
#define INFOMAN_FIELD_METADATA_H__

#include "LookupManager.h"

#define prefixFieldGetUrl                         "Get-Url-"

#define fieldVerifyRegCode                     "Verify-Registration-Code"
#define fieldGetRegCodeDaysToExpire    "Get-Reg-Code-Days-To-Expire"
#define fieldGetLatestClientVersion          "Get-Latest-Client-Version"
#define fieldGetUrl                                  "Get-Url"
#define fieldClientInfo                             clientInfoField
#define fieldRegistrationCode                  regCodeField
#define fieldProtocolVersion                    protocolVersionField
#define fieldGetCookie                           getCookieField
#define fieldCurrentBoxOffice                 "Current-Box-Office"

#define fieldError                               "Error"
#define fieldCookie                            cookieField
#define fieldEBookVersion                  "eBook-Version"
#define fieldLatestClientVersion           "Latest-Client-Version"
#define fieldTransactionId                   transactionIdField
#define fieldNoResults                        "No-Results"


#define fieldHoroscope                       "Horoscope"
#define fieldDream                            "Dream-Interpretation"
#define fieldGetUrlEBookBrowse          prefixFieldGetUrl "eBook-browse"

#define fieldRecipesList                      "Recipes-List"
#define fieldRecipe                            "Recipe"

#define fieldStocksList                        "Stocks-List"
#define fieldStocksListByName            "Stocks-List-By-Name"
#define fieldStock                              "Stock"

#define fieldWeather                          "Weather"
#define fieldWeatherMultiselect           "Weather-Multiselect"
#define fieldLocationUnknown             "Location-Unknown"

#define fieldCurrency                    "Currency-Conversion"

#define fieldJoke                        "Joke"
#define fieldJokesList                   "Jokes-List"

enum ResponseFieldType {
    fieldTypeValue,
    fieldTypePayload,
};

class BinaryIncrementalProcessor;
class InfoManConnection;

typedef status_t (InfoManConnection::* ResponseFieldValueHandler)(const char*, ulong_t, const char*, ulong_t);
typedef status_t (InfoManConnection::* ResponsePayloadCompletionHandler)(BinaryIncrementalProcessor&);

struct ResponseFieldDescriptor {

// WARNING: Visual C++ 2005 B2 seems to be broken when aligning pointers-to-member-functions.
// That's why these ugly fillXxxx__ unions are used. (eVC 3.0 seems to be broken as well)
#if _MSC_VER

    union {
        const char* name;
        ulong_t fillName__;
    };

    union {
        ResponseFieldType type;
        ulong_t fillType__;
    };

    union {
        ResponseFieldValueHandler valueHandler;
        ulong_t fillValueHandler__[4];
    };
    
    union {
        ResponsePayloadCompletionHandler payloadCompletionHandler;
        ulong_t fillPayloadCompletionHandler__[4];
    };

    union {
        LookupResult lookupResult;
        ulong_t fillLookupResult__;
    };

    union {
        const char* dataSinkName;
        ulong_t fillDataSinkName__;
    };

    union {
        bool dataSinkIsHistoryCache;
        ulong_t fillDataSinkIsHistoryCache__;
    };
    
#else
    const char* name;
    ResponseFieldType type;
    ResponseFieldValueHandler valueHandler;
    ResponsePayloadCompletionHandler payloadCompletionHandler;
    LookupResult lookupResult;
    const char* dataSinkName;
    bool dataSinkIsHistoryCache;
#endif

    bool operator<(const ResponseFieldDescriptor& other) const;
};

const ResponseFieldDescriptor* ResponseFieldFind(const char* name);

#ifndef NDEBUG
void test_ResponseFieldsSorted();
#endif

#endif