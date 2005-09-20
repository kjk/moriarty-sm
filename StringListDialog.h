#ifndef STRING_LIST_DIALOG_H__
#define STRING_LIST_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

class StringListDialogModel {
public:
    virtual ulong_t size()  const = 0;
    virtual const char_t* operator[](ulong_t i) const = 0;
    virtual ~StringListDialogModel();
};


class StringListDialog: public MenuDialog {

protected:

    ListView list_;

public:

    enum ModelOwner {
        ownModelNot,
        ownModel
    };
    
protected:

    ModelOwner modelOwner_;
    StringListDialogModel* model_;
    UINT formTitleId_;

    StringListDialog(UINT formTitleId, StringListDialogModel* model, ModelOwner modelOwner);
    ~StringListDialog();
    
    long handleCommand(ushort nc, ushort id, HWND sender);
    long handleListItemActivate(int controlId, const NMLISTVIEW& h);
    long handleResize(UINT, ushort, ushort);
    bool handleInitDialog(HWND fw, long ip);
    
    void fillList();

public:

    static long showModal(UINT formTitleId, HWND parent, StringListDialogModel* model, ModelOwner modelOwner = ownModel);
    
};

class StringArrayModel: public StringListDialogModel {
    char_t** strings_;
    ulong_t strCount_;
    
    bool owner_;
    
    StringArrayModel(const StringArrayModel&);
    StringArrayModel& operator=(const StringArrayModel&);
    
public:

    enum ArrayOwner {
        ownArrayNot,
        ownArray
    };

    StringArrayModel(char_t** strings, ulong_t strCount, ArrayOwner owner = ownArray);
    ~StringArrayModel();
    
    ulong_t size() const;
    const char_t* operator[](ulong_t i) const;
};

class NarrowStringArrayModel: public StringListDialogModel {
    mutable char_t* text_;
    const char** strings_;
    ulong_t strCount_;

    NarrowStringArrayModel(const NarrowStringArrayModel&);
    NarrowStringArrayModel& operator=(const NarrowStringArrayModel&);
    
public:
    
    NarrowStringArrayModel(const char** strings, ulong_t strCount);
    ~NarrowStringArrayModel();

    ulong_t size() const;
    const char_t* operator[](ulong_t i) const;
};

#endif // STRING_LIST_DIALOG_H__