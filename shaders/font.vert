#version 330 core

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)

uniform vec2 resolution;
uniform vec2 scale;

layout(location = 0) in ivec2 tile;
layout(location = 1) in int ch;
layout(location = 2) in vec4 color;

out vec2 uv;
flat out int glyph_ch;
out vec4 glyph_color;

vec2 project_point(vec2 point)
{
    return 2.0 * point / resolution;
}

void main() {
    uv = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
    vec2 char_size = vec2(float(FONT_CHAR_WIDTH), float(FONT_CHAR_HEIGHT));
    vec2 pos = tile * char_size * scale;
    gl_Position = vec4(project_point(uv * char_size * scale + pos), 0.0, 1.0);
    glyph_ch = ch;
    glyph_color = color;
}
