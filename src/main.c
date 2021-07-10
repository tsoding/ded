#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define SV_IMPLEMENTATION
#include "./sv.h"

#include "./editor.h"
#include "./la.h"
#include "./sdl_extra.h"
#include "./gl_extra.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define DELTA_TIME (1.0f / FPS)

#define FONT_SCALE 5
#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)

Editor editor = {0};
Vec2f camera_pos = {0};
Vec2f camera_vel = {0};

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

typedef struct {
    Vec2i tile;
    int ch;
    Vec4f fg_color;
    Vec4f bg_color;
} Tile_Glyph;

typedef enum {
    TILE_GLYPH_ATTR_TILE = 0,
    TILE_GLYPH_ATTR_CH,
    TILE_GLYPH_ATTR_FG_COLOR,
    TILE_GLYPH_ATTR_BG_COLOR,
    COUNT_TILE_GLYPH_ATTRS,
} Tile_Glyph_Attr;

typedef struct {
    size_t offset;
    GLint comps;
    GLenum type;
} Attr_Def;

static const Attr_Def glyph_attr_defs[COUNT_TILE_GLYPH_ATTRS] = {
    [TILE_GLYPH_ATTR_TILE]   = {
        .offset = offsetof(Tile_Glyph, tile),
        .comps = 2,
        .type = GL_INT
    },
    [TILE_GLYPH_ATTR_CH]    = {
        .offset = offsetof(Tile_Glyph, ch),
        .comps = 1,
        .type = GL_INT
    },
    [TILE_GLYPH_ATTR_FG_COLOR] = {
        .offset = offsetof(Tile_Glyph, fg_color),
        .comps = 4,
        .type = GL_FLOAT
    },
    [TILE_GLYPH_ATTR_BG_COLOR] = {
        .offset = offsetof(Tile_Glyph, bg_color),
        .comps = 4,
        .type = GL_FLOAT
    },
};
static_assert(COUNT_TILE_GLYPH_ATTRS == 4, "The amount of glyph vertex attributes have changed");

#define TILE_GLYPH_BUFFER_CAP (640 * 1024)

Tile_Glyph tile_glyph_buffer[TILE_GLYPH_BUFFER_CAP];
size_t tile_glyph_buffer_count = 0;

void tile_glyph_buffer_clear(void)
{
    tile_glyph_buffer_count = 0;
}

void tile_glyph_buffer_push(Tile_Glyph glyph)
{
    assert(tile_glyph_buffer_count < TILE_GLYPH_BUFFER_CAP);
    tile_glyph_buffer[tile_glyph_buffer_count++] = glyph;
}

void tile_glyph_buffer_sync(void)
{
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    tile_glyph_buffer_count * sizeof(Tile_Glyph),
                    tile_glyph_buffer);
}

void gl_render_text_sized(const char *text, size_t text_size, Vec2i tile, Vec4f fg_color, Vec4f bg_color)
{
    for (size_t i = 0; i < text_size; ++i) {
        tile_glyph_buffer_push((Tile_Glyph) {
            .tile = vec2i_add(tile, vec2i((int) i, 0)),
            .ch = text[i],
            .fg_color = fg_color,
            .bg_color = bg_color,
        });
    }
}

void gl_render_text(const char *text, Vec2i tile, Vec4f fg_color, Vec4f bg_color)
{
    gl_render_text_sized(text, strlen(text), tile, fg_color, bg_color);
}

void gl_render_cursor()
{
    const char *c = editor_char_under_cursor(&editor);
    Vec2i tile = vec2i((int) editor.cursor_col, -(int) editor.cursor_row);
    gl_render_text_sized(c ? c : " ", 1, tile, vec4fs(0.0f), vec4fs(1.0f));
}

// OPENGL
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

    GLint time_uniform;
    GLint resolution_uniform;
    GLint scale_uniform;
    GLint camera_uniform;

    // Initialize Shaders
    {
        GLuint vert_shader = 0;
        if (!compile_shader_file("./shaders/font.vert", GL_VERTEX_SHADER, &vert_shader)) {
            exit(1);
        }
        GLuint frag_shader = 0;
        if (!compile_shader_file("./shaders/font.frag", GL_FRAGMENT_SHADER, &frag_shader)) {
            exit(1);
        }

        GLuint program = 0;
        if (!link_program(vert_shader, frag_shader, &program)) {
            exit(1);
        }

        glUseProgram(program);

        time_uniform = glGetUniformLocation(program, "time");
        resolution_uniform = glGetUniformLocation(program, "resolution");
        scale_uniform = glGetUniformLocation(program, "scale");
        camera_uniform = glGetUniformLocation(program, "camera");

        glUniform2f(scale_uniform, FONT_SCALE, FONT_SCALE);
    }

    // Init Font Texture
    {
        const char *font_file_path = "charmap-oldschool_white.png";
        int width, height, n;
        unsigned char *pixels = stbi_load(font_file_path, &width, &height, &n, STBI_rgb_alpha);
        if (pixels == NULL) {
            fprintf(stderr, "ERROR: could not load file %s: %s\n",
                    file_path, stbi_failure_reason());
            exit(1);
        }

        glActiveTexture(GL_TEXTURE0);

        GLuint font_texture = 0;
        glGenTextures(1, &font_texture);
        glBindTexture(GL_TEXTURE_2D, font_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     width,
                     height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     pixels);
    }

    // Init Buffers
    {
        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(tile_glyph_buffer),
                     tile_glyph_buffer,
                     GL_DYNAMIC_DRAW);

        for (Tile_Glyph_Attr attr = 0; attr < COUNT_TILE_GLYPH_ATTRS; ++attr) {
            glEnableVertexAttribArray(attr);
            switch (glyph_attr_defs[attr].type) {
            case GL_FLOAT:
                glVertexAttribPointer(
                    attr,
                    glyph_attr_defs[attr].comps,
                    glyph_attr_defs[attr].type,
                    GL_FALSE,
                    sizeof(Tile_Glyph),
                    (void*) glyph_attr_defs[attr].offset);
                break;

            case GL_INT:
                glVertexAttribIPointer(
                    attr,
                    glyph_attr_defs[attr].comps,
                    glyph_attr_defs[attr].type,
                    sizeof(Tile_Glyph),
                    (void*) glyph_attr_defs[attr].offset);
                break;

            default:
                assert(false && "unreachable");
                exit(1);
            }
            glVertexAttribDivisor(attr, 1);
        }
    }

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

            case SDL_MOUSEBUTTONDOWN: {
                const Vec2f mouse_pos = vec2f((float) event.button.x, (float) event.button.y);
                switch(event.button.button) {
                case SDL_BUTTON_LEFT: {
                    const Vec2f click_pos =
                        vec2f_add(mouse_pos, vec2f_sub(vec2f(camera_pos.x, -camera_pos.y + FONT_CHAR_HEIGHT * FONT_SCALE),
                                                       vec2f_mul(window_size(window), vec2fs(0.5f))));

                    if(click_pos.x > 0.0f && click_pos.y > 0.0f) {
                        editor.cursor_col = (size_t) floorf(click_pos.x / ((float) FONT_CHAR_WIDTH * FONT_SCALE));
                        editor.cursor_row = (size_t) floorf(click_pos.y / ((float) FONT_CHAR_HEIGHT * FONT_SCALE));
                    }
                }
                break;
                }
            }
            break;
            }
        }

        {
            const Vec2f cursor_pos =
                vec2f((float) editor.cursor_col * FONT_CHAR_WIDTH * FONT_SCALE,
                      (float) (-(int)editor.cursor_row) * FONT_CHAR_HEIGHT * FONT_SCALE);

            camera_vel = vec2f_mul(
                             vec2f_sub(cursor_pos, camera_pos),
                             vec2fs(2.0f));

            camera_pos = vec2f_add(camera_pos, vec2f_mul(camera_vel, vec2fs(DELTA_TIME)));
        }

        {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            // TODO(#19): update the viewport and the resolution only on actual window change
            glViewport(0, 0, w, h);
            glUniform2f(resolution_uniform, (float) w, (float) h);
        }

        tile_glyph_buffer_clear();
        {
            for (size_t row = 0; row < editor.size; ++row) {
                const Line *line = editor.lines + row;
                gl_render_text_sized(line->chars, line->size, vec2i(0, -(int)row), vec4fs(1.0f), vec4fs(0.0f));
            }
        }
        tile_glyph_buffer_sync();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1f(time_uniform, (float) SDL_GetTicks() / 1000.0f);
        glUniform2f(camera_uniform, camera_pos.x, camera_pos.y);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei) tile_glyph_buffer_count);

        tile_glyph_buffer_clear();
        {
            gl_render_cursor();
        }
        tile_glyph_buffer_sync();

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei) tile_glyph_buffer_count);

        SDL_GL_SwapWindow(window);

        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 delta_time_ms = 1000 / FPS;
        if (duration < delta_time_ms) {
            SDL_Delay(delta_time_ms - duration);
        }
    }

    return 0;
}
