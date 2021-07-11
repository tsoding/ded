#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "./stb_image.h"
#include "./tile_glyph.h"
#include "./gl_extra.h"

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

void tile_glyph_buffer_clear(Tile_Glyph_Buffer *tgb)
{
    tgb->glyphs_count = 0;
}

void tile_glyph_buffer_push(Tile_Glyph_Buffer *tgb, Tile_Glyph glyph)
{
    assert(tgb->glyphs_count < TILE_GLYPH_BUFFER_CAP);
    tgb->glyphs[tgb->glyphs_count++] = glyph;
}

void tile_glyph_buffer_sync(Tile_Glyph_Buffer *tgb)
{
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    tgb->glyphs_count * sizeof(Tile_Glyph),
                    tgb->glyphs);
}

void tile_glyph_render_line_sized(Tile_Glyph_Buffer *tgb, const char *text, size_t text_size, Vec2i tile, Vec4f fg_color, Vec4f bg_color)
{
    for (size_t i = 0; i < text_size; ++i) {
        tile_glyph_buffer_push(tgb, (Tile_Glyph) {
            .tile = vec2i_add(tile, vec2i((int) i, 0)),
            .ch = text[i],
            .fg_color = fg_color,
            .bg_color = bg_color,
        });
    }
}

void tile_glyph_render_line(Tile_Glyph_Buffer *tgb, const char *text, Vec2i tile, Vec4f fg_color, Vec4f bg_color)
{
    tile_glyph_render_line_sized(tgb, text, strlen(text), tile, fg_color, bg_color);
}

void tile_glyph_buffer_init(Tile_Glyph_Buffer *tgb, const char *texture_file_path, const char *vert_file_path, const char *frag_file_path)
{
    // Init Vertex Attributes
    {
        glGenVertexArrays(1, &tgb->vao);
        glBindVertexArray(tgb->vao);

        glGenBuffers(1, &tgb->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, tgb->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tgb->glyphs), tgb->glyphs, GL_DYNAMIC_DRAW);

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

        tgb->time_uniform = glGetUniformLocation(program, "time");
        tgb->resolution_uniform = glGetUniformLocation(program, "resolution");
        tgb->scale_uniform = glGetUniformLocation(program, "scale");
        tgb->camera_uniform = glGetUniformLocation(program, "camera");

        glUniform2f(tgb->scale_uniform, FONT_SCALE, FONT_SCALE);
    }

    // Init Texture
    {
        int width, height, n;
        unsigned char *pixels = stbi_load(texture_file_path, &width, &height, &n, STBI_rgb_alpha);
        if (pixels == NULL) {
            fprintf(stderr, "ERROR: could not load file %s: %s\n",
                    texture_file_path, stbi_failure_reason());
            exit(1);
        }

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &tgb->font_texture);
        glBindTexture(GL_TEXTURE_2D, tgb->font_texture);

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

        stbi_image_free(pixels);
    }
}

void tile_glyph_buffer_draw(Tile_Glyph_Buffer *tgb)
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei) tgb->glyphs_count);
}
