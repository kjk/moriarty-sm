#include "LookupManager.h"
#include <Definition.hpp>
#include <UniversalDataFormat.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

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

void DestroyLookupManager()
{
	delete lookupManager;
	lookupManager = NULL;
}

LookupManager::LookupManager():
	LookupManagerBase(extEventLookupStarted),
	definitionModel(NULL),
	udf(NULL),
	strings(NULL),
	stringsCount(0)
{}

LookupManager::~LookupManager()
{
	delete definitionModel;
	delete udf;
	StrArrFree(strings, stringsCount);
}

void LookupManager::setDefinitionModel(DefinitionModel* model)
{
	delete definitionModel;
	definitionModel = model;
}

void LookupManager::setUDF(UniversalDataFormat* udff)
{
	delete udf;
	udf = udff;
}

void LookupManager::setStrings(char_t** ss, ulong_t sc)
{
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

