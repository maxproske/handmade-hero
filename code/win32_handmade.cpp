#include <windows.h>
#include <stdint.h>

#define local_persist static // Can't be used outside this translation unit (source file)
#define global_variable static 
#define internal static

typedef int8_t int8; 
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void RenderWeirdGradient(int BlueOffset, int GreenOffset)
{
    int Pitch = BitmapWidth*BytesPerPixel;

    // Cast void pointer to unsigned char (typedef uint8)
    uint8 *Row = (uint8 *)BitmapMemory;
    for(int Y = 0; Y < BitmapHeight; ++Y)
    {
        uint8 Green = (Y + GreenOffset);
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < BitmapWidth; ++X)
        {
            // Little endian (255,0,0,0 will draw blue)
            uint8 Blue = (X + BlueOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Pitch;
    }
}

// Allocate back buffer
internal void Win32ResizeDIBSection(int Width, int Height)
{
    // Fill out bitmap info header
    BitmapWidth = Width;
    BitmapHeight = Height;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight; // Start from top left. Performance penalty?
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32; // RGB + 8 bit padding so we fall on 32-bit boundaries
    BitmapInfo.bmiHeader.biCompression = BI_RGB; // No compression

    // Free first
    if (BitmapMemory) VirtualFree(BitmapMemory, 0, MEM_RELEASE); // 0 remembers how it was allocated

    // Allocate memory ourselves using low-level, Windows API
    int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

// Blit to whole window
internal void Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;

    StretchDIBits(DeviceContext,
        0,0, BitmapWidth, BitmapHeight, // dst
        0,0, WindowWidth, WindowHeight, // src
        BitmapMemory, &BitmapInfo,
        DIB_RGB_COLORS, // RGB, not using a palette town
        SRCCOPY); // Direct copy bits, no bitwise ops necessary
}

// Handle Windows events
LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) 
{
    LRESULT Result = 0; // 64 bits so WindowProc can return any type

    switch (Message)
    {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break;   
        case WM_DESTROY:
        {
            Running = false;
        } break;
        case WM_CLOSE:
        {
            Running = false; 
        } break;
        case WM_ACTIVATEAPP:
        {
        } break;
        case WM_SETCURSOR:
        {
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            // Draw to rect
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;
        default:
            Result = DefWindowProc(Window,Message,WParam,LParam); // Let Windows do the default behaviour
            break;
    }
    return Result;
}

// Entry point for Windows
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
    WNDCLASS WindowClass = {}; // Clear window initialization to 0
    WindowClass.lpfnWndProc = WindowProc; // Pointer to our Windows event handler
    WindowClass.hInstance = Instance; // Handle to ourselves
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    // Register window class for subsequent use
    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowExA(
            0, WindowClass.lpszClassName, "Handemade Hero",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE, // Overlap of several parameters
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            0,0,
            Instance,
            0);
        if (Window)
        {
            int XOffset = 0;
            int YOffset = 0;

            // Pull messages off our queue
            Running = true;
            while(Running)
            {
                // GetMessage blocks graphics API, so use PeekMessage
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    // Quit anytime we get a quit message
                    if (Message.message == WM_QUIT) Running = false;

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
