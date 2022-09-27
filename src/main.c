#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <GLFW/glfw3.h>

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
// TODO: Delete line
// TODO: Split the line on Enter

#define FREE_GLYPH_FONT_SIZE 64
#define ZOOM_OUT_GLYPH_THRESHOLD 30

void input(SDL_Event event, Editor * editor,const Cursor_Renderer *cr, const char *file_path);
FT_Library initializeFreeType();
FT_Face initializeFace(FT_Library library, const char *const font_file_path);
void setPixelSize(FT_Face face);
void render_editor_into_fgb(GLFWwindow *window,Free_Glyph_Buffer *fgb, const Cursor_Renderer *cr, const Editor *editor);
void reset_cursor_blink_timer(const Cursor_Renderer *cr);
GLFWwindow * initializeSDL();
void initializeGLEW();
void resizeWindow(int windowWidth, int windowHeight);
void messageCallback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam);
void windowSizeCallback(GLFWwindow* window, int width, int height);
void keyboardCharacterCallback(GLFWwindow* window, unsigned int codepoint);
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

static Cursor_Renderer cr = {0};
const char *file_path = NULL;

int main(int argc, char **argv){
    static Free_Glyph_Buffer fgb = {0};
    FT_Library library = initializeFreeType();
    const char *const font_file_path = "./VictorMono-Regular.ttf";
    FT_Face face = initializeFace(library,font_file_path);
    setPixelSize(face);

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

    GLFWwindow *window = initializeSDL();
    initializeGLEW();

    free_glyph_buffer_init(&fgb,
                           face,
                           "./shaders/free_glyph.vert",
                           "./shaders/free_glyph.frag");
    cursor_renderer_init(&cr,
                         "./shaders/cursor.vert",
                         "./shaders/cursor.frag");


    glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, keyboardCharacterCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    
    while (!glfwWindowShouldClose(window)) {
        const Uint32 start = SDL_GetTicks();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        render_editor_into_fgb(window,&fgb, &cr, &editor);

        glfwSwapBuffers(window);
        glfwPollEvents();

        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 delta_time_ms = 1000 / FPS;
        if (duration < delta_time_ms) {
            SDL_Delay(delta_time_ms - duration);
        }
    }
    glfwTerminate();
    return 0;
}

FT_Library initializeFreeType(){
    FT_Library library = {0};
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "ERROR: could not initialize FreeType2 library\n");
        exit(1);
    }
    return library;
}

FT_Face initializeFace(FT_Library library, const char *const font_file_path){
    FT_Face face;
    FT_Error error = FT_New_Face(library, font_file_path, 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "ERROR: `%s` has an unknown format\n", font_file_path);
        exit(1);
    } else if (error) {
        fprintf(stderr, "ERROR: could not load file `%s`\n", font_file_path);
        exit(1);
    }
    return face;
}

void setPixelSize(FT_Face face){
    FT_UInt pixel_size = FREE_GLYPH_FONT_SIZE;
    // TODO: FT_Set_Pixel_Sizes does not produce good looking results
    // We need to use something like FT_Set_Char_Size and properly set the device resolution
    FT_Error error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
    if (error) {
        fprintf(stderr, "ERROR: could not set pixel size to %u\n", pixel_size);
        exit(1);
    }
}

GLFWwindow * initializeSDL(){
    if (!glfwInit()){
        fprintf(stderr, "ERROR: not could initialize GLFW3\n");
        exit(1);
    }
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Text Editor", NULL, NULL);;
    if (!window)
    {
        fprintf(stderr, "ERROR: not could initialize GLFW3 Window\n");
        glfwTerminate();
        exit(1);
    }

    scc(SDL_Init(SDL_INIT_VIDEO));
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    int major;
    int minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    printf("GL version %d.%d\n", major, minor);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwMakeContextCurrent(window);
    return window;
}

void initializeGLEW(){
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
    if (GLEW_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(messageCallback, 0);
    } else {
        fprintf(stderr, "WARNING! GLEW_ARB_debug_output is not available");
    }
}

void keyboardCharacterCallback(GLFWwindow* window, unsigned int unicodeCodepoint){
    (void) window;
    // TODO: UTF-8/UTF-16 support
    const char text = (unsigned char) unicodeCodepoint;
    editor_insert_text_before_cursor(&editor,&text);
    cursor_renderer_use(&cr);
    reset_cursor_blink_timer(&cr);
}
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos){
    (void) window;
    (void) xpos;
    (void) ypos;
    // TODO: cursor moving with mouse?
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    (void) window;
    (void) scancode;
    (void) mods;
    if(action == GLFW_PRESS || action == GLFW_REPEAT){
        switch(key){
            default:
            break;
            case GLFW_KEY_UP:
                if (editor.cursor_row > 0) {
                    editor.cursor_row -= 1;
                }
                cursor_renderer_use(&cr);
                reset_cursor_blink_timer(&cr);
            break;
            case GLFW_KEY_DOWN:
                editor.cursor_row += 1;
                cursor_renderer_use(&cr);
                reset_cursor_blink_timer(&cr);
            break;
            case GLFW_KEY_LEFT:
                if (editor.cursor_col > 0) {
                    editor.cursor_col -= 1;
                    cursor_renderer_use(&cr);
                    reset_cursor_blink_timer(&cr);
                }
            break;
            case GLFW_KEY_RIGHT:
                editor.cursor_col += 1;
                cursor_renderer_use(&cr);
                reset_cursor_blink_timer(&cr);
            break;

            case GLFW_KEY_BACKSPACE:
                editor_backspace(&editor);
                cursor_renderer_use(&cr);
                reset_cursor_blink_timer(&cr);
            break;
            case GLFW_KEY_ENTER:
                editor_insert_new_line(&editor);
                cursor_renderer_use(&cr);
                reset_cursor_blink_timer(&cr);
            break;
            case GLFW_KEY_F2:
                if (file_path != NULL && *file_path) {
                    editor_save_to_file(&editor, file_path);
                }
            break;
            case GLFW_KEY_DELETE:
                editor_delete(&editor);
                cursor_renderer_use(&cr);
                reset_cursor_blink_timer(&cr);
            break;
        }
    }
}

void reset_cursor_blink_timer(const Cursor_Renderer *cr){
    glUniform1f(cr->uniforms[UNIFORM_SLOT_LAST_STROKE], (float) SDL_GetTicks() / 1000.0f);
}

void render_editor_into_fgb(GLFWwindow *window,Free_Glyph_Buffer *fgb, const Cursor_Renderer *cr, const Editor *editor)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);        
    float max_line_len = 0.0f;

    free_glyph_buffer_use(fgb);
    {
        glUniform2f(fgb->uniforms[UNIFORM_SLOT_RESOLUTION], (float) windowWidth, (float) windowHeight);
        glUniform1f(fgb->uniforms[UNIFORM_SLOT_TIME], (float) SDL_GetTicks() / 1000.0f);
        glUniform2f(fgb->uniforms[UNIFORM_SLOT_CAMERA_POS], camera_pos.x, camera_pos.y);
        glUniform1f(fgb->uniforms[UNIFORM_SLOT_CAMERA_SCALE], camera_scale);

        free_glyph_buffer_clear(fgb);

        {
            for (size_t row = 0; row < editor->size; ++row) {
                const Line *line = editor->lines + row;

                const Vec2f begin = vec2f(0, -(float)row * FREE_GLYPH_FONT_SIZE);
                Vec2f end = begin;
                free_glyph_buffer_render_line_sized(
                    fgb, line->chars, line->size,
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
        cursor_pos.y = -(float) editor->cursor_row * FREE_GLYPH_FONT_SIZE;

        if (editor->cursor_row < editor->size) {
            const Line *line = &editor->lines[editor->cursor_row];
            cursor_pos.x = free_glyph_buffer_cursor_pos(fgb, line->chars, line->size, vec2f(0.0, cursor_pos.y), editor->cursor_col);
        }
    }

    cursor_renderer_use(cr);
    {
        glUniform2f(cr->uniforms[UNIFORM_SLOT_RESOLUTION], (float) windowWidth, (float) windowHeight);
        glUniform1f(cr->uniforms[UNIFORM_SLOT_TIME], (float) SDL_GetTicks() / 1000.0f);
        glUniform2f(cr->uniforms[UNIFORM_SLOT_CAMERA_POS], camera_pos.x, camera_pos.y);
        glUniform1f(cr->uniforms[UNIFORM_SLOT_CAMERA_SCALE], camera_scale);
        glUniform1f(cr->uniforms[UNIFORM_SLOT_CURSOR_HEIGHT], FREE_GLYPH_FONT_SIZE);

        cursor_renderer_move_to(cr, cursor_pos);
        cursor_renderer_draw();
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

void messageCallback(GLenum source,
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

void windowSizeCallback(GLFWwindow* window, int windowWidth, int windowHeight){
    (void) window;
    glViewport(0, 0, windowWidth, windowHeight);
}