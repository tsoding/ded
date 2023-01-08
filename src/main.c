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

#define SV_IMPLEMENTATION
#include "./sv.h"

#include "./editor.h"
#include "./la.h"
#include "./sdl_extra.h"
#include "./gl_extra.h"
#include "./free_glyph.h"
#include "./simple_renderer.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define DELTA_TIME (1.0f / FPS)

Editor editor = {0};
Uint32 last_stroke = 0;
Vec2f camera_pos = {0};
float camera_scale = 3.0f;
float camera_scale_vel = 0.0f;
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
static Simple_Renderer sr = {0};

#define FREE_GLYPH_FONT_SIZE 64
#define ZOOM_OUT_GLYPH_THRESHOLD 30

void render_editor_into_fgb(SDL_Window *window, Free_Glyph_Buffer *fgb, Simple_Renderer *sr, Editor *editor)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    float max_line_len = 0.0f;

    free_glyph_buffer_use(fgb);
    {
        glUniform2f(fgb->uniforms[UNIFORM_SLOT_RESOLUTION], (float) w, (float) h);
        glUniform1f(fgb->uniforms[UNIFORM_SLOT_TIME], (float) SDL_GetTicks() / 1000.0f);
        glUniform2f(fgb->uniforms[UNIFORM_SLOT_CAMERA_POS], camera_pos.x, camera_pos.y);
        glUniform1f(fgb->uniforms[UNIFORM_SLOT_CAMERA_SCALE], camera_scale);

        free_glyph_buffer_clear(fgb);

        {
            for (size_t row = 0; row < editor->lines.count; ++row) {
                Line line = editor->lines.items[row];

                const Vec2f begin = vec2f(0, -(float)row * FREE_GLYPH_FONT_SIZE);
                Vec2f end = begin;
                free_glyph_buffer_render_line_sized(
                    fgb, editor->data.items + line.begin, line.end - line.begin,
                    &end,
                    vec4fs(1.0f), vec4fs(0.0f));
                // TODO: the max_line_len should be calculated based on what's visible on the screen right now
                float line_len = fabsf(end.x - begin.x);
                if (line_len > max_line_len) {
                    max_line_len = line_len;
                }
            }
        }

        free_glyph_buffer_sync(fgb);
        free_glyph_buffer_draw(fgb);
    }

    Vec2f cursor_pos = vec2fs(0.0f);
    {
        size_t cursor_row = editor_cursor_row(editor);
        Line line = editor->lines.items[cursor_row];
        size_t cursor_col = editor->cursor - line.begin;
        cursor_pos.y = -(float) cursor_row * FREE_GLYPH_FONT_SIZE;
        cursor_pos.x = free_glyph_buffer_cursor_pos(
                           fgb,
                           editor->data.items + line.begin, line.end - line.begin,
                           vec2f(0.0, cursor_pos.y),
                           cursor_col
                       );
    }

    simple_renderer_use(sr);
    {
        float CURSOR_WIDTH = 5.0f;
        Uint32 CURSOR_BLINK_THRESHOLD = 500;
        Uint32 CURSOR_BLINK_PERIOD = 1000;
        Uint32 t = SDL_GetTicks() - last_stroke;

        //////////////////////////////

        simple_renderer_set_shader(sr, SHADER_FOR_EPICNESS);
        glUniform2f(sr->uniforms[UNIFORM_SLOT_RESOLUTION], (float) w, (float) h);
        glUniform1f(sr->uniforms[UNIFORM_SLOT_TIME], (float) SDL_GetTicks() / 1000.0f);
        glUniform2f(sr->uniforms[UNIFORM_SLOT_CAMERA_POS], camera_pos.x, camera_pos.y);
        glUniform1f(sr->uniforms[UNIFORM_SLOT_CAMERA_SCALE], camera_scale);

        sr->verticies_count = 0;
        if (t < CURSOR_BLINK_THRESHOLD || t/CURSOR_BLINK_PERIOD%2 != 0) {
            simple_renderer_image_rect(
                sr,
                cursor_pos, vec2f(CURSOR_WIDTH, FREE_GLYPH_FONT_SIZE));
        }
        simple_renderer_sync(sr);
        simple_renderer_draw(sr);

        //////////////////////////////

        simple_renderer_set_shader(sr, SHADER_FOR_COLOR);
        glUniform2f(sr->uniforms[UNIFORM_SLOT_RESOLUTION], (float) w, (float) h);
        glUniform1f(sr->uniforms[UNIFORM_SLOT_TIME], (float) SDL_GetTicks() / 1000.0f);
        glUniform2f(sr->uniforms[UNIFORM_SLOT_CAMERA_POS], camera_pos.x, camera_pos.y);
        glUniform1f(sr->uniforms[UNIFORM_SLOT_CAMERA_SCALE], camera_scale);
        sr->verticies_count = 0;
        if (t < CURSOR_BLINK_THRESHOLD || t/CURSOR_BLINK_PERIOD%2 != 0) {
            simple_renderer_solid_rect(
                sr,
                vec2f_add(cursor_pos, vec2f(CURSOR_WIDTH, 0)),
                vec2f(CURSOR_WIDTH, FREE_GLYPH_FONT_SIZE),
                vec4fs(1));
        }
        simple_renderer_sync(sr);
        simple_renderer_draw(sr);
    }

    {
        float target_scale = 3.0f;
        if (max_line_len > 0.0f) {
            target_scale = SCREEN_WIDTH / max_line_len;
        }

        if (target_scale > 3.0f) {
            target_scale = 3.0f;
        }

        camera_vel = vec2f_mul(
                         vec2f_sub(cursor_pos, camera_pos),
                         vec2fs(2.0f));
        camera_scale_vel = (target_scale - camera_scale) * 2.0f;

        camera_pos = vec2f_add(camera_pos, vec2f_mul(camera_vel, vec2fs(DELTA_TIME)));
        camera_scale = camera_scale + camera_scale_vel * DELTA_TIME;
    }
}

int main(int argc, char **argv)
{
    editor_recompute_lines(&editor);

    FT_Library library = {0};

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "ERROR: could initialize FreeType2 library\n");
        exit(1);
    }

    const char *const font_file_path = "./VictorMono-Regular.ttf";

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

    simple_renderer_init(&sr,
                         "./shaders/simple.vert",
                         "./shaders/simple_color.frag",
                         "./shaders/simple_image.frag",
                         "./shaders/simple_epic.frag");

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
                    last_stroke = SDL_GetTicks();
                }
                break;

                case SDLK_F2: {
                    if (file_path) {
                        editor_save_to_file(&editor, file_path);
                    }
                }
                break;

                case SDLK_RETURN: {
                    editor_insert_char(&editor, '\n');
                    last_stroke = SDL_GetTicks();
                }
                break;

                case SDLK_DELETE: {
                    editor_delete(&editor);
                    last_stroke = SDL_GetTicks();
                }
                break;

                case SDLK_UP: {
                    editor_move_line_up(&editor);
                    last_stroke = SDL_GetTicks();
                }
                break;

                case SDLK_DOWN: {
                    editor_move_line_down(&editor);
                    last_stroke = SDL_GetTicks();
                }
                break;

                case SDLK_LEFT: {
                    editor_move_char_left(&editor);
                    last_stroke = SDL_GetTicks();
                }
                break;

                case SDLK_RIGHT: {
                    editor_move_char_right(&editor);
                    last_stroke = SDL_GetTicks();
                }
                break;
                }
            }
            break;

            case SDL_TEXTINPUT: {
                const char *text = event.text.text;
                size_t text_len = strlen(text);
                for (size_t i = 0; i < text_len; ++i) {
                    editor_insert_char(&editor, text[i]);
                }
                last_stroke = SDL_GetTicks();
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

        render_editor_into_fgb(window, &fgb, &sr, &editor);

        SDL_GL_SwapWindow(window);

        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 delta_time_ms = 1000 / FPS;
        if (duration < delta_time_ms) {
            SDL_Delay(delta_time_ms - duration);
        }
    }

    return 0;
}
