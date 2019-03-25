#include "SysWindow.hpp"
SysWindow::SysWindow()
{

}
SysWindow::~SysWindow()
{
	if (m_editBox) delete m_editBox;
}

inline void InitMenu(HWND hWnd)
{
	HMENU hMenu = CreateMenu();
	AppendMenu(hMenu, MF_STRING, IDM_OPEN_FILE, "&Open File");
	AppendMenu(hMenu, MF_STRING, IDM_CLOSE_FILE, "&Close File");
	SetMenu(hWnd, hMenu);
}

//															 width = 0	height = 0
bool SysWindow::Initialize(LPCSTR name, HINSTANCE hInstance, INT width, INT height)
{
	/* Create a window class */
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof WNDCLASSEX);
	wc.cbSize = sizeof WNDCLASSEX;
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = wc.hIcon;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpszClassName = name;
	wc.lpszMenuName = nullptr;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
		return false;
	}
	
	if (!height) height = DEFAULT_HEIGHT;
	if (!width) width = DEFAULT_WIDTH;

	DWORD windowStyle = WS_OVERLAPPEDWINDOW;

	RECT windowSize = { 0, 0, width, height };
	AdjustWindowRect(&windowSize, windowStyle, TRUE);

	height = windowSize.bottom - windowSize.top;
	width = windowSize.right - windowSize.left;

	UINT screenWidth = GetSystemMetrics(SM_CXSCREEN);
	UINT screenHeight = GetSystemMetrics(SM_CYSCREEN);

	m_hWnd = CreateWindow(name, name, windowStyle,
		screenWidth / 2 - width / 2, screenHeight / 2 - height / 2,
		width, height,
		NULL,
		(HMENU)NULL,
		hInstance,
		this);
	if (!m_hWnd)
	{
		MessageBox(NULL, "Failed to create window", "Error", MB_OK);
		return false;
	}
	
	m_hInstance = hInstance;
	m_width = width;
	m_height = height;
	if (!InitializeElements()) return false;
	InitMenu(m_hWnd);
	UpdateWindow(m_hWnd);
	ShowWindow(m_hWnd, SW_SHOW);
	return true;
}

bool SysWindow::InitializeElements()
{
	/* Create a font */
	HFONT font = FONT;

	/* Create Text box for hex view of a open file */
	m_editBox = new EditBox;
	if (!m_editBox)
	{
		MessageBox(NULL, "Failed to allocate memory for edit box", "Error", MB_OK);
		return false;
	}
	SIZE fSize = GetFontSize(m_hWnd);

	if (!m_editBox->Initialize("editWind", "", m_hWnd, m_hInstance, 0, 0, m_width, m_height))
	{
		
		MessageBox(NULL, "Failed to create hex box", "Error", MB_OK);
		return false;
	}
	/* Set font to all boxes */
	return true;
}

void SysWindow::run()
{
	MSG msg = { 0 };
	BOOL Ret;
	while (Ret = GetMessage(&msg, NULL, NULL, NULL))
	{
		if (-1 == Ret) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

inline SysWindow* GetLPData(HWND hWnd)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	SysWindow* pBox = (SysWindow*)ptr;
	return pBox;
}

LRESULT CALLBACK SysWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		//on create make it able to catch the windows class
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		SysWindow* pState = reinterpret_cast<SysWindow*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState);
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0L;
	}
	case WM_SIZE:
	{
		SIZE fSize = GetFontSize(hWnd);
		RECT rc;
		GetWindowRect(hWnd, &rc);
		SysWindow* wndPtr = (SysWindow*)GetLPData(hWnd);
		SendMessage(wndPtr->m_editBox->GetHwnd(), WM_SETSIZE, ((WORD)(fSize.cy * 2) << 16) | (WORD)(fSize.cx * 7), (((WORD)(rc.bottom - rc.top)) << 16) | ((WORD)(rc.right - rc.left)));
		return 0L;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDM_OPEN_FILE:
		{
			SysWindow* wndPtr = (SysWindow*)GetLPData(hWnd);
			wndPtr->OpenFile();
			break;
		}
		case IDM_CLOSE_FILE:
			SysWindow* wndPtr = (SysWindow*)GetLPData(hWnd);
			wndPtr->CloseFile();
			break;
		}
		return 0L;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0L;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void SysWindow::OpenFile()
{
	CloseFile();

	TCHAR szFile[100];
	ZeroMemory(szFile, 100);
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof OPENFILENAME);
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = nullptr;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = "Choose file";
	ofn.lpstrInitialDir = "C:\\";
	ofn.Flags = OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		m_editBox->SetFile(ofn.lpstrFile);
	}
}

inline void SysWindow::CloseFile()
{
	m_editBox->CloseFile();
}
