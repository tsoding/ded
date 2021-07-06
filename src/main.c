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

// #define OPENGL_RENDERER

#ifdef OPENGL_RENDERER
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
    Vec2f pos;
    float scale;
    float ch;
    Vec4f color;
} Glyph;

typedef enum {
    GLYPH_ATTR_POS = 0,
    GLYPH_ATTR_SCALE,
    GLYPH_ATTR_CH,
    GLYPH_ATTR_COLOR,
    COUNT_GLYPH_ATTRS,
} Glyph_Attr;

typedef struct {
    size_t offset;
    size_t comps;
} Glyph_Attr_Def;

static const Glyph_Attr_Def glyph_attr_defs[COUNT_GLYPH_ATTRS] = {
    [GLYPH_ATTR_POS]   = {
        .offset = offsetof(Glyph, pos),
        .comps = 2,
    },
    [GLYPH_ATTR_SCALE] = {
        .offset = offsetof(Glyph, scale),
        .comps = 1,
    },
    [GLYPH_ATTR_CH]    = {
        .offset = offsetof(Glyph, ch),
        .comps = 1,
    },
    [GLYPH_ATTR_COLOR] = {
        .offset = offsetof(Glyph, color),
        .comps = 4,
    },
};
static_assert(COUNT_GLYPH_ATTRS == 4, "The amount of glyph vertex attributes have changed");

#define GLYPH_BUFFER_CAP 1024

Glyph glyph_buffer[GLYPH_BUFFER_CAP];
size_t glyph_buffer_count = 0;

void glyph_buffer_push(Glyph glyph)
{
    assert(glyph_buffer_count < GLYPH_BUFFER_CAP);
    glyph_buffer[glyph_buffer_count++] = glyph;
}

void glyph_buffer_sync(void)
{
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    glyph_buffer_count * sizeof(Glyph),
                    glyph_buffer);
}

void gl_render_text(const char *text, size_t text_size,
                    Vec2f pos, float scale, Vec4f color)
{
    for (size_t i = 0; i < text_size; ++i) {
        const Vec2f char_size = vec2f(FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT);
        const Glyph glyph = {
            .pos = vec2f_add(pos, vec2f_mul3(char_size,
                                             vec2f((float) i, 0.0f),
                                             vec2fs(scale))),
            .scale = scale,
            .ch = (float) text[i],
            .color = color
        };
        glyph_buffer_push(glyph);
    }
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

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

        glUniform2f(resolution_uniform, SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    // Init Font Texture
    {
        const char *file_path = "charmap-oldschool_white.png";
        int width, height, n;
        unsigned char *pixels = stbi_load(file_path, &width, &height, &n, STBI_rgb_alpha);
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
                     sizeof(glyph_buffer),
                     glyph_buffer,
                     GL_DYNAMIC_DRAW);

        for (Glyph_Attr attr = 0; attr < COUNT_GLYPH_ATTRS; ++attr) {
            glEnableVertexAttribArray(attr);
            glVertexAttribPointer(
                attr,
                glyph_attr_defs[attr].comps,
                GL_FLOAT,
                GL_FALSE,
                sizeof(Glyph),
                (void*) glyph_attr_defs[attr].offset);
            glVertexAttribDivisor(attr, 1);
        }
    }

    const char *text = "Hello, World";
    Vec4f color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
    gl_render_text(text, strlen(text), vec2fs(0.0f), 5.0f, color);
    glyph_buffer_sync();

    bool quit = false;
    while (!quit) {
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
            }
            break;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1f(time_uniform, (float) SDL_GetTicks() / 1000.0f);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, glyph_buffer_count);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
#else
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

            case SDL_MOUSEBUTTONDOWN: {
                Vec2f mouse_click = vec2f((float) event.button.x, (float) event.button.y);
                switch(event.button.button) { 
                case SDL_BUTTON_LEFT: {
                    Vec2f cursor_click = vec2f_add(mouse_click, vec2f_sub(camera_pos, vec2f_div(window_size(window), vec2fs(2.0f))));
                    if(cursor_click.x > 0.0f && cursor_click.y > 0.0f) {
                        editor.cursor_col = (size_t) floorf(cursor_click.x / ((float) FONT_CHAR_WIDTH * FONT_SCALE));
                        editor.cursor_row = (size_t) floorf(cursor_click.y / ((float) FONT_CHAR_HEIGHT * FONT_SCALE));
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
#endif // OPENGL_RENDERER
