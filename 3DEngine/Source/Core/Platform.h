#pragma once


#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#ifndef FORCE_UINT16_INDEX
using IndexType = uint32_t;
#else 
using IndexType = uint16_t;
#endif