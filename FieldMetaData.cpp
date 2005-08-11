#include "FieldMetaData.h"
#include "InfoManConnection.h"
#include <algorithm>
#include <Text.hpp>

#define FIELD_VALUE_RESULT(name, handler, result) \
    {(name), fieldTypeValue, (handler), NULL, NULL, (result), false}
#define FIELD_VALUE(name, handler) FIELD_VALUE_RESULT(name, handler, lookupResultNone)

#define FIELD_NAME(name) field##name
#define FIELD_HANDLER(name) &InfoManConnection:: FIELD_HANDLER_NAME(name)
  
#define FVAL(name) \
    FIELD_VALUE(FIELD_NAME(name), FIELD_HANDLER(name)) 
#define FVRS(name, res) \
    FIELD_VALUE_RESULT(FIELD_NAME(name), FIELD_HANDLER(name), (res))

static const ResponseFieldDescriptor descriptors[] = {
    FVAL(Cookie),
    FVRS(Error, lookupResultError), 
    FVAL(LatestClientVersion),
	FVAL(TransactionId),
    FVAL(EBookVersion),
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

#ifndef NDEBUG
void test_ResponseFieldsSorted()
{
    for (ulong_t i = 1; i < ARRAY_SIZE(descriptors); ++i)
    {
        const ResponseFieldDescriptor& prev = descriptors[i - 1];
        const ResponseFieldDescriptor& curr = descriptors[i];
        assert(prev < curr);
    }  
}
#endif