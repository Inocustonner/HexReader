#ifndef _EDIT_BOX_HPP_
#pragma once
#define _EDIT_BOX_HPP_
#define FONT_SIZE_DEFAULT 16
#define WM_SETSIZE 0x5123
#define IF(equation, ifTrue, ifFalse) ((INT)(ifTrue)*((BOOL)(equation)) + (INT)(ifFalse)*(!(equation)))
#include <Windows.h>
#include <string>
/* Globals */
static INT FONT_SIZE = FONT_SIZE_DEFAULT;
static HFONT FONT = CreateFont(FONT_SIZE, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
	OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, "Courier New");
static TCHAR hexademical[] = { '0', '1', '2','3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
static SIZE GetFontSize(HWND hWnd) 
{
	SIZE sz;
	HDC hdc = GetDC(hWnd);
	SelectObject(hdc, FONT);
	GetTextExtentPoint32(hdc, "0", 1, &sz);
#if 0
	POINT p = { sz.cx, sz.cy };
	LPtoDP(hdc, &p, MM_TEXT);
	ReleaseDC(hWnd, hdc);
#endif
	return sz;
}

class EditBox
{

public:
	EditBox();
	~EditBox();
	bool InitAdditional(INT lines);
	bool Initialize(LPCSTR className, LPCSTR name, HWND parent, HINSTANCE hInstance, UINT x, UINT y, UINT width, UINT height);
	static LRESULT CALLBACK EditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LONGLONG GetCurrentFileSize();
	void ReadFileBytes(LONG count, BOOL direction);
	void OpenFile(LONG count);
	void CloseFile();
	void SetFile(LPSTR fileName);
	HWND GetHwnd() const noexcept;
private:
	HWND m_hWnd;
	HWND m_hTrack;
	HINSTANCE m_hInstance;
	UINT m_height;
	UINT m_width;

	DWORD m_style;
	DWORD m_exStyle;

	SHORT m_spaces = 1;
	SHORT m_base = 16;
	SHORT m_lineLen = m_base;
	LONGLONG m_pos = 0;

	LPSTR m_fileName;
	std::string m_data;
	std::string m_nums;
	std::string m_hexData;
	SIZE m_fSize;
};
#endif