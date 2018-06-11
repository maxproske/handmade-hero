#include <windows.h>

// Function declaration
void foo(void);

// Windows entry point
int CALLBACK WinMain(
	HINSTANCE hInstance, // Handle to ourself
	HINSTANCE hPrevInstance, // Legacy thing (older versions of your program running in memory)
	LPSTR     lpCmdLine, // What got sent to us when we got run
	int       nCmdShow) // Legacy thing (minimized or maximized)
{
	foo();
}

// Function definition
void foo(void)
{
	OutputDebugStringA("This is the first thing we have actually printed.\n");
}