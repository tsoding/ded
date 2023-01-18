#ifndef FILE_BROWSER_H_
#define FILE_BROWSER_H_

#include "common.h"
#include "free_glyph.h"

#include <SDL2/SDL.h>

typedef struct {
    Files files;
    size_t cursor;
} File_Browser;

Errno fb_open_dir(File_Browser *fb, const char *dir_path);
void fb_render(const File_Browser *fb, SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr);

#endif // FILE_BROWSER_H_
