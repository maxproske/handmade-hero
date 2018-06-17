#include <windows.h>

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
            OutputDebugStringA("WM_SIZE\n");
            break;
        case WM_DESTROY: // Killed
            OutputDebugStringA("WM_DESTROY\n");
            break;
        case WM_CLOSE: // User closed
            OutputDebugStringA("WM_CLOSE\n");
            break;
        case WM_ACTIVATEAPP: // User clicked
            OutputDebugStringA("WM_ACTIVATEAPP\n");
            break;
        case WM_PAINT: // Paint using Windows graphics API
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint); // DC

            // Get rect
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            static DWORD Operation = WHITENESS; // Persist like a global variable, only initialize the first time through

            PatBlt(DeviceContext, X, Y, Width, Height, Operation); // Paint using rect passed back from BeginPaint
            
            if(Operation == WHITENESS)
            {
                Operation = BLACKNESS;
            }
            else
            {
                Operation = WHITENESS;
            }
            
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
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW; // Binary flags (bitfield) properties we want our window to have. CS_OWNDC for multiple window support.
    WindowClass.lpfnWndProc = WindowProc; // Pointer to a function that we define how our window responds to events
    WindowClass.hInstance = Instance; // Handle to ourselves
    WindowClass.lpszClassName = "HandmadeHeroWindowClass"; // Name for our window class

    // Register window class for subsequent use
    if (RegisterClass(&WindowClass))
    {
        // CreateWindowEx is a newer version of CreateWindow, with extras
        HWND WindowHandle = 
			CreateWindowEx(
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
            // Pull messages off our queue
            MSG Message;
            // Grab all messages coming in
            for(;;)
            {
                // BOOL is an int because it can receieve -1 if hInstance is invalid
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0); 
                /*  Signature:
                    LPMSG lpMsg,
                    HWND  hWnd,
                    UINT  wMsgFilterMin,
                    UINT  wMsgFilterMax
                */
                if (MessageResult > 0)
                {
                    TranslateMessage(&Message); // Turn messages into proper keyboard messages
                    DispatchMessage(&Message); // Dispatch message to Windows to have them
                }
                else
                {
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
