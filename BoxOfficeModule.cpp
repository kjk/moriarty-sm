#include "BoxOfficeModule.h"

#ifdef _WIN32
#include "BoxOfficeMainDialog.h"
#endif

MODULE_STARTER_DEFINE(BoxOffice)

enum BoxOfficeUDFIndex {
    lastWeekPosIndexInUDF,
    titleIndexInUDF,
    weekGrossIndexInUDF,
    cumulativeGrossIndexInUDF,
    releaseWeeksIndexInUDF,
    theatersNumberIndexInUDF,
    abbrevGrossIndexInUDF
}; 
