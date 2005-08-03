#include "FieldMetaData.h"
#include "InfoManConnection.h"
#include <algorithm>
#include <Text.hpp>

#define FIELD_VALUE(name, handler) \
    {(name), fieldTypeValue, (handler), NULL, NULL, lookupResultNone, false} 

static const ResponseFieldDescriptor descriptors[] = {
	FIELD_VALUE(fieldTransactionId, &InfoManConnection::handleTransactionIdField),
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
