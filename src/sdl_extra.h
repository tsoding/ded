#ifndef SDL_EXTRA_H_
#define SDL_EXTRA_H_

#include <SDL2/SDL.h>

#include "./la.h"

#define UNHEX(color) \
    ((color) >> (8 * 0)) & 0xFF, \
    ((color) >> (8 * 1)) & 0xFF, \
    ((color) >> (8 * 2)) & 0xFF, \
    ((color) >> (8 * 3)) & 0xFF

// SDL Check Code
void scc(int code);

// SDL Check Pointer
void *scp(void *ptr);

Vec2f window_size(SDL_Window *window);

#endif // SDL_EXTRA_H_
