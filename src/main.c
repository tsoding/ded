#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define SV_IMPLEMENTATION
#include "./sv.h"

#include "./editor.h"
#include "./la.h"
#include "./sdl_extra.h"
#include "./font.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define DELTA_TIME (1.0f / FPS)

Editor editor = {0};
Vec2f camera_pos = {0};
Vec2f camera_vel = {0};

Vec2f camera_project_point(SDL_Window *window, Vec2f point)
{
    return vec2f_add(
               vec2f_sub(point, camera_pos),
               vec2f_mul(window_size(window), vec2fs(0.5f)));
}


void render_cursor(SDL_Renderer *renderer, SDL_Window *window, const Font *font)
{
    const Vec2f pos =
        camera_project_point(
            window,
            vec2f((float) editor.cursor_col * FONT_CHAR_WIDTH * FONT_SCALE,
                  (float) editor.cursor_row * FONT_CHAR_HEIGHT * FONT_SCALE));

    const SDL_Rect rect = {
        .x = (int) floorf(pos.x),
        .y = (int) floorf(pos.y),
        .w = FONT_CHAR_WIDTH * FONT_SCALE,
        .h = FONT_CHAR_HEIGHT * FONT_SCALE,
    };

    scc(SDL_SetRenderDrawColor(renderer, UNHEX(0xFFFFFFFF)));
    scc(SDL_RenderFillRect(renderer, &rect));

    const char *c = editor_char_under_cursor(&editor);
    if (c) {
        set_texture_color(font->spritesheet, 0xFF000000);
        render_char(renderer, font, *c, pos, FONT_SCALE);
    }
}

void usage(FILE *stream)
{
    fprintf(stream, "Usage: te [FILE-PATH]\n");
}

// TODO: Save file
// TODO: ncurses renderer
// TODO: Jump forward/backward by a word
// TODO: Delete a word
// TODO: Blinking cursor
// TODO: Delete line
// TODO: Split the line on Enter

int main(int argc, char **argv)
{
    const char *file_path = NULL;

    if (argc > 1) {
        file_path = argv[1];
    }

    if (file_path) {
        FILE *file = fopen(file_path, "r");
        if (file != NULL) {
            editor_load_from_file(&editor, file);
            fclose(file);
        }
    }

    scc(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window =
        scp(SDL_CreateWindow("Text Editor",
                             0, 0,
                             SCREEN_WIDTH, SCREEN_HEIGHT,
                             SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer =
        scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));


    Font font = font_load_from_file(renderer, "./charmap-oldschool_white.png");

    bool quit = false;
    while (!quit) {
        const Uint32 start = SDL_GetTicks();
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
            }
            break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_BACKSPACE: {
                    editor_backspace(&editor);
                }
                break;

                case SDLK_F2: {
                    if (file_path) {
                        editor_save_to_file(&editor, file_path);
                    }
                }
                break;

                case SDLK_RETURN: {
                    editor_insert_new_line(&editor);
                }
                break;

                case SDLK_DELETE: {
                    editor_delete(&editor);
                }
                break;

                case SDLK_UP: {
                    if (editor.cursor_row > 0) {
                        editor.cursor_row -= 1;
                    }
                }
                break;

                case SDLK_DOWN: {
                    editor.cursor_row += 1;
                }
                break;

                case SDLK_LEFT: {
                    if (editor.cursor_col > 0) {
                        editor.cursor_col -= 1;
                    }
                }
                break;

                case SDLK_RIGHT: {
                    editor.cursor_col += 1;
                }
                break;
                }
            }
            break;

            case SDL_TEXTINPUT: {
                editor_insert_text_before_cursor(&editor, event.text.text);
            }
            break;
            }
        }

        {
            const Vec2f cursor_pos =
                vec2f((float) editor.cursor_col * FONT_CHAR_WIDTH * FONT_SCALE,
                      (float) editor.cursor_row * FONT_CHAR_HEIGHT * FONT_SCALE);

            camera_vel = vec2f_mul(
                             vec2f_sub(cursor_pos, camera_pos),
                             vec2fs(2.0f));

            camera_pos = vec2f_add(camera_pos, vec2f_mul(camera_vel, vec2fs(DELTA_TIME)));
        }


        scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
        scc(SDL_RenderClear(renderer));

        for (size_t row = 0; row < editor.size; ++row) {
            const Line *line = editor.lines + row;
            const Vec2f line_pos = camera_project_point(window, vec2f(0.0f, (float) row * FONT_CHAR_HEIGHT * FONT_SCALE));
            render_text_sized(renderer, &font, line->chars, line->size, line_pos, 0xFFFFFFFF, FONT_SCALE);
        }
        render_cursor(renderer, window, &font);

        SDL_RenderPresent(renderer);

        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 delta_time_ms = 1000 / FPS;
        if (duration < delta_time_ms) {
            SDL_Delay(delta_time_ms - duration);
        }
    }

    SDL_Quit();

    return 0;
}
