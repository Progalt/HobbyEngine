#pragma once


#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

// Default index type is 32 bit
// Could force 16 bit indices

#ifndef FORCE_UINT16_INDEX
using IndexType = uint32_t;
#else 
using IndexType = uint16_t;
#endif