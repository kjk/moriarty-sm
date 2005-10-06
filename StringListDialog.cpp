#include "StringListDialog.h"
#include "InfoMan.h"

#include <Text.hpp>
#include <UTF8_Processor.hpp>
#include <SysUtils.hpp>

StringListDialogModel::~StringListDialogModel()
{
}

StringListDialog::StringListDialog(UINT formTitleId, StringListDialogModel *model, StringListDialog::ModelOwner modelOwner):
    MenuDialog(IDR_DONE_CANCEL_MENU),
    model_(model),
    modelOwner_(modelOwner),
    formTitleId_(formTitleId)
{
    setAutoDelete(autoDeleteNot);
}

StringListDialog::~StringListDialog()
{
    if (ownModel == modelOwner_)
        delete model_;
}

bool StringListDialog::handleInitDialog(HWND fw, long ip)
{
    Rect r;
    innerBounds(r);
    list_.create(WS_VISIBLE | WS_TABSTOP | LVS_LIST | LVS_ALIGNLEFT | LVS_SINGLESEL, r, handle());
    list_.setStyleEx(LVS_EX_FULLROWSELECT | LVS_EX_GRADIENT | LVS_EX_NOHSCROLL | LVS_EX_ONECLICKACTIVATE);
    list_.setTextBkColor(CLR_NONE);
    
    setCaption(formTitleId_);
    
    MenuDialog::handleInitDialog(fw, ip);
    
    fillList();
    list_.focus();
    if (0 != list_.itemCount())
        list_.setItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

    return false;
}

long StringListDialog::handleListItemActivate(int controlId, const NMLISTVIEW &h)
{
    assert(ulong_t(h.iItem) < model_->size());
    endModal(h.iItem);
    return messageHandled;
}

long StringListDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id) {
        case IDOK:
        {
            long item = list_.selection();
            if (-1 == item)
            {
                list_.focus();
                return messageHandled;
            }
            endModal(item);
            return messageHandled;
        }
        
        case IDCANCEL:
            endModal(-1);
            return messageHandled;
    }
    return MenuDialog::handleCommand(nc, id, sender);
}

long StringListDialog::handleResize(UINT, ushort, ushort)
{
    list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    return messageHandled;
}

void StringListDialog::fillList()
{
    list_.clear();
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_TEXT | LVIF_PARAM;
    ulong_t size = model_->size();
    for (ulong_t i = 0; i < size; ++i)
    {
        const char_t* text = (*model_)[i];
        item.pszText = const_cast<char_t*>(text);
        item.iItem = item.lParam = i;
        list_.insertItem(item);
    }
}

long StringListDialog::showModal(UINT formTitleId, HWND parent, StringListDialogModel *model, StringListDialog::ModelOwner modelOwner)
{
    StringListDialog* dlg = new_nt StringListDialog(formTitleId, model, modelOwner);
    if (NULL == dlg)
    {
        delete model;
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return -1;
    }
    long res = dlg->MenuDialog::showModal(NULL, IDD_EMPTY, parent);
    delete dlg;
    return res;
}

StringArrayModel::StringArrayModel(char_t **strings, ulong_t strCount, StringArrayModel::ArrayOwner owner):
    strings_(strings),
    strCount_(strCount),
    owner_(ownArray == owner)
{
}

StringArrayModel::~StringArrayModel()
{
    if (owner_)
        StrArrFree(strings_, strCount_);
}

ulong_t StringArrayModel::size() const 
{
    return strCount_;
}

const char_t* StringArrayModel::operator [](ulong_t i) const
{
    assert(i < strCount_);
    return strings_[i];
}

NarrowStringArrayModel::NarrowStringArrayModel(const char **strings, ulong_t strCount):
    strings_(strings),
    strCount_(strCount),
    text_(NULL)
{
}

NarrowStringArrayModel::~NarrowStringArrayModel()
{
    free(text_);
}

ulong_t NarrowStringArrayModel::size() const 
{
    return strCount_;
}

const char_t* NarrowStringArrayModel::operator [](ulong_t i) const
{
    assert(i < strCount_);
    free(text_);
    text_ = UTF8_ToNative(strings_[i]);
    return text_;
}
