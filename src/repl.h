#ifndef REPL_H
#define REPL_H

#include "sv.h"
#include "common.h"
#include "simple_renderer.h"
#include "./free_glyph.h"
#include <SDL2/SDL.h>


typedef struct {
    String_Builder output_line;
} Repl;

Errno repl_execute(Repl *repl, const char *command);
void repl_render(const Repl *repl, SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr);

#endif // REPL_H
