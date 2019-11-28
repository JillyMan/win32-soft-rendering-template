#include <memory>
#include "game_main.h"

static void GameUpdataAndRender(game_screen_buffer* Buffer) 
{
	u8* Pixels = (u8*)Buffer->Memory;

	u32 Size = Buffer->Width * Buffer->Height;

	for (u32 i = 0; i < Size; i++)
	{
		//BGRA
		*Pixels++ = 0;
		*Pixels++ = 0;
		*Pixels++ = 225;
		*Pixels++ = 1;
	}
}

game_code InitGame()
{
	game_code Game;
	Game.UpdateAndRender = &GameUpdataAndRender;
	return Game;
}