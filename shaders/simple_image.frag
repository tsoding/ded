#version 330 core

uniform sampler2D image;

in vec2 out_uv;
out vec4 FragColor;

void main() {
    FragColor = texture(image, out_uv);
}
