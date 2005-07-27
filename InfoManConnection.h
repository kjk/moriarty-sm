#ifndef INFOMAN_CONNECTION_H__
#define INFOMAN_CONNECTION_H__

#include <FieldPayloadProtocolConnection.hpp>
#include "LookupManager.h"

class InfoManConnection: public FieldPayloadProtocolConnection
{
	LookupManager& lookupManager_;
	LookupResult result_;
	ServerError serverError_;
	
	char* url_;
	
public:
	
	ulong_t transactionId;
	const char* url() const {return url_;}

protected:


private:
	
};

#endif