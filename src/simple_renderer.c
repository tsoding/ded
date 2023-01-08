#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "./simple_renderer.h"
#include "./gl_extra.h"

#define UNIMPLEMENTED(...)               \
    do {                                 \
        printf("%s:%d: UNIMPLEMENTED: %s \n", __FILE__, __LINE__, __VA_ARGS__); \
        exit(1);                         \
    } while(0)
#define UNUSED(x) (void)(x)

void simple_renderer_init(Simple_Renderer *sr,
                          const char *vert_file_path,
                          const char *color_frag_file_path,
                          const char *image_frag_file_path,
                          const char *epic_frag_file_path)
{
    {
        glGenVertexArrays(1, &sr->vao);
        glBindVertexArray(sr->vao);

        glGenBuffers(1, &sr->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, sr->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sr->verticies), sr->verticies, GL_DYNAMIC_DRAW);

        // position
        glEnableVertexAttribArray(SIMPLE_VERTEX_ATTR_POSITION);
        glVertexAttribPointer(
            SIMPLE_VERTEX_ATTR_POSITION,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Simple_Vertex),
            (GLvoid *) offsetof(Simple_Vertex, position));

        // color
        glEnableVertexAttribArray(SIMPLE_VERTEX_ATTR_COLOR);
        glVertexAttribPointer(
            SIMPLE_VERTEX_ATTR_COLOR,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Simple_Vertex),
            (GLvoid *) offsetof(Simple_Vertex, color));

        // uv
        glEnableVertexAttribArray(SIMPLE_VERTEX_ATTR_UV);
        glVertexAttribPointer(
            SIMPLE_VERTEX_ATTR_UV,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Simple_Vertex),
            (GLvoid *) offsetof(Simple_Vertex, uv));
    }

    GLuint shaders[2] = {0};

    if (!compile_shader_file(vert_file_path, GL_VERTEX_SHADER, &shaders[0])) {
        exit(1);
    }

    // Shader for color
    {
        if (!compile_shader_file(color_frag_file_path, GL_FRAGMENT_SHADER, &shaders[1])) {
            exit(1);
        }
        sr->programs[SHADER_FOR_COLOR] = glCreateProgram();
        attach_shaders_to_program(shaders, sizeof(shaders) / sizeof(shaders[0]), sr->programs[SHADER_FOR_COLOR]);
        if (!link_program(sr->programs[SHADER_FOR_COLOR], __FILE__, __LINE__)) {
            exit(1);
        }
    }

    // Shader for image
    {
        if (!compile_shader_file(image_frag_file_path, GL_FRAGMENT_SHADER, &shaders[1])) {
            exit(1);
        }
        sr->programs[SHADER_FOR_IMAGE] = glCreateProgram();
        attach_shaders_to_program(shaders, sizeof(shaders) / sizeof(shaders[0]), sr->programs[SHADER_FOR_IMAGE]);
        if (!link_program(sr->programs[SHADER_FOR_IMAGE], __FILE__, __LINE__)) {
            exit(1);
        }
    }

    // Shader for epicness
    {
        if (!compile_shader_file(epic_frag_file_path, GL_FRAGMENT_SHADER, &shaders[1])) {
            exit(1);
        }
        sr->programs[SHADER_FOR_EPICNESS] = glCreateProgram();
        attach_shaders_to_program(shaders, sizeof(shaders) / sizeof(shaders[0]), sr->programs[SHADER_FOR_EPICNESS]);
        if (!link_program(sr->programs[SHADER_FOR_EPICNESS], __FILE__, __LINE__)) {
            exit(1);
        }
    }
}

void simple_renderer_use(const Simple_Renderer *sr)
{
    glBindVertexArray(sr->vao);
    glBindBuffer(GL_ARRAY_BUFFER, sr->vbo);
}

void simple_renderer_vertex(Simple_Renderer *sr,
                            Vec2f p, Vec4f c, Vec2f uv)
{
    assert(sr->verticies_count < SIMPLE_VERTICIES_CAP);
    Simple_Vertex *last = &sr->verticies[sr->verticies_count];
    last->position = p;
    last->color    = c;
    last->uv       = uv;
    sr->verticies_count += 1;
}

void simple_renderer_triangle(Simple_Renderer *sr,
                              Vec2f p0, Vec2f p1, Vec2f p2,
                              Vec4f c0, Vec4f c1, Vec4f c2,
                              Vec2f uv0, Vec2f uv1, Vec2f uv2)
{
    simple_renderer_vertex(sr, p0, c0, uv0);
    simple_renderer_vertex(sr, p1, c1, uv1);
    simple_renderer_vertex(sr, p2, c2, uv2);
}

// 2-3
// |\|
// 0-1
void simple_renderer_quad(Simple_Renderer *sr,
                          Vec2f p0, Vec2f p1, Vec2f p2, Vec2f p3,
                          Vec4f c0, Vec4f c1, Vec4f c2, Vec4f c3,
                          Vec2f uv0, Vec2f uv1, Vec2f uv2, Vec2f uv3)
{
    simple_renderer_triangle(sr, p0, p1, p2, c0, c1, c2, uv0, uv1, uv2);
    simple_renderer_triangle(sr, p1, p2, p3, c1, c2, c3, uv1, uv2, uv3);
}

void simple_renderer_image_rect(Simple_Renderer *sr, Vec2f p, Vec2f s, Vec2f uvp, Vec2f uvs)
{
    Vec4f c = vec4fs(0);
    simple_renderer_quad(
        sr,
        p, vec2f_add(p, vec2f(s.x, 0)), vec2f_add(p, vec2f(0, s.y)), vec2f_add(p, s),
        c, c, c, c,
        uvp, vec2f_add(uvp, vec2f(uvs.x, 0)), vec2f_add(uvp, vec2f(0, uvs.y)), vec2f_add(uvp, uvs));
}

void simple_renderer_solid_rect(Simple_Renderer *sr, Vec2f p, Vec2f s, Vec4f c)
{
    Vec2f uv = vec2fs(0);
    simple_renderer_quad(
        sr,
        p, vec2f_add(p, vec2f(s.x, 0)), vec2f_add(p, vec2f(0, s.y)), vec2f_add(p, s),
        c, c, c, c,
        uv, uv, uv, uv);
}

void simple_renderer_sync(Simple_Renderer *sr)
{
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    sr->verticies_count * sizeof(Simple_Vertex),
                    sr->verticies);
}

void simple_renderer_draw(Simple_Renderer *sr)
{
    glDrawArrays(GL_TRIANGLES, 0, sr->verticies_count);
}

void simple_renderer_set_shader(Simple_Renderer *sr, Simple_Shader shader)
{
    sr->current_shader = shader;
    glUseProgram(sr->programs[sr->current_shader]);
    get_uniform_location(sr->programs[sr->current_shader], sr->uniforms);
}
