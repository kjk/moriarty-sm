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

#define fieldError                               "Error"
#define fieldCookie                            cookieField
#define fieldEBookVersion                  "eBook-Version"
#define fieldLatestClientVersion           "Latest-Client-Version"
#define fieldTransactionId                   transactionIdField

#define fieldGetUrlEBookBrowse          prefixFieldGetUrl "eBook-browse"

enum ResponseFieldType {
	fieldTypeValue,
	fieldTypePayload,
};

class BinaryIncrementalProcessor;
class InfoManConnection;

typedef status_t (InfoManConnection::* ResponseFieldValueHandler)(const char*, ulong_t, const char*, ulong_t);
typedef status_t (InfoManConnection::* ResponsePayloadCompletionHandler)(BinaryIncrementalProcessor&);

struct ResponseFieldDescriptor {
	const char* name;
	ResponseFieldType type;
	ResponseFieldValueHandler valueHandler;
	ResponsePayloadCompletionHandler payloadCompletionHandler;
	const char* dataSinkName;
	LookupResult lookupResult;
	bool dataSinkIsHistoryCache;
	
	bool operator<(const ResponseFieldDescriptor& other) const;
};

const ResponseFieldDescriptor* ResponseFieldFind(const char* name);

#ifndef NDEBUG
void test_ResponseFieldsSorted();
#endif

#endif