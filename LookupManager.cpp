#include "LookupManager.h"
#include "InfoManConnection.h"
#include "InfoManPreferences.h"
#include <Definition.hpp>
#include <UniversalDataFormat.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

#ifdef _WIN32
#include "ConnectionProgressDialog.h"
#endif

struct ServerErrorMapping {
	ServerError code;
	uint_t alertId;
};

#ifdef _WIN32
#include "resource.h"
#define DEF_SERVER_ERR(error, code, palmAlert, winAlert) {(error), (winAlert)},
#endif

#ifdef _PALM_OS
#define DEF_SERVER_ERR(error, code, palmAlert, winAlert) {(error), (palmAlert)},
#endif

static const ServerErrorMapping serverErrors[] =  {
#include "ServerErrors.hpp"
};

static LookupManager* lookupManager = NULL;

LookupManager* GetLookupManager()
{
	if (NULL == lookupManager)
		lookupManager = new_nt LookupManager();
	return lookupManager;
}

void LookupManagerDispose()
{
	delete lookupManager;
	lookupManager = NULL;
}

LookupManager::LookupManager():
	LookupManagerBase(extEventLookupStarted),
	definitionModel(NULL),
	udf(NULL),
	strings(NULL),
	stringsCount(0),
	
	regCodeDaysToExpire(regCodeDaysNotSet),
	clientVersionChecked(false),
	eBookVersion(-1)
{}

LookupManager::~LookupManager()
{
	delete definitionModel;
	delete udf;
	StrArrFree(strings, stringsCount);
}

void LookupManager::setDefinitionModel(DefinitionModel* model)
{
    Guard g(*this);
	delete definitionModel;
	definitionModel = model;
}

void LookupManager::setUDF(UniversalDataFormat* udff)
{
    Guard g(*this);
	delete udf;
	udf = udff;
}

void LookupManager::setStrings(char_t** ss, ulong_t sc)
{
    Guard g(*this);
	StrArrFree(strings, stringsCount);
	strings = ss;
	stringsCount = sc;
}

void LookupManager::handleServerError(ServerError serverError)
{
    assert(serverErrorNone != serverError);

    uint_t errorsCount = ARRAY_SIZE(serverErrors);
#ifdef _PALM_OS
    uint_t alertId = unknownServerErrorAlert;
#endif
#ifdef _WIN32
	uint_t alertId = IDS_ALERT_UNKNOWN_SERVER_ERROR;
#endif   
    for (uint_t i = 0; i < errorsCount; ++i)
    {
        if (serverErrors[i].code == serverError)
        {
            alertId = serverErrors[i].alertId;
            break;
        }
    }
	Alert(alertId);
}

struct ErrorMapping {
	status_t error;
	uint_t alertId;
};

#ifdef _WIN32
#define DEF_ERROR(code, palmAlert, winAlert) {(code), (winAlert)},
#endif
#ifdef _PALM_OS
#define _DEF_ERROR(code, palmAlert, winAlert) {(code), (palmAlert)},
#endif

// TODO: add more error codes
static const ErrorMapping errors[] = {
	DEF_ERROR(SocketConnection::errResponseMalformed, malformedResponseAlert, IDS_ALERT_MALFORMED_RESPONSE)
	DEF_ERROR(memErrNotEnoughSpace, notEnoughMemoryAlert, IDS_ALERT_NOT_ENOUGH_MEMORY)
	DEF_ERROR(netErrTimeout, connectionTimedOutAlert, IDS_ALERT_CONNECTION_TIMEOUT)
};

void LookupManager::handleConnectionError(status_t error)
{
	assert(errNone != error);

    uint_t errorsCount = ARRAY_SIZE(errors);
    uint_t alertId = uint_t(-1);
    for (uint_t i = 0; i < errorsCount; ++i)
    {
        if (errors[i].error == error)
        {
            alertId = errors[i].alertId;
            break;
        }
    }
	if (uint_t(-1) != alertId) 
		Alert(alertId);
}

status_t LookupManager::fetchUrl(const char* url)
{
    InfoManConnection* conn = createConnection();
    if (NULL == conn)
        return memErrNotEnoughSpace;
    
    status_t err = conn->setUrl(url);
    if (errNone != err)
    {
        delete conn;
        return err; 
    }
    return enqueueConnection(conn); 
}

InfoManConnection* LookupManager::createConnection()
{
    InfoManConnection* conn = new_nt InfoManConnection(*this);
    if (NULL == conn)
        return NULL;
    
    Preferences* prefs = GetPreferences(); 
    conn->serverAddress = prefs->serverAddress;    
    conn->setTransferTimeout(ticksPerSecond() * 30L);
    return conn;
}

status_t LookupManager::enqueueConnection(InfoManConnection* conn)
{
    assert(NULL != conn);

    status_t error = conn->enqueue();
    if (errNone != error)
    { 
        delete conn;
        return error;
    }
#ifdef _PALM_OS
    MoriartyApplication::popupForm(connectionProgressForm);
#endif

#ifdef _WIN32
    ConnectionProgressDialog::create(ExtEventGetWindow());
#endif
    
    return errNone; 
}

const LookupFinishedEventData* LookupFinishedData(Event& event)
{
    assert(extEventLookupFinished == ExtEventGetID(event));
    const LookupFinishedEventData* data = static_cast<const LookupFinishedEventData*>(ExtEventGetObject(event));
    return data;    
}


bool LookupManager::handleLookupFinishedInForm(Event& event)
{  
    const LookupFinishedEventData* data = LookupFinishedData(event);
    assert(data != NULL);
    switch (data->result)
    {
        case lookupResultError:
            handleConnectionError(data->error);
            return true;
    
        case lookupResultServerError:
            handleServerError(data->serverError);  
            return true;
        
        case lookupResultConnectionCancelledByUser:
            return true;
            
        // TODO: handle the cases below
        //case lookupResultLocationUnknown:
        //    MoriartyApplication::alert(locationUnknownAlert);
        //    handled=true;
        //    break;

        //case lookupResultNoResults:
        //    MoriartyApplication::alert(noResultsAlert);
        //    handled=true;
        //    break;
    }
    return false;  
}

DefinitionModel* LookupManager::releaseDefinitionModel()
{
    Guard g(*this);
    DefinitionModel* m = definitionModel;
    definitionModel = NULL;
    return m;
}