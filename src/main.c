#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_image_write.h"

#define SV_IMPLEMENTATION
#include "./sv.h"

#include "./editor.h"
#include "./la.h"
#include "./sdl_extra.h"
#include "./gl_extra.h"
#include "./free_glyph.h"
#include "./cursor_renderer.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define DELTA_TIME (1.0f / FPS)


Editor editor = {0};
Vec2f camera_pos = {0};
Vec2f camera_vel = {0};

void usage(FILE *stream)
{
    fprintf(stream, "Usage: te [FILE-PATH]\n");
}

// TODO: Save file dialog
// Needed when ded is ran without any file so it does not know where to save.
// TODO: File Manager
// Any modern Text Editor should also be a File Manager

// TODO: Jump forward/backward by a word
// TODO: Delete a word
// TODO: Blinking cursor
// TODO: Delete line
// TODO: Split the line on Enter

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam)
{
    (void) source;
    (void) id;
    (void) length;
    (void) userParam;
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

static Free_Glyph_Buffer fgb = {0};
static Cursor_Renderer cr = {0};

#define FREE_GLYPH_FONT_SIZE 64

void render_editor_into_fgb(SDL_Window *window, Free_Glyph_Buffer *fgb, Cursor_Renderer *cr, Editor *editor)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    free_glyph_buffer_use(fgb);
    {
        glUniform2f(fgb->resolution_uniform, (float) w, (float) h);
        glUniform1f(fgb->time_uniform, (float) SDL_GetTicks() / 1000.0f);
        glUniform2f(fgb->camera_uniform, camera_pos.x, camera_pos.y);

        free_glyph_buffer_clear(fgb);

        {
            for (size_t row = 0; row < editor->size; ++row) {
                const Line *line = editor->lines + row;
                free_glyph_buffer_render_line_sized(
                    fgb, line->chars, line->size,
                    vec2f(0, -(float)row * FREE_GLYPH_FONT_SIZE),
                    vec4fs(1.0f), vec4fs(0.0f));
            }
        }

        free_glyph_buffer_sync(fgb);
        free_glyph_buffer_draw(fgb);
    }

    Vec2f cursor_pos = vec2fs(0.0f);
    {
        cursor_pos.y = -(float) editor->cursor_row * FREE_GLYPH_FONT_SIZE;

        if (editor->cursor_row < editor->size) {
            Line *line = &editor->lines[editor->cursor_row];
            cursor_pos.x = free_glyph_buffer_cursor_pos(fgb, line->chars, line->size, vec2f(0.0, cursor_pos.y), editor->cursor_col);
        }
    }

    cursor_renderer_use(cr);
    {
        glUniform2f(cr->resolution_uniform, (float) w, (float) h);
        glUniform1f(cr->time_uniform, (float) SDL_GetTicks() / 1000.0f);
        glUniform2f(cr->camera_uniform, camera_pos.x, camera_pos.y);
        glUniform1f(cr->height_uniform, FREE_GLYPH_FONT_SIZE);

        cursor_renderer_move_to(cr, cursor_pos);
        cursor_renderer_draw();
    }

    {
        camera_vel = vec2f_mul(
                         vec2f_sub(cursor_pos, camera_pos),
                         vec2fs(2.0f));

        camera_pos = vec2f_add(camera_pos, vec2f_mul(camera_vel, vec2fs(DELTA_TIME)));
    }
}

int main(int argc, char **argv)
{
    FT_Library library = {0};

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "ERROR: could initialize FreeType2 library\n");
        exit(1);
    }

    // const char *const font_file_path = "./VictorMono-Regular.ttf";
    const char *const font_file_path = "./ComicNeue-Regular.otf";

    FT_Face face;
    error = FT_New_Face(library, font_file_path, 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "ERROR: `%s` has an unknown format\n", font_file_path);
        exit(1);
    } else if (error) {
        fprintf(stderr, "ERROR: could not load file `%s`\n", font_file_path);
        exit(1);
    }

    FT_UInt pixel_size = FREE_GLYPH_FONT_SIZE;
    // TODO: FT_Set_Pixel_Sizes does not produce good looking results
    // We need to use something like FT_Set_Char_Size and properly set the device resolution
    error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
    if (error) {
        fprintf(stderr, "ERROR: could not set pixel size to %u\n", pixel_size);
        exit(1);
    }

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
                             SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));

    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        int major;
        int minor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
        printf("GL version %d.%d\n", major, minor);
    }

    scp(SDL_GL_CreateContext(window));

    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "Could not initialize GLEW!");
        exit(1);
    }

    if (!GLEW_ARB_draw_instanced) {
        fprintf(stderr, "ARB_draw_instanced is not supported; game may not work properly!!\n");
        exit(1);
    }

    if (!GLEW_ARB_instanced_arrays) {
        fprintf(stderr, "ARB_instanced_arrays is not supported; game may not work properly!!\n");
        exit(1);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (GLEW_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
    } else {
        fprintf(stderr, "WARNING! GLEW_ARB_debug_output is not available");
    }


    free_glyph_buffer_init(&fgb,
                           face,
                           "./shaders/free_glyph.vert",
                           "./shaders/free_glyph.frag");
    cursor_renderer_init(&cr,
                         "./shaders/cursor.vert",
                         "./shaders/cursor.frag");

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
#ifndef TILE_GLYPH_RENDER
                    cursor_renderer_use(&cr);
                    glUniform1f(cr.last_stroke_uniform, (float) SDL_GetTicks() / 1000.0f);
#endif
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
#ifndef TILE_GLYPH_RENDER
                    cursor_renderer_use(&cr);
                    glUniform1f(cr.last_stroke_uniform, (float) SDL_GetTicks() / 1000.0f);
#endif
                }
                break;

                case SDLK_DELETE: {
                    editor_delete(&editor);
#ifndef TILE_GLYPH_RENDER
                    cursor_renderer_use(&cr);
                    glUniform1f(cr.last_stroke_uniform, (float) SDL_GetTicks() / 1000.0f);
#endif
                }
                break;

                case SDLK_UP: {
                    if (editor.cursor_row > 0) {
                        editor.cursor_row -= 1;
                    }
#ifndef TILE_GLYPH_RENDER
                    cursor_renderer_use(&cr);
                    glUniform1f(cr.last_stroke_uniform, (float) SDL_GetTicks() / 1000.0f);
#endif
                }
                break;

                case SDLK_DOWN: {
                    editor.cursor_row += 1;
#ifndef TILE_GLYPH_RENDER
                    cursor_renderer_use(&cr);
                    glUniform1f(cr.last_stroke_uniform, (float) SDL_GetTicks() / 1000.0f);
#endif
                }
                break;

                case SDLK_LEFT: {
                    if (editor.cursor_col > 0) {
                        editor.cursor_col -= 1;
#ifndef TILE_GLYPH_RENDER
                        cursor_renderer_use(&cr);
                        glUniform1f(cr.last_stroke_uniform, (float) SDL_GetTicks() / 1000.0f);
#endif
                    }
                }
                break;

                case SDLK_RIGHT: {
                    editor.cursor_col += 1;
#ifndef TILE_GLYPH_RENDER
                    cursor_renderer_use(&cr);
                    glUniform1f(cr.last_stroke_uniform, (float) SDL_GetTicks() / 1000.0f);
#endif

                }
                break;
                }
            }
            break;

            case SDL_TEXTINPUT: {
                editor_insert_text_before_cursor(&editor, event.text.text);
#ifndef TILE_GLYPH_RENDER
                cursor_renderer_use(&cr);
                glUniform1f(cr.last_stroke_uniform, (float) SDL_GetTicks() / 1000.0f);
#endif
            }
            break;
            }
        }

        {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            // TODO(#19): update the viewport and the resolution only on actual window change
            glViewport(0, 0, w, h);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        render_editor_into_fgb(window, &fgb, &cr, &editor);

        SDL_GL_SwapWindow(window);

        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 delta_time_ms = 1000 / FPS;
        if (duration < delta_time_ms) {
            SDL_Delay(delta_time_ms - duration);
        }
    }

    return 0;
}
