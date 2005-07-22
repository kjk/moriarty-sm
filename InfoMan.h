#ifndef INFOMAN_H__
#define INFOMAN_H__

#ifdef POCKETPC2003_UI_MODEL
#include "resourceppc.h"
#endif 
#ifdef SMARTPHONE2003_UI_MODEL
#include "resourcesp.h"
#endif

#include "resource.h"

HINSTANCE GetInstance();

#endif
