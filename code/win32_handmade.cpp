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

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackbuffer;

struct win32_window_dimension
{
    int Width;
    int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueOffset, int GreenOffset)
{
    // Cast void pointer to unsigned char (typedef uint8)
    uint8 *Row = (uint8 *)Buffer.Memory;
    for(int Y = 0; Y < Buffer.Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < Buffer.Width; ++X)
        {
            // Little endian (255,0,0,0 will draw blue)
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);

            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer.Pitch;
    }
}

// Allocate back buffer
internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    // Free first
    if (Buffer->Memory) 
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE); // 0 remembers how it was allocated
    }

    // Fill out bitmap info header
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // Start from top left. Performance penalty?
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32; // RGB + 8 bit padding so we fall on 32-bit boundaries
    Buffer->Info.bmiHeader.biCompression = BI_RGB; // No compression

    // Allocate memory ourselves using low-level, Windows API
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Buffer->Width*Buffer->BytesPerPixel;
}

// Blit to whole window
internal void Win32DisplayBufferInWindow(
    HDC DeviceContext,
    int WindowWidth, int WindowHeight, 
    win32_offscreen_buffer Buffer, 
    int X, int Y, int Width, int Height)
{
    // TODO(max): Correct aspect ratio on resize
    StretchDIBits(DeviceContext,
        0,0, WindowWidth, WindowHeight, // dst
        0,0, Buffer.Width, Buffer.Height, // src
        Buffer.Memory, &Buffer.Info,
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
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer, X, Y, Width, Height);

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
    
    // Resize our bitmap here instead of in WM_SIZE
    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW; // Redraw on horizontal or vertical scale
    WindowClass.lpfnWndProc = WindowProc; // Pointer to our Windows event handler
    WindowClass.hInstance = Instance; // Handle to ourselves
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    // Register window class for subsequent use
    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowExA(
            0, WindowClass.lpszClassName, "Handmade Hero",
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
                RenderWeirdGradient(GlobalBackbuffer, XOffset, YOffset);

                // Blit to screen
                HDC DeviceContext = GetDC(Window);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer, 0, 0, Dimension.Width, Dimension.Height);
                ReleaseDC(Window, DeviceContext);

                // Increment offset
                ++XOffset;
                YOffset += 2;
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
