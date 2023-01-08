#version 330 core

uniform sampler2D image;

in vec2 out_uv;

void main() {
    gl_FragColor = texture(image, out_uv);
    // gl_FragColor = vec4(out_uv, 0, 1);
}
