#ifndef INFOMAN_MAIN_WINDOW_HPP__
#define INFOMAN_MAIN_WINDOW_HPP__

#include <WindowsCE/Window.hpp>
#include <WindowsCE/CommandBar.hpp>
#include <WindowsCE/Controls.hpp>

class MainWindow: public Window
{
	MainWindow();

#ifdef SHELL_MENUBAR
	CommandBar menuBar_;
#endif
	
	ScrollBar scrollBar_;
	
public:

	static MainWindow* create(const char_t* title, const char_t* windowClass);
	
protected:
	
	long handleCreate(const CREATESTRUCT& cs);
	
	long handleDestroy();
	
	long handleCommand(ushort notify_code, ushort id, HWND sender);
	
	long handleResize(UINT sizeType, ushort width, ushort height);
	
};

#endif