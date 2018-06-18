#include <windows.h>
#include <stdint.h>

#define local_persist static
#define global_variable static 
#define internal static // Don't use outside this source file

// TODO(max): Move to a platform-independent header
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// Global variables for debug only
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;

// Allocate back buffer
internal void Win32ResizeDIBSection(int Width, int Height)
{
    // Free first
    if (BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE); // 0 remembers how it was allocated
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    // Fill out bitmap info header
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight; // Start at top left. TODO(max): performance penalty?
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32; // RGB + 8 bit offset, so we fall onto 32-bit boundaries
    BitmapInfo.bmiHeader.biCompression = BI_RGB; // No compression

    // Allocate memory ourselves using low-level, Windows API
    int BytesPerPixel = 4;
    int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;

    // VirtualAlloc vs. HeapAlloc vs. malloc vs. new
    // https://stackoverflow.com/questions/872072/whats-the-differences-between-virtualalloc-and-heapalloc
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    int Pitch = Width*BytesPerPixel;
    // Cast void pointer to unsigned char (we typedef uint8)
    uint8 *Row = (uint8 *)BitmapMemory;
    for(int Y = 0; Y < BitmapHeight; ++Y)
    {
        uint8 *Pixel = (uint8 *)Row;
        for(int X = 0; X < BitmapWidth; ++X)
        {
            /*
                Why does 255 0 0 draw blue?
                ---------------------------
                Pixel in memory you would think is 0xRRGGBBPP (where P is padding)
                Except it uses a little endian architecture
                Engineers wanted it to look correct in the registers, so they reversed it
                0xBBGGRRPP
            */

            *Pixel = (uint8)X;
            ++Pixel;

            *Pixel = (uint8)Y;
            ++Pixel;

            *Pixel = 0;
            ++Pixel;

            *Pixel = 0;
            ++Pixel;
        }
        Row += Pitch;
    }
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;

    // Blit to whole window first
    StretchDIBits(DeviceContext,
        /*
        X, Y, Width, Height,
        X, Y, Width, Height,
        */
        0,0, BitmapWidth, BitmapHeight,
        0,0, WindowWidth, WindowHeight,
        BitmapMemory, // Get bits somewhere
        &BitmapInfo, // Get info bits somewhere
        DIB_RGB_COLORS, // Specify RGB or PALLETTE
        SRCCOPY); // What kind of bitwise ops we want to do. Directly copy bits.

        
    // Dirty rect to rect copy (only update repaint region)
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

            RECT ClientRect;
            GetClientRect(Window, &ClientRect); // Get size of rect we can draw to

            // Pass rect to update function
            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);

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
