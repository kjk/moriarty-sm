#ifndef INFOMAN_CONNECTION_H__
#define INFOMAN_CONNECTION_H__

#include <FieldPayloadProtocolConnection.hpp>
#include "LookupManager.h"

class DataStoreWriter;
struct ResponseFieldDescriptor;
class HistoryCache;

class InfoManConnection: public FieldPayloadProtocolConnection
{
	LookupManager& lookupManager_;
	LookupResult result_;
	ServerError serverError_;
	
	char* url_;
	DataStoreWriter* writer_;
	HistoryCache* historyCache_;
	const ResponseFieldDescriptor* currentField_;
	
public:

	InfoManConnection(LookupManager& lm);

	~InfoManConnection();
	
	ulong_t transactionId;
	const char* url() const {return url_;}
	
	status_t setUrl(const char* url);

protected:

    friend class LookupManager;
    
    status_t enqueue();
    
    status_t handleField(const char* name, ulong_t nameLen, const char* value, ulong_t valueLen);
    
    void handleError(status_t error);
    
    status_t open();
    
    status_t notifyProgress();
    
    status_t notifyFinished();

    status_t notifyPayloadFinished();

    status_t handlePayloadIncrement(const char * payload, ulong_t& length, bool finish);
    
private:

	void prepareWriter();
	
	status_t prepareRequest();

public:

	status_t handleUdfField(const char* name, ulong_t nlen, const char* value, ulong_t vlen);
	status_t handleDefinitionModelField(const char* name, ulong_t nlen, const char* value, ulong_t vlen);
	status_t handleStringListField(const char* name, ulong_t nlen, const char* value, ulong_t vlen);
	
	status_t completeUdfField(BinaryIncrementalProcessor& processor);
	status_t completeDefinitionModelField(BinaryIncrementalProcessor& processor);
	status_t completeStringListField(BinaryIncrementalProcessor& processor);

	status_t handleTransactionIdField(const char* name, ulong_t nlen, const char* value, ulong_t vlen);
	
};

#endif