#ifndef _SYS_WINDOW_HPP_
#pragma once
#define _SYS_WINDOW_HPP
#define DEFAULT_WIDTH 600
#define DEFAULT_HEIGHT 600
#define IDM_OPEN_FILE 0x0739
#define IDM_CLOSE_FILE 0xF8C6
#include <Windows.h>
#include "EditBox.hpp"
class SysWindow
{
public:
	SysWindow();
	~SysWindow();
	bool Initialize(LPCSTR name, HINSTANCE hInstance, INT width = 0, INT height = 0);
	bool InitializeElements();
	void run();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void OpenFile();
	inline void CloseFile();
private:
	HWND m_hWnd;
	HWND m_charBox;
	HINSTANCE m_hInstance;
	UINT m_width, m_height;
	EditBox* m_editBox = nullptr;
};
#endif // ! _SYS_WINDOW_HPP_