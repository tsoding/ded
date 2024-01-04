// GRADIENT
#version 330 core

in vec2 fragTexCoord; // Texture coordinate, if you're using textures
uniform float time;   // Uniform variable for time
uniform vec2 cursorPos; // Cursor position in screen coordinates
uniform vec2 resolution; // Screen resolution

out vec4 color;

void main() {
    // Calculate normalized coordinates
    vec2 uv = fragTexCoord / resolution;

    // Create a dynamic gradient based on time
    vec3 gradient = 0.5 + 0.9 * cos(time + uv.xyx + vec3(0, 2, 4));

    // Create a glow effect
    float dist = distance(uv, cursorPos / resolution);
    float glow = exp(20.0 * dist);

    // Combine the gradient and glow
    vec3 finalColor = gradient * glow;

    // Output the color
    color = vec4(finalColor, 0.7); // Fully opaque
}

