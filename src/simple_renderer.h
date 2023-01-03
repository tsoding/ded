#ifndef SIMPLE_RENDERER_H_
#define SIMPLE_RENDERER_H_

#define GLEW_STATIC
#include <GL/glew.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include "./la.h"
#include "./uniforms.h"

typedef enum {
    SIMPLE_VERTEX_ATTR_POSITION = 0,
    SIMPLE_VERTEX_ATTR_COLOR,
    SIMPLE_VERTEX_ATTR_UV,
} Simple_Vertex_Attr;

typedef struct {
    Vec2f position;
    Vec4f color;
    Vec2f uv;
} Simple_Vertex;

#define SIMPLE_VERTICIES_CAP (3*640*1000)

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint program;

    GLint uniforms[COUNT_UNIFORM_SLOTS];
    Simple_Vertex verticies[SIMPLE_VERTICIES_CAP];
    size_t verticies_count;
} Simple_Renderer;

void simple_renderer_init(Simple_Renderer *sr,
                          const char *vert_file_path,
                          const char *frag_file_path);

void simple_renderer_use(const Simple_Renderer *sr);
void simple_renderer_vertex(Simple_Renderer *sr,
                            Vec2f p, Vec4f c, Vec2f uv);
void simple_renderer_triangle(Simple_Renderer *sr,
                              Vec2f p0, Vec2f p1, Vec2f p2,
                              Vec4f c0, Vec4f c1, Vec4f c2,
                              Vec2f uv0, Vec2f uv1, Vec2f uv2);
void simple_renderer_quad(Simple_Renderer *sr,
                          Vec2f p0, Vec2f p1, Vec2f p2, Vec2f p3,
                          Vec4f c0, Vec4f c1, Vec4f c2, Vec4f c3,
                          Vec2f uv0, Vec2f uv1, Vec2f uv2, Vec2f uv3);
void simple_renderer_solid_rect(Simple_Renderer *sr, Vec2f p, Vec2f s, Vec4f c);
void simple_renderer_sync(Simple_Renderer *sr);
void simple_renderer_draw(Simple_Renderer *sr);

#endif  // SIMPLE_RENDERER_H_
