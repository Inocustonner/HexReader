#include <Windows.h>
#include "SysWindow.hpp"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmdline, INT cmdmode)
{
	SysWindow window;
	if (!window.Initialize("HexReader", hInstance, 800, 600))
	{
		return 0;
	}
	window.run();
	return 0;
}
int main()
{
	return WinMain(GetModuleHandle(0), NULL, NULL, 5);
}