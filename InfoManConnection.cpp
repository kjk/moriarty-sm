#include "InfoManConnection.h"
#include "FieldMetaData.h"
#include "InfoManGlobals.h"
#include "InfoManPreferences.h"

#include <SysUtils.hpp>
#include <DataStore.hpp>
#include <HistoryCache.hpp>
#include <UTF8_Processor.hpp>
#include <UniversalDataHandler.hpp>
#include <ByteFormatParser.hpp>
#include <StringListPayloadHandler.hpp>
#include <DeviceInfo.hpp>

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
    if (errNone != error)
		return error; 
    
    lookupManager_.setStatusText(_T("Resolving host..."));
    return ExtEventSendEmpty(extEventLookupStarted);
}

status_t InfoManConnection::open()
{
    status_t error = prepareRequest();
    if (errNone != error)
        return error;

    error = FieldPayloadProtocolConnection::open();
    if (errNone != error)
		return error;
		 
    lookupManager_.setStatusText(_T("Opening connection..."));
    return ExtEventSendEmpty(extEventLookupProgress);
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
    return ExtEventSendEmpty(extEventLookupProgress);
}

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

    LookupFinishedEventData* data = new_nt LookupFinishedEventData();
	if (NULL == data)
		return memErrNotEnoughSpace;
	
	data->result = result_; 
    if (lookupResultServerError == result_)
    {
        assert(serverErrorNone != serverError_);
        data->serverError = serverError_;
    }

/*    
    if (!hasDisabledModules_)
        resetDisabledRemotelyForAll(MORIARTY_MODULES_COUNT, MoriartyApplication::modules());
    const uint_t activeMods = activeModulesCount(MORIARTY_MODULES_COUNT, MoriartyApplication::modules());
    if (activeMods != initialActiveModulesCount_)
        sendEvent(MoriartyApplication::appActiveModulesCountChangedEvent); 
 */  
    return ExtEventSendObject(extEventLookupFinished, data);
} 

void InfoManConnection::handleError(status_t error)
{
    Log(eLogError, _T("InfoManConnection::handleError(): error code: "), false);
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

	if (lookupResultNone != currentField_->lookupResult)
	{
		assert(lookupResultNone == result_);
		result_ = currentField_->lookupResult;
	}
	
	if (NULL != currentField_->payloadCompletionHandler)
	{
		assert(fieldTypePayload == currentField_->type);
	}
	
	if (NULL != currentField_->valueHandler)
	{
		status_t err = (this->*(currentField_->valueHandler))(name, nameLen, value, valueLen);
		if (errNone != err)
			return err;
	}
	
    return errNone;
}

status_t InfoManConnection::handleUdfField(const char* name, ulong_t nlen, const char* value, ulong_t vlen)
{
	long len;
	if (errNone != numericValue(value, vlen, len))
		return errResponseMalformed;
	
	UniversalDataHandler* handler = new_nt UniversalDataHandler();
	if (NULL == handler)
		return memErrNotEnoughSpace;
	
	prepareWriter();
	startPayload(handler, len);
	return errNone;
}

status_t InfoManConnection::handleDefinitionModelField(const char* name, ulong_t nlen, const char* value, ulong_t vlen)
{
	long len;
	if (errNone != numericValue(value, vlen, len))
		return errResponseMalformed;
	
	ByteFormatParser* parser = new_nt ByteFormatParser();
	if (NULL == parser)
		return memErrNotEnoughSpace;
	
	prepareWriter();
	startPayload(parser, len);
	return errNone;	
}

status_t InfoManConnection::handleStringListField(const char* name, ulong_t nlen, const char* value, ulong_t vlen)
{
	long len;
	if (errNone != numericValue(value, vlen, len))
		return errResponseMalformed;
	
	StringListPayloadHandler* handler = new_nt StringListPayloadHandler();
	if (NULL == handler)
		return memErrNotEnoughSpace;

	UTF8_Processor* processor = new_nt UTF8_Processor(handler);
	if (NULL == processor)
	{
		delete handler;
		return memErrNotEnoughSpace;
	}
	
	prepareWriter();
	startPayload(processor, len);
	return errNone;
}


status_t InfoManConnection::completeUdfField(BinaryIncrementalProcessor& processor)
{
	UniversalDataHandler& handler = static_cast<UniversalDataHandler&>(processor);
	
	delete lookupManager_.udf;
	lookupManager_.udf = NULL;
	
	UniversalDataFormat* udf = new_nt UniversalDataFormat();
	if (NULL == udf)
		return memErrNotEnoughSpace;
	
	handler.universalData.swap(*udf);
	lookupManager_.udf = udf;
	return errNone;
}

status_t InfoManConnection::completeDefinitionModelField(BinaryIncrementalProcessor& processor)
{
	ByteFormatParser& parser = static_cast<ByteFormatParser&>(processor);
	
	delete lookupManager_.definitionModel;
	lookupManager_.definitionModel = parser.releaseModel();
	if (NULL == lookupManager_.definitionModel)
		return memErrNotEnoughSpace;
	
	return errNone;
}

status_t InfoManConnection::completeStringListField(BinaryIncrementalProcessor& processor)
{
	UTF8_Processor& p = static_cast<UTF8_Processor&>(processor);
	StringListPayloadHandler* handler = static_cast<StringListPayloadHandler*>(p.chainedTextProcessor());
	assert(NULL != handler);
	
	lookupManager_.setStrings(handler->strings, handler->stringsCount);
	handler->strings = NULL;
	handler->stringsCount = 0;
	
	return errNone;
}

status_t InfoManConnection::notifyPayloadFinished()
{
    assert(NULL != currentField_);
    if (NULL != writer_)
    {
        delete writer_;
        writer_ = NULL;
        if (currentField_->dataSinkIsHistoryCache && NULL != historyCache_)
            historyCache_->close();
    }
    if (NULL != currentField_->payloadCompletionHandler)
    {
		BinaryIncrementalProcessor* processor = releasePayloadHandler();
		assert(NULL != processor);
		status_t err = (this->*(currentField_->payloadCompletionHandler))(*processor);
		delete processor;
		if (errNone != err)
			return err;		 
    }
    return FieldPayloadProtocolConnection::notifyPayloadFinished();
}

status_t InfoManConnection::setUrl(const char* url)
{
	free(url_);
	url_ = StringCopy(url);
	if (NULL == url_)
		return memErrNotEnoughSpace;
	
	return errNone;
}

bool StrAppendField(char*& req, ulong_t& length, const char* name, const char* value = NULL, long vlen = -1)
{
	assert(NULL != name);
	ulong_t nlen = Len(name);
	req = StrAppend(req, length, name, nlen);
	if (NULL == req)
		return false;
	
	length += nlen;
	const char* sep = ": ";
	if (NULL == value)
		sep = ":\n";
	
	req = StrAppend(req, length, sep, 2);
	if (NULL == req)
		return false;
	
	length += 2;
	if (NULL == value)
		return true;
	
	if (-1 == vlen) 
		vlen = Len(value);
	req = StrAppend(req, length, value, vlen);
	if (NULL == req)
		return false;
	
	length += vlen;
	req = StrAppend(req, length, "\n", 1);
	if (NULL == req)
		return false;
	
	length += 1;
	return true;
}

bool StrAppendField(char*& req, ulong_t& length, const char* name, ulong_t value, bool hex = false)
{
	char buffer[12];
	if (hex) 
		StrPrintF(buffer, "%08x", value);
	else
		StrPrintF(buffer, "%lu", value);
	return StrAppendField(req, length, name, buffer);
}

#define protocolVersion "1"

#define verifyRegCodeField      "Verify-Registration-Code"
#define getRegCodeDaysToExpireField "Get-Reg-Code-Days-To-Expire"
#define getLatestClientVersionField "Get-Latest-Client-Version"
#define getUrl "Get-Url"


status_t InfoManConnection::prepareRequest()
{
	Preferences* prefs = GetPreferences();
	
	char* req = NULL;
	ulong_t len = 0;

	if (!StrAppendField(req, len, protocolVersionField, protocolVersion))
		return memErrNotEnoughSpace;
		
	if (!StrAppendField(req, len, clientInfoField, clientInfo))
		return memErrNotEnoughSpace;

	if (!StrAppendField(req, len, transactionIdField, transactionId, true))
		return memErrNotEnoughSpace;
		
	if (0 != Len(prefs->regCode))
	{
		if (!StrAppendField(req, len, regCodeField, prefs->regCode))
			return memErrNotEnoughSpace;
			
		if (LookupManager::regCodeDaysNotSet == lookupManager_.regCodeDaysToExpire 
		&& !StrAppendField(req, len, getRegCodeDaysToExpireField, (const char*)NULL))
			return memErrNotEnoughSpace;
	}
	else if (0 == Len(prefs->cookie))
	{
		char* token = deviceInfoToken();
		if (NULL == token)
		{
		    free(req);
		    return memErrNotEnoughSpace;
		}
		bool res = StrAppendField(req, len, getCookieField, token);
		free(token);
		if (!res)
		    return memErrNotEnoughSpace;
	}
	else 
	{
	    if (!StrAppendField(req, len, cookieField, prefs->cookie))
	        return memErrNotEnoughSpace;
	}

	if (!lookupManager_.clientVersionChecked && !StrAppendField(req, len, getLatestClientVersionField))
	    return memErrNotEnoughSpace;

	// TODO: check db stats in case of Pedia
	// TODO: send flickrPictureCount w/ 1st request
	
    if (NULL != url_ && !StrAppendField(req, len, 	getUrl, url_))
        return memErrNotEnoughSpace;
        
    req = StrAppend(req, len, "\n", 1);
    if (NULL == req)
        return memErrNotEnoughSpace;
    
    len += 1;
    
    setRequestOwn(req, len);       
	return errNone;
}