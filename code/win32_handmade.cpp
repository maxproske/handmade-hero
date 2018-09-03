#include <windows.h>
#include <stdint.h>
#include <xinput.h>

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
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;

struct win32_window_dimension
{
	int Width;
	int Height;
};

// Hoist out Windows XInput, so we don't rely on a dll being on someone's machine.
// There is an external function called this in case you didn't know
// So bind to this during the linker stage, and do not throw a LNK error
// -----
// Use macros to define function pointers
// Macros are a very efficient way of defining a function prototype once
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
// Defining a type of that, so you can use it as a pointer
typedef X_INPUT_GET_STATE(x_input_get_state);
// And creating a stub. (If the function doesn't exist, just return 0. No harm done)
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return(0);
}
// So our function pointer will never be 0x000000
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return(0);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// Take loading Windows DLL into our own hands
internal void Win32LoadXInput(void) {
	HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll"); // Looks in a number of places (exe folder first, then Windows)
	if (XInputLibrary)
	{
		// Get address of procedures from .dll
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
	}
}

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return(Result);
}

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
	// Cast void pointer to unsigned char (typedef uint8)
	uint8 *Row = (uint8 *)Buffer->Memory;
	for (int Y = 0; Y < Buffer->Height; ++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int X = 0; X < Buffer->Width; ++X)
		{
			// Little endian (255,0,0,0 will draw blue)
			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
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
	int BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // Start from top left. Performance penalty?
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32; // RGB + 8 bit padding so we fall on 32-bit boundaries
	Buffer->Info.bmiHeader.biCompression = BI_RGB; // No compression

	// Allocate memory ourselves using low-level, Windows API
	int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Buffer->Width*BytesPerPixel;
}

// Blit to whole window
internal void Win32DisplayBufferInWindow(
	win32_offscreen_buffer *Buffer,
	HDC DeviceContext,
	int WindowWidth, int WindowHeight,
	int X, int Y, int Width, int Height)
{
	// TODO(max): Correct aspect ratio on resize
	StretchDIBits(DeviceContext,
		0, 0, WindowWidth, WindowHeight, // dst
		0, 0, Buffer->Width, Buffer->Height, // src
		Buffer->Memory, &Buffer->Info,
		DIB_RGB_COLORS, // RGB, not using a palette town
		SRCCOPY); // Direct copy bits, no bitwise ops necessary
}

// Handle Windows events
LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
	LRESULT Result = 0; // 64 bits so WindowProc can return any type

	switch (Message)
	{
	case WM_SIZE:
	{
	} break;
	case WM_DESTROY:
	{
		GlobalRunning = false;
	} break;
	case WM_CLOSE:
	{
		GlobalRunning = false;
	} break;
	case WM_ACTIVATEAPP:
	{
	} break;
	case WM_SETCURSOR:
	{
	} break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 VKCode = WParam; // Virtual-key code for non-ANSI keys 
		// LParam gives you even additional info (LParam & (1 << 30); for up before the message was sent or after)
		#define KeyMessageWasDownBit (1 << 30)
		#define KeyMessageIsDownBit (1 << 31)
		bool WasDown = ((LParam & KeyMessageWasDownBit) != 0);
		bool IsDown = ((LParam & KeyMessageIsDownBit) == 0);

		if (WasDown != IsDown)
		{
			if (VKCode == 'W')
			{
				OutputDebugStringA("W\n");
			}
			else if (VKCode == 'A')
			{
				OutputDebugStringA("A\n");
			}
			else if (VKCode == 'S')
			{
				OutputDebugStringA("S\n");
			}
			else if (VKCode == 'D')
			{
				OutputDebugStringA("D\n");
			}
			else if (VKCode == 'Q')
			{
				OutputDebugStringA("Q\n");
			}
			else if (VKCode == 'E')
			{
				OutputDebugStringA("E\n");
			}
			else if (VKCode == VK_UP)
			{
				OutputDebugStringA("VK_UP\n");
			}
			else if (VKCode == VK_LEFT)
			{
				OutputDebugStringA("VK_LEFT\n");
			}
			else if (VKCode == VK_RIGHT)
			{
				OutputDebugStringA("VK_RIGHT\n");
			}
			else if (VKCode == VK_DOWN)
			{
				OutputDebugStringA("VK_DOWN\n");
			}
			else if (VKCode == VK_ESCAPE)
			{
				if (IsDown) {
					OutputDebugStringA("VK_ESCAPE: is down.\n");
				}
				if (WasDown) {
					OutputDebugStringA("VK_ESCAPE: was down.\n");
				}
			}
			else if (VKCode == VK_SPACE)
			{
				OutputDebugStringA("VK_SPACE\n");
			}
		}
	} break;
	case WM_PAINT:
	{
		// Manually repaint obscured/dirty areas (offscreen/during resizing)
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint); // For WM_PAINT
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;
		int Width = Paint.rcPaint.right - Paint.rcPaint.left;
		int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

		// Draw to rect
		win32_window_dimension Dimension = Win32GetWindowDimension(Window);
		Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height, X, Y, Width, Height);

		EndPaint(Window, &Paint); // For WM_PAINT
	} break;
	default:
		Result = DefWindowProc(Window, Message, WParam, LParam); // Let Windows do the default behaviour
		break;
	}
	return Result;
}

// Entry point for Windows
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
	// Load XInput from DLL manually
	Win32LoadXInput();

	WNDCLASSA WindowClass = {}; // Clear window initialization to 0

	// Resize our bitmap here instead of in WM_SIZE
	Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw on horizontal or vertical scale. Get one device context and use it forever
	WindowClass.lpfnWndProc = Win32MainWindowCallback; // Pointer to our Windows event handler
	WindowClass.hInstance = Instance; // Handle to ourselves
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	// Register window class for subsequent use
	if (RegisterClassA(&WindowClass))
	{
		HWND Window = CreateWindowExA(
			0, WindowClass.lpszClassName, "Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Overlap of several parameters
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			0, 0,
			Instance,
			0);
		if (Window)
		{
			HDC DeviceContext = GetDC(Window);

			int XOffset = 0;
			int YOffset = 0;

			// Pull messages off our queue
			GlobalRunning = true;
			while (GlobalRunning)
			{
				// GetMessage blocks graphics API, so use PeekMessage
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					// Quit anytime we get a quit message
					if (Message.message == WM_QUIT) GlobalRunning = false;

					// Flush out our queue
					TranslateMessage(&Message); // Turn messages into proper keyboard messages
					DispatchMessageA(&Message); // Dispatch message to Windows to have them
				}

				// TODO(max): Should we poll this more frequently?
				for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
				{
					XINPUT_STATE ControllerState;
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
					{
						// This controller is plugged in
						XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
						bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
						bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
						bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
						bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
						bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);
						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;

						if (AButton)
						{
							YOffset += 2;
						}
					}
					else
					{
						// The controller is not available
					}
				}

				XINPUT_VIBRATION Vibration;
				Vibration.wLeftMotorSpeed = 60000;
				XInputSetState(0, &Vibration);

				RenderWeirdGradient(&GlobalBackbuffer, XOffset, YOffset);

				// Blit to screen
				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height,0, 0, Dimension.Width, Dimension.Height);
				//ReleaseDC(Window, DeviceContext);

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
