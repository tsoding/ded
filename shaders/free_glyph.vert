#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 size;
layout(location = 2) in int ch;
layout(location = 3) in vec4 fg_color;
layout(location = 4) in vec4 bg_color;

out vec2 uv;

void main() {
    uv = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
    gl_Position = vec4(uv - vec2(0.5), 0.0, 1.0);
}
