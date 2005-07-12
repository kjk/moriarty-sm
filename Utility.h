#pragma once

template<class T> 
inline void Zero(T& var) {
	ZeroMemory(&var, sizeof(var));
}

template <class T> 
inline T* GetWindowData(HWND wnd) {
	return reinterpret_cast<T*>(GetWindowLong(wnd, GWL_USERDATA));
}