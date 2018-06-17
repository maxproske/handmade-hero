#include <windows.h>

#define internal static // Don't allow to be used outside this source file
#define local_persist static 
#define global_variable static 

// TODO(max): This is a global for now
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

// Create buffer. Pass device-independent bitmaps to Windows to draw using GDI
internal void Win32ResizeDIBSection(int Width, int Height)
{
    // Free old DIBSection if initialized
    if (BitmapHandle) 
    {
        DeleteObject(BitmapHandle);
    }

    // Get device context compatible with screen context (even though we're not using it)
    if (!BitmapDeviceContext)
    {
        BitmapDeviceContext = CreateCompatibleDC(0);
    }

    // Fill out bmiHeader
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader) ; // Size of struct in bytes
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32; // 8 bytes/32 bits
    BitmapInfo.bmiHeader.biCompression = BI_RGB; // No compression

    // Allocate our new DIBSection
    BitmapHandle = CreateDIBSection(
            BitmapDeviceContext, &BitmapInfo,
            DIB_RGB_COLORS,
            &BitmapMemory, // Pointer to the bits
            0,0);
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
    // Rectangle to rectangle copy (can be different sizes using scaling if we need to)
    StretchDIBits(DeviceContext,
        X, Y, Width, Height,
        X, Y, Width, Height,
        BitmapMemory, // Get bits somewhere
        &BitmapInfo, // Get info bits somewhere
        DIB_RGB_COLORS, // Specify RGB or PALLETTE
        SRCCOPY); // What kind of bitwise ops we want to do. Directly copy bits.
}

// Handles Window notifications
LRESULT CALLBACK WindowProc(HWND Window,
                            UINT Message,
                            WPARAM WParam,
                            LPARAM LParam) 
{
    LRESULT Result = 0; // What we did with the message
    // Note(max): sizeof(Result) is 64 bits (long pointer) so WindowProc can return anything

    switch (Message)
    {
        case WM_SIZE: // Resizef
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect); // Get size of rect we can draw to
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);

            OutputDebugStringA("WM_SIZE\n");
        } break;   
        case WM_DESTROY: // Killed
        {
            Running = false;
        } break;
        case WM_CLOSE: // User closed
        {
            Running = false; 
        } break;
        case WM_ACTIVATEAPP: // User clicked
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n"); 
        } break;
        case WM_PAINT: // Paint using Windows graphics API
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint); // DC

            // Get rect
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            // Pass rect to update function
            Win32UpdateWindow(DeviceContext, X, Y, Width, Height);

            EndPaint(Window, &Paint);
        } break;
        default:
            //OutputDebugStringA("Default\n");
            Result = DefWindowProc(Window,Message,WParam,LParam); // Let Windows do the default behaviour
            break;
    }

    return Result;
}


// Entry point for Windows
int CALLBACK WinMain(HINSTANCE Instance, 
                    HINSTANCE PrevInstance,
                    LPSTR CommandLine, 
                    int ShowCode)
{
    WNDCLASS WindowClass = {}; // Clear window initialization to 0
    /*  Signature:
        UINT      style; [*]
        WNDPROC   lpfnWndProc; [*]
        int       cbClsExtra;
        int       cbWndExtra;
        HINSTANCE hInstance; [*]
        HICON     hIcon;
        HCURSOR   hCursor;
        HBRUSH    hbrBackground;
        LPCTSTR   lpszMenuName;
        LPCTSTR   lpszClassName; [*]
    */
   
    // TODO(max): Check if HREDRAW/VREDRAW/OWNDC still matter
    WindowClass.lpfnWndProc = WindowProc; // Pointer to a function that we define how our window responds to events
    WindowClass.hInstance = Instance; // Handle to ourselves
    WindowClass.lpszClassName = "HandmadeHeroWindowClass"; // Name for our window class

    // Register window class for subsequent use
    if (RegisterClass(&WindowClass))
    {
        // CreateWindowEx is a newer version of CreateWindow, with extras
        HWND WindowHandle = 
			CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Handemade Hero",
				WS_OVERLAPPEDWINDOW|WS_VISIBLE, // Overlap of several parameters
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
        /*  Signature:
            DWORD     dwExStyle,
            LPCTSTR   lpClassName,
            LPCTSTR   lpWindowName,
            DWORD     dwStyle,
            int       x,
            int       y,
            int       nWidth,
            int       nHeight,
            HWND      hWndParent,
            HMENU     hMenu,
            HINSTANCE hInstance,
            LPVOID    lpParam
        */

        if (WindowHandle)
        {
            Running = true;
            // Pull messages off our queue
            // Grab all messages coming in
            // while(1) will complain if debug level is -Wall (warning all)
            while(Running)
            {
                MSG Message;
                // BOOL is an int because it can receieve -1 if hInstance is invalid
                BOOL MessageResult = GetMessageA(&Message, 0, 0, 0); 
                /*  Signature:
                    LPMSG lpMsg,
                    HWND  hWnd,
                    UINT  wMsgFilterMin,
                    UINT  wMsgFilterMax
                */
                if (MessageResult > 0)
                {
                    TranslateMessage(&Message); // Turn messages into proper keyboard messages
                    DispatchMessageA(&Message); // Dispatch message to Windows to have them
                }
                else
                {
					// PostQuitMessage(0)
                    break;
                }
            }
        }
        else
        {
            // TODO(max): Logging
        }
    }
    else
    {
        // TODO(max): Logging
    }

    return (0);
}
