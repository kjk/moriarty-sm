#ifndef HOROSCOPES_MODULE_H__
#define HOROSCOPES_MODULE_H__

#include "Modules.h"
#include <Serializer.hpp>

MODULE_STARTER_DECLARE(Horoscopes);

struct HoroscopesPrefs: public Serializable {
    
   HoroscopesPrefs();
   ~HoroscopesPrefs();
   
   char* lastQuery;
   ulong_t lastSign;
   enum {signNotSet = ulong_t(-1)};
   
   void serialize(Serializer& ser);
   
}; 

struct UniversalDataFormat;
class DefinitionModel;
DefinitionModel* HoroscopeExtractFromUDF(const UniversalDataFormat& udf, char_t*& date);

enum {horoscopesSignCount = 12};
status_t HoroscopeFetch(uint_t index);

#endif // HOROSCOPES_MODULE_H__