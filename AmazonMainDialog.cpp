#include "AmazonMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "HyperlinkHandler.h"
#include "Modules.h"
#include "History.h"

#include <HistorySupport.hpp>
#include <ByteFormatParser.hpp>
#include <SysUtils.hpp>

using namespace DRA;

AmazonMainDialog::AmazonMainDialog()
:
ModuleDialog(IDR_AMAZON_MENU),
history_(NULL)
{
}

AmazonMainDialog::~AmazonMainDialog()
{
    delete history_;
}

// MODULE_DIALOG_CREATE_IMPLEMENT(AmazonMainDialog, IDD_EMPTY)
MODULE_DIALOG_CREATE_IMPLEMENT(AmazonMainDialog, IDD_AMAZON_MAIN)

bool AmazonMainDialog::handleInitDialog(HWND fw, long ip)
{
    history_ = new_nt HistorySupport(handle());
    if (NULL != history_)
    {
        history_->popupMenuFillHandlerData = (void*)(urlSchemaAmazonForm ":main");
        history_->setup(amazonHistoryCacheName, ID_VIEW_HISTORY, GetHyperlinkHandler(), ReadUrlFromCache);
    }
    Rect r;
    innerBounds(r);
    r.set(r.x(), r.y(), r.width(), r.height());
    renderer_.create(WS_TABSTOP, r, handle());
    renderer_.definition.setInteractionBehavior(Definition::behavHyperlinkNavigation | Definition::behavUpDownScroll | Definition::behavMouseSelection);
    
    const char* data = (const char*)LoadBinaryData(IDR_AMAZON_START);
    if (NULL != data)
    {
        ByteFormatParser* parser = new_nt ByteFormatParser();
        if (NULL != parser)
        {
            status_t err = parser->parseAll(data, -1);
            if (errNone == err)
            {
                DefinitionModel* model = parser->releaseModel();
                if (model != NULL)
                    renderer_.setModel(model, Definition::ownModel);
            }
            delete parser;
        }
    }
    
    return ModuleDialog::handleInitDialog(fw, ip);
}

long AmazonMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    renderer_.anchor(anchorLeft, 0, anchorBottom, 0, repaintWidget);
    return ModuleDialog::handleResize(st, w, h);
}

long AmazonMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    if (NULL != history_ && history_->handleCommandInForm(nc, id, sender))
        return messageHandled;
        
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool AmazonMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    //switch (data->result)
    //{
    //}
    return ModuleDialog::handleLookupFinished(event, data);
}
