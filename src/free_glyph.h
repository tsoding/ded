#ifndef FREE_GLYPH_H_
#define FREE_GLYPH_H_

#include <stdlib.h>
#include "./la.h"

#define GLEW_STATIC
#include <GL/glew.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

typedef struct {
    Vec2f pos;
    Vec2f size;
    int ch;
    Vec4f fg_color;
    Vec4f bg_color;
} Free_Glyph;

typedef enum {
    FREE_GLYPH_ATTR_POS = 0,
    FREE_GLYPH_ATTR_SIZE,
    FREE_GLYPH_ATTR_CH,
    FREE_GLYPH_ATTR_FG_COLOR,
    FREE_GLYPH_ATTR_BG_COLOR,
    COUNT_FREE_GLYPH_ATTRS,
} Free_Glyph_Attr;

#define FREE_GLYPH_BUFFER_CAP (640 * 1000)

typedef struct {
    GLuint vao;
    GLuint vbo;

    GLint time_uniform;
    GLint resolution_uniform;
    GLint camera_uniform;

    size_t glyphs_count;
    Free_Glyph glyphs[FREE_GLYPH_BUFFER_CAP];
} Free_Glyph_Buffer;

void free_glyph_buffer_init(Free_Glyph_Buffer *fgb,
                            const char *vert_file_path,
                            const char *frag_file_path);
void free_glyph_buffer_clear(Free_Glyph_Buffer *fgb);
void free_glyph_buffer_push(Free_Glyph_Buffer *fgb, Free_Glyph glyph);
void free_glyph_buffer_sync(Free_Glyph_Buffer *fgb);
void free_glyph_buffer_draw(Free_Glyph_Buffer *fgb);


#endif // FREE_GLYPH_H_
