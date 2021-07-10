#include <assert.h>
#include <stdbool.h>
#include "./free_glyph.h"
#include "./gl_extra.h"

typedef struct {
    size_t offset;
    GLint comps;
    GLenum type;
} Attr_Def;

static const Attr_Def glyph_attr_defs[COUNT_FREE_GLYPH_ATTRS] = {
    [FREE_GLYPH_ATTR_POS]   = {
        .offset = offsetof(Free_Glyph, pos),
        .comps = 2,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_SIZE]   = {
        .offset = offsetof(Free_Glyph, size),
        .comps = 2,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_CH]    = {
        .offset = offsetof(Free_Glyph, ch),
        .comps = 1,
        .type = GL_INT
    },
    [FREE_GLYPH_ATTR_FG_COLOR] = {
        .offset = offsetof(Free_Glyph, fg_color),
        .comps = 4,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_BG_COLOR] = {
        .offset = offsetof(Free_Glyph, bg_color),
        .comps = 4,
        .type = GL_FLOAT
    },
};
static_assert(COUNT_FREE_GLYPH_ATTRS == 5, "The amount of glyph vertex attributes have changed");


void free_glyph_buffer_init(Free_Glyph_Buffer *fgb,
                            const char *vert_file_path,
                            const char *frag_file_path)
{
    // Init Vertex Attributes
    {
        glGenVertexArrays(1, &fgb->vao);
        glBindVertexArray(fgb->vao);

        glGenBuffers(1, &fgb->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, fgb->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fgb->glyphs), fgb->glyphs, GL_DYNAMIC_DRAW);

        for (Free_Glyph_Attr attr = 0; attr < COUNT_FREE_GLYPH_ATTRS; ++attr) {
            glEnableVertexAttribArray(attr);
            switch (glyph_attr_defs[attr].type) {
            case GL_FLOAT:
                glVertexAttribPointer(
                    attr,
                    glyph_attr_defs[attr].comps,
                    glyph_attr_defs[attr].type,
                    GL_FALSE,
                    sizeof(Free_Glyph),
                    (void*) glyph_attr_defs[attr].offset);
                break;

            case GL_INT:
                glVertexAttribIPointer(
                    attr,
                    glyph_attr_defs[attr].comps,
                    glyph_attr_defs[attr].type,
                    sizeof(Free_Glyph),
                    (void*) glyph_attr_defs[attr].offset);
                break;

            default:
                assert(false && "unreachable");
                exit(1);
            }
            glVertexAttribDivisor(attr, 1);
        }
    }

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

        GLuint program = 0;
        if (!link_program(vert_shader, frag_shader, &program)) {
            exit(1);
        }

        glUseProgram(program);

        fgb->time_uniform = glGetUniformLocation(program, "time");
        fgb->resolution_uniform = glGetUniformLocation(program, "resolution");
        fgb->camera_uniform = glGetUniformLocation(program, "camera");
    }

}

void free_glyph_buffer_clear(Free_Glyph_Buffer *fgb)
{
    (void) fgb;
    assert(0);
}

void free_glyph_buffer_push(Free_Glyph_Buffer *fgb, Free_Glyph glyph)
{
    (void) fgb;
    (void) glyph;
    assert(0);
}

void free_glyph_buffer_sync(Free_Glyph_Buffer *fgb)
{
    (void) fgb;
    assert(0);
}

void free_glyph_buffer_draw(Free_Glyph_Buffer *fgb)
{
    (void) fgb;
    assert(0);
}
