#pragma once

#include "types.h"

struct game_screen_buffer
{
	u32 Width;
	u32 Height;
	u32 BytesPerPixel;
	u32 MemorySize;
	u32* Memory;
};

#define GAME_UPDATE_AND_RENDER(name) void name(game_screen_buffer *Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

struct game_code
{
	game_update_and_render* UpdateAndRender;
};