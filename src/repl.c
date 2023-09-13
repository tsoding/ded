#include "repl.h"
#include "sv.h"
#include "simple_renderer.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 256

Errno repl_execute(Repl *repl, const char *command) {
    char buffer[BUFFER_SIZE];
    FILE *fp = popen(command, "r");

    if (!fp) {
        return 1; // Adjust error codes as needed
    }

    repl->output_line.count = 0; // Reset the string builder

    while (fgets(buffer, sizeof(buffer), fp)) {
        sb_append_cstr(&repl->output_line, buffer);
    }

    pclose(fp);
    return 0;
}

void repl_render(const Repl *repl, SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr) {
    // Implement rendering based on your application's needs.
    // For now, we'll stick to a basic implementation similar to the one provided in file_browser.c

    Vec2f cursor_pos = vec2f(0, 0);
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    sr->resolution = vec2f(w, h);
    sr->time = (float) SDL_GetTicks() / 1000.0f;

    simple_renderer_set_shader(sr, SHADER_FOR_COLOR);

    simple_renderer_set_shader(sr, SHADER_FOR_EPICNESS);
    free_glyph_atlas_render_line_sized(
        atlas, sr, repl->output_line.items, repl->output_line.count,
        &cursor_pos,
        vec4fs(0));

    simple_renderer_flush(sr);
}
