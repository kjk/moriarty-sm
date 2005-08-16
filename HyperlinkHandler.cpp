#include "InfoManGlobals.h"
#include "HyperlinkHandler.h"

static HyperlinkHandler* hyperlinkHandler = NULL;

void HyperlinkHandlerDispose() 
{
    delete hyperlinkHandler;
    hyperlinkHandler = NULL; 
}

HyperlinkHandler* GetHyperlinkHandler()
{
    if (NULL == hyperlinkHandler)
        hyperlinkHandler = new_nt HyperlinkHandler();
    return hyperlinkHandler;  
}