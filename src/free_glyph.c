#include <assert.h>
#include <stdbool.h>
#include "./free_glyph.h"
#include "./gl_extra.h"

#include "./stb_image_write.h"
#include "./stb_image.h"

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
    [FREE_GLYPH_ATTR_UV_POS]    = {
        .offset = offsetof(Free_Glyph, uv_pos),
        .comps = 2,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_UV_SIZE]    = {
        .offset = offsetof(Free_Glyph, uv_size),
        .comps = 2,
        .type = GL_FLOAT
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
static_assert(COUNT_FREE_GLYPH_ATTRS == 6, "The amount of glyph vertex attributes have changed");


void free_glyph_buffer_init(Free_Glyph_Buffer *fgb,
                            FT_Face face,
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

    // Glyph Texture Atlas
    {
        for (int i = 32; i < 128; ++i) {
            if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
                fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
                exit(1);
            }

            fgb->atlas_width += face->glyph->bitmap.width;
            if (fgb->atlas_height < face->glyph->bitmap.rows) {
                fgb->atlas_height = face->glyph->bitmap.rows;
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &fgb->glyphs_texture);
        glBindTexture(GL_TEXTURE_2D, fgb->glyphs_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            (GLsizei) fgb->atlas_width,
            (GLsizei) fgb->atlas_height,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            NULL);

        int x = 0;
        for (int i = 32; i < 128; ++i) {
            if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
                fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
                exit(1);
            }

            if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
                fprintf(stderr, "ERROR: could not render glyph of a character with code %d\n", i);
                exit(1);
            }

            fgb->metrics[i].ax = face->glyph->advance.x >> 6;
            fgb->metrics[i].ay = face->glyph->advance.y >> 6;
            fgb->metrics[i].bw = face->glyph->bitmap.width;
            fgb->metrics[i].bh = face->glyph->bitmap.rows;
            fgb->metrics[i].bl = face->glyph->bitmap_left;
            fgb->metrics[i].bt = face->glyph->bitmap_top;
            fgb->metrics[i].tx = (float) x / (float) fgb->atlas_width;

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                x,
                0,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);
            x += face->glyph->bitmap.width;
        }
    }
}

void free_glyph_buffer_clear(Free_Glyph_Buffer *fgb)
{
    fgb->glyphs_count = 0;
}

void free_glyph_buffer_push(Free_Glyph_Buffer *fgb, Free_Glyph glyph)
{
    assert(fgb->glyphs_count < FREE_GLYPH_BUFFER_CAP);
    fgb->glyphs[fgb->glyphs_count++] = glyph;
}

void free_glyph_buffer_sync(Free_Glyph_Buffer *fgb)
{
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    fgb->glyphs_count * sizeof(Free_Glyph),
                    fgb->glyphs);
}

void free_glyph_buffer_draw(Free_Glyph_Buffer *fgb)
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei) fgb->glyphs_count);
}

void free_glyph_buffer_render_line_sized(Free_Glyph_Buffer *fgb, const char *text, size_t text_size, Vec2f pos, Vec4f fg_color, Vec4f bg_color)
{
    for (size_t i = 0; i < text_size; ++i) {
        Glyph_Metric metric = fgb->metrics[(int) text[i]];
        float x2 = pos.x + metric.bl;
        float y2 = -pos.y - metric.bt;
        float w  = metric.bw;
        float h  = metric.bh;

        pos.x += metric.ax;
        pos.y += metric.ay;

        Free_Glyph glyph = {0};
        glyph.pos = vec2f(x2, -y2);
        glyph.size = vec2f(w, -h);
        glyph.uv_pos = vec2f(metric.tx, 0.0f);
        glyph.uv_size = vec2f(metric.bw / (float) fgb->atlas_width, metric.bh / (float) fgb->atlas_height);
        glyph.fg_color = fg_color;
        glyph.bg_color = bg_color;
        free_glyph_buffer_push(fgb, glyph);
    }
}

void free_glyph_buffer_render_line(Free_Glyph_Buffer *fgb, const char *text, Vec2f pos, Vec4f fg_color, Vec4f bg_color)
{
    free_glyph_buffer_render_line_sized(fgb, text, strlen(text), pos, fg_color, bg_color);
}
