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
global_variable int BytesPerPixel = 4;

internal void RenderWeirdGradient(int XOffset, int YOffset)
{
    int Width = BitmapWidth;
    int Height = BitmapHeight;
    int Pitch = Width*BytesPerPixel;
    // Cast void pointer to unsigned char (we typedef uint8)
    uint8 *Row = (uint8 *)BitmapMemory;
    for(int Y = 0; Y < BitmapHeight; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < BitmapWidth; ++X)
        {
            /*
            Why does 255 0 0 0 draw blue?
            ---------------------------
            Pixel in memory you would think is 0xRRGGBBPP (where P is padding)
            Except it uses a little endian architecture
            Engineers wanted it to look correct in the registers, so they reversed it
            0xBBGGRRPP
            */

            uint8 Blue = (X + XOffset);
            uint8 Green = (Y + YOffset);

            /*
            Memory:     BB GG RR PP
            Register:   PP RR GG BB
            */
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Pitch;
    }
}

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
    int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;

    // VirtualAlloc vs. HeapAlloc vs. malloc vs. new
    // https://stackoverflow.com/questions/872072/whats-the-differences-between-virtualalloc-and-heapalloc
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO(max): Clear this to black.
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;

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
   
    // TODO(max): Check if HREDRAW/VREDRAW/OWNDC still matter
    WindowClass.lpfnWndProc = WindowProc; // Pointer to a function that we define how our window responds to events
    WindowClass.hInstance = Instance; // Handle to ourselves
    WindowClass.lpszClassName = "HandmadeHeroWindowClass"; // Name for our window class

    // Register window class for subsequent use
    if (RegisterClass(&WindowClass))
    {
        // CreateWindowEx is a newer version of CreateWindow, with extras
        HWND Window = 
			CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Handemade Hero",
				WS_OVERLAPPEDWINDOW|WS_VISIBLE, // Overlap of several parameters
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,0,
				Instance,
				0);

        if (Window)
        {
            int XOffset = 0;
            int YOffset = 0;

            // Pull messages off our queue
            // while(1) will complain if debug level is -Wall (warning all)
            Running = true;
            while(Running)
            {
                // GetMessageA is blocking, so use PeekMessage
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    // Quit anytime we get a quit message
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }

                    // Flush out our queue
                    TranslateMessage(&Message); // Turn messages into proper keyboard messages
                    DispatchMessageA(&Message); // Dispatch message to Windows to have them
                }

                RenderWeirdGradient(XOffset, YOffset);

                // Blit to screen
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);

                // Increment offset
                ++XOffset;
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
