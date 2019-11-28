#include "game_main.h"

#include "win32_main.h"

u32 GlobalPause;
u32 GlobalIsRunning;
win32_screen_buffer GlobalBackBuffer;

void Win32GetWindowSize(u32& Width, u32& Height, HWND Window)
{
	RECT ClientRectangle;
	GetClientRect(Window, &ClientRectangle);
	Width = ClientRectangle.right - ClientRectangle.left;
	Height = ClientRectangle.bottom - ClientRectangle.top;
}

void ResizeBackBuffer(win32_screen_buffer& Buffer, u32 Width, u32 Height)
{
	if (Buffer.Memory) 
	{
		free(Buffer.Memory);
	}

	Buffer.Width = Width;
	Buffer.Height = Height;
	u32 BytesPerPixel = 4;
	Buffer.BytesPerPixel = BytesPerPixel;

	Buffer.Info.bmiHeader.biSize = sizeof(Buffer.Info.bmiHeader);
	Buffer.Info.bmiHeader.biWidth = Buffer.Width;
	Buffer.Info.bmiHeader.biHeight = Buffer.Height;
	Buffer.Info.bmiHeader.biBitCount = 32;
	Buffer.Info.bmiHeader.biPlanes = 1;
	Buffer.Info.bmiHeader.biCompression = BI_RGB;

	Buffer.MemorySize = (Buffer.Width * Buffer.Height) * BytesPerPixel;
	Buffer.Memory = (u32*)malloc(Buffer.MemorySize);
}

void Win32DisplayBufferInWindow(win32_screen_buffer& Buffer, HDC DeviceContext, u32 ScreenWidth, u32 ScreenHeight)
{
	u32 OffsetX = 10;
	u32 OffsetY = 10;

	PatBlt(DeviceContext, 0, 0, ScreenWidth, OffsetY, BLACKNESS);
	PatBlt(DeviceContext, 0, OffsetY + Buffer.Height, ScreenWidth, ScreenHeight, BLACKNESS);
	PatBlt(DeviceContext, 0, 0, OffsetX, ScreenHeight, BLACKNESS);
	PatBlt(DeviceContext, OffsetX + Buffer.Width, 0, ScreenWidth, ScreenHeight, BLACKNESS);

	StretchDIBits(DeviceContext,
		OffsetX, OffsetY,
		Buffer.Width, Buffer.Height,
		0, 0,
		Buffer.Width, Buffer.Height,
		Buffer.Memory,
		&Buffer.Info,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

LRESULT CALLBACK Win32Callback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
	LRESULT Result = 0;
	switch (Message)
	{
	case WM_CLOSE:
		GlobalIsRunning = false;
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		u32 Width, Height;
		Win32GetWindowSize(Width, Height, Window);
		Win32DisplayBufferInWindow(GlobalBackBuffer, DeviceContext, Width, Height);

		EndPaint(Window, &Paint);
		break;
	}
	default:
		Result = DefWindowProcA(Window, Message, WParam, LParam);
		break;
	}
	return Result;
}

void Win32PendingMessage() 
{
	MSG Message;

	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		switch (Message.message)
		{
		case WM_QUIT:
			GlobalIsRunning = false;
			break;
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			u32 VKCode = (u32)Message.wParam;
			bool WasDown = (Message.lParam & (1 << 30)) != 0;
			bool IsDown = (Message.lParam & (1 << 31)) == 0;

			if (WasDown != IsDown) 
			{
				//todo: process keyboard button messages
			}

			if (IsDown) 
			{
				bool AltKeyWasDown = (Message.lParam & (1 << 29)) != 0;
				if ((VKCode == VK_F4) && AltKeyWasDown) 
				{
					GlobalIsRunning = false;
				}

				if (VKCode == VK_F1)
				{
					//todo: maketoogle foolscreen mode
				}
			}
		}
		default:
			TranslateMessage(&Message);
			DispatchMessage(&Message);
			break;
		}
	}
}

int main()
{
	ResizeBackBuffer(GlobalBackBuffer, 940, 560);

	WNDCLASS WindowClass = {};
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32Callback;
	WindowClass.hInstance = GetModuleHandle(NULL);
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	WindowClass.lpszClassName = "SoftRenderer";

	if (RegisterClassA(&WindowClass)) 
	{
		HWND Window = CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"Soft Renderer",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			1200,
			800,
			0,
			0,
			WindowClass.hInstance,
			0
		);

		if (Window) 
		{
			u32 MonitorRefreshHz = 60;
			HDC RefreshDC = GetDC(Window);
			u32 Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
			ReleaseDC(Window, RefreshDC);

			if (Win32RefreshRate > 1) 
			{
				MonitorRefreshHz = Win32RefreshRate;
			}

			real32 GameUpdateHz = real32(MonitorRefreshHz);
			real32 TargetSecondsPerFrame = (1.0f / GameUpdateHz);

			GlobalIsRunning = true;

			game_code Game = InitGame();

			while (GlobalIsRunning)
			{
				Win32PendingMessage();

				game_screen_buffer Buffer = { };
				Buffer.Width = GlobalBackBuffer.Width;
				Buffer.Height = GlobalBackBuffer.Height;
				Buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;
				Buffer.MemorySize = GlobalBackBuffer.MemorySize;
				Buffer.Memory = GlobalBackBuffer.Memory;

				if (Game.UpdateAndRender) 
				{
					Game.UpdateAndRender(&Buffer);
				}

				if (!GlobalPause)
				{
					HDC DeviceContext = GetDC(Window);
					u32 Width, Height;
					Win32GetWindowSize(Width, Height, Window);
					Win32DisplayBufferInWindow(GlobalBackBuffer, DeviceContext, Width, Height);
					ReleaseDC(Window, DeviceContext);
				}
			}
		}
	}
	return 0;
}