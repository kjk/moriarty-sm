#include "InfoManConnection.h"
#include "FieldMetaData.h"
#include <SysUtils.hpp>
#include <DataStore.hpp>
#include <HistoryCache.hpp>
#include <UTF8_Processor.hpp>

InfoManConnection::InfoManConnection(LookupManager& lm):
	FieldPayloadProtocolConnection(lm.connectionManager()),
	lookupManager_(lm),
	result_(lookupResultNone),
	serverError_(serverErrorNone),
	url_(NULL),
	transactionId(random(LONG_MAX)),
	writer_(NULL),
	currentField_(NULL),
	historyCache_(NULL)
{}
	
InfoManConnection::~InfoManConnection()
{
	free(url_);
	delete writer_;
	delete historyCache_;
}

status_t InfoManConnection::handleTransactionIdField(const char* name, ulong_t nlen, const char* value, ulong_t vlen)
{
	long val;
	status_t err = numericValue(value, vlen, val, 16);
	if (errNone != err || val != transactionId)
		return errResponseMalformed;
	return errNone;
}



void InfoManConnection::prepareWriter()
{
    status_t err;
    assert(NULL == writer_);
    if (NULL == currentField_->dataSinkName)
        return;
        
    if (currentField_->dataSinkIsHistoryCache)
    {
		if (NULL == historyCache_)
			historyCache_ = new_nt HistoryCache();
		
		if (NULL == historyCache_)
			return;
			
		char_t* name = UTF8_ToNative(currentField_->dataSinkName);
		if (NULL == name)
			return;
 
		err = historyCache_->open(name) ;
		free(name);
        if (errNone != err)
            return;
            
        ulong_t index;
        if (errNone != (err = historyCache_->appendEntry(url_, index)))
            return;
            
        writer_ = historyCache_->writerForEntry(index);
    }
    else
    {
        DataStore* ds = DataStore::instance();
        if (NULL == ds)
            return;
        
        DataStoreWriter* w = new_nt DataStoreWriter(*ds);
        if (NULL == w)
            return;
            
        if (errNone != (err = w->open(currentField_->dataSinkName)))
        {
            delete w;
            return;
        }
        
        writer_ = w;
    }
}

status_t InfoManConnection::handlePayloadIncrement(const char* payload, ulong_t& length, bool finish)
{
    status_t err = FieldPayloadProtocolConnection::handlePayloadIncrement(payload, length, finish);
    if (errNone != err)
        return err;
    
    if (NULL == writer_)
        return errNone;
        
    if (errNone != (err = writer_->write(payload, length)))
    {
        delete writer_;
        writer_ = NULL;
    }
    return errNone;
}

status_t InfoManConnection::enqueue()
{
    status_t error=FieldPayloadProtocolConnection::enqueue();
    if (errNone == error)
    {
        lookupManager_.setStatusText(_T("Resolving host..."));
        ExtEventSendEmpty(extEventLookupStarted);
    }        
    return error;
}

status_t InfoManConnection::open()
{
    status_t error = prepareRequest();
    if (errNone != error)
        return error;
    error = FieldPayloadProtocolConnection::open();
    if (errNone == error)
    {
        lookupManager_.setStatusText(_T("Opening connection..."));
        ExtEventSendEmpty(extEventLookupProgress);
    }        
    return error;        
}

status_t InfoManConnection::notifyProgress()
{
    status_t error = FieldPayloadProtocolConnection::notifyProgress();
    if (errNone != error)
        return error;
    lookupManager_.setStatusText(_T("Retrieving data..."));
    uint_t progress = LookupManager::percentProgressDisabled;
    if (inPayload_)
        progress=((payloadLength()-payloadLengthLeft()) * 100L)/payloadLength();
    lookupManager_.setPercentProgress(progress);
    ExtEventSendEmpty(extEventLookupProgress);
    return error;
}

/*
status_t InfoManConnection::notifyFinished()
{
    status_t error = FieldPayloadProtocolConnection::notifyFinished();
    if (errNone != error)
        return error;
    lookupManager_.setStatusText(_T("Finished"));
    assert(lookupResultError != result_);

    // if we didn't get any specific result frot the server
    // then change it into server failure error (shouldn't happen)
    if (lookupResultNone == result_)
    {
        result_ = lookupResultServerError;
        serverError_ = serverErrorFailure;
    }

    LookupFinishedEventData data(result_);
    if (lookupResultServerError==result_)
    {
        assert(serverErrorNone!=serverError_);
        data.serverError = serverError_;
    }
    if (!hasDisabledModules_)
        resetDisabledRemotelyForAll(MORIARTY_MODULES_COUNT, MoriartyApplication::modules());
    const uint_t activeMods = activeModulesCount(MORIARTY_MODULES_COUNT, MoriartyApplication::modules());
    if (activeMods != initialActiveModulesCount_)
        sendEvent(MoriartyApplication::appActiveModulesCountChangedEvent); 
    sendEvent(lookupManager_.lookupFinishedEvent, data);
    return error;        
} 
*/

void InfoManConnection::handleError(status_t error)
{
    Log(eLogError, _T(" MoriartyConnection::handleError(): error code: "), false);
    LogUlong(eLogError, error, true);
	LookupFinishedEventData* data = new_nt LookupFinishedEventData();
	if (NULL != data)
	{
		data->result = lookupResultError;
		data->error = error;
		ExtEventSendObject(extEventLookupFinished, data);
	} 
    FieldPayloadProtocolConnection::handleError(error);
}

status_t InfoManConnection::handleField(const char* name, ulong_t nameLen, const char* value, ulong_t valueLen)
{
    currentField_ = NULL;
	char* n = StringCopyN(name, nameLen);
	if (NULL == n)
		return memErrNotEnoughSpace;
	
	currentField_ = ResponseFieldFind(n);
	free(n); 
	
	if (NULL == currentField_)    
		return FieldPayloadProtocolConnection::handleField(name, nameLen, value, valueLen);
        
/*
    if (FieldDescriptor::fieldPayloadUDF == currentField_->type)
    {
        assert(NULL == currentField_->fieldHandler);
        long length;
        status_t error = numericValue(value, value + valueLen, length);
        if (errNone != error)
            return errResponseMalformed;
        
        prepareWriter();
        UniversalDataHandler* dataHandler = new_nt UniversalDataHandler();
        if (NULL != dataHandler)
            startPayload(dataHandler, length);
            
        return errNone;
    }
    if (NULL == currentField_->fieldHandler)
    {
        if (lookupResultNone != currentField_->lookupResult)
            setLookupResult(currentField_->lookupResult);
        return errNone;
    }
    FieldHandler handler = field->fieldHandler;

    String v;
    if (NULL != value)
        v.assign(value, valueLen);
    status_t err = (this->*handler)(n, v);
    if (errNone != err)
        return err;
    if (lookupResultNone != currentField_->lookupResult)
        setLookupResult(currentField_->lookupResult);
 */        
    return errNone;
}

