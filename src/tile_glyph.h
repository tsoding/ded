#ifndef TILE_GLYPH_H_
#define TILE_GLYPH_H_

#include <stdlib.h>
#include "./la.h"

#define GLEW_STATIC
#include <GL/glew.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define FONT_SCALE 5
#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)

typedef struct {
    Vec2i tile;
    int ch;
    Vec4f fg_color;
    Vec4f bg_color;
} Tile_Glyph;

typedef enum {
    TILE_GLYPH_ATTR_TILE = 0,
    TILE_GLYPH_ATTR_CH,
    TILE_GLYPH_ATTR_FG_COLOR,
    TILE_GLYPH_ATTR_BG_COLOR,
    COUNT_TILE_GLYPH_ATTRS,
} Tile_Glyph_Attr;

#define TILE_GLYPH_BUFFER_CAP (640 * 1024)

typedef struct {
    GLuint vao;
    GLuint vbo;

    GLuint font_texture;

    GLint time_uniform;
    GLint resolution_uniform;
    GLint scale_uniform;
    GLint camera_uniform;

    size_t glyphs_count;
    Tile_Glyph glyphs[TILE_GLYPH_BUFFER_CAP];
} Tile_Glyph_Buffer;

void tile_glyph_buffer_init(Tile_Glyph_Buffer *buffer,
                            const char *texture_file_path,
                            const char *vert_file_path,
                            const char *frag_file_path);
void tile_glyph_buffer_clear(Tile_Glyph_Buffer *buffer);
void tile_glyph_buffer_push(Tile_Glyph_Buffer *buffer, Tile_Glyph glyph);
void tile_glyph_buffer_sync(Tile_Glyph_Buffer *buffer);
void tile_glyph_buffer_draw(Tile_Glyph_Buffer *buffer);

void tile_glyph_render_line_sized(Tile_Glyph_Buffer *buffer, const char *text, size_t text_size, Vec2i tile, Vec4f fg_color, Vec4f bg_color);
void tile_glyph_render_line(Tile_Glyph_Buffer *buffer, const char *text, Vec2i tile, Vec4f fg_color, Vec4f bg_color);


#endif // TILE_GLYPH_H_
