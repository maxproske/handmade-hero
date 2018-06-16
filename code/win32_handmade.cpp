#include <tchar.h>
#include <windows.h>

// Entry point for Windows
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    GetModuleHandle(0);
    // Dialogue box that has OK with info symbol
    // Enclose strings in _T() and use the TCHAR equivalents
    MessageBox(0, _T("Hello, world!"), _T("Handmade Hero"), MB_OK|MB_ICONINFORMATION);
    return (0);
}

