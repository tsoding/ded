#version 330 core

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_CHAR_WIDTH_UV  (float(FONT_CHAR_WIDTH) / float(FONT_WIDTH))
#define FONT_CHAR_HEIGHT_UV (float(FONT_CHAR_HEIGHT) / float(FONT_HEIGHT))

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

uniform sampler2D font;
uniform float time;

in vec2 uv;
in float glyph_ch;
in vec4 glyph_color;

void main() {
    int ch = int(glyph_ch);
    if (!(ASCII_DISPLAY_LOW <= ch && ch <= ASCII_DISPLAY_HIGH)) {
        ch = 63;
    }

    int index = ch - 32;
    float x = float(index % FONT_COLS) * FONT_CHAR_WIDTH_UV;
    float y = float(index / FONT_COLS) * FONT_CHAR_HEIGHT_UV;
    vec2 pos = vec2(x, y + FONT_CHAR_HEIGHT_UV);
    vec2 size = vec2(FONT_CHAR_WIDTH_UV, -FONT_CHAR_HEIGHT_UV);
    vec2 t = pos + size * uv;

    gl_FragColor = texture(font, t)* glyph_color;
}
