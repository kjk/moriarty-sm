#ifndef INFOMAN_MAIN_WINDOW_HPP__
#define INFOMAN_MAIN_WINDOW_HPP__

#include <WindowsCE/Window.hpp>
#include <WindowsCE/CommandBar.hpp>
#include <WindowsCE/Controls.hpp>
#include <ExtendedEvent.hpp>

class MainWindow: public Window
{
    MainWindow();
    ExtEventHelper extEventHelper_;

#ifdef SHELL_MENUBAR
    CommandBar menuBar_;
#endif

    // TextRenderer renderer_;
    ListView listView_;
    long lastItemIndex_;
    void updateListViewFocus();

public:

    static MainWindow* create(const char_t* title, const char_t* windowClass);

    bool createModuleItems();

protected:

    long handleCreate(const CREATESTRUCT& cs);

    long handleDestroy();

    long handleCommand(ushort notify_code, ushort id, HWND sender);

    long handleResize(UINT sizeType, ushort width, ushort height);

    long handleNotify(int controlId, const NMHDR& header);

    long handleActivate(ushort action, bool minimized, HWND prev);

};

#endif