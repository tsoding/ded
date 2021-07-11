#include <stdlib.h>
#include "./gl_extra.h"
#include "./cursor_renderer.h"

void cursor_renderer_init(Cursor_Renderer *cr,
                          const char *vert_file_path,
                          const char *frag_file_path)
{
    // Init Shaders
    {
        GLuint vert_shader = 0;
        if (!compile_shader_file(vert_file_path, GL_VERTEX_SHADER, &vert_shader)) {
            exit(1);
        }
        GLuint frag_shader = 0;
        if (!compile_shader_file(frag_file_path, GL_FRAGMENT_SHADER, &frag_shader)) {
            exit(1);
        }

        if (!link_program(vert_shader, frag_shader, &cr->program)) {
            exit(1);
        }

        glUseProgram(cr->program);

        cr->time_uniform = glGetUniformLocation(cr->program, "time");
        cr->resolution_uniform = glGetUniformLocation(cr->program, "resolution");
        cr->camera_uniform = glGetUniformLocation(cr->program, "camera");
        cr->pos_uniform = glGetUniformLocation(cr->program, "pos");
        cr->height_uniform = glGetUniformLocation(cr->program, "height");
        cr->last_stroke_uniform = glGetUniformLocation(cr->program, "last_stroke");
    }
}

void cursor_renderer_use(const Cursor_Renderer *cr)
{
    glUseProgram(cr->program);
}

void cursor_renderer_move_to(const Cursor_Renderer *cr, Vec2f pos)
{
    glUniform2f(cr->pos_uniform, pos.x, pos.y);
}

void cursor_renderer_draw()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
