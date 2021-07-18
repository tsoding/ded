#version 330 core

uniform vec2 resolution;
uniform vec2 cursor_pos;
uniform float cursor_height;

#define WIDTH 5.0

out vec2 uv;

vec2 camera_project(vec2 point);

void main() {
    uv = vec2(float(gl_VertexID & 1),
              float((gl_VertexID >> 1) & 1));
    gl_Position = vec4(
        camera_project(uv * vec2(WIDTH, cursor_height) + cursor_pos),
        0.0,
        1.0);
}
