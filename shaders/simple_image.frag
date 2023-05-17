#version 330 core

uniform sampler2D image;

in vec2 out_uv;

out vec4 fragColor;
void main() {
    fragColor = texture(image, out_uv);
}
