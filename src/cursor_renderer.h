#ifndef CURSOR_RENDERER_H_
#define CURSOR_RENDERER_H_

#define GLEW_STATIC
#include <GL/glew.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include "./la.h"

typedef struct {
    GLuint program;

    GLint time_uniform;
    GLint resolution_uniform;
    GLint camera_uniform;
    GLint pos_uniform;
    GLint height_uniform;
    GLint last_stroke_uniform;
} Cursor_Renderer;

void cursor_renderer_init(Cursor_Renderer *cr,
                          const char *vert_file_path,
                          const char *frag_file_path);

void cursor_renderer_use(const Cursor_Renderer *cr);
void cursor_renderer_move_to(const Cursor_Renderer *cr, Vec2f pos);
void cursor_renderer_draw(void);

#endif // CURSOR_RENDERER_H_
