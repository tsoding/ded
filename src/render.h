#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "free_glyph.h"
#include "simple_renderer.h"
#include "editor.h"

void editor_render(SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr, Editor *editor);
void render_search_text(Free_Glyph_Atlas *minibuffer_atlas, Simple_Renderer *sr, Editor *editor);


#endif // RENDER_H
