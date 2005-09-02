#ifndef BOX_OFFICE_MODULE_H__
#define BOX_OFFICE_MODULE_H__

#include "Modules.h"

MODULE_STARTER_DECLARE(BoxOffice);

enum BoxOfficeUDFIndex {
    boxOfficeLastWeekPosIndexInUDF,
    boxOfficeTitleIndexInUDF,
    boxOfficeWeekGrossIndexInUDF,
    boxOfficeCumulativeGrossIndexInUDF,
    boxOfficeReleaseWeeksIndexInUDF,
    boxOfficeTheatersNumberIndexInUDF,
    boxOfficeAbbrevGrossIndexInUDF
}; 


#endif // BOX_OFFICE_MODULE_H__