#pragma once


#include "Singleton.h"
#include <cstdint>

#include "Platform.h"

class GlobalState 
{
public:

	DECLARE_SINGLETON(GlobalState)


	void InitSDL()
	{
		if (SDLRefs == 0)
		{
			SDL_Init(SDL_INIT_EVERYTHING);
			SDLRefs++;
		}
		else {
			SDLRefs++;
		}
	}

	void ShutdownSDL()
	{
		if (SDLRefs > 0)
		{
			SDLRefs--;

			if (SDLRefs == 0)
				SDL_Quit();
		}
	}

	uint32_t width, height;

private:
	uint32_t SDLRefs = 0;
};