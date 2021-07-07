#include "./sdl_extra.h"
#include "./stb_image.h"

void scc(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

void *scp(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }

    return ptr;
}

Vec2f window_size(SDL_Window *window)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return vec2f((float) w, (float) h);
}
