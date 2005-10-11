#ifndef MOVIES_MODULE_H__
#define MOVIES_MODULE_H__

#include "Modules.h"
#include "MoviesData.hpp"

MODULE_STARTER_DECLARE(Movies);

class DefinitionModel;
struct UniversalDataFormat;

DefinitionModel* MoviesPrepareMovie(const Movies_t& m, ulong_t index);
DefinitionModel* MoviesPrepareTheatre(const UniversalDataFormat& udf, ulong_t index);
status_t MoviesFetchData();

#endif // MOVIES_MODULE_H__