#ifndef INFOMAN_FIELD_METADATA_H__
#define INFOMAN_FIELD_METADATA_H__

#include "LookupManager.h"

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

#define fieldTransactionId "Transaction-ID"

#endif