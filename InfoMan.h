#ifndef INFOMAN_H__
#define INFOMAN_H__

#ifdef _WIN32

#ifdef POCKETPC2003_UI_MODEL
#include "resourceppc.h"
#endif 
#ifdef SMARTPHONE2003_UI_MODEL
#include "resourcesp.h"
#endif

#include "resource.h"
#endif

#ifdef _PALM_OS
#include "moriarty_Rsc.h"
#endif

#endif
