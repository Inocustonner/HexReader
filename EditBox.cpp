#include "EditBox.hpp"
#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
/////////////////////////////////////////////////////////////////////////////////////////////////
#endif
EditBox::EditBox()
{
	//set styles
	m_style = WS_POPUP;
	m_exStyle = NULL;
}
EditBox::~EditBox()
{
	CloseFile();
	DestroyWindow(m_hWnd);
}

bool EditBox::InitAdditional(INT lines)
{
	if (m_hTrack)
		DestroyWindow(m_hTrack);
	constexpr int scrlWidth = 15 * 2;
	m_hTrack = CreateWindowEx(0L,
		"SCROLLBAR",
		NULL,
		WS_CHILD | WS_VISIBLE | SBS_VERT,
		m_width - scrlWidth - 15, 0,
		scrlWidth, m_height - 59,
		m_hWnd,
		NULL,
		m_hInstance,
		NULL);
	if (!m_hTrack)
	{
		MessageBox(NULL, "Failed to create track bar", "Error", MB_OK);
		return false;
	}
	return true;
}


bool EditBox::Initialize(LPCSTR className, LPCSTR name, HWND parent, HINSTANCE hInstance, UINT x, UINT y, UINT width, UINT height)
{
	/* Create window class */
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof WNDCLASSEX);
	wc.cbSize = sizeof WNDCLASSEX;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = wc.hIcon;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = EditProc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpszClassName = className;

	if (!RegisterClassEx(&wc))
	{
		return false;
	}
	RECT rc = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rc, m_style, FALSE);
	m_width = rc.right - rc.left;
	m_height = rc.bottom - rc.top;

	m_hWnd = CreateWindowEx(m_exStyle, wc.lpszClassName, name, m_style,
		x, y,
		m_width, m_height,
		NULL,
		NULL,
		hInstance,
		this);

	if (!m_hWnd)
		return false;
	SetParent(m_hWnd, parent);
	m_hInstance = hInstance;
	UpdateWindow(m_hWnd);
	ShowWindow(m_hWnd, SW_SHOW);
	m_fSize = GetFontSize(m_hWnd);
	return true;
}

//function to decode class from lParam
inline EditBox* GetLPData(HWND hWnd)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	EditBox* pBox = (EditBox*)ptr;
	return pBox;
}
inline std::string enhex(LONGLONG x)
{
	if (x == 0) return std::string("0");

	std::string num;
	for (INT i = 0; x >> i > 0; i += 4)
	{
		num.insert(num.cbegin(), hexademical[(x >> i) & 0xF]);
	}
	return num;
}
inline void CreateNums(std::string& nums, INT lines, LONGLONG offset, INT base, INT spaces)
{
	UINT i;
	/* create horisontal line */
	for (i = 0; i < base; ++i)
	{
		nums.push_back(hexademical[i]);
		nums.insert(nums.cend(), spaces + 1, ' ');
	}
	/* create vertical line */
	for (i = 0; i < lines; ++i)
	{
		nums.append(enhex(i + offset));
		nums.push_back('0');
		nums.push_back('\n');
	}
}
inline void update_nums(std::string& nums, BOOL direction, INT lines, LONGLONG offset, INT base, INT spaces)
{
	INT pos;
	if (1 == direction)
	{
		/* remove first number from vertical line */
		pos = nums.find('\n', base * 3);
		nums.erase(nums.cbegin() + base * (spaces + 2), nums.cbegin() + pos + 1);
		/* append at the end a number of new line */
		nums.append(enhex(lines + offset - 1));
		nums.push_back('0');
		nums.push_back('\n');
	}
	else /* if 0 == direction */
	{
		/* remove last number */
		nums.pop_back();
		pos = nums.find_last_of('\n');
		nums.erase(nums.cbegin() + pos + 1, nums.cend());
		/* append at the beginning a number of new line */
		std::string num = enhex(offset);
		nums.insert(base * 3, num);
		nums.insert(nums.cbegin() + base * 3 + num.size(), '0');
		nums.insert(nums.cbegin() + base * 3 + num.size() + 1, '\n');
	}
	return;
}

LRESULT CALLBACK EditBox::EditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		//on create make it able to catch the windows class
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		EditBox* pState = reinterpret_cast<EditBox*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState);
		return 0L;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EditBox* boxPtr = GetLPData(hWnd);
		SelectObject(hdc, FONT);
		SetBkMode(hdc, TRANSPARENT);
		static RECT textPos = { boxPtr->m_fSize.cx * 10, boxPtr->m_fSize.cy * 2, 0, 0 };
		static RECT textNormalPos = { boxPtr->m_fSize.cx * 60, boxPtr->m_fSize.cy * 2, 0, 0 };
		static RECT numsPosH = { boxPtr->m_fSize.cx * 10, 0, 0, 0 };
		static RECT numsPosV = { 0, boxPtr->m_fSize.cy * 2, 0, 0 };
		INT d = boxPtr->m_nums.size() > 0;
		DrawTextEx(hdc, (LPSTR)boxPtr->m_hexData.data(), -1, &textPos, DT_NOCLIP | DT_LEFT | DT_TOP, NULL);
		DrawTextEx(hdc, (LPSTR)boxPtr->m_data.data(), -1, &textNormalPos, DT_NOCLIP | DT_LEFT | DT_TOP, NULL);
		DrawTextEx(hdc, (LPSTR)boxPtr->m_nums.data(), 16 * 3 * d, &numsPosH, DT_NOCLIP | DT_LEFT | DT_TOP, NULL);
		DrawTextEx(hdc, (LPSTR)boxPtr->m_nums.data() + 16 * 3, -1 * d, &numsPosV, DT_NOCLIP | DT_LEFT | DT_TOP, NULL);
		EndPaint(hWnd, &ps);
		return 0L;
	}/* 
		TO DO: Move caret by keyboard; 
		TO DO: make selection function
	 */
	case WM_LBUTTONUP:
	{
		//show caret on click
		POINT p;
		EditBox* boxPtr = GetLPData(hWnd);
		GetCursorPos(&p);
		ScreenToClient(hWnd, &p);
		CreateCaret(hWnd, LoadBitmap(NULL, IDC_HAND), 2, FONT_SIZE);
		/* convert positions */
		p.x = p.x - p.x % (boxPtr->m_fSize.cx);
		p.y = boxPtr->m_fSize.cy * (p.y / boxPtr->m_fSize.cy);
		/* Implement  spaces */
		//SetCaretPos(p.x + boxPtr->m_fSize.cx*((p.x + 2) % (boxPtr->m_spaces + 2) == 0), p.y);
		SetCaretPos(p.x, p.y);
		ShowCaret(hWnd);
		return 0L;
	}
	case WM_MOUSEWHEEL:
	{
		EditBox* boxPtr = GetLPData(hWnd);
		LONGLONG oldPos = boxPtr->m_pos;
		if (120 != HIWORD(wParam))
		{
			/* rotating down */
			if (boxPtr->m_fileName)
			{
				/* update hex buffer */
				boxPtr->ReadFileBytes(boxPtr->m_lineLen, 1);
				/* update side lines if pos is updated */
				if ((boxPtr->m_pos ^ oldPos) | (boxPtr->m_pos == 0))
					update_nums(boxPtr->m_nums, 1, boxPtr->m_hexData.size() / 48, (boxPtr->m_pos + boxPtr->m_lineLen) / 16, boxPtr->m_base, boxPtr->m_spaces);
			}
		}
		else
		{
			/* rotating up */
			if (boxPtr->m_fileName)
			{
				/* update hex buffer */
				boxPtr->ReadFileBytes(boxPtr->m_lineLen, 0);
				/* update side lines if pos is updated */
				if ((boxPtr->m_pos ^ oldPos))
					update_nums(boxPtr->m_nums, 0, boxPtr->m_hexData.size() / 48, boxPtr->m_pos / 16, boxPtr->m_base, boxPtr->m_spaces);
			}
		}
		break;
	}
	case WM_KILLFOCUS:
	{
		DestroyCaret();
		return 0L;
	}
	case WM_SETSIZE:
	{
		EditBox* boxPtr = GetLPData(hWnd);
		MoveWindow(hWnd, 0, 0, boxPtr->m_width = LOWORD(lParam), boxPtr->m_height = HIWORD(lParam), FALSE);
		return 0L;
	}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

inline void popBackLine(std::string& str)
{
	size_t pos;
	if (str.back() == '\n') str.pop_back();
	if ((pos = str.find_last_of('\n')) != std::string::npos)
		str.erase(str.cbegin() + pos + 1, str.cend());
}
inline void popFrontLine(std::string& str)
{
	size_t pos;
	if ((pos = str.find_first_of('\n')) != std::string::npos)
		str.erase(str.cbegin(), str.cbegin() + pos + 1);
}
/* push front data */
inline void pushFrontLine(std::string& str, const char* ln, size_t count)
{
	std::string temp;
	unsigned int c;
	for (size_t i = 0; i < count; ++i)
	{
		c = *(ln + i);
		if (!((c >= 0x30 && c <= 0x39) | (c >= 0x61 && c <= 0x7A) | (c >= 0x41 && c <= 0x5A)))
		{
			if (!((c > 0x2D && c < 0x7F)
				| (c == 0x80)
				| (c > 0x81 && c < 0x8D)
				| (c == 0x8C)
				| (c > 0x90 && c < 0x9D)
				| (c > 0x9D && c < 0xA0)
				| (c > 0xA0 && c <0xFF)))
			{
				c = '.';
			}
		}
		temp.push_back(c);
		if (!(temp.size() % 16))
			temp.push_back('\n');
	}
	str.insert(0, temp);
}
/* append data */
inline void pushBackLine(std::string& str, const char* ln, const size_t count)
{
	unsigned int c;
	for (size_t i = 0; i < count; ++i)
	{
		c = *(ln + i);
		if (!((c >= 0x30 && c <= 0x39) | (c >= 0x61 && c <= 0x7A) |(c >= 0x41 && c <= 0x5A)))
		{
			if (!((c > 0x2D && c < 0x7F)
				| (c == 0x80)
				| (c > 0x81 && c < 0x8D)
				| (c == 0x8C)
				| (c > 0x90 && c < 0x9D)
				| (c > 0x9D && c < 0xA0)
				| (c > 0xA0 && c < 0xFF)))
			{
				c = '.';
			}
		}
		str.push_back(c);
		if (0 == (i + 1) % 16)
			str.push_back('\n');
	}
}
/* append hexed data */
inline void HexPushBack(std::string& hexData, const char* ln, size_t count, UINT spaces)
{
	for (UINT i = 0; i < count; ++i)
	{
		hexData.push_back(hexademical[(UCHAR)ln[i] >> 4]);
		hexData.push_back(hexademical[(UCHAR)ln[i] & 0x0f]);
		hexData.insert(hexData.cend(), spaces, ' ');
		if (!(hexData.size() % 48))
			*(hexData.end() - 1) = '\n';
	}
}
/* push front hexed data*/
inline void HexPushFront(std::string& hexData, const char* ln, size_t count, UINT spaces)
{
	UINT i;
	std::string line;
	for (i = 0; i < count; ++i)
	{
		line.push_back(hexademical[(UCHAR)ln[i] >> 4]);
		line.push_back(hexademical[(UCHAR)ln[i] & 0x0f]);
		line.insert(line.cend(), spaces, ' ');
		if (!(line.size() % 48))
			*(line.end() - 1) = '\n';
	}
	hexData.insert(0, line);
}

LONGLONG EditBox::GetCurrentFileSize()
{
	HANDLE file = CreateFile(
		m_fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	LARGE_INTEGER size;
	if (!GetFileSizeEx(file, &size))
	{
		MessageBox(NULL, "Failed to read file", "Error", MB_OK);
		CloseHandle(file);
		return 0;
	}
	CloseHandle(file);
	return size.QuadPart;
}

__int64 SeekFile(HANDLE file, LONGLONG distance, DWORD method)
{
	LARGE_INTEGER li;
	li.QuadPart = distance;

	li.LowPart = SetFilePointer(file, li.LowPart, &li.HighPart, method);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
		li.QuadPart = -1;
	return li.QuadPart;
}

/* if direction is 1 then we go down if 0 we go up*/
void EditBox::ReadFileBytes(LONG count, BOOL direction)
{	
	static INT cnt = 0;
	/* open the file */
	HANDLE file = CreateFile(
		m_fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE == file)
	{
		MessageBox(NULL, "Failed to open the file", "Error", MB_OK);
		CloseHandle(file);
		return;
	}
	/* get file size */
	LONGLONG size = GetCurrentFileSize();

	/* calculate position and post position. post position is position with dependency on hexData size*/
	/* post position */
	LONGLONG ppos;
	LONGLONG off = m_hexData.size() / 3;
	LONGLONG oldPos = m_pos;
	if (1 == direction) 
	{
		/* -----------------------------------------------------------------------*/
		m_pos += cnt * (m_pos + off < size);
		ppos = m_pos + (off - count) * (m_pos + off - count < size) * (count < off);
		/* -----------------------------------------------------------------------*/
	}
	else /* 0 == direction */
	{
		m_pos -= count - (count - m_pos)*(count > m_pos);
		ppos = m_pos;
	}
	
	/* calculate count bytes available  to read */
	if ((LONGLONG)ppos + count > size)
	{
		cnt = (INT)(size - ppos);
	}
	else
	{
		cnt = (INT)count;
	}
#ifdef DEBUG
	std::cout << "pos :" << m_pos << '\n' <<
		"ppos :" << ppos << '\n' <<
		"count :" << cnt << '\n' <<
		"size :" << size << '\n' <<
		"data size :" << off <<std::endl;
	std::cout << cnt + ppos << " " << size << std::endl;
#endif
	if ((m_pos ^ oldPos) | ((cnt + ppos < size) * (1 == direction)))
	{
		/* create temp buffer for bytes */
		LPSTR buffer = (LPSTR)malloc(cnt);
		if (buffer == nullptr)
		{
			MessageBox(NULL, "Errro to allocate memory for temp buffer. Try again.", "Error", MB_OK);
			CloseHandle(file);
			return;
		}
		/* set start reading pos in the file */
		if (-1 == SeekFile(file, ppos, FILE_BEGIN))
		{
			MessageBox(NULL, "Error seek file", "Error", MB_OK);
		}

		DWORD rdBytes = 0; //aka readed bytes

		if (0 == ReadFile(file, buffer, cnt, &rdBytes, NULL))
		{
			MessageBox(NULL, "Failed to read bytes", "Error", MB_OK);
			free(buffer);
			CloseHandle(file);
			return;
		}
		if (1 == direction)
		{
			popFrontLine(m_hexData);
			HexPushBack(m_hexData, buffer, cnt, 1);

			popFrontLine(m_data);
			pushBackLine(m_data, buffer, cnt);
		}
		else
		{
			popBackLine(m_hexData);
			HexPushFront(m_hexData, buffer, cnt, 1);

			popBackLine(m_data);
			pushFrontLine(m_data, buffer, cnt);
		}

		free(buffer);

		InvalidateRect(m_hWnd, NULL, TRUE);
	}
	CloseHandle(file);
}

void EditBox::OpenFile(LONG count)
{
	/* open the file */
	HANDLE file = CreateFile(
		m_fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE == file)
	{
		MessageBox(NULL, "Failed to open the file", "Error", MB_OK);
		CloseHandle(file);
		return;
	}
	/* get file size */
	LONGLONG size = GetCurrentFileSize();
	count = IF(count > size, size, count);
#ifdef DEBUG
	std::cout << "Count readed :" << count << std::endl;
#endif
	LPSTR buffer = (LPSTR)malloc(count);
	DWORD rdBytes = 0; //aka readed bytes

	if (0 == ReadFile(file, buffer, count, &rdBytes, NULL))
	{
		MessageBox(NULL, "Failed to read bytes", "Error", MB_OK);
		free(buffer);
		CloseHandle(file);
		return;
	}
	/* push hex */
	HexPushBack(m_hexData, buffer, count, m_spaces);
	/* push normal */
	pushBackLine(m_data, buffer, count);

	free(buffer);
	CloseHandle(file);
	InvalidateRect(m_hWnd, NULL, TRUE);
}

void EditBox::CloseFile()
{
	if (m_fileName)
		delete[] m_fileName;
	m_fileName = nullptr;

	if (m_hTrack)
		DestroyWindow(m_hTrack);
	m_hTrack = 0;

	m_pos = 0;

	m_data.erase();
	m_hexData.erase();
	m_nums.erase();
	InvalidateRect(m_hWnd, NULL, TRUE);
#ifdef DEBUG
	system("cls");
#endif
}

void EditBox::SetFile(LPSTR fileName)
{
	if (m_fileName)
		CloseFile();
	m_fileName = (LPSTR)malloc(strlen(fileName) + 1);

	std::move(fileName, fileName + strlen(fileName), m_fileName);
	*(m_fileName + strlen(fileName)) = '\000';

	/* How many symbols needs to read */
	INT count = (m_height / (m_fSize.cy + 2)) * m_base;//To calculate line size: symbol hex = 2 + spaces; base * (symbol hex); 
	
	OpenFile(count);
	CreateNums(m_nums, m_hexData.size() / 48, m_pos, m_base, m_spaces);
	return;
}

HWND EditBox::GetHwnd() const noexcept
{
	return m_hWnd;
}
