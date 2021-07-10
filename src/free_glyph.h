#ifndef FREE_GLYPH_H_
#define FREE_GLYPH_H_

#include <stdlib.h>
#include "./la.h"

#define GLEW_STATIC
#include <GL/glew.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02

typedef struct {
    Vec2f pos;
    Vec2f size;
    Vec2f uv_pos;
    Vec2f uv_size;
    Vec4f fg_color;
    Vec4f bg_color;
} Free_Glyph;

typedef enum {
    FREE_GLYPH_ATTR_POS = 0,
    FREE_GLYPH_ATTR_SIZE,
    FREE_GLYPH_ATTR_UV_POS,
    FREE_GLYPH_ATTR_UV_SIZE,
    FREE_GLYPH_ATTR_FG_COLOR,
    FREE_GLYPH_ATTR_BG_COLOR,
    COUNT_FREE_GLYPH_ATTRS,
} Free_Glyph_Attr;

typedef struct {
  float ax; // advance.x
  float ay; // advance.y
  
  float bw; // bitmap.width;
  float bh; // bitmap.rows;
  
  float bl; // bitmap_left;
  float bt; // bitmap_top;
  
  float tx; // x offset of glyph in texture coordinates
} Glyph_Metric;

#define FREE_GLYPH_BUFFER_CAP (640 * 1000)

typedef struct {
    GLuint vao;
    GLuint vbo;

    FT_UInt atlas_width;
    FT_UInt atlas_height;

    GLuint glyphs_texture;

    GLint time_uniform;
    GLint resolution_uniform;
    GLint camera_uniform;

    size_t glyphs_count;
    Free_Glyph glyphs[FREE_GLYPH_BUFFER_CAP];
    
    Glyph_Metric metrics[128];
} Free_Glyph_Buffer;

void free_glyph_buffer_init(Free_Glyph_Buffer *fgb,
                            FT_Face face,
                            const char *vert_file_path,
                            const char *frag_file_path);
void free_glyph_buffer_clear(Free_Glyph_Buffer *fgb);
void free_glyph_buffer_push(Free_Glyph_Buffer *fgb, Free_Glyph glyph);
void free_glyph_buffer_sync(Free_Glyph_Buffer *fgb);
void free_glyph_buffer_draw(Free_Glyph_Buffer *fgb);

void free_glyph_buffer_render_line_sized(Free_Glyph_Buffer *fgb, const char *text, size_t text_size, Vec2f pos, Vec4f fg_color, Vec4f bg_color);
void free_glyph_buffer_render_line(Free_Glyph_Buffer *fgb, const char *text, Vec2f pos, Vec4f fg_color, Vec4f bg_color);

#endif // FREE_GLYPH_H_
