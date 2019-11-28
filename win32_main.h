#pragma once

#include <Windows.h>
#include "types.h"

struct win32_screen_buffer
{
	BITMAPINFO Info;
	u32 Width;
	u32 Height;
	u32 BytesPerPixel;
	u32 MemorySize;
	u32* Memory;
};